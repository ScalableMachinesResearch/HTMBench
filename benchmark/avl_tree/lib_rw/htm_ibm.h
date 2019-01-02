/* Copyright (c) IBM Corp. 2014. */
#ifndef HTM_TSX_H
#define HTM_TSX_H 1

/* The underscore after each function name is used for Fortran */
extern void tm_startup_();
extern void tm_begin_();
extern void tm_end_();
extern void tm_abort_();
extern unsigned int get_tsx_status(int opt);
#endif
