#define TM

#include <stdint.h>
#include "tsx.h"

TM_PURE
uint64_t rdtsc() 
{
   uint32_t lo, hi;
   __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
   return (uint64_t)hi << 32 | lo;
}