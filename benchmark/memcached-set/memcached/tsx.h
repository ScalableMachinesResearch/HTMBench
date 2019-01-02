//This header file defines the transactional memory
#ifndef TSX_H
#define TSX_H
#ifdef RTM
#include "tm.h"
#include <semaphore.h>
#include <pthread.h>
//#define pthread_mutex_lock(lock) TM_BEGIN()
//#define pthread_mutex_unlock(lock) TM_END()

//For trylock, we can eliminate it according to the context

//#define pthread_mutex_trylock(lock)  /* nothing */

//#define pthread_mutex_init(lock, attr)  /* nothing */
//#define pthread_mutex_destroy(lock) /* nothing */

//#define pthread_cond_init(cond, attr) //sem_init(cond, 0, 0)
//#define pthread_cond_destroy(cond) //sem_destroy(cond)
//#define pthread_cond_signal(cond) //sem_post(cond)
//#define pthread_cond_wait(cond, mutex) TM_END();TM_BEGIN();/* sem_wait(cond) */

//#define fake_wait() /*nothing*/

//#define pthread_mutex_t  /* nothing */
//#define pthread_cond_t  sem_t

/*
enum pseudo_lock_type {PSEUDO_LOCK_INUSE, PSEUDO_LOCK_FREE};
//int inline pseudo_trylock(enum pseudo_lock_type *lock);
static int inline pseudo_trylock(enum pseudo_lock_type *lock){
  int ret;
  TM_BEGIN();
  if (*lock == PSEUDO_LOCK_INUSE){
    ret = 1;
  }
  else {
    *lock = PSEUDO_LOCK_INUSE;
    ret = 0;
  }
  TM_END();
  return ret;
}

//void inline pseudo_initlock(enum pseudo_lock_type *lock);
static void inline pseudo_initlock(enum pseudo_lock_type *lock){
   TM_BEGIN();
   *lock = PSEUDO_LOCK_FREE;
   TM_END();
}
*/

#endif

#endif // TSX_H
