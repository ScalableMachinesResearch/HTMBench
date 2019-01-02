#include <math.h>
#include "pclass.h"

/*
#define Debug
*/

typedef struct ginfoo{
  double gini;  /* gini index */
  float sval;  /* split value */
  int   said;  /* split atr id */
  int   satp;  /* split atr type */
  int   sptr;  /* split pointer */
} GININFO;


TM_PURE
void increase_number(int* as_transaction_or_commit_number)
{
  (*as_transaction_or_commit_number)++;
}


void atomic_block_times(int pid, atomic_block * atomic_block,enter_exit_times enter_exit_times_inside,enter_exit_times enter_exit_times_outside)
{
 
	uint64_t waiting_time = enter_exit_times_inside.enter_ab_time - enter_exit_times_outside.enter_ab_time;

	uint64_t executing_time = enter_exit_times_inside.exit_ab_time - enter_exit_times_inside.enter_ab_time;
	

	uint64_t total_time = enter_exit_times_outside.exit_ab_time - enter_exit_times_outside.enter_ab_time;
  
        atomic_block[pid].max_housekeeping_time = max(atomic_block[pid].max_housekeeping_time,waiting_time);
        atomic_block[pid].min_housekeeping_time = min(atomic_block[pid].min_housekeeping_time,waiting_time);
        atomic_block[pid].max_executing_time = max(atomic_block[pid].max_executing_time,executing_time);
	atomic_block[pid].min_executing_time = min(atomic_block[pid].min_executing_time,executing_time);
	
	atomic_block[pid].total_atomic_time += total_time;


}

void rearrange_AS_totaltime(int pid, all_as_time * atomic_blocks_time, uint64_t outside_enter, uint64_t outside_exit, uint64_t inside_enter, uint64_t inside_exit)
{
   AS_totalhousekeeping_time(pid, atomic_blocks_time, inside_enter, inside_exit, outside_enter, outside_exit);
   AS_totalexecuting_time(pid, atomic_blocks_time, inside_enter,inside_exit);
   AS_totaltime(pid, atomic_blocks_time, outside_enter, outside_exit);
}

void AS_totalhousekeeping_time(int pid, all_as_time * atomic_blocks_time, uint64_t enter_time_inside, uint64_t exit_time_inside, uint64_t enter_time_outside, uint64_t exit_time_outside)
{
  uint64_t executing_time;
  uint64_t total_time;

  executing_time = exit_time_inside - enter_time_inside;
  total_time = exit_time_outside - enter_time_outside;

  atomic_blocks_time[pid].total_waiting_time += total_time - executing_time;

}

void AS_totalexecuting_time(int pid, all_as_time * atomic_blocks_time, uint64_t start_time, uint64_t end_time)
{
  
    atomic_blocks_time[pid].total_executing_time += end_time-start_time;

}

void AS_totaltime(int pid, all_as_time * atomic_blocks_time, uint64_t start_time, uint64_t end_time)
{

    atomic_blocks_time[pid].total_as_time += end_time-start_time;

}

double Calculate_Gini(int pid, VR *Recs,int size,int *Cabove,int *Cbelow,
		int *spointer,float *svalue) 
{
  long i,j;
  double v1,v2,k,l,v,split_above,split_below,gini,mgini;

  if(size == 0) return 1.0;

  split_above = 0.0;
  split_below = 0.0;
  v1 = 0.0;
  v2 = 0.0;
  for(j=0;j<nclass;j++) {
    k = (double)Cabove[j];
    l = (double)Cbelow[j];
    split_above += k*k;
    split_below += l*l;
    v1 += k;
    v2 += l;
  }
  v = v1+v2;
  if(fabs(v2) < EPSILON)
    if(fabs(v1) < EPSILON)
      return 1.0;
    else
      mgini = 1.0-(split_above/(v1*v));
  else
    if(fabs(v1) < EPSILON)
      mgini = 1.0-split_below/(v2*v);
    else
      mgini = 1.0-split_above/(v1*v)-split_below/(v2*v);
  *spointer = 0;

  for(i=0;i<size-1;i++) {
    j = Recs[i].cid;
    v1 -= 1.0;
    v2 += 1.0;
    k = (double)Cabove[j];
    l = (double)Cbelow[j];
    split_above += 1.0-2.0*k;
    split_below += 1.0+2.0*l;
    Cabove[j]--;
    Cbelow[j]++;
    gini = 1.0 - split_above/(v1*v) - split_below/(v2*v);
    if(gini < mgini) {
      mgini = gini;
      *spointer = i+1;
    }
  }

  j = Recs[i].cid;
  v1 -= 1.0;
  v2 += 1.0;
  k = (double)Cabove[j];
  l = (double)Cbelow[j];
  split_above += 1.0-2.0*k;
  split_below += 1.0+2.0*l;
  Cabove[j]--;
  Cbelow[j]++;
  if(fabs(v1) < EPSILON)
    gini = 1.0 - split_below/(v2*v);
  else
    gini = 1.0 - split_above/(v1*v) - split_below/(v2*v);
  if(gini < mgini) {
    mgini = gini;
    *spointer = i+1;
  }
  return mgini;
}

