/* Copyright (c) IBM Corp. 2014. */
#ifndef HTM_IBM_H
#define HTM_IBM_H 1

/* The underscore after each function name is used for Fortran */
#ifdef __cplusplus
extern "C" {
#endif
void tm_startup_ibm_();
void tm_shutdown_ibm_();

void tm_thread_enter_ibm_();
void tm_thread_exit_ibm_();


#ifdef FORTRAN_FLAG
void tbegin_ibm_(int *region_id_tmp);
#else
void tbegin_ibm_(int region_id);
#endif
void tend_ibm_();
void tabort_ibm_();

unsigned int get_tsx_status(int opt);
#ifdef __cplusplus
}
#endif

#endif
