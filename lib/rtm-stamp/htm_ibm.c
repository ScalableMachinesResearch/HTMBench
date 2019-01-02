/* Copyright (c) IBM Corp. 2014. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "htm_ibm.h"
#include "htm_util.h"
#include "thread.h"
#include "mfence.h"
#include "tm.h"
#ifdef __bgq__
#include <speculation.h>
#endif

#define NUM_HTM_STATS_EVENTS 14
#define NUM_HTM_ABORT_REASON_CODES 19
#define NUM_HTM_TBEGIN_RETURNS 3
#define NUM_ATOMIC_REGIONS 20
#ifndef DETECT_DELINQUENTS
#define DETECT_DELINQUENTS 0
#endif

enum {
  event_tx = 0,
  event_tx_enter,
  event_abort,
  event_first_abort,
  event_global_lock_wait_before_tx_spin,
  event_global_lock_wait_before_tx_sleep,
  event_global_lock_wait_and_retry_spin,
  event_global_lock_wait_and_retry_sleep,
  event_global_lock_acquired,
  event_persistent_abort_retry,
  event_global_lock_persistent_abort,
  event_transient_abort_retry,
  event_global_lock_transient_abort,
  event_detected_delinquent
};

#define ABORT_CODE_RESET (0xfffffff0)

struct tsx_status_struct {
        unsigned short isInCriticalSection;
        unsigned short isInWaitingLock;
        unsigned short isInFallbackPath;
};
__thread struct tsx_status_struct tsx_inflight_status={0,0,0};
__thread  TransactionDiagnosticInfo thread_diag;


typedef struct htm_stats_struct {
  unsigned long long event_counter[NUM_HTM_STATS_EVENTS];
#if defined(__370__)
  unsigned long long abort_reason_code[NUM_HTM_ABORT_REASON_CODES][NUM_HTM_TBEGIN_RETURNS];
#elif defined(__x86_64__)
#define HTM_IA32_STAT(nam) long long unsigned int nam;
  struct{
#include "htm_ia32_stat.h"
  } abort_reason_counters;
#undef HTM_IA32_STAT
#elif defined(__PPC__) || defined(_ARCH_PPC)
#define HTM_PPC_STAT(NAM) long long unsigned int NAM;
  struct {
#include "htm_ppc_stat.h"
  } abort_reason_counters;
#undef HTM_PPC_STAT
#else
  char dummy;
#endif
} htm_stats_t;

typedef struct tls_struct {
  htm_stats_t htm_stats[NUM_ATOMIC_REGIONS];
} tls_t;

/*#define USE_MUTEX*/

static union {
  char two_cache_lines[512];
  struct {
    char one_cache_line[256];
    volatile int global_lock;
    char another_cache_line[252];
  } a;
} gl;
#ifdef USE_MUTEX
static THREAD_MUTEX_T global_lock_mutex;
static THREAD_COND_T global_lock_cond;
#endif
static THREAD_KEY_T global_tls_key;

static int transient_retry_max = 16;
static int persistent_retry_max = 1;
static int global_lock_retry_max = 16;
static int collect_stats = 0;
static htm_stats_t global_htm_stats_per_region[NUM_ATOMIC_REGIONS];
static htm_stats_t global_htm_stats;
static THREAD_MUTEX_T global_htm_stats_lock;


/*#define ABORTED_INSN_ADDRESS_STATS*/
#ifdef ABORTED_INSN_ADDRESS_STATS
#define ABORTED_INSN_ADDRESS_STATS_MAP_SIZE (15 * 1024 * 1024 / 2)
#define ABORTED_INSN_ADDRESS_STATS_START_ADDRESS 0x13b08000
/*#define ABORTED_INSN_ADDRESS_STATS_START_ADDRESS 0x25c08000*/
uint64_t aborted_insn_address_stats[ABORTED_INSN_ADDRESS_STATS_MAP_SIZE];
uint64_t aborted_insn_address_reason_code;
#endif

