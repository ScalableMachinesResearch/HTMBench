#include <inttypes.h>
#include "htm_tsx.h"
#include "htm_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#if defined(__x86_64__)
#include <emmintrin.h>
#define cpu_relax() asm volatile("pause\n": : :"memory")
#endif

#define ABORT_CODE_RESET (0xfffffff0)

struct tsx_status_struct {
        unsigned short isInCriticalSection;
        unsigned short isInWaitingLock;
        unsigned short isInFallbackPath;
};
__thread struct tsx_status_struct tsx_inflight_status={0,0,0};
__thread  TransactionDiagnosticInfo thread_diag;
__thread int nest_level = 0;

TM_LOCK_T global_lock;

#ifdef HTM_STATS
struct tsx_counter_struct {
  unsigned long tx_try;
};
struct tsx_counter_struct global_counter = {0};
#define INCREMENT(var) __sync_fetch_and_add(&(var),1)
#define STATS(args) args
#else
#define STATS(args) /*nothing*/
#endif

#ifdef USE_MUTEX
static THREAD_MUTEX_T global_lock_mutex;
static THREAD_COND_T global_lock_cond;
#endif /*USE_MUTEX*/

static int transient_retry_max = 16;
static int persistent_retry_max = 1;
static int global_lock_retry_max = 16;
static int backoff_once_iterations = 10;

void tm_startup_() {
  const char *env_transient_retry_max;
  const char *env_persistent_retry_max;
  const char *env_global_lock_retry_max;
  const char *env_backoff_once_iterations;

  global_lock.lock = 0;
#ifdef GLOCK_RW
  global_lock.write_flag = 0;
#endif
#ifdef USE_MUTEX
  THREAD_MUTEX_INIT(global_lock.mutex);
  THREAD_COND_INIT(global_lock.cond);
#endif

  env_transient_retry_max = getenv("HTM_TRETRY");
  if (env_transient_retry_max) {
    transient_retry_max = atoi(env_transient_retry_max);
    //fprintf(stderr, "<HTM_TRETRY=%d>\n", transient_retry_max);
  }

  env_persistent_retry_max = getenv("HTM_PRETRY");
  if (env_persistent_retry_max) {
    persistent_retry_max = atoi(env_persistent_retry_max);
    //fprintf(stderr, "<HTM_PRETRY=%d>\n", persistent_retry_max);
  }

  env_global_lock_retry_max = getenv("HTM_GRETRY");
  if (env_global_lock_retry_max) {
    global_lock_retry_max = atoi(env_global_lock_retry_max);
    //fprintf(stderr, "<HTM_GRETRY=%d>\n", global_lock_retry_max);
  }

  env_backoff_once_iterations = getenv("BACKOFF_ONCE_ITERATIONS");
  if (env_backoff_once_iterations) {
    backoff_once_iterations = atoi(env_backoff_once_iterations);
    //fprintf(stderr, "<HTM_GRETRY=%d>\n", global_lock_retry_max);
  }
  srand(time(NULL));
}

static int fall_back_lock(int acquire, TM_LOCK_T *lock_set) {
  tsx_inflight_status.isInWaitingLock = 1;
#ifdef USE_MUTEX
  const int spin_max = 2000000000;
  int spin_count = spin_max;

  while (lock_set->lock && --spin_count > 0);
  if (! acquire && ! lock_set->lock) {
    tsx_inflight_status.isInWaitingLock = 0;
    return 0;
  }
  THREAD_MUTEX_LOCK(lock_set->mutex);

  while (lock_set->lock) {
    THREAD_COND_WAIT(lock_set->cond, lock_set->mutex);
  }
  lock_set->lock = 1;
  THREAD_MUTEX_UNLOCK(lock_set->mutex);
  tsx_inflight_status.isInWaitingLock = 0;
  return 1;

#else /* !USE_MUTEX */
#ifdef GLOCK_RW
  if (acquire == 0) { //special case....
    while (lock_set->write_flag);
    tsx_inflight_status.isInWaitingLock = 0;
    return 0;
  }
#else
  while (lock_set->lock);
  if (! acquire){
    tsx_inflight_status.isInWaitingLock = 0;
    return 0;
  }
#endif /*GLOCK_RW*/
  while (__sync_val_compare_and_swap(&(lock_set->lock), 0, 1)) {
    while (lock_set->lock);
  }
  tsx_inflight_status.isInWaitingLock = 0;
  return 1;
#endif /* USE_MUTEX */
}

static int isAbortPersistent(int tbegin_result,TransactionDiagnosticInfo *diag) {
  #ifdef __x86_64__
    return ( (tbegin_result != 4) && !(diag->transactionAbortCode&XABORT_RETRY));
  #else
    return 0;
  #endif
}

#ifdef GLOCK_RW
void set_write_flag(){
  if ( !xtest()) { //if protected by lock, we need to set the write flag
    global_lock.write_flag = 1 ;
    return;
  }
  if ( global_lock.lock ) {
    tm_abort_();
    return;
  }
}
#endif

unsigned int get_tsx_status(int opt){
  unsigned int ret_val = 0;
  //bit 0: in critical section?
  //bit 1: in HTM?
  //bit 2: in fallbackpath?
  //bit 3: in lock waiting?

  ret_val |= tsx_inflight_status.isInWaitingLock;
  ret_val <<= 1;

  ret_val |= tsx_inflight_status.isInFallbackPath;
  ret_val <<= 1;

  //if ( thread_diag.transactionAbortCode  == ABORT_CODE_RESET) ret_val |= 1;
  ret_val <<= 1;

  ret_val |= tsx_inflight_status.isInCriticalSection;

	return ret_val;
}

