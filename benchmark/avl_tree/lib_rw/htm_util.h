/* Copyright (c) IBM Corp. 2014. */
#ifndef _HTM_UTIL_H
#define _HTM_UTIL_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint64_t transactionAbortCode;
} TransactionDiagnosticInfo;

/*
  int tbegin(TransactionDiagnosticInfo *diag);
  Returns 0 for success
          1 for inderminate
          2 for transient abort
	  3 for persistent abort
 */

#define XABORT_EXPLICIT	0x01
#define XABORT_RETRY	0x02
#define XABORT_CONFLICT	0x04
#define XABORT_CAPACITY	0x08
#define XABORT_DEBUG	0x10
#define XABORT_NESTED	0x20

static inline int tbegin(TransactionDiagnosticInfo *diag) {
  int xbegin_result;
  xbegin_result = 0xffffffff;
  asm volatile("mov %0, %%eax;"
	       ".byte 0xc7; .byte 0xf8; .int 0x00000000;" /* xbegin */
	       "mov %%eax, %0"
	       : "+r"(xbegin_result)
	       :
	       : "%eax");
  if (xbegin_result == 0xffffffff)
    return 0;
  else {
    diag->transactionAbortCode = xbegin_result;
    if (xbegin_result & XABORT_RETRY)
      return 2;
    return 3;
  }
}
#define tend() do {							\
    asm volatile(".byte 0x0f; .byte 0x01; .byte 0xd5" :: ); /* xend */	\
  } while (0)
#define tabort(code) do {						\
    asm volatile(".byte 0xc6; .byte 0xf8; .byte 0xff" :: ); /* xabort */ \
  } while (0)

static inline int xtest(void)
{
	unsigned char out;
	asm volatile(".byte 0x0f,0x01,0xd6 ; setnz %0" : "=r" (out) :: "memory");
	return out;
}

#define NUM_HTM_FAILURE_REASONS 7

#ifdef __cplusplus
}
#endif

#endif /* _HTM_UTIL_H */