void
tm_startup_ibm_()
{
  const char *env_transient_retry_max;
  const char *env_persistent_retry_max;
  const char *env_global_lock_retry_max;
  const char *env_collect_stats;
#ifdef ABORTED_INSN_ADDRESS_STATS
  const char *env_aborted_insn_address_reason_code;
#endif

  gl.a.global_lock = 0;
#ifdef USE_MUTEX
  THREAD_MUTEX_INIT(global_lock_mutex);
  THREAD_COND_INIT(global_lock_cond);
#endif
  THREAD_KEY_INIT(global_tls_key);

  env_transient_retry_max = getenv("HTM_TRETRY");
  if (env_transient_retry_max) {
#ifdef __bgq__
    fprintf(stderr, "<HTM_TRETRY has no meaning on Blue Gene/Q>");
#else
    transient_retry_max = atoi(env_transient_retry_max);
    fprintf(stderr, "<HTM_TRETRY=%d>\n", transient_retry_max);
#endif
  }

  env_persistent_retry_max = getenv("HTM_PRETRY");
  if (env_persistent_retry_max) {
#ifdef __bgq__
    fprintf(stderr, "<HTM_PRETRY has no meaning on Blue Gene/Q>");
#else
    persistent_retry_max = atoi(env_persistent_retry_max);
    fprintf(stderr, "<HTM_PRETRY=%d>\n", persistent_retry_max);
#endif
  }

  env_global_lock_retry_max = getenv("HTM_GRETRY");
  if (env_global_lock_retry_max) {
#ifdef __bgq__
    fprintf(stderr, "<HTM_GRETRY has no meaning on Blue Gene/Q>");
#else
    global_lock_retry_max = atoi(env_global_lock_retry_max);
    fprintf(stderr, "<HTM_GRETRY=%d>\n", global_lock_retry_max);
#endif
  }

  env_collect_stats = getenv("HTM_STATS");
  if (DETECT_DELINQUENTS || env_collect_stats) {
    collect_stats = 1;
  }

#ifdef ABORTED_INSN_ADDRESS_STATS
  env_aborted_insn_address_reason_code = getenv("HTM_AIACODE");
  if (env_aborted_insn_address_reason_code) {
    aborted_insn_address_reason_code = (uint64_t)atoi(env_aborted_insn_address_reason_code);
  }
#endif

#ifndef __bgq__
  if (collect_stats) {
    memset(&global_htm_stats_per_region, 0, sizeof(global_htm_stats_per_region));
    memset(&global_htm_stats, 0, sizeof(global_htm_stats));
    THREAD_MUTEX_INIT(global_htm_stats_lock);
  }
#endif
}

