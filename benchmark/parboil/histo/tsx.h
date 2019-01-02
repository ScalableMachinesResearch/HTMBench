#ifndef TSX_H
#define TSX_H

#ifdef RTM
#include "tm.h"

#else
#define TM_STARTUP(num_threads) /*nothing*/
#endif /*RTM*/
#endif /* TSX_H */
