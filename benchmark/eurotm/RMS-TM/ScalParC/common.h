#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#ifdef INT8
typedef short	int4;
#define INT4_MPI	MPI_SHORT
#else
typedef int	int4;
#define INT4_MPI	MPI_INT
#endif

#define NAME_LEN	256
#define CID		1
#define DID		0
#define EPSILON		1.0E-10

typedef char		string[NAME_LEN];
typedef char		boolean;

#ifdef MAIN_FILE
int natr=0;
#else
extern int natr;
#endif

#define memerr(a)	{printf("Not enough Memory %d\n",a);exit(1);}

#endif
