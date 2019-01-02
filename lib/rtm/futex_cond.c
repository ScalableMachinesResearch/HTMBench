#include "futex_cond.h"
#include <linux/futex.h>
#include <syscall.h>
#include <stdlib.h>
#include <limits.h>
int futex_cond_init(FUTEX_T *cond){
  *cond = 0;
  return 0;
}
int futex_cond_wait(FUTEX_T *cond){
  return syscall(SYS_futex, cond, FUTEX_WAIT_PRIVATE, (*cond), NULL, NULL, 0);
}
int futex_cond_timewait(FUTEX_T *cond, struct timespec *time){
  return syscall(SYS_futex, cond, FUTEX_WAIT_PRIVATE, (*cond), time, NULL, 0);
}
int futex_cond_signal(FUTEX_T *cond){
  return syscall(SYS_futex, cond, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}
int futex_cond_broadcast(FUTEX_T *cond){
  return syscall(SYS_futex, cond, FUTEX_WAKE_PRIVATE, INT_MAX, NULL, NULL, 0);
}
