#ifndef	_TSX_H
#define _TSX_H
#include "tm.h"
#define TRANSACTION_BEGIN TM_BEGIN();
#define TRANSACTION_END TM_END();
#define TM_CALLABLE
#define TM_PURE
#endif
