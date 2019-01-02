#ifndef TSX_H_
#define TSX_H_

#ifdef RTM
#include "tm.h"

#define omp_lock_t /*nothing*/

#define omp_init_lock(lock) /* nothing */
#define omp_set_lock(lock) TM_BEGIN()
#define omp_unset_lock(lock) TM_END()

#else
#define TM_BEGIN() /* nothing */
#define TM_END() /*nothing*/

#endif //RTM

#endif //TSX_H_
