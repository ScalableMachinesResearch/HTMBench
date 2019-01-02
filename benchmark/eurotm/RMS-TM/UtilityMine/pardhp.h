#ifdef __cplusplus
extern "C" {
#endif

#define TM
#include <time.h>
#include <unistd.h>
#include <stdint.h> 
#include <sys/types.h>
#include <omp.h>
#include "tsx.h"


extern double MIN_UTILITY;
extern char transaction_file_name[100], profit_file_name[100];
extern float *profit_array, *transaction_utility_array;
extern int num_trans;
extern int maxitem;
extern int avg_trans_sz;

extern double *item_t_utility;
extern double **local_item_t_utility, *local_tran_utility;
extern double total_tran_utility, tran_utility;

extern float MIN_UTILITY_PER;
// extern FILE *summary;
extern int tot_cand;
extern int max_pattern_length;

extern int nthreads;
extern int *hash_indx;
extern struct timeval tp;

// #define seconds(tm) gettimeofday(&tp,(struct timezone *)0);\
//    tm=tp.tv_sec+tp.tv_usec/1000000.0
      
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#ifndef ITEM_RANK
#define ITEM_RANK
typedef struct item_rank_2{
   double t_utility;
   int item2;
   }item_2;
typedef struct itemset_length_2 
{
   int count;
   int size;
   item_2 *t2;
} itemset2;

typedef struct ALL_ATOMIC_TIME
{
  uint64_t total_housekeeping_time;
  uint64_t total_executing_time;
  uint64_t total_atomic_time;
} all_atomic_time;
#endif

/*BSC*/
extern TM_PURE uint64_t rdtsc();
extern TM_CALLABLE void rearrange_atomic_total_time(int, all_atomic_time *, uint64_t, uint64_t, uint64_t, uint64_t);
extern TM_CALLABLE void atomic_totalhousekeeping_time(int, all_atomic_time *, uint64_t, uint64_t, uint64_t, uint64_t);
extern TM_CALLABLE void atomic_totalexecuting_time(int, all_atomic_time *, uint64_t, uint64_t);
extern TM_CALLABLE void atomic_totaltime(int, all_atomic_time *, uint64_t, uint64_t);

extern all_atomic_time * atomic_secs_time;
extern uint64_t * thr_io_times;
/*BSC*/

#ifdef __cplusplus   
   }
#endif

