#include <stddef.h>
#include "HashTree.h"
#include "pardhp.h"
#include <malloc.h>

TM_CALLABLE
HashTree::HashTree (int Depth_P, int hash, int thresh)
{
   Leaf = YES; 
   Count = 0;
   Hash_function = hash;
   Depth = Depth_P;
   List_of_itemsets = NULL;
   Hash_table = (HashTree**) calloc(Hash_function, sizeof(HashTree_Ptr));
   Threshold = thresh;
}

TM_CALLABLE
HashTree::~HashTree()
{
   clear();
}


TM_CALLABLE
void *HashTree::operator new(size_t size) 
{
   HashTree * hashtree = (HashTree*)malloc(size);
   return (void *)hashtree;
}

TM_CALLABLE
void HashTree::operator delete(void *ptr_hashtree) 
{
    HashTree * ptr = (HashTree*) ptr_hashtree;
    free(ptr);
}

TM_CALLABLE
void *HashTree::operator new[] (size_t size) 
{
   HashTree * hashtree = (HashTree*)malloc(size);
   return (void *)hashtree;
}

TM_CALLABLE
void HashTree::operator delete[] (void *ptr_hashtree) 
{
    HashTree * ptr = (HashTree*) ptr_hashtree;
    free(ptr);
}

TM_CALLABLE
int HashTree::is_root()
{
     return (Depth == 0);
};

TM_CALLABLE
void HashTree::increase_nested_depth(int pid, int * nestedLock_depth,int * max_nested_depth)
{
	nestedLock_depth[pid]++;
        max_nested_depth[pid] = max(nestedLock_depth[pid], max_nested_depth[pid]);
}

TM_CALLABLE
void HashTree::decrease_nested_depth(int pid, int * nestedLock_depth)
{
	nestedLock_depth[pid]--;
}

ostream& operator << (ostream& outputStream, HashTree& hashtree){
      if (hashtree.Depth == 0)
         outputStream << " ROOT : C:" << hashtree.Count
                      << " H:" << hashtree.Hash_function << "\n";
      if (hashtree.Leaf){
         if (hashtree.List_of_itemsets != NULL){
            outputStream << " T:" << hashtree.Threshold
                         << " D:" << hashtree.Depth << "\n";
            outputStream << *(hashtree.List_of_itemsets) << flush;
         }
      }
      else{
         for(int i=0; i < hashtree.Hash_function; i++){
            if (hashtree.Hash_table[i]){
#if defined CCPD
               outputStream << "child = " << i << "\n";
#else
               outputStream << "child = " << i
                            << ", Count = " << hashtree.Count << "\n";
#endif
               outputStream << *hashtree.Hash_table[i] << flush;
            }
         }
      }
   
   return outputStream;
}

TM_CALLABLE
void HashTree::clear(){
   if (Leaf){
      if (List_of_itemsets)
         delete List_of_itemsets;
      List_of_itemsets = NULL;
   }
   else{
      if (Hash_table){
         for(int i=0; i < Hash_function; i++){
            if (Hash_table[i]) delete Hash_table[i];
         }
         free(Hash_table);
         Hash_table = NULL;
      }  
   }
   Leaf = YES; 
   Count = 0;
   Hash_function = 0;
   Threshold = 0;
   Depth = 0;
}

TM_CALLABLE
int HashTree::hash(int Value)
{
   if(Value != 0)
      return (Value%Hash_function);
   else
      return 0;
}

TM_CALLABLE
void HashTree::rehash(int pid, all_atomic_time * atomic_secs_time, int* nested_depth, int * max_nested_depth)
{
   Leaf = NO;
   Hash_table = (HashTree**) calloc(Hash_function, sizeof(HashTree_Ptr));
   for (int i=0; i < Hash_function; i++)
      Hash_table[i] = NULL;
   
   while(!(List_of_itemsets->first() == NULL)) { // iterate over current itemsets
      Itemset *temp = List_of_itemsets->remove();
#ifdef BALT
      int val = hash_indx[temp->item(Depth)];//according to current Depth
#else
      int val = hash(temp->item(Depth)); // according to current Depth
#endif
      if (Hash_table[val] == NULL){
         Hash_table[val] = new HashTree(Depth+1, Hash_function, Threshold);
      }
      Hash_table[val]->add_element(*temp,pid,atomic_secs_time,nested_depth, max_nested_depth);
   }

   delete List_of_itemsets;
   List_of_itemsets = NULL;
}


