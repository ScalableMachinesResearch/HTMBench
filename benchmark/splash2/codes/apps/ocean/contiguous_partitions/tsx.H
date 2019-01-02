#ifndef _TSX_H
#define _TSX_H

#ifdef RTM
#include "tm.h"

#define pthread_mutex_t        /* nothing */
#define pthread_mutex_init(mutex,attr)    (0)  
#define pthread_mutex_destroy(mutex)  (0) 
#define pthread_mutex_lock(mutex)     (TM_BEGIN(),0)
#define pthread_mutex_unlock(mutex)    (TM_END(),0)
#define pthread_cond_t          /* nothing */
#define pthread_cond_init(cond,attr)      (0)
#define pthread_cond_destroy(cond)    (0)
#define pthread_cond_signal(cond) (0)
#define pthread_cond_broadcast(cond)  (0)
#define pthread_cond_wait(cond, mutex) (TM_END(),TM_BEGIN(),0)

#else
#define TM_BEGIN() /* nothing */
#define TM_END() /* nothing */
#define TM_THREAD_ENTER() /* nothing */
#define TM_THREAD_EXIT() /* nothing */
#define TM_STARTUP(threads_num) /* nothing */
#define TM_SHUTDOWN() /* nothing */
#endif /*RTM */

#endif /*_TSX_H*/
