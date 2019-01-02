#ifndef TM_H
#define TM_H 1

/* =============================================================================
 * HTM - Hardware Transactional Memory
 * =============================================================================
 */
#include "htm_tsx.h"
#include "futex_cond.h"

#define TM_STARTUP(numThread)         tm_startup_()
#define TM_SHUTDOWN()                 tm_shutdown_()

#define TM_THREAD_ENTER()             /* nothing */
#define TM_THREAD_EXIT()           /* nothing */
#define TM_BEGIN()            tm_begin_(NULL)
#define TM_END()              tm_end_(NULL)
#define TM_BEGIN_ARGS(lock_set)  tm_begin_(lock_set)
#define TM_END_ARGS(lock_set)       tm_end_(lock_set)
#define TM_BEGIN_BACKOFF(lock_set) tm_begin_backoff_(lock_set)

#define TM_COND_INIT_FUTEX(cond, attr) futex_cond_init(cond)
#define TM_COND_DESTROY_FUTEX(cond) /* nothing */
#define TM_COND_WAIT_FUTEX(cond) {TM_END();futex_cond_wait(cond);TM_BEGIN();}
#define TM_COND_BROADCAST_FUTEX(cond) futex_cond_broadcast(cond)
#define TM_COND_SIGNAL_FUTEX(cond) futex_cond_signal(cond)

#define TM_COND_INIT_SPIN(cond, attr) /* nothing */
#define TM_COND_DESTROY_SPIN(cond) /* nothing */
#define TM_COND_WAIT_SPIN(cond) {TM_END();TM_BEGIN();}
#define TM_COND_BROADCAST_SPIN(cond) /* nothing */
#define TM_COND_SIGNAL_SPIN(cond) /* nothing */

#endif /* TM_H */