static void
print_stats(htm_stats_t *stats)
{
#if defined(__bgq__)
  TmReport_t bgq_htm_stats;

  tm_get_all_stats(&bgq_htm_stats);
  printf("#HTM_STATS %15lu totalTransactions\n", bgq_htm_stats.totalTransactions);
  printf("#HTM_STATS %15lu totalRollbacks\n", bgq_htm_stats.totalRollbacks);
  printf("#HTM_STATS %15lu totalSerializedJMV\n", bgq_htm_stats.totalSerializedJMV);
  printf("#HTM_STATS %15lu totalSerializedMAXRB\n", bgq_htm_stats.totalSerializedMAXRB);
  printf("#HTM_STATS %15lu totalSerializedOTHER\n", bgq_htm_stats.totalSerializedOTHER);
#else /* ! __bgq__ */
#if defined(__370__)
  int i;
  const char *reason_string[] = {
    "TDB_not_set",
    "Restart_interruption",
    "External_interruption",
    NULL,
    "Program_interruption",
    "Machine-check_interruption",
    "I/O_interruption",
    "Fetch_overflow",
    "Store_overflow",
    "Fetch_conflict",
    "Store_conflict",
    "Restricted_instruction",
    "Program-interruption_condition",
    "Nesting_depth_exceeded",
    "Cache_fetch-related",
    "Cache_store-related",
    "Cache_other",
    "Undetermined_condition",
    "TABORT_instruction"
  };
#endif

  fprintf(stderr, "#HTM_STATS %15llu           tx_enter\n", stats->event_counter[event_tx_enter]);
  fprintf(stderr, "#HTM_STATS %15llu           tx\n", stats->event_counter[event_tx]);
  fprintf(stderr, "#HTM_STATS %15llu %6.2f %%  global_lock_wait_before_tx_spin\n", stats->event_counter[event_global_lock_wait_before_tx_spin], 100 * stats->event_counter[event_global_lock_wait_before_tx_spin] / (double)stats->event_counter[event_tx_enter]);
  fprintf(stderr, "#HTM_STATS %15llu %6.2f %%  global_lock_wait_before_tx_sleep\n", stats->event_counter[event_global_lock_wait_before_tx_sleep], 100 * stats->event_counter[event_global_lock_wait_before_tx_sleep] / (double)stats->event_counter[event_tx_enter]);
  fprintf(stderr, "#HTM_STATS %15llu %6.2f %%  abort\n", stats->event_counter[event_abort], 100 * stats->event_counter[event_abort] / (double)stats->event_counter[event_tx]);
  fprintf(stderr, "#HTM_STATS %15llu %6.2f %%  first_abort\n", stats->event_counter[event_first_abort], 100 * stats->event_counter[event_first_abort] / (double)stats->event_counter[event_tx_enter]);
  fprintf(stderr, "#HTM_STATS %15llu %6.2f %%  global_lock_wait_and_retry_spin\n", stats->event_counter[event_global_lock_wait_and_retry_spin], 100 * stats->event_counter[event_global_lock_wait_and_retry_spin] / (double)stats->event_counter[event_abort]);
  fprintf(stderr, "#HTM_STATS %15llu %6.2f %%  global_lock_wait_and_retry_sleep\n", stats->event_counter[event_global_lock_wait_and_retry_sleep], 100 * stats->event_counter[event_global_lock_wait_and_retry_sleep] / (double)stats->event_counter[event_abort]);
  fprintf(stderr, "#HTM_STATS %15llu %6.2f %%  global_lock_acquired\n", stats->event_counter[event_global_lock_acquired], 100 * stats->event_counter[event_global_lock_acquired] / (double)stats->event_counter[event_abort]);
  fprintf(stderr, "#HTM_STATS %15llu %6.2f %%  persistent_abort_retry\n", stats->event_counter[event_persistent_abort_retry], 100 * stats->event_counter[event_persistent_abort_retry] / (double)stats->event_counter[event_abort]);
  fprintf(stderr, "#HTM_STATS %15llu %6.2f %%  global_lock_persistent_abort\n", stats->event_counter[event_global_lock_persistent_abort], 100 * stats->event_counter[event_global_lock_persistent_abort] / (double)stats->event_counter[event_abort]);
  fprintf(stderr, "#HTM_STATS %15llu %6.2f %%  transient_abort_retry\n", stats->event_counter[event_transient_abort_retry], 100 * stats->event_counter[event_transient_abort_retry] / (double)stats->event_counter[event_abort]);
  fprintf(stderr, "#HTM_STATS %15llu %6.2f %%  global_lock_transient_abort\n", stats->event_counter[event_global_lock_transient_abort], 100 * stats->event_counter[event_global_lock_transient_abort] / (double)stats->event_counter[event_abort]);
  fprintf(stderr, "#HTM_STATS %15llu %6.2f %%  detected_delinquent\n", stats->event_counter[event_detected_delinquent], 100 * stats->event_counter[event_detected_delinquent] / (double)stats->event_counter[event_tx_enter]);

#if defined(__370__)
  for (i = 0; i < NUM_HTM_ABORT_REASON_CODES; i++) {
    if (reason_string[i]) {
      unsigned long long total = stats->abort_reason_code[i][0] + stats->abort_reason_code[i][1] + stats->abort_reason_code[i][2];
      fprintf(stderr, "#HTM_STATS %15llu %s\n", total, reason_string[i]);
    }
  }
#elif defined(__x86_64__)
#define HTM_IA32_STAT(nam) fprintf(stderr, "#HTM_STATS %15llu " #nam "\n", stats->abort_reason_counters.nam );
#include "htm_ia32_stat.h"
#undef HTM_IA32_STAT
#elif defined(__PPC__) || defined(_ARCH_PPC)
#define HTM_PPC_STAT(nam) fprintf(stderr, "#HTM_STATS %15llu " #nam "\n", stats->abort_reason_counters.nam );
#include "htm_ppc_stat.h"
#undef HTM_PPC_STAT
#endif
#endif /* ! __bgq__ */
}

