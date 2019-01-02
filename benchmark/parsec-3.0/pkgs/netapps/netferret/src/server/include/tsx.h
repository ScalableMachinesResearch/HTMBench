#ifndef TSX_H
#define TSX_H

#include <pthread.h>

#ifdef RTM
#include "tm.h"

#define pthread_mutex_t       /* nothing */
#define pthread_mutex_init(mutex,attr)      /* nothing */
#define pthread_mutex_destroy(mutex)  /* nothing */
#define pthread_mutex_lock(mutex)     TM_BEGIN()
#define pthread_mutex_unlock(mutex)    TM_END()
#define pthread_cond_t          /* nothing */
#define pthread_cond_init(cond,attr)      /* nothing */
#define pthread_cond_destroy(cond)    /* nothing */
#define pthread_cond_signal(cond) TM_COND_SIGNAL_SPIN(cond)
#define pthread_cond_broadcast(cond)  TM_COND_BROADCAST_SPIN(cond)
#define pthread_cond_wait(cond, mutex)  {TM_END();TM_BEGIN();}

#else
#define TM_STARTUP(numThreads) /*nothing*/
#define TM_SHUTDOWN() /*nothing*/

#endif /*RTM */



#endif /*TSX_H*/
