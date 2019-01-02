/* Copyright (c) IBM Corp. 2014. */
#ifndef HTM_TSX_H
#define HTM_TSX_H 1

#include <stdlib.h>

#ifdef USE_MUTEX
#include <pthread.h>

#define THREAD_MUTEX_T                      pthread_mutex_t
#define THREAD_MUTEX_INIT(lock)             pthread_mutex_init(&(lock), NULL)
#define THREAD_MUTEX_LOCK(lock)             pthread_mutex_lock(&(lock))
#define THREAD_MUTEX_UNLOCK(lock)           pthread_mutex_unlock(&(lock))

#define THREAD_COND_T                       pthread_cond_t
#define THREAD_COND_INIT(cond)              pthread_cond_init(&(cond), NULL)
#define THREAD_COND_SIGNAL(cond)            pthread_cond_signal(&(cond))
#define THREAD_COND_BROADCAST(cond)         pthread_cond_broadcast(&(cond))
#define THREAD_COND_WAIT(cond, lock)        pthread_cond_wait(&(cond), &(lock))
#endif /*USE_MUTEX*/
typedef struct TM_LOCK_T{
   volatile unsigned short lock;
#ifdef GLOCK_RW
   volatile unsigned short write_flag;
#endif
#ifdef USE_MUTEX
   THREAD_MUTEX_T mutex;
   THREAD_COND_T cond;
#endif /*USE_MUTEX*/
} TM_LOCK_T;

#ifdef USE_MUTEX
#define TM_LOCK_INITIALIZER {0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER}
#else
#define TM_LOCK_INITIALIZER {0}
#endif /*USE_MUTEX*/

/* The underscore after each function name is used for Fortran */
#ifdef __cplusplus
extern "C" {
#endif
void tm_startup_();
void tm_begin_(TM_LOCK_T *lock_set);
void tm_begin_fallback_(TM_LOCK_T *lock_set);
void tm_begin_backoff_(TM_LOCK_T *lock_set);
void tm_end_(TM_LOCK_T *lock_set);
void tm_abort_();
void tm_shutdown_();
int tm_test_();
#ifdef GLOCK_RW
void set_write_flag_to();
#endif
unsigned int get_tsx_status(int opt);
#ifdef __cplusplus
}
#endif
#endif
