#ifndef FUTEX_COND_H
#define FUTEX_COND_H
#include<stdint.h>
#include<sys/time.h>
typedef uint32_t FUTEX_T;

int futex_cond_init(FUTEX_T *cond);
int futex_cond_wait(FUTEX_T *cond);
int futex_cond_timewait(FUTEX_T *cond, struct timespec *time);
int futex_cond_signal(FUTEX_T *cond);
int futex_cond_broadcast(FUTEX_T *cond);
#endif /*FUTEX_COND_H*/