TM_CALLABLE
int HashTree::add_element(Itemset& Value, int pid, all_atomic_time * atomic_secs_time,int* nested_depth, int * max_nested_depth)
{
   /*BSC*/
     uint64_t ab_enter_time_outside;
     uint64_t ab_exit_time_outside;
     uint64_t ab_enter_time_inside;
     uint64_t ab_exit_time_inside;
    /*BSC*/
   
   if (Leaf)
   {
	#if defined CCPD
                ab_enter_time_outside = rdtsc();
         
        	TRANSACTION_BEGIN
		  ab_enter_time_inside = rdtsc();
		  increase_nested_depth(pid,nested_depth,max_nested_depth);

		  if (List_of_itemsets == NULL)
         		List_of_itemsets = new ListItemset;
      
		  List_of_itemsets->append(Value);
		  if(List_of_itemsets->numitems() > Threshold)
		  {
         		if (Depth+1 > Value.numitems())
            			Threshold++;
         		else rehash(pid,atomic_secs_time,nested_depth,max_nested_depth);     // if so rehash
		  }	

		  ab_exit_time_inside = rdtsc();
		TRANSACTION_END
	
                ab_exit_time_outside = rdtsc();
		decrease_nested_depth(pid, nested_depth);

		if (nested_depth[pid] == 0)
      		{
        		rearrange_atomic_total_time(pid, atomic_secs_time, ab_enter_time_inside, ab_exit_time_inside,  ab_enter_time_outside, ab_exit_time_outside);
      		}
	#else
		 if (List_of_itemsets == NULL)
         		List_of_itemsets = new ListItemset;
      
      		List_of_itemsets->append(Value);
      		if(List_of_itemsets->numitems() > Threshold)
		{
         		if (Depth+1 > Value.numitems())
            			Threshold++;
         		else rehash(pid,atomic_secs_time,nested_depth,max_nested_depth);     // if so rehash
      		}
	#endif
   }
   else
   {
	#ifdef BALT
		int val = hash_indx[Value.item(Depth)];
	#else
		int val = hash(Value.theItemset[Depth]);
	#endif

	if (Hash_table[val] == NULL)
 	{
		#if defined CCPD
			ab_enter_time_outside = rdtsc();
			TRANSACTION_BEGIN
			   ab_enter_time_inside = rdtsc();	
		#endif
         	if (Hash_table[val] == NULL)
            		Hash_table[val] = new HashTree(Depth+1, Hash_function, Threshold);
		
                #if defined CCPD
			  ab_exit_time_inside = rdtsc();
			TRANSACTION_END	
                      	ab_exit_time_outside = rdtsc();
			
                        rearrange_atomic_total_time(pid, atomic_secs_time, ab_enter_time_inside, ab_exit_time_inside,  ab_enter_time_outside, ab_exit_time_outside);
		#endif
        }
      	Hash_table[val]->add_element(Value,pid, atomic_secs_time,nested_depth, max_nested_depth);
   }
   
   #if defined CCPD
   	if (is_root())
	{
      		int tt;
      		
   		ab_enter_time_outside = rdtsc();
        	TRANSACTION_BEGIN
		  ab_enter_time_inside = rdtsc();	

      			Count++;
      			tt = Count;
		
		  ab_exit_time_inside = rdtsc();
		TRANSACTION_END
                ab_exit_time_outside = rdtsc();

		rearrange_atomic_total_time(pid, atomic_secs_time, ab_enter_time_inside, ab_exit_time_inside,  ab_enter_time_outside, ab_exit_time_outside);
      		return tt;
   	}
   #else
   	Count++;
   	return Count;
   #endif
}	
