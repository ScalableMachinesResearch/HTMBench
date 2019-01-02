/* Copyright (c) IBM Corp. 2014. */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "htm_util.h"

#if defined(__370__)

static int tbegin_impl(void *diag);
#if 1
/* lghi gpr3,0; tbegin 0(gpr1),FF00; brc 0x8,$+16; lghi gpr3,2; brc 0x6,$+8; lghi gpr3,3 */
#pragma mc_func tbegin_impl {"A7390000" "E5601000FF00" "A7840008" "A7390002" "A7640004" "A7390003"}
#else
/* lghi gpr3,0; tbegin 0(gpr1),FF00; brc 0x8,$+24; lghi gpr3,1; brc 0x4,$+16; lghi gpr3,2; brc 0x2,$+8; lghi gpr3,3 */
#pragma mc_func tbegin_impl {"A7390000" "E5601000FF00" "A784000C" "A7390001" "A7440008" "A7390002" "A7240004" "A7390003"}
#endif

static int tbegin_fp_impl(void *diag);
#if 1
#pragma mc_func tbegin_fp_impl {"A7390000" "E5601000FF04" "A7840008" "A7390002" "A7640004" "A7390003"}
#else
#pragma mc_func tbegin_fp_impl {"A7390000" "E5601000FF04" "A784000C" "A7390001" "A7440008" "A7390002" "A7240004" "A7390003"}
#endif

static void tend_impl(void *diag);
#pragma mc_func tend_impl {"B2F81000"}  /* tend 0(gpr1) */

static void ctend_impl(volatile int64_t *addr);
#pragma mc_func ctend_impl {"E30010000028"}  /* ctend 0(gpr1) */

static void tabort_impl(int code);
#pragma mc_func tabort_impl {"B2FC1000"}  /* tabort 0(gpr1) */

static void tabort_no_abend_in_no_tx_impl(int code);
/* tbegin 0(gpr0),FF00; brc 0x7,$+8; tabort 0(gpr1) */
#pragma mc_func tabort_no_abend_in_no_tx_impl {"E5600000FF00" "A7740004" "B2FC1000"}

static void non_tx_store_impl(int64_t *addr, int64_t data);
#pragma mc_func non_tx_store_impl { "E32010000025" }  /* ntstg gpr2,0(gpr1) */

static uint64_t
get_transaction_nesting_depth_impl();
#pragma mc_func get_transaction_nesting_depth_impl { "B2EC0030" }  /* ETNDG gpr3 */

/* std fpr8,0(gpr1); ...; std fpr15,56(gpr1) */
static void save_preserved_fpr_impl(uint64_t *addr);
#pragma mc_func save_preserved_fpr_impl {"60801000" "60901008" "60A01010" "60B01018" "60C01020" "60D01028" "60E01030" "60F01038"}

/* ld fpr8,0(gpr1); ...; ld fpr15,56(gpr1) */
static void restore_preserved_fpr_impl(uint64_t *addr);
#pragma mc_func restore_preserved_fpr_impl {"68801000" "68901008" "68A01010" "68B01018" "68C01020" "68D01028" "68E01030" "68F01038"}

int
tbegin(TransactionDiagnosticInfo *diag)
{
  /*return tbegin_impl(diag);*/
  return tbegin_fp_impl(diag);
}

#if defined(__370__) && __COMPILER_VER__ >= 0x410d0000
/* Macro-expanded to intrinsic */
#else
void
tend(void)
{
  TransactionDiagnosticInfo *diag = NULL;
  tend_impl(diag);
}
#endif

void
ctend(volatile int64_t *addr)
{
  ctend_impl(addr);
}

void
tabort(uint64_t code)
{
  tabort_impl(code);
}

void
tabort_no_abend_in_no_tx(uint64_t code)
{
  tabort_no_abend_in_no_tx_impl(code);
}

#if defined(__370__) && __COMPILER_VER__ >= 0x410d0000
/* Macro-expanded to intrinsic */
#else
void
non_tx_store(int64_t *addr, int64_t data)
{
  /* addr must be 8-byte aligned */
  non_tx_store_impl(addr, data);
}
#endif

#if defined(__370__) && __COMPILER_VER__ >= 0x410d0000
/* Macro-expanded to intrinsic */
#else
int
get_transaction_nesting_depth(void)
{
  uint64_t ret;

  ret = get_transaction_nesting_depth_impl();
  return ret & 0xFFFF;
}
#endif

void
save_preserved_fpr(uint64_t *addr)
{
  save_preserved_fpr_impl(addr);
}

void
restore_preserved_fpr(uint64_t *addr)
{
  restore_preserved_fpr_impl(addr);
}

#elif defined(__x86_64__)  /* __370__ */

/* Everything is in htm_util.h */

#elif defined(__PPC__) || defined(_ARCH_PPC)

