// IMPORTANT:  READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
// By downloading, copying, installing or using the software you agree
// to this license.  If you do not agree to this license, do not download,
// install, copy or use the software.
// 
// 
// Copyright (c) 2008 Northwestern University
// All rights reserved.
// 
// Redistribution of the software in source and binary forms,
// with or without modification, is permitted provided that the
// following conditions are met: 
// 
// 1       Redistributions of source code must retain the above copyright
//         notice, this list of conditions and the following disclaimer. 
// 
// 2       Redistributions in binary form must reproduce the above copyright
//         notice, this list of conditions and the following disclaimer in the
//         documentation and/or other materials provided with the distribution. 
// 
// 3       Neither the name of Northwestern University nor the names of its
//         contributors may be used to endorse or promote products derived
//         from this software without specific prior written permission.
//  
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
// IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// NORTHWESTERN UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAIN_FILE
#include "pclass.h"

#define FILE_NAME_LEN 	256
#define LINE_LEN	2048

#define itag		1

atomic_block * first_as_time;
atomic_block * second_as_time;
atomic_block * third_as_time;
atomic_block * fourth_as_time;

int VRCompare(VR *,VR *);
struct timeval tp;

int main(int argc, char *argv[])
{
  FILE *fp_data;
  char datafile[FILE_NAME_LEN], file_name[FILE_NAME_LEN];
  int i,j,k;
  int (*fcnt)(const void*, const void*);
  int mm,nn,nthreads, pid, lb, ub, local_natr;
  uint64_t ts_uint,te_uint, io_t_1_uint, io_t_2_uint, *io_time_uint;
  all_as_time * atomic_blocks_time;
  uint64_t max_uint;
  uint64_t max_atomic_block_time = 0; 

  if(argc < 6) {
      printf(" Usage: %s <datafile> <#recoreds> <#attributes> <#classes> <#threads>\n", argv[0]);
      printf("   <datafile>, is used to read data file\n");
      printf("   <#records>, number of records\n");
      printf("   <#attributes>, number of attributes\n");
      printf("   <#classes>, number of classes\n");
      printf("   <#threads>, numner of threads\n");
  }
  TM_STARTUP(-1);
  strcpy(datafile,argv[1]);
  nrec = atoi(argv[2]);
  natr = atoi(argv[3]);
  nclass = atoi(argv[4]);
  nthreads = atoi(argv[5]);
  catr = (Continuous *)calloc(natr,sizeof(Continuous));
  io_time_uint = (uint64_t *)calloc(nthreads,sizeof(uint64_t));
  
  /*Gokcen*/
  first_as_time = (atomic_block*)calloc(nthreads,sizeof(atomic_block));
  second_as_time = (atomic_block*)calloc(nthreads,sizeof(atomic_block));
  third_as_time = (atomic_block*)calloc(nthreads,sizeof(atomic_block));
  fourth_as_time = (atomic_block*)calloc(nthreads,sizeof(atomic_block));
  atomic_blocks_time = (all_as_time*) calloc (nthreads, sizeof(all_as_time));
  /*Gokcen*/
  

  for(i=0;i<natr;i++) {
    if(!(catr[i].valsrids = (VR *)calloc(nrec, sizeof(VR)))) {
      printf("Memory Crunch @ catr[%d].valsrids\n",i);
    }
  }

  fcnt = (int (*)(const void *, const void *))VRCompare;
  strcat(datafile, ".att");

  ts_uint = rdtsc();
  omp_set_num_threads(nthreads);
  #pragma omp parallel private(io_t_1_uint, io_t_2_uint,mm, nn, fp_data,file_name,pid, local_natr, lb, ub,i,j)
  {
   pid = omp_get_thread_num();
   local_natr = (natr + nthreads - 1)/nthreads;
   lb = pid*local_natr;
   ub = min((pid+1)*local_natr, natr);

   io_t_1_uint =  rdtsc();

   for(i=lb;i<ub;i++) {
     sprintf(file_name, "%s.%d", datafile, i);
     fp_data = fopen(file_name, "r");
     for (j=0; j<nrec; j++) 
     {
       fscanf(fp_data, "%d\n", &mm);
       fscanf(fp_data, "%d\n", &nn);
       catr[i].valsrids[j].val=mm;
       catr[i].valsrids[j].cid = nn;
       catr[i].valsrids[j].rid = j;
     }
     fclose(fp_data);
  }

  io_t_2_uint = rdtsc();
  for(i=lb;i<ub;i++) 
    qsort(catr[i].valsrids,nrec,sizeof(VR),(int (*)(const void *, const void *))VRCompare);

  io_time_uint[pid] = io_t_2_uint - io_t_1_uint;
  
}

  printf("Starting Classify\n");
  uint64_t parallel_sec_time = 0;
  ParClassify(nthreads,first_as_time,second_as_time,third_as_time,fourth_as_time,atomic_blocks_time,&parallel_sec_time);

  te_uint = rdtsc();

  max_uint = io_time_uint[0];
  for (i=1; i<nthreads; i++) 
  {
   if (io_time_uint[i]>max_uint)
      max_uint=io_time_uint[i];  
  }

  for (int i=0; i<nthreads; i++)
  {
     printf("*****************************\n");
     printf("THREAD_ID= %d\tFIRST_CS_transaction_NUMBER= %d\n", i,first_as_time[i].transaction);
     printf("THREAD_ID= %d\tFIRST_CS_COMMIT_NUMBER= %d\n", i,first_as_time[i].commit);
     printf("THREAD_ID= %d\tFIRST_CS_MAX_WAIT= %llu\n",i, first_as_time[i].max_housekeeping_time);
     printf("THREAD_ID= %d\tFIRST_CS_MIN_WAIT= %llu\n",i,first_as_time[i].min_housekeeping_time);
     printf("THREAD_ID= %d\tFIRST_CS_MAX_COMMIT= %llu\n",i,first_as_time[i].max_executing_time);
     printf("THREAD_ID= %d\tFIRST_CS_MIN_COMMIT= %llu\n",i,first_as_time[i].min_executing_time);
     printf("THREAD_ID= %d\tFIRST_CS_TOTALTIME= %llu\n\n",i,first_as_time[i].total_atomic_time);
     
 
     printf("THREAD_ID= %d\tSECOND_CS_transaction_NUMBER= %d\n", i,second_as_time[i].transaction);
     printf("THREAD_ID= %d\tSECOND_CS_COMMIT_NUMBER= %d\n", i,second_as_time[i].commit);
     printf("THREAD_ID= %d\tSECOND_CS_MAX_WAIT= %llu\n",i, second_as_time[i].max_housekeeping_time);
     printf("THREAD_ID= %d\tSECOND_CS_MIN_WAIT= %llu\n",i,second_as_time[i].min_housekeeping_time);
     printf("THREAD_ID= %d\tSECOND_CS_MAX_COMMIT= %llu\n",i,second_as_time[i].max_executing_time);
     printf("THREAD_ID= %d\tSECOND_CS_MIN_COMMIT= %llu\n",i,second_as_time[i].min_executing_time);
     printf("THREAD_ID= %d\tSECOND_CS_TOTALTIME= %llu\n\n",i,second_as_time[i].total_atomic_time);
     

     printf("THREAD_ID= %d\tTHIRD_CS_transaction_NUMBER= %d\n", i,third_as_time[i].transaction);
     printf("THREAD_ID= %d\tTHIRD_CS_COMMIT_NUMBER= %d\n", i,third_as_time[i].commit);
     printf("THREAD_ID= %d\tTHIRD_CS_MAX_WAIT= %llu\n",i, third_as_time[i].max_housekeeping_time);
     printf("THREAD_ID= %d\tTHIRD_CS_MIN_WAIT= %llu\n",i,third_as_time[i].min_housekeeping_time);
     printf("THREAD_ID= %d\tTHIRD_CS_MAX_COMMIT= %llu\n",i,third_as_time[i].max_executing_time);
     printf("THREAD_ID= %d\tTHIRD_CS_MIN_COMMIT= %llu\n",i,third_as_time[i].min_executing_time);
     printf("THREAD_ID= %d\tTHIRD_CS_TOTALTIME= %llu\n\n",i,third_as_time[i].total_atomic_time);
     

     printf("THREAD_ID= %d\tFOURTH_CS_transaction_NUMBER= %d\n", i,fourth_as_time[i].transaction);
     printf("THREAD_ID= %d\tFOURTH_CS_COMMIT_NUMBER= %d\n", i,fourth_as_time[i].commit);
     printf("THREAD_ID= %d\tFOURTH_CS_MAX_WAIT= %llu\n",i, fourth_as_time[i].max_housekeeping_time);
     printf("THREAD_ID= %d\tFOURTH_CS_MIN_WAIT= %llu\n",i,fourth_as_time[i].min_housekeeping_time);
     printf("THREAD_ID= %d\tFOURTH_CS_MAX_COMMIT= %llu\n",i,fourth_as_time[i].max_executing_time);
     printf("THREAD_ID= %d\tFOURTH_CS_MIN_COMMIT= %llu\n",i,fourth_as_time[i].min_executing_time);
     printf("THREAD_ID= %d\tFOURTH_CS_TOTALTIME= %llu\n\n",i,fourth_as_time[i].total_atomic_time);

     printf("THREAD_ID= %d\tTOTAL_TIME_FOR_ALL_SECTIONS= %llu\n",i,first_as_time[i].total_atomic_time+second_as_time[i].total_atomic_time+third_as_time[i].total_atomic_time+fourth_as_time[i].total_atomic_time); 
     printf("*****************************\n\n");
   }

   

   for (int i=0; i<nthreads; i++)
   {
     printf("THREAD_ID= %d\tTOTAL_WAITING_TIME_FOR_ACQURING_LOCK= %llu\n", i, atomic_blocks_time[i].total_waiting_time);
     printf("THREAD_ID= %d\tTOTAL_EXECUTING_TIME= %llu\n", i, atomic_blocks_time[i].total_executing_time);
     printf("\t\tTHREAD_ID= %d\tTOTAL_TIME_WITH_WAITING_TIME= %llu\n", i, atomic_blocks_time[i].total_as_time);
     
     if ( atomic_blocks_time[i].total_as_time > max_atomic_block_time)
       max_atomic_block_time =  atomic_blocks_time[i].total_as_time;
   }

  printf("MAX_ATOMIC_BLOCK_TIME= %llu\n",max_atomic_block_time);
  printf("PARALLEL_SECTION_TIME_IN_RDTSC= %llu\n",parallel_sec_time); 
  printf("I/O_TIME_IN_RDTSC= %llu\n", max_uint);
  printf("COMPUTATION_TIME_IN_RDTSC= %llu\n", te_uint-ts_uint-max_uint);
  printf("TOTAL_EXC_TIME_IN_RDTSC= %llu\n",te_uint-ts_uint); 
  //Critical section rate = (MAX_CRITICAL_SECTION_TIME / COMPUTATION_TIME_IN_RDTSC) *100

  free(io_time_uint);
  free(first_as_time);
  free(second_as_time);
  free(third_as_time);
  free(fourth_as_time);
  free(atomic_blocks_time);
  TM_SHUTDOWN();
}