void
tm_shutdown_ibm_()
{
  if (collect_stats) {
#ifndef __bgq__
    int i;
    int region;

    for (region = 0; region < NUM_ATOMIC_REGIONS; region++) {
      for (i = 0; i < NUM_HTM_STATS_EVENTS; i++) {
	global_htm_stats.event_counter[i] += global_htm_stats_per_region[region].event_counter[i];
      }
#if defined(__370__)
      for (i = 0; i < NUM_HTM_ABORT_REASON_CODES; i++) {
	int j;
	for (j = 0; j < NUM_HTM_TBEGIN_RETURNS; j++) {
	  global_htm_stats.abort_reason_code[i][j] += global_htm_stats_per_region[region].abort_reason_code[i][j];
	}
      }
#elif defined(__x86_64__)
#define HTM_IA32_STAT(nam) global_htm_stats.abort_reason_counters.nam += global_htm_stats_per_region[region].abort_reason_counters.nam;
#include "htm_ia32_stat.h"
#undef HTM_IA32_STAT
#elif defined(__PPC__) || defined(_ARCH_PPC)
#define HTM_PPC_STAT(nam) global_htm_stats.abort_reason_counters.nam += global_htm_stats_per_region[region].abort_reason_counters.nam;
#include "htm_ppc_stat.h"
#undef HTM_PPC_STAT
#endif
    }
#endif /* ! __bgq__ */

    print_stats(&global_htm_stats);
    if (getenv("HTM_STATS_PER_REGION")) {
#ifdef __bgq__
      fprintf(stderr, "<HTM_STATS_PER_REGION is not supported on Blue Gene/Q>\n");
#else
      for (region = 0; region < NUM_ATOMIC_REGIONS; region++) {
	if (global_htm_stats_per_region[region].event_counter[event_tx] != 0) {
	  fprintf(stderr, "--- region %d ------------------------------\n", region);
	  print_stats(&global_htm_stats_per_region[region]);
	}
      }
#endif
    }
  }

#ifdef ABORTED_INSN_ADDRESS_STATS
  {
    int insn_addr;
    for (insn_addr = 0; insn_addr < ABORTED_INSN_ADDRESS_STATS_MAP_SIZE; insn_addr++) {
      if (aborted_insn_address_stats[insn_addr] != 0) {
	fprintf(stderr, "ABORTED_INSN_ADDRESS %" PRIx64 " %x %" PRIu64 "\n", (uint64_t)(ABORTED_INSN_ADDRESS_STATS_START_ADDRESS + insn_addr * 2), insn_addr * 2, aborted_insn_address_stats[insn_addr]);
      }
    }
  }
#endif

  if (getenv("LOOP_AT_END")) {
    for ( ; ; ) {
      sleep(3600);
    }
  }
}

void
tm_thread_enter_ibm_()
{
#ifndef __bgq__
  tls_t *tls;

  tls = (tls_t *) malloc(sizeof(tls_t));
  if (tls == NULL) {
    fprintf(stderr, "malloc error\n");
    exit(1);
  }
  memset(tls, 0, sizeof(tls_t));

  THREAD_KEY_SET(global_tls_key, tls);
#endif /* ! __bgq__ */
}

void
tm_thread_exit_ibm_()
{
#ifndef __bgq__
  if (collect_stats) {
    tls_t *tls;
    int i;
    int region;

    tls = (tls_t *) THREAD_KEY_GET(global_tls_key);

    THREAD_MUTEX_LOCK(global_htm_stats_lock);
    for (region = 0; region < NUM_ATOMIC_REGIONS; region++) {
      for (i = 0; i < NUM_HTM_STATS_EVENTS; i++) {
	global_htm_stats_per_region[region].event_counter[i] += tls->htm_stats[region].event_counter[i];
      }
#if defined(__370__)
      for (i = 0; i < NUM_HTM_ABORT_REASON_CODES; i++) {
	int j;
	for (j = 0; j < NUM_HTM_TBEGIN_RETURNS; j++) {
	  global_htm_stats_per_region[region].abort_reason_code[i][j] += tls->htm_stats[region].abort_reason_code[i][j];
	}
      }
#elif defined(__x86_64__)
#define HTM_IA32_STAT(nam) global_htm_stats_per_region[region].abort_reason_counters.nam += tls->htm_stats[region].abort_reason_counters.nam;
#include "htm_ia32_stat.h"
#undef HTM_IA32_STAT
#elif defined(__PPC__) || defined(_ARCH_PPC)
#define HTM_PPC_STAT(nam) global_htm_stats_per_region[region].abort_reason_counters.nam += tls->htm_stats[region].abort_reason_counters.nam;
#include "htm_ppc_stat.h"
#undef HTM_PPC_STAT
#endif
    }
    THREAD_MUTEX_UNLOCK(global_htm_stats_lock);
  }
#endif /* ! __bgq__ */
}

