#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <memory.h>
#include <fcntl.h>

#include "pardhp.h"
#include "Database.h"

int DBASE_BUFFER_SIZE = 8192*8;

void init_DCB(Dbase_Ctrl_Blk &DCB, int infile)
{
   DCB.fd = infile;
   DCB.buf_size = DBASE_BUFFER_SIZE;
   DCB.buf = new int [DCB.buf_size];
   DCB.cur_buf_pos = 0;
   DCB.cur_blk_size = 0;
}

void close_DCB(Dbase_Ctrl_Blk &DCB)
{
   delete [] DCB.buf;
}

void reset_database(Dbase_Ctrl_Blk& DCB, int file_offset)
{
   lseek(DCB.fd, 3*ITSZ + file_offset*ITSZ, SEEK_SET);
   DCB.cur_buf_pos = 0;
   DCB.cur_blk_size = 0;
}

void get_next_trans_ext(Dbase_Ctrl_Blk& DCB, int &numitem, int &tid, int pid)
{
   // Need to get more items from file
   uint64_t start_time = rdtsc();
   int res = DCB.cur_blk_size - DCB.cur_buf_pos+2;
   if (res > 0)
   {
      // First copy partial transaction to beginning of buffer
      //DCB.cur_blk_size -= DCB.cur_buf_pos;
      memcpy(DCB.buf,
              (char *)(DCB.buf + DCB.cur_buf_pos - 2),
              res * ITSZ);
      DCB.cur_blk_size = res;
   }
   else
   {
      // No partial transaction in buffer
      DCB.cur_blk_size = 0;
   }

   DCB.cur_blk_size +=
      (read(DCB.fd,
            (char *)(DCB.buf + DCB.cur_blk_size),
            ((DCB.buf_size - DCB.cur_blk_size)*ITSZ)))/ITSZ;
   
   if (DCB.cur_blk_size > 0)
   {
	
      tid = DCB.buf[0];
      numitem = DCB.buf[1];
      DCB.cur_buf_pos = 2;   
   }  

   uint64_t end_time = rdtsc();
   thr_io_times[pid] += end_time - start_time; 
}