pQueue NewQueueNode(int natr)
{
  pQueue temp;
  int i;

  temp = (pQueue)malloc(sizeof(Queue));
  temp->CatrPointer = (int *)calloc(natr,sizeof(int));
  temp->CatrSize = (int *)calloc(natr,sizeof(int));
  temp->CatrCabove = (int **)calloc(natr,sizeof(int *));
  for(i=0;i<natr;i++)
    temp->CatrCabove[i] = (int *)calloc(nclass,sizeof(int));

  return temp;
}

void ParClassify(int nthreads,atomic_block* first_as_time, atomic_block* second_as_time, atomic_block* third_as_time, atomic_block* fourth_as_time,all_as_time *atomic_blocks_time, uint64_t* parallel_sec_time)
{

  int *CollectedCabove, *CollectedCbelow; 
  VR *TempArray1,*TempArray2; 
  pQueue BFSqLast,BFSqFirst,LastNodeAtThisLevel,qptr,qptr1;
  pTNode Parent;
  pQueue KidL,KidR;
  int GlobalNoOfClasses;
  int SplitAid,SplitPointer;
  float SplitValue,v, SplitPointValue;
  int AtrBegin,AtrSize, SplittingValAtrBegin;
  int LeftKidIndex,RightKidIndex;
  int CurrentLevel;
  int LeftPointer,RightPointer;
  int ic,i,j,k,l, chunk;
  int NodesAtThisLevel,NodesAtNextLevel;
  double MinGiniIndex,GiniIndex;
  GININFO *GiniInfo, **local_GiniInfo;
  int PerNodeHistSize,HistSize,HistBeginNode,HistBeginAtr;
  int *Hash_Table;

  /*Gokcen*/
  uint64_t parallel_sec_start_uint, parallel_sec_end_uint;
  /*Gokcen*/

/* Our Approach */
/* create first node and add it to the queue */
  TreeRoot = (pTNode)malloc(sizeof(TreeNode));

  BFSqFirst = NewQueueNode(natr);
  BFSqFirst->node = TreeRoot;
  BFSqFirst->next = NULL;
  BFSqFirst->id = 0;
  for(ic=0;ic<natr;ic++) {
    BFSqFirst->CatrPointer[ic]=0;
    BFSqFirst->CatrSize[ic]=nrec;
    for(i=0;i<nrec;i++) {
      j = Ccid(catr,ic,i);
      BFSqFirst->CatrCabove[ic][j]++;
    }
  }
 
  BFSqFirst->node->index = 0;
 
  BFSqLast = BFSqFirst;

  LastNodeAtThisLevel = BFSqLast;
  CurrentLevel = 0;
  
  NodesAtNextLevel=1;
  
  PerNodeHistSize = natr*nclass;
  
  LeftKidIndex = 0;
  RightKidIndex = 1;
  Hash_Table = (int *) calloc (nrec, sizeof(int));
  for (k=0; k<nrec; k++) Hash_Table[k]=-1;

  parallel_sec_start_uint = rdtsc();
  int share_value = 0;
  omp_set_num_threads(nthreads);
#pragma omp parallel private(chunk,TempArray1, TempArray2, Parent, KidL, KidR, GlobalNoOfClasses, SplitAid, SplitPointer, SplitValue, v, SplitPointValue, AtrBegin, AtrSize, SplittingValAtrBegin, LeftPointer, RightPointer, ic, i, j, k, l, MinGiniIndex, GiniIndex, HistBeginNode, HistBeginAtr, qptr)
{
  int pid = omp_get_thread_num();
  int lb, ub;
  enter_exit_times  enter_exit_times_outside;
  enter_exit_times  enter_exit_times_inside;

  while(BFSqFirst) 
  {
    if (pid==0) 
    {
    /* Process current level */
      NodesAtThisLevel = NodesAtNextLevel;
      NodesAtNextLevel = 0;

      HistSize = PerNodeHistSize*NodesAtThisLevel;
      CollectedCabove = (int *)calloc(HistSize,sizeof(int));
      if(!CollectedCabove) {
        printf(" Memory Crunch @ CollectedCabove (%d)! \n",HistSize);
      }
      CollectedCbelow = (int *)calloc(HistSize,sizeof(int));
      if(!CollectedCbelow) {
        printf(" Memory Crunch @ CollectedCbelow (%d)! \n",HistSize);
      }

      GiniInfo  = (GININFO *)calloc(NodesAtThisLevel,sizeof(GININFO));
      if(!GiniInfo) {
        printf(" Memory Crunch @ GiniInfo (%d)! \n", NodesAtThisLevel*sizeof(GININFO));
      }
 
      local_GiniInfo = (GININFO **)calloc(NodesAtThisLevel,sizeof(GININFO *));
      for (i=0; i<NodesAtThisLevel; i++) {
        local_GiniInfo[i] = (GININFO *)calloc(nthreads,sizeof(GININFO));
        for (j=0; j<nthreads; j++)
          local_GiniInfo[i][j].gini=2;
      }
    }

    #pragma omp barrier
  /* Gini Compute Phase.*/
  
  /* collect the Cbelows of all the attributes for all the nodes */
    qptr = BFSqFirst;
    HistBeginNode = 0;

    while(qptr!=LastNodeAtThisLevel->next) 
    {
      Parent = qptr->node;

      lb=0;
      while (lb<natr) 
      {
        enter_exit_times_outside.enter_ab_time = rdtsc();
        TRANSACTION_BEGIN
                enter_exit_times_inside.enter_ab_time = rdtsc();
		increase_number(&(first_as_time[pid].transaction));

		lb = Parent->index;
		chunk = (natr + nthreads -1)/nthreads; 
		Parent->index +=chunk;
		ub=min(lb+chunk, natr);
		enter_exit_times_inside.exit_ab_time = rdtsc();
        TRANSACTION_END

        enter_exit_times_outside.exit_ab_time = rdtsc();
        increase_number(&(first_as_time[pid].commit));
        /*Gokcen*/
        atomic_block_times(pid, first_as_time,enter_exit_times_inside,enter_exit_times_outside);

//  
        rearrange_AS_totaltime(pid, atomic_blocks_time,
        enter_exit_times_outside.enter_ab_time, 
        enter_exit_times_outside.exit_ab_time, 
        enter_exit_times_inside.enter_ab_time, 
        enter_exit_times_inside.exit_ab_time);
//        

        /*Gokcen*/
  /*calculate global class distribution at this node */
        for(ic=lb;ic<ub;ic++) 
        {
          HistBeginAtr = HistBeginNode+ic*nclass;
          for(i=0;i<nclass;i++) {
            CollectedCabove[HistBeginAtr+i] = 
	      	qptr->CatrCabove[ic][i];
            CollectedCbelow[HistBeginAtr+i] = 0;
          }
        }

#ifdef Debug
        for(ic=0;ic<natr;ic++) 
          for(i=0;i<nclass;i++) 
             printf("Cabove[%d][%d] %d\n, ", ic, i,qptr->CatrCabove[ic][i]);
#endif

      /* check for the termination criterion using atr 0's histogram.
       simultaneously calculate Cabove for atr 0 */

        if (lb < natr) {
          GlobalNoOfClasses = 0;
          for(i=0;i<nclass;i++) {
            if(CollectedCabove[HistBeginNode+lb*nclass+i]) GlobalNoOfClasses++;
          }

          enter_exit_times_outside.enter_ab_time = rdtsc();

          TRANSACTION_BEGIN
		enter_exit_times_inside.enter_ab_time = rdtsc();
                increase_number(&(second_as_time[pid].transaction));

		Parent->MoreThanOneClasses = 0;
		enter_exit_times_inside.exit_ab_time = rdtsc();
          TRANSACTION_END

          enter_exit_times_outside.exit_ab_time = rdtsc();

          increase_number(&(second_as_time[pid].commit));
          atomic_block_times(pid, second_as_time,enter_exit_times_inside,enter_exit_times_outside);
        
          rearrange_AS_totaltime(pid, atomic_blocks_time,
          enter_exit_times_outside.enter_ab_time, 
          enter_exit_times_outside.exit_ab_time, 
          enter_exit_times_inside.enter_ab_time, 
          enter_exit_times_inside.exit_ab_time);

//           unset(pid,enter_exit_times_inside,enter_exit_times_outside);

          if(GlobalNoOfClasses > 1) 
          { /* split is needed at this node */
            enter_exit_times_outside.enter_ab_time = rdtsc();

            TRANSACTION_BEGIN
	      enter_exit_times_inside.enter_ab_time = rdtsc();
              increase_number(&(third_as_time[pid].transaction));
              Parent->MoreThanOneClasses = 1;
              enter_exit_times_inside.exit_ab_time = rdtsc();
            TRANSACTION_END

            enter_exit_times_outside.exit_ab_time = rdtsc();

            increase_number(&(third_as_time[pid].commit));
            atomic_block_times(pid, third_as_time,enter_exit_times_inside,enter_exit_times_outside);
        
            rearrange_AS_totaltime(pid, atomic_blocks_time,
            enter_exit_times_outside.enter_ab_time, 
            enter_exit_times_outside.exit_ab_time, 
            enter_exit_times_inside.enter_ab_time, 
            enter_exit_times_inside.exit_ab_time);

      /* Find "best" split. */
        /* attribute 0 */
            AtrBegin = qptr->CatrPointer[lb];

            MinGiniIndex = Calculate_Gini(pid, &(catr[lb].valsrids[AtrBegin]),
                        qptr->CatrSize[lb],
                        &(CollectedCabove[HistBeginNode+lb*nclass]),
                        &(CollectedCbelow[HistBeginNode+lb*nclass]),
                        &SplitPointer,&SplitValue);
            
            SplitAid = lb;
            for(ic=lb+1;ic<ub;ic++) {
              HistBeginAtr = HistBeginNode+ic*nclass;
              AtrBegin = qptr->CatrPointer[ic];
              GiniIndex = Calculate_Gini(pid, &(catr[ic].valsrids[AtrBegin]),
                                qptr->CatrSize[ic],
                                &(CollectedCabove[HistBeginAtr]),
                                &(CollectedCbelow[HistBeginAtr]),
                                &k,&v);
              if(GiniIndex < MinGiniIndex) {
                MinGiniIndex = GiniIndex;
                SplitPointer = k;
                SplitAid = ic;
                SplitValue = v;
              }
            }
            if (MinGiniIndex < local_GiniInfo[qptr->id][pid].gini) {
              local_GiniInfo[qptr->id][pid].gini = MinGiniIndex;
              local_GiniInfo[qptr->id][pid].sval = SplitValue;
              local_GiniInfo[qptr->id][pid].said = SplitAid;
              local_GiniInfo[qptr->id][pid].sptr = SplitPointer;
            }
          }
        }
      }
      HistBeginNode += PerNodeHistSize;
      qptr = qptr->next;
    }
    #pragma omp barrier
    
    if (pid==0) 
    { 

      int id=0;
      qptr = BFSqFirst;

      while(qptr!=LastNodeAtThisLevel->next) {
        Parent = qptr->node;
        if(Parent->MoreThanOneClasses) {
          MinGiniIndex=local_GiniInfo[qptr->id][0].gini;
          SplitPointer = local_GiniInfo[qptr->id][0].sptr;
          SplitAid = local_GiniInfo[qptr->id][0].said;
          SplitValue = local_GiniInfo[qptr->id][0].sval;
          for (i=1; i<nthreads; i++) {
            if (local_GiniInfo[qptr->id][i].gini < MinGiniIndex) {
              MinGiniIndex = local_GiniInfo[qptr->id][i].gini;
              SplitPointer = local_GiniInfo[qptr->id][i].sptr;
              SplitAid = local_GiniInfo[qptr->id][i].said;
              SplitValue = local_GiniInfo[qptr->id][i].sval; 
            }
          }
          GiniInfo[qptr->id].gini = MinGiniIndex;
          GiniInfo[qptr->id].sval = SplitValue;
          GiniInfo[qptr->id].said = SplitAid;
          GiniInfo[qptr->id].sptr = SplitPointer;

          Parent->Aid = SplitAid;
          Parent->SplitVal=SplitValue;
          AtrBegin = qptr->CatrPointer[SplitAid];
          AtrSize  = qptr->CatrSize[SplitAid];
#ifdef Debug
printf("Aid=%d, gini =%f, Pointer = %d, AtrBegin = %d, AtrSize = %d\n", SplitAid, GiniInfo[qptr->id].gini,SplitPointer, AtrBegin, AtrSize);
#endif
      /* SplitPointer is effectively the size of left kid 
          For the processor on which the Split lies, its the
	 same as was obtained while its local gini computation.
      */

      /* create left and right child nodes */
          Parent->left  = (pTNode)malloc(sizeof(TreeNode));
          Parent->right = (pTNode)malloc(sizeof(TreeNode));
        
          KidL = NewQueueNode(natr);
          KidR = NewQueueNode(natr);

          KidL->node = Parent->left;
          KidL->node->index = 0;
          KidL->id = id++;
          KidR->node = Parent->right;
          KidR->node->index = 0;
          KidR->id = id++;
          qptr->left = KidL;
          qptr->right = KidR;
          qptr->node->index = 0;
      
      /* add left and right nodes to the queue */
          BFSqLast->next = KidL;
          BFSqLast = BFSqLast->next;
          BFSqLast->next = KidR;
          BFSqLast = BFSqLast->next;
          BFSqLast->next = NULL;

      /* partition the splitting attribute's list into those for left and 
	 right children */
      /* For continuous splitting attribute this involves just updating the
         pointers for the attribute in left and right children */

          KidL->CatrPointer[SplitAid] = AtrBegin;
          KidR->CatrPointer[SplitAid] = AtrBegin+SplitPointer;
          KidL->CatrSize[SplitAid] = SplitPointer;
          KidR->CatrSize[SplitAid] = AtrSize-SplitPointer;
          NodesAtNextLevel+=2;
        
          l = KidL->CatrPointer[SplitAid]+KidL->CatrSize[SplitAid];
          for(i=KidL->CatrPointer[SplitAid];i<l;i++) {
            KidL->CatrCabove[SplitAid][Ccid(catr,SplitAid,i)]++;
            Hash_Table[Crid(catr, SplitAid, i)] = LeftKidIndex;
          } 

          l = KidR->CatrPointer[SplitAid]+KidR->CatrSize[SplitAid];
          for(i=KidR->CatrPointer[SplitAid];i<l;i++) {
            KidR->CatrCabove[SplitAid][Ccid(catr,SplitAid,i)]++;
            Hash_Table[Crid(catr, SplitAid, i)] = RightKidIndex;
          }
        } /* End of Splitting the Parent Node */
        qptr = qptr->next;
      }

    }/*end of pid==0*/  
    
    #pragma omp barrier
    qptr = BFSqFirst;

    while(qptr!=LastNodeAtThisLevel->next) {
      Parent = qptr->node;
      if(Parent->MoreThanOneClasses) {
        SplitAid = Parent->Aid;
        KidL = qptr->left;
        KidR = qptr->right;

SplittingValAtrBegin = qptr->CatrPointer[SplitAid];
        /*process non-splitting Attributes. */

        lb=0;
        while (lb<natr) 
        {
          enter_exit_times_outside.enter_ab_time = rdtsc();

          TRANSACTION_BEGIN
            enter_exit_times_inside.enter_ab_time = rdtsc();
            increase_number(&(fourth_as_time[pid].transaction));

            lb = Parent->index;
            chunk = (natr + nthreads -1)/nthreads;
            Parent->index +=chunk;
            ub=min(lb+chunk, natr);
            enter_exit_times_inside.exit_ab_time = rdtsc();
          TRANSACTION_END

          enter_exit_times_outside.exit_ab_time = rdtsc();
          increase_number(&(fourth_as_time[pid].commit));

          atomic_block_times(pid, fourth_as_time,enter_exit_times_inside,enter_exit_times_outside);
        
          rearrange_AS_totaltime(pid, atomic_blocks_time,
          enter_exit_times_outside.enter_ab_time, 
          enter_exit_times_outside.exit_ab_time, 
          enter_exit_times_inside.enter_ab_time, 
          enter_exit_times_inside.exit_ab_time);

          if (lb < natr) { 
            for (ic=lb; ic<ub; ic++) {
              if (ic!=SplitAid) {
                AtrBegin = qptr->CatrPointer[ic];
                AtrSize  = qptr->CatrSize[ic];
                LeftPointer = 0;
                RightPointer = 0;
        
                TempArray1 = (VR*) calloc(AtrSize, sizeof(VR));
                TempArray2 = (VR*) calloc(AtrSize, sizeof(VR)); 

                for(i=AtrBegin;i<AtrBegin+AtrSize;i++) {
                  if (Hash_Table[Crid(catr, ic, i)] == LeftKidIndex)
                    TempArray1[LeftPointer++] = catr[ic].valsrids[i];
                  else
                    if (Hash_Table[Crid(catr, ic, i)] == RightKidIndex)
                      TempArray2[RightPointer++] = catr[ic].valsrids[i];
                }
#ifdef Debug 
printf("ic=%d, AtrBegin=%d, AtrSize=%d, LeftPointer=%d, RightPointer=%d\n", ic, AtrBegin, AtrSize, LeftPointer, RightPointer);
#endif
                i=AtrBegin;
                for(j=0;j<LeftPointer;j++,i++) {
                  catr[ic].valsrids[i] = TempArray1[j];
#ifdef Debug
printf("ic =%d, %d, %d, %d\n", ic,TempArray1[j].rid, TempArray1[j].val, TempArray1[j].cid);
#endif 
                  KidL->CatrCabove[ic][Ccid(catr,ic,i)]++;
                }
                for(j=0;j<RightPointer;j++,i++) {
                  catr[ic].valsrids[i] = TempArray2[j];
                  KidR->CatrCabove[ic][Ccid(catr,ic,i)]++;
                }

                KidL->CatrPointer[ic] = AtrBegin;
                KidR->CatrPointer[ic] = AtrBegin+LeftPointer;
                KidL->CatrSize[ic] = LeftPointer;
                KidR->CatrSize[ic] = RightPointer;
                free(TempArray1);
                free(TempArray2);
              } 
            }
          }

#ifdef Debug
printf("pid =%d, ****Level = %d, SplitAid=%d, %d, %d, LeftSize=%d, RighSize=%d\n", pid, CurrentLevel, SplitAid, SplittingValAtrBegin+LeftPointer-1, SplittingValAtrBegin + LeftPointer, LeftPointer, RightPointer);
#endif
        }
      }
      qptr = qptr->next;
    } /*End of the CurrentLevel processing for Splitting Attributes*/
    
    /*delete all the nodes at CurrentLevel from the queue. 
      Adjust BFSqFirst and LastNodeAtThisLevel. */ 
    #pragma omp barrier
    if (pid==0) {
      qptr = BFSqFirst;
      qptr1 = BFSqFirst->next;
      while(qptr1 != LastNodeAtThisLevel->next) {
        for(i=0;i<natr;i++)
          free(qptr->CatrCabove[i]);
        free(qptr->CatrCabove);
        free(qptr->CatrSize);
        free(qptr->CatrPointer);
        free(qptr);
        qptr = qptr1;
        qptr1 = qptr->next;
      }

      BFSqFirst = LastNodeAtThisLevel->next;
      LastNodeAtThisLevel=BFSqLast;
      CurrentLevel++;

      for (i=0; i<nrec; i++) Hash_Table[i]=-1; 
      free(CollectedCabove);
      free(CollectedCbelow);
      free(GiniInfo);
      for (i=0; i<NodesAtThisLevel; i++) 
        free(local_GiniInfo[i]);
      free(local_GiniInfo);

#ifdef Debug
      printf("Number of Nodes at Next level = %d\n",NodesAtNextLevel);
#endif
    }/*end of pid==0*/
    #pragma omp barrier
  }
}/*end of parallelization*/
  
  parallel_sec_end_uint = rdtsc();

  *parallel_sec_time = parallel_sec_end_uint-parallel_sec_start_uint;
  printf("records out of %d recs\n",nrec);
  printf("******************************************************\n");
  printf("Number of Levels     = %d\n",CurrentLevel);
  printf("******************************************************\n");
}