void tm_begin_(TM_LOCK_T *lock_set) {
  if (nest_level++)
     return;
  int tbegin_result;
  int transient_retry_count;
  int persistent_retry_count;
  int global_lock_retry_count;
  if (lock_set == NULL){
    lock_set = &global_lock;
  }
  tsx_inflight_status.isInCriticalSection = 1;
  if (lock_set->lock) {
    if (fall_back_lock(0, lock_set)) {
      return;
    }
  }
  //printf("TM_BEGIN\n");
  transient_retry_count = transient_retry_max;
  persistent_retry_count = persistent_retry_max;
  global_lock_retry_count = global_lock_retry_max;
TX_RETRY:
  //thread_diag.transactionAbortCode = ABORT_CODE_RESET;
  STATS(INCREMENT(global_counter.tx_try));
  tbegin_result = tbegin(&thread_diag);
  if (tbegin_result == 0) {
    /* Transaction */
#ifdef GLOCK_RW
    if (lock_set->write_flag)
#else
    if (lock_set->lock)
#endif
    {
      tend();
      tbegin_result = 4;
    }
  }
  if (tbegin_result != 0) {
    /* Abort */
    uint64_t reason = thread_diag.transactionAbortCode;
#ifdef GLOCK_RW
    if (lock_set->write_flag)
#else
    if (lock_set->lock)
#endif
    {
      if (--global_lock_retry_count > 0) {
        if (fall_back_lock(0, lock_set)) {
          return;
        }
        goto TX_RETRY;
      }
      fall_back_lock(1, lock_set);
    }
    else {
      if (isAbortPersistent(tbegin_result,&thread_diag)) {
        /* Persistent abort */
        if (--persistent_retry_count > 0) {
          goto TX_RETRY;
        }
        fall_back_lock(1, lock_set);
      }
      else {
        /* Transient abort */
        if (--transient_retry_count > 0) {
          goto TX_RETRY;
        }
        fall_back_lock(1, lock_set);
      }
    }
    tsx_inflight_status.isInFallbackPath = 1;
  }
}

void tm_begin_fallback_(TM_LOCK_T *lock_set) {
  if (nest_level++)
     return;
  if (lock_set == NULL){
    lock_set = &global_lock;
  }
  tsx_inflight_status.isInCriticalSection = 1;
  STATS(INCREMENT(global_counter.tx_try));
  fall_back_lock(1, lock_set);
  tsx_inflight_status.isInFallbackPath = 1;
}

static void backoff(int *backoff_number) {
  int i;
  int backoff_time = rand() % ((int) pow(2,*backoff_number) - 1);
  for(i=0; i < backoff_once_iterations * backoff_time; i++){
    cpu_relax();
  }
  (*backoff_number)++;
}

void tm_begin_backoff_(TM_LOCK_T *lock_set) {
  int tbegin_result;
  int transient_retry_count;
  int persistent_retry_count;
  int global_lock_retry_count;
  int backoff_number=1;

  if (lock_set == NULL){
    lock_set = &global_lock;
  }
  tsx_inflight_status.isInCriticalSection = 1;
  if (lock_set->lock) {
    if (fall_back_lock(0, lock_set)) {
      return;
    }
  }

  transient_retry_count = transient_retry_max;
  persistent_retry_count = persistent_retry_max;
  global_lock_retry_count = global_lock_retry_max;

TX_RETRY:
  STATS(INCREMENT(global_counter.tx_try));
  tbegin_result = tbegin(&thread_diag);
  if (tbegin_result == 0) {
    /* Transaction */
    if (lock_set->lock) {
      tend();
      tbegin_result = 4;
    }
  }
  if (tbegin_result != 0) {
    /* Abort */
    uint64_t reason = thread_diag.transactionAbortCode;
    if (lock_set->lock) {
      if (--global_lock_retry_count > 0) {
        backoff(&backoff_number);
        if (fall_back_lock(0, lock_set)) {
          return;
        }
        goto TX_RETRY;
      }
      fall_back_lock(1, lock_set);
    }
    else {
      if (isAbortPersistent(tbegin_result,&thread_diag)) {
        /* Persistent abort */
        if (--persistent_retry_count > 0) {
          goto TX_RETRY;
        }
        fall_back_lock(1, lock_set);
      }
      else {
        /* Transient abort */
        if (--transient_retry_count > 0) {
          backoff(&backoff_number);
          goto TX_RETRY;
        }
        fall_back_lock(1, lock_set);
      }
    }
    tsx_inflight_status.isInFallbackPath = 1;
  }
}

void tm_end_(TM_LOCK_T *lock_set) {
  if (--nest_level == 0) {
    if (lock_set == NULL){
        lock_set = &global_lock;
    }
    if (lock_set->lock) {
#ifdef GLOCK_RW
        if (lock_set->write_flag) {
           lock_set->write_flag = 0;
        }
#endif
        #ifdef USE_MUTEX
        THREAD_MUTEX_LOCK(lock_set->mutex);
        lock_set->lock = 0;
        THREAD_COND_SIGNAL(lock_set->cond);
        THREAD_MUTEX_UNLOCK(lock_set->mutex);
        #else
        //memory fence
        #if defined(__x86_64__)
        _mm_sfence();
        #elif defined(__GNUC__) || defined(__IBMC__)
        __sync_synchronize();
        #endif
        lock_set->lock = 0;
        #endif
    }
    else {
        tend();
    }
    //printf("TM_END\n");
    tsx_inflight_status.isInFallbackPath = 0;
    tsx_inflight_status.isInCriticalSection = 0;
  }
}
void tm_thread_exit_(){
}
void tm_shutdown_(){
  STATS(printf("TX_TRY = %ld",global_counter.tx_try););
}

void tm_abort_() {
  tabort(300);
}
int tm_test_(){
  return xtest();
}
