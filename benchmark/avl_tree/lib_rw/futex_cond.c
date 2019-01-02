#include "futex_cond.h"
#include <linux/futex.h>
#include <syscall.h>
#include <stdlib.h>
#include <limits.h>
int futex_cond_init(int *cond){
  *cond = 0;
  return 0;
}
int futex_cond_wait(int *cond){
  return syscall(SYS_futex, cond, FUTEX_WAIT_PRIVATE, 0, NULL, NULL, 0);
}
int futex_cond_signal(int *cond){
  return syscall(SYS_futex, cond, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}
int futex_cond_broadcast(int *cond){
  return syscall(SYS_futex, cond, FUTEX_WAKE_PRIVATE, INT_MAX, NULL, NULL, 0);
}
