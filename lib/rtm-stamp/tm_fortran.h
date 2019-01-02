#ifndef TM_FORTRAN_H
#define TM_FORTRAN_H 1
#if defined(HTM_IBM)
#define TM_STARTUP(numThread)         CALL tm_startup_ibm()
#define TM_SHUTDOWN()                 CALL tm_shutdown_ibm()

#define TM_THREAD_ENTER()             CALL tm_thread_enter_ibm()
#define TM_THREAD_EXIT()              CALL tm_thread_exit_ibm()

#define TM_BEGIN()                    CALL tbegin_ibm(0)

#define TM_END()                      CALL tend_ibm()
#define TM_RESTART()                  CALL tabort_ibm()
#else
#define TM_STARTUP(numThread)        
#define TM_SHUTDOWN()                 

#define TM_THREAD_ENTER()             
#define TM_THREAD_EXIT()              

#define TM_BEGIN()                    

#define TM_END()                     
#define TM_RESTART()                  
#endif
#endif