static int
fall_back_global_lock(int acquire)
{
  tsx_inflight_status.isInWaitingLock = 1;
#ifdef USE_MUTEX
  const int spin_max = 2000000000;
  int spin_count = spin_max;
  while (gl.a.global_lock && --spin_count > 0)
    ;
  if (! acquire && ! gl.a.global_lock) {
    tsx_inflight_status.isInWaitingLock = 0;
    return 0;
  }

  THREAD_MUTEX_LOCK(global_lock_mutex);
  while (gl.a.global_lock) {
    THREAD_COND_WAIT(global_lock_cond, global_lock_mutex);
  }
  gl.a.global_lock = 1;
  THREAD_MUTEX_UNLOCK(global_lock_mutex);
  tsx_inflight_status.isInWaitingLock = 0;
  return 1;
#else /* !USE_MUTEX */
#if defined(__370__)
  cs_t local_value = 0;
  cs_t new_value = 1;
#endif

  while (gl.a.global_lock)
    ;
  if (! acquire){
    tsx_inflight_status.isInWaitingLock = 0;
    return 0;
  }

#if defined(__370__)
  while (cs(&local_value, (cs_t *)&gl.a.global_lock, new_value)) {
    while (local_value = gl.a.global_lock)
      ;
  }
#elif defined(__IBMC__)
  memory_fence();
  while(__check_lock_mp(&gl.a.global_lock, 0, 1));
  memory_fence();
#elif defined(__GNUC__)
  while (__sync_val_compare_and_swap(&gl.a.global_lock, 0, 1)) {
    while (gl.a.global_lock)
      ;
  }
#else
#error
#endif
  tsx_inflight_status.isInWaitingLock = 0;
  return 1;
#endif /* USE_MUTEX */
}

