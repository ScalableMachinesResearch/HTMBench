
#ifndef	_PCLASS_H
#define _PCLASS_H

/*Gokcen*/
#define TM
/*Gokcen_end*/

#include "common.h"
#include <omp.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include "tsx.h"



extern struct timeval tp;
#define seconds(tm) gettimeofday(&tp,(struct timezone *)0);\
   tm=tp.tv_sec+tp.tv_usec/1000000.0

/*** TYPE DEFINITIONS ***/

extern TM_PURE uint64_t rdtsc();

typedef struct v{
  int val;
  int rid;
  int cid;
} VR;

#define VR_nTypes 	2

typedef struct cnt {
  int attid; 
  VR *valsrids;
  float max,min;
} Continuous;

#define Cval(p,i,j)	p[i].valsrids[j].val
#define Crid(q,i,j)	q[i].valsrids[j].rid
#define Ccid(q,i,j)	q[i].valsrids[j].cid

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

typedef struct tnd 	*pTNode;
typedef struct tnd {
  int 		Aid;			/* Attribute Id */
  float 	SplitVal;		/* if continuous atr */
  boolean	MoreThanOneClasses;  	/* 0: only one class */
  pTNode 	left; 			/* left kid */
  pTNode 	right;			/* right kid */
  int           index;
} TreeNode;

typedef struct bfsq	*pQueue;
typedef struct bfsq {
  pTNode node;
  int 	*CatrPointer;		/* Pointer into list for this node */
  int 	*CatrSize;		/* Size of the list for this node */
  int	**CatrCabove;		/* Cabove for the attribute */
  pQueue left;
  pQueue right;
  pQueue next;
  int id;
} Queue;

typedef struct ATOMIC_BLOCK 
{
  int transaction;
  int commit;
  uint64_t max_housekeeping_time;
  uint64_t min_housekeeping_time;
  uint64_t max_executing_time;
  uint64_t min_executing_time;
  uint64_t total_atomic_time;
} atomic_block;

typedef struct ENTER_EXIT_TIMES
{
  uint64_t enter_ab_time;
  uint64_t exit_ab_time;
} enter_exit_times;

typedef struct ALL_AS_TIME
{
  uint64_t total_waiting_time; //thread waits for share variable to acquire
  uint64_t total_executing_time; //total execution time is between acquiring lock and releasing lock
  uint64_t total_as_time; //waiting time <acquring lock> + executing time
} all_as_time;

/*** FUNCTION PROTOTYPES ***/
void atomic_block_times(int, atomic_block *,enter_exit_times ,enter_exit_times);
void rearrange_AS_totaltime(int, all_as_time *, uint64_t, uint64_t, uint64_t, uint64_t);
void AS_totalhousekeeping_time(int, all_as_time *, uint64_t, uint64_t,uint64_t, uint64_t);
void AS_totalexecuting_time(int, all_as_time *, uint64_t, uint64_t);
void AS_totaltime(int, all_as_time *, uint64_t, uint64_t);
double Calculate_Gini(int, VR *,int,int *,int *,int *,float *); 
pQueue NewQueueNode(int);
void ParClassify(int,atomic_block*, atomic_block*, atomic_block*, atomic_block*,all_as_time *,uint64_t*);
/*** GLOBAL VARIABLES ***/

#ifdef MAIN_FILE 

string *ClassNames;
Continuous *catr;
int *atype;
int nrec;
int nclass;

pTNode TreeRoot;

FILE *fpout;

#else

extern int nclass;
extern string *ClassNames;
extern Continuous *catr;
extern int *atype;
extern int nrec;

extern pTNode TreeRoot;

extern FILE *fpout;

#endif

#endif