/* Everything is in htm_util.h */

#else /* __370__ , __x86_64, __PPC__ */

#error

#endif  /* __370__ -> __x86_64 -> __PPC__ */

void
countHTMFailures(TransactionDiagnosticInfo *diag, uint64_t *counters)
{
#if defined(__370__)
  uint64_t abortCode = diag->transactionAbortCode;
  if (diag->format != 1) {
    counters[0]++;
  } else if (abortCode <= 2) {
    counters[abortCode]++;
    /* No such abort code as 3  */
  } else if (abortCode <= 17) {
    counters[abortCode - 1]++;
  } else if (abortCode == 255) {
    counters[17]++;
  } else {
    counters[18]++;
  }
#elif defined(__x86_64__)
  uint64_t abortCode = diag->transactionAbortCode;
  counters[0] += (abortCode & XABORT_EXPLICIT) ? 1 : 0;
  counters[1] += (abortCode & XABORT_RETRY) ? 1 : 0;
  counters[2] += (abortCode & XABORT_CONFLICT) ? 1 : 0;
  counters[3] += (abortCode & XABORT_CAPACITY) ? 1 : 0;
  counters[4] += (abortCode & XABORT_DEBUG) ? 1 : 0;
  counters[5] += (abortCode & XABORT_NESTED) ? 1 : 0;
  counters[6] += (abortCode & (XABORT_EXPLICIT | XABORT_RETRY | XABORT_CONFLICT | XABORT_CONFLICT | XABORT_DEBUG | XABORT_NESTED)) ? 0 : 1;  /* Other */
#elif defined(__PPC__) || defined(_ARCH_PPC)
  uint64_t texasr = diag->transactionAbortCode;
#define PPC_BFIELD(x) ((0x1ULL)<<(63-x))
  counters[ 0] += (texasr & PPC_BFIELD( 7))? 1 : 0;
  counters[ 1] += (texasr & PPC_BFIELD( 8))? 1 : 0;
  counters[ 2] += (texasr & PPC_BFIELD( 9))? 1 : 0;
  counters[ 3] += (texasr & PPC_BFIELD(10))? 1 : 0;
  counters[ 4] += (texasr & PPC_BFIELD(11))? 1 : 0;
  counters[ 5] += (texasr & PPC_BFIELD(12))? 1 : 0;
  counters[ 6] += (texasr & PPC_BFIELD(13))? 1 : 0;
  counters[ 7] += (texasr & PPC_BFIELD(14))? 1 : 0;
  counters[ 8] += (texasr & PPC_BFIELD(15))? 1 : 0;
  counters[ 9] += (texasr & PPC_BFIELD(16))? 1 : 0;
  counters[10] += (texasr & PPC_BFIELD(31))? 1 : 0;
  counters[11] += (texasr & (PPC_BFIELD(7) | PPC_BFIELD(8) | PPC_BFIELD(9) | PPC_BFIELD(10) | PPC_BFIELD(11) | PPC_BFIELD(12) | PPC_BFIELD(13) | PPC_BFIELD(14) | PPC_BFIELD(15) | PPC_BFIELD(16) | PPC_BFIELD(31))) ? 0 : 1;  /* Other */
#else
#error
#endif
}

const char *
getHTMFailureName(int id)
{
  const char *reason_strings[] = {
#if defined(__370__)
    "TDB_not_set",
    "Restart_interruption",
    "External_interruption",
    "Program_interruption",
    "Machine-check_interruption",
    "I/O_interruption",
    "Fetch_overflow",
    "Store_overflow",
    "Fetch_conflict",
    "Store_conflict",
    "Restricted_instruction",
    "Program-interruption_condition",
    "Nesting_depth_exceeded",
    "Cache_fetch-related",
    "Cache_store-related",
    "Cache_other",
    "CTEND_abort",
    "Miscellaneous_condition",
    "TABORT_instruction"
#elif defined(__x86_64__)
    "XABORT",
    "TRANSIENT",
    "MEMADDR_CONFLICT",
    "BUFFER_OVERFLOW",
    "HIT_DEBUG_BREAKPOINT",
    "DURING_NESTED_TRANSACTION",
    "OTHER"
#elif defined(__PPC__) || defined(_ARCH_PPC)
    "Failure_Persistent",
    "Disallowed",
    "Nesting_Overflow",
    "Footprint_Overflow",
    "Self_Induced_Conflict",
    "Non_Tx_Conflict",
    "Tx_Conflict",
    "Translation_Invalidation",
    "Implementation_Specific",
    "Instruction_Fetch_Conflict",
    "Tabort_Treclaim",
    "Other"
#else
#error
#endif
  };

  if (id < 0 || NUM_HTM_FAILURE_REASONS <= id) {
    return NULL;
  }
  return reason_strings[id];
}