#define INCREMENT_STAT(field)				\
  do {							\
    if (collect_stats) {				\
      tls->htm_stats[region_id].event_counter[event_ ## field]++;	\
    }							\
  } while (0)

static int isAbortPersistent(int tbegin_result,TransactionDiagnosticInfo *diag) {
#if defined(__370__)
  uint64_t reason=diag->transactionAbortCode;
  return (tbegin_result != 4 &&
	  (diag->format != 1 || reason == 7 || reason == 8 || (11 <= reason && reason <= 13)));
#elif defined(__x86_64__)
  return ( (tbegin_result != 4) && !(diag->transactionAbortCode&XABORT_RETRY));
#elif defined(__PPC__) || defined(_ARCH_PPC)
  return (tbegin_result != 4) && (diag->transactionAbortCode&0x0100000000000000ULL);
#else
  return 0;
#endif
}

static int isTransactionDelinquent(htm_stats_t* stats) {
  static const long long min_tx_before_detection=1000;
  static const long long delinquent_abort_threshold=80;
  long long abort_ratio;
  int res=0;
  if(min_tx_before_detection<(stats->event_counter[event_tx])) {
    abort_ratio=(100LL* stats->event_counter[event_abort]) / stats->event_counter[event_tx];
    if(abort_ratio>delinquent_abort_threshold) {
      res=1;
    }
  }
  return res;
}


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


/*#define ABORT_CC_AND_RETRY_STATS*/

#ifdef FORTRAN_FLAG
void tbegin_ibm_(int *region_id_tmp)
#else
void tbegin_ibm_(int region_id)
#endif
{
  int tbegin_result;
  int transient_retry_count;
  int persistent_retry_count;
  int global_lock_retry_count;
  tls_t *tls = NULL;
  int first_retry;
#if defined(__370__)
  uint64_t saved_fprs[8];  /* FPR8 - FPR15 */
#endif
#ifdef ABORT_CC_AND_RETRY_STATS
  uint64_t saved_reason = 0; uint8_t saved_LSUAbortCode = 0; int saved_tbegin_result = 0;
#endif
#ifdef FORTRAN_FLAG
  int region_id;
  region_id = 0; //*region_id_tmp;
#endif

  tsx_inflight_status.isInCriticalSection = 1;
  if (collect_stats) {
    tls = (tls_t *) THREAD_KEY_GET(global_tls_key);
  }

  INCREMENT_STAT(tx_enter);

  /* Do not use HTM for delinquent transactions */
  if(DETECT_DELINQUENTS && isTransactionDelinquent(&(tls->htm_stats[region_id]))) {
    fall_back_global_lock(1);
    INCREMENT_STAT(detected_delinquent);
    return;
  }

  if (gl.a.global_lock) {
    if (fall_back_global_lock(0)) {
      INCREMENT_STAT(global_lock_wait_before_tx_sleep);
      return;
    }
    INCREMENT_STAT(global_lock_wait_before_tx_spin);
  }

  transient_retry_count = transient_retry_max;
  persistent_retry_count = persistent_retry_max;
  global_lock_retry_count = global_lock_retry_max;
  first_retry = 1;
#if defined(__370__)
  save_preserved_fpr(saved_fprs);
#endif

 tx_retry:
  INCREMENT_STAT(tx);
  tbegin_result = tbegin(&thread_diag);
  if (tbegin_result == 0) {
    /* Transaction */
    if (gl.a.global_lock) {
      tend();
      tbegin_result = 4;
    }
#ifdef HTM_CONSERVE_RWBUF
    else {
      suspend_tx();
    }
#endif
  }
  if (tbegin_result != 0) {
    /* Abort */
    uint64_t reason = thread_diag.transactionAbortCode;
    /*int saved_first_retry;*/

    if (tbegin_result != 4) {
#if defined(__370__)
      restore_preserved_fpr(saved_fprs);
#endif
    }

#ifdef ABORT_CC_AND_RETRY_STATS
    if (global_lock_retry_count == global_lock_retry_max
	&& reason == saved_reason && diag.LSUAbortCode == saved_LSUAbortCode && tbegin_result == saved_tbegin_result) {
      printf("AbortCcAndRetryStats r %d %llu %u %d\n", transient_retry_count, saved_reason, saved_LSUAbortCode, saved_tbegin_result);
    }
#endif

    INCREMENT_STAT(abort);
    /*saved_first_retry = first_retry;*/
    if (first_retry) {
      first_retry = 0;
      INCREMENT_STAT(first_abort);
    }

    if (gl.a.global_lock) {
      if (--global_lock_retry_count > 0) {
	if (fall_back_global_lock(0)) {
	  INCREMENT_STAT(global_lock_wait_and_retry_sleep);
	  return;
	}
	INCREMENT_STAT(global_lock_wait_and_retry_spin);
	goto tx_retry;
      }
      INCREMENT_STAT(global_lock_acquired);
      fall_back_global_lock(1);
    } else {
      if (collect_stats /*&& saved_first_retry*/) {
#if defined(__370__)
	if (tbegin_result == 4) {
	  tls->htm_stats[region_id].abort_reason_code[18][1]++;
	} else if (diag.format != 1) {
	  tls->htm_stats[region_id].abort_reason_code[0][tbegin_result - 1]++;
	} else if (reason <= 16) {
	  tls->htm_stats[region_id].abort_reason_code[reason][tbegin_result - 1]++;
	} else if (reason == 255) {
	  tls->htm_stats[region_id].abort_reason_code[17][tbegin_result - 1]++;
	} else {
	  tls->htm_stats[region_id].abort_reason_code[18][tbegin_result - 1]++;
	}
#elif defined(__x86_64__)
#define TLSCTR tls->htm_stats[region_id].abort_reason_counters
	if(tbegin_result==4) {
	  TLSCTR.GLOBAL_LOCK_ACQUIRED ++;
	} else {
	  TLSCTR.XABORT                    += (reason&XABORT_EXPLICIT)?1L:0L;
	  TLSCTR.TRANSIENT                 += (reason&XABORT_RETRY)?1L:0L;
	  TLSCTR.MEMADDR_CONFLICT          += (reason&XABORT_CONFLICT)?1L:0L;
	  TLSCTR.BUFFER_OVERFLOW           += (reason&XABORT_CAPACITY)?1L:0L;
	  TLSCTR.HIT_DEBUG_BREAKPOINT      += (reason&XABORT_DEBUG)?1L:0L;
	  TLSCTR.DURING_NESTED_TRANSACTION += (reason&XABORT_NESTED)?1L:0L;
	}
#elif defined(__PPC__) || defined(_ARCH_PPC)
#define TLSCTR tls->htm_stats[region_id].abort_reason_counters
#define PPC_BFIELD(x) ((0x1ULL)<<(63-x))
	if(tbegin_result==4) {
	  TLSCTR.Global_Lock_Acquired ++;
	} else {
	  TLSCTR.Failure_Persistent                += (reason&PPC_BFIELD( 7))?1:0;
	  TLSCTR.Disallowed                        += (reason&PPC_BFIELD( 8))?1:0;
	  TLSCTR.Nesting_Overflow                  += (reason&PPC_BFIELD( 9))?1:0;
	  TLSCTR.Footprint_Overflow                += (reason&PPC_BFIELD(10))?1:0;
	  TLSCTR.Self_Induced_Conflict             += (reason&PPC_BFIELD(11))?1:0;
	  TLSCTR.Non_Transactional_Conflict        += (reason&PPC_BFIELD(12))?1:0;
	  TLSCTR.Transaction_Conflict              += (reason&PPC_BFIELD(13))?1:0;
	  TLSCTR.Translation_Invalidation_Conflict += (reason&PPC_BFIELD(14))?1:0;
	  TLSCTR.Implementation_Specific           += (reason&PPC_BFIELD(15))?1:0;
	  TLSCTR.Instruction_Fetch_Conflict        += (reason&PPC_BFIELD(16))?1:0;
	}
#endif
      }

#ifdef ABORTED_INSN_ADDRESS_STATS
      if (reason == aborted_insn_address_reason_code) {
	int idx;

	if (diag.abortedTransactionInstructionAddress < ABORTED_INSN_ADDRESS_STATS_START_ADDRESS) {
	  idx = 0;
	} else {
	  idx = (diag.abortedTransactionInstructionAddress - ABORTED_INSN_ADDRESS_STATS_START_ADDRESS) / 2;
	  if (idx >= ABORTED_INSN_ADDRESS_STATS_MAP_SIZE)
	    idx = ABORTED_INSN_ADDRESS_STATS_MAP_SIZE - 1;
	}
	aborted_insn_address_stats[idx]++;
      }
#endif

#ifdef ABORT_CC_AND_RETRY_STATS
      if (global_lock_retry_count == global_lock_retry_max
	  && (transient_retry_count == transient_retry_max ||
	      reason == saved_reason && diag.LSUAbortCode == saved_LSUAbortCode && tbegin_result == saved_tbegin_result)) {
	printf("AbortCcAndRetryStats f %d %llu %u %d\n", transient_retry_count, reason, diag.LSUAbortCode, tbegin_result);
	saved_reason = reason; saved_LSUAbortCode = diag.LSUAbortCode; saved_tbegin_result = tbegin_result;
      } else {
	saved_reason = 0; saved_LSUAbortCode = 0; saved_tbegin_result = 0;
      }
#else /* ABORT_CC_AND_RETRY_STATS */
      if (isAbortPersistent(tbegin_result,&thread_diag)) {
	/* Persistent abort */
	if (--persistent_retry_count > 0) {
	  INCREMENT_STAT(persistent_abort_retry);
	  goto tx_retry;
	}
	INCREMENT_STAT(global_lock_persistent_abort);
	fall_back_global_lock(1);
      } else
#endif /* ! ABORT_CC_AND_RETRY_STATS */
      {
	/* Transient abort */
	if (--transient_retry_count > 0) {
	  INCREMENT_STAT(transient_abort_retry);
	  goto tx_retry;
	}
	INCREMENT_STAT(global_lock_transient_abort);
	fall_back_global_lock(1);
      }
    }
    tsx_inflight_status.isInFallbackPath = 1;
  }
}


void
tend_ibm_()
{
#ifdef HTM_CONSERVE_RWBUF
  resume_tx();
#endif
  if (gl.a.global_lock) {
#ifdef USE_MUTEX
    THREAD_MUTEX_LOCK(global_lock_mutex);
    gl.a.global_lock = 0;
    THREAD_COND_SIGNAL(global_lock_cond);
    THREAD_MUTEX_UNLOCK(global_lock_mutex);
#else
    memory_fence();
#if !defined(__370__) && defined(__IBMC__)
    __clear_lock_mp(&gl.a.global_lock,0);
#else
    gl.a.global_lock = 0;
#endif /* __IBMC__ */
#endif
    /*memory_fence();*/
  } else {
    tend();
  }
  tsx_inflight_status.isInFallbackPath = 0;
  tsx_inflight_status.isInCriticalSection = 0;
}

void
tabort_ibm_()
{
  tabort(300);
}
