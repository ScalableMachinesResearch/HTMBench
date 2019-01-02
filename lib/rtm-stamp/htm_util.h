/* Copyright (c) IBM Corp. 2014. */
#ifndef _HTM_UTIL_H
#define _HTM_UTIL_H

#include <stdint.h>
#if defined(__370__) && __COMPILER_VER__ >= 0x410d0000
#include <builtins.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
#if defined(__370__)
  uint8_t format;
  uint8_t flags;
  uint16_t reserved0;
  uint16_t reserved1;
  uint16_t transactionNestingDepth;
  uint64_t transactionAbortCode;
  uint64_t conflictToken;
  uint64_t abortedTransactionInstructionAddress;
  uint8_t EAID;
  uint8_t DXC;
  uint16_t reserved2;
  uint32_t programInterruptionIdentification;
  uint64_t translationExceptionIdentification;
  uint64_t breakingEventAddress;
  uint64_t reserved3[8];
  uint32_t reserved4;
  uint16_t reserved5;
  uint8_t reserved6;
  uint8_t LSUAbortCode;
  uint64_t generalRegisters[16];
#elif defined(__x86_64)
  uint64_t transactionAbortCode;
#elif defined(__PPC__) || defined(_ARCH_PPC)
  uint64_t transactionAbortCode;
  uint64_t abortedTransactionInstructionAddress;
#endif
} TransactionDiagnosticInfo;

/*
  int tbegin(TransactionDiagnosticInfo *diag);
  Returns 0 for success
          1 for inderminate
          2 for transient abort
	  3 for persistent abort
 */

#if defined(__370__)

int tbegin(TransactionDiagnosticInfo *diag);
#if __COMPILER_VER__ >= 0x410d0000
#define tend() __TM_end()
#else
void tend();
#endif
void ctend(volatile int64_t *addr);
void tabort(uint64_t code);
void tabort_no_abend_in_no_tx(uint64_t code);
#if __COMPILER_VER__ >= 0x410d0000
/* addr must be 8-byte aligned */
#define non_tx_store(addr, data) __TM_non_transactional_store((addr), (long long)data)
#else
void non_tx_store(int64_t *addr, int64_t data);
#endif
#if __COMPILER_VER__ >= 0x410d0000
#define get_transaction_nesting_depth() __TM_nesting_depth(NULL)
#else
int get_transaction_nesting_depth();
#endif
void save_preserved_fpr(uint64_t *addr);
void restore_preserved_fpr(uint64_t *addr);

#define NUM_HTM_FAILURE_REASONS 19

#elif defined(__x86_64)

#define XABORT_EXPLICIT	0x01
#define XABORT_RETRY	0x02
#define XABORT_CONFLICT	0x04
#define XABORT_CAPACITY	0x08
#define XABORT_DEBUG	0x10
#define XABORT_NESTED	0x20

static inline int tbegin(TransactionDiagnosticInfo *diag)
{
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

#define NUM_HTM_FAILURE_REASONS 7

#elif defined(__PPC__) || defined(_ARCH_PPC)

static inline int tbegin(TransactionDiagnosticInfo *diag)
{
  uint32_t tbegin_status;

  asm volatile(".long 0x7c00051d;"
	       "mfcr %0"
	       :"=r"(tbegin_status): : "cr0");	// tbegin. 0
  if (! (tbegin_status & 0x20000000U))
    return 0;
  else {
#ifdef _ARCH_PPC64
    uint64_t texasr;
    uint64_t tfiar;

    asm volatile("mfspr %0,130;"
		 "mfspr %1,129"
		 :"=r"(texasr), "=r"(tfiar): );
    diag->transactionAbortCode = texasr;
#else
    uint32_t texasr;
    uint32_t texasru;
    uint32_t tfiar;

    asm volatile("mfspr %0,130;"
		 "mfspr %1,131;"
		 "mfspr %2,129"
		 :"=r"(texasr), "=r"(texasru), "=r"(tfiar): );
    diag->transactionAbortCode = (uint64_t)texasru << 32 | (uint64_t)texasr;
#endif
    diag->abortedTransactionInstructionAddress = (uint64_t)tfiar & ~0x3ULL;
    if (texasr & 0x0100000000000000ULL)
      return 3;
    return 2;
  }
}

/* tend. 0 */
#define tend() do {				\
    asm volatile(".long 0x7c00055d":::"cr0" );	\
  } while (0)

static inline void tabort(uint64_t code)
{
  uint64_t int_code = code << 56;
  asm volatile("mr 3,%0;"
	       ".long 0x7c03071d": : "r" (int_code) : "r3", "cr0"); // tabort. 3
  //asm(".long 0x7c00071d":::"cr0" ); // tabort. 0
}

#define tsuspend() do {				\
    asm volatile (".long 0x7C0005DD");		\
  } while (0)

#define tresume() do {				\
    asm volatile (".long 0x7C2005DD");		\
  } while (0)

#define NUM_HTM_FAILURE_REASONS 12

#else

#error

#endif

void countHTMFailures(TransactionDiagnosticInfo *diag, uint64_t *counters);
const char *getHTMFailureName(int id);

#ifdef __cplusplus
}
#endif

#endif /* _HTM_UTIL_H */
