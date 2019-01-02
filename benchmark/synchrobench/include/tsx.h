

#ifdef ESTM
#  include "estm.h"
#elif defined(RTM)
#  include "tm.h"
#  define TX_START(type) TM_BEGIN()
#  define TX_LOAD(addr) (*addr)
#  define TX_UNIT_LOAD(addr) TX_LOAD(addr)
#  define UNIT_LOAD(addr) TX_LOAD(addr)
#  define TX_UNIT_LOAD_TS(addr, timestamp) TX_LOAD(addr)
#  define TX_STORE(addr, val)   (*addr)=val
#  define TX_END TM_END()
#  define FREE(addr, size) free(addr)
#  define MALLOC(size) malloc(size)
#  define TM_CALLABLE                    /* nothing */
#  define TM_ARGDECL_ALONE               /* nothing */
#  define TM_ARGDECL                     /* nothing */
#  define TM_ARG                         /* nothing */
#  define TM_ARG_LAST                    /* nothing */
#  define TM_ARG_ALONE                   /* nothing */
//#  define TM_STARTUP()  tm_startup_() /* already defined */
//#  define TM_SHUTDOWN()  tm_shutdown_() /* already defined */
//#  define TM_THREAD_ENTER()           /* already defined */
//#  define TM_THREAD_EXIT() /* already defined */
#elif defined(TINY100) || defined(TINY10B) || defined(TINY099) || defined(TINY098)
#  include "tinystm.h"
#elif defined WPLDSTM
#  include "wlpdstm.h"
#elif defined LOCKFREE
#  include "lockfree.h"
#elif defined TL2
#  include "tl2-mbench.h"
#elif defined ICC
#  include "icc.h"
#elif defined SEQUENTIAL
#  include "sequential.h"
#endif
