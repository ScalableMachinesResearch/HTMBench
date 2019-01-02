#ifndef TSX_H
#define TSX_H

#ifdef RTM
#include "tm.h"

inline int wrapper_mutext_lock(){TM_BEGIN(); return 0;}
inline int wrapper_mutext_unlock() {TM_END(); return 0;}
inline int wrapper_cond_wait() {TM_COND_WAIT_SPIN(c); return 0;}

#define pthread_mutex_t
#define pthread_cond_t
#define pthread_mutex_lock(l) wrapper_mutext_lock()
#define pthread_mutex_unlock(l) wrapper_mutext_unlock()
#define pthread_mutext_init(l, attr)
#define pthread_mutext_destroy(l)
#define safe_mutex_lock(l) TM_BEGIN()
#define safe_mutex_unlock(l) TM_END()
#define safe_cond_wait(c, l) TM_COND_WAIT_SPIN(c)
#define safe_cond_timed_wait(c,l, time, caller) TM_COND_WAIT_SPIN(c)
#define pthread_cond_wait(c,l) wrapper_cond_wait()

#define pthread_cond_signal(c) TM_COND_SIGNAL_SPIN(c)
#define safe_cond_signal(c) TM_COND_SIGNAL_SPIN(c)
#define pthread_cond_broadcast(c) TM_COND_BROADCAST_SPIN(c)
#define safe_cond_broadcast(c) TM_COND_BROADCAST_SPIN(c)

#else
#define TM_STARTUP(num_threads) /*nothing*/
#endif /*RTM*/
#endif /* TSX_H */
