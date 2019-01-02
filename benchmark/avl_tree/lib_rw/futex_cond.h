#ifndef FUTEX_COND_H
#define FUTEX_COND_H

int futex_cond_init(int *cond);
int futex_cond_wait(int *cond);
int futex_cond_signal(int *cond);
int futex_cond_broadcast(int *cond);
#endif /*FUTEX_COND_H*/
