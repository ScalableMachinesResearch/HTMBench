#ifndef __HASHTREE_H
#define __HASHTREE_H
#include "ListItemset.h"
#define CCPD
#define YES 1 
#define NO 0 

class HashTree {
public:

   TM_CALLABLE
   void rehash(int,all_atomic_time *, int*, int*); // procedure for converting leaf node to a interior node

   TM_CALLABLE
   int hash(int); // procedure to find out which node item hashes to

   TM_CALLABLE
   HashTree(int, int, int);

   TM_CALLABLE
   ~HashTree();
   
    void *operator new(size_t);

   TM_CALLABLE
   void operator delete(void *);

   TM_CALLABLE  
   void *operator new[](size_t);

   TM_CALLABLE
   void operator delete[](void *);

   TM_CALLABLE
   int add_element(Itemset&,int,all_atomic_time *,int*, int*);

   TM_CALLABLE
   void clear();
   
   TM_CALLABLE
   int is_root();
  
   TM_CALLABLE
   void increase_nested_depth(int, int *,int *);

   TM_CALLABLE
   void decrease_nested_depth(int, int*);

   int Count;
   friend ostream& operator << (ostream&, HashTree&);

   inline int is_leaf()
   {
      return (Leaf == YES);
   }

   inline ListItemset * list()
   {
      return List_of_itemsets;
   }

   inline int hash_function()
   {
      return Hash_function;
   }

   inline int depth()
   {
      return Depth;
   }

   inline int hash_table_exists()
   {
      return (Hash_table != NULL);
   }
   
   inline HashTree *hash_table(int pos)
   {
      return Hash_table[pos];
   }
   
private:
   int Leaf;
   HashTree **Hash_table;
   int Hash_function;
   int Depth;
   ListItemset *List_of_itemsets;
   int Threshold;
};

typedef HashTree * HashTree_Ptr;
#endif //__HASHTREE_H