int Database_readfrom(char *infile, itemset2 * &t_utilitycnt, int pid, int file_offset, int data_size, all_atomic_time * atomic_secs_time)
{
   int i,j,k,m;
   
   int max_trans_sz = 1;
   int tid, numitem;
   Dbase_Ctrl_Blk DCB;
   //int *buf;
   int * buf = new int [DBASE_BUFFER_SIZE];
   int transaction_file;
   int maxsize, lb, ub, blk;
   double tran_utility=0;
  
   /*BSC*/
   uint64_t ab_enter_time_outside;
   uint64_t ab_exit_time_outside;
   uint64_t ab_enter_time_inside;
   uint64_t ab_exit_time_inside;
   /*BSC*/

   transaction_file = open(infile,O_RDONLY);
   read(transaction_file,buf,ITSZ);
   read(transaction_file,buf,ITSZ);
   read(transaction_file,buf,ITSZ);

   local_item_t_utility[pid] = (double *)calloc(maxitem, sizeof(double));
   
   init_DCB(DCB, transaction_file); 
   reset_database(DCB, file_offset);
   get_first_blk(DCB, pid);

   blk = (num_trans+nthreads - 1)/nthreads;
   lb = pid*blk;
   ub = min((pid+1)*blk, num_trans);

   for (i=lb; i < ub; i++){
      get_next_trans(DCB, buf, numitem, tid, pid);
      if (numitem > max_trans_sz) max_trans_sz = numitem;
      for (j=0; j < numitem*2-1; j=j+2)
      {
        tran_utility +=buf[j+1]*profit_array[buf[j]]; 
      }
      local_tran_utility[pid] += tran_utility;
      transaction_utility_array[tid] = tran_utility;
      
      for (j=0; j < numitem*2-1; j=j+2)
        local_item_t_utility[pid][buf[j]] +=tran_utility;

      tran_utility = 0;
   }
#pragma omp barrier

   if (pid==0) 
   {
      for (i=0; i<nthreads; i++)
         total_tran_utility += local_tran_utility[i];
      for (i=0; i<nthreads; i++)
         for (j=0; j<maxitem; j++)
            item_t_utility[j] += local_item_t_utility[i][j];
      
      free(local_tran_utility);
      for (i=0; i<nthreads; i++)   
         free(local_item_t_utility[i]);
      free(local_item_t_utility);   
   
      MIN_UTILITY = MIN_UTILITY_PER *total_tran_utility;
 
      int count=0;
      for (i=0; i<maxitem; i++) {
         if (item_t_utility[i] >= MIN_UTILITY) 
             count++;
      }

      printf("level 1 >MIN_UTILITY=%d\n", count);

      if (count > 0)  max_pattern_length =1;
      else max_pattern_length = 0;

      for (i=0; i< maxitem-1; i++)
         for (j=i+1; j<maxitem; j++) {
            if ((item_t_utility[i] >= MIN_UTILITY) && (item_t_utility[j] >= MIN_UTILITY))
               tot_cand++;
         }

      maxsize = min(maxitem, 1000);
      t_utilitycnt = (itemset2 *)calloc(maxitem, sizeof(itemset2));
      for (i=0; i< maxitem-1; i++) 
      {
         t_utilitycnt[i].size = maxsize;
         t_utilitycnt[i].t2 = (item_2 *)calloc(maxsize, sizeof(item_2));
      }
      printf("Thread_id: %d in pid=0\n", pid);
   }
#pragma omp barrier
   
   reset_database(DCB, file_offset);
   get_first_blk(DCB, pid);
   
   printf("Thread_id: %d lb= %d ub= %d\n", pid, lb, ub);
   for (i=lb; i < ub; i++)
   {
      //printf("Thread_id: %d difference: %d\n", pid, ub-i);
      get_next_trans(DCB, buf, numitem, tid, pid);
      for (j=0; j < numitem*2-1; j=j+2){
         if (item_t_utility[buf[j]] >= MIN_UTILITY) {
            for (k=j+2; k < numitem*2-1; k=k+2)
            {
               if (item_t_utility[buf[k]] >= MIN_UTILITY) 
               {

		ab_enter_time_outside = rdtsc();
        		TRANSACTION_BEGIN
                ab_enter_time_inside = rdtsc();

			for (m=0; m < t_utilitycnt[buf[j]].count; m++)
				if (t_utilitycnt[buf[j]].t2[m].item2==buf[k]) 
				{
					t_utilitycnt[buf[j]].t2[m].t_utility  += transaction_utility_array[tid];
					break;
				}
			
			if (m==t_utilitycnt[buf[j]].count) 
			{
				
				t_utilitycnt[buf[j]].count++;
				
				if (t_utilitycnt[buf[j]].count > t_utilitycnt[buf[j]].size) 
				{
					
					t_utilitycnt[buf[j]].size += maxsize;
					t_utilitycnt[buf[j]].t2 = (item_2 *)realloc(t_utilitycnt[buf[j]].t2, t_utilitycnt[buf[j]].size*sizeof(item_2));
				}
				
				t_utilitycnt[buf[j]].t2[m].item2 = buf[k];
				
				t_utilitycnt[buf[j]].t2[m].t_utility = transaction_utility_array[tid];
				
			}

                ab_exit_time_inside = rdtsc();
			TRANSACTION_END	
                ab_exit_time_outside = rdtsc();

		rearrange_atomic_total_time(pid, atomic_secs_time, ab_enter_time_inside, ab_exit_time_inside,  ab_enter_time_outside, ab_exit_time_outside);
               
               }      
            }
         }
      }
   }

   printf("MIN_UTILITY=%f\n", MIN_UTILITY);
   printf("level 2 tot=%d\n", tot_cand);
   return max_trans_sz;
}
