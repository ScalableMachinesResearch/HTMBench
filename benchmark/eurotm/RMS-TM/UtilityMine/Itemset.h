#ifndef __ITEMSET_H
#define __ITEMSET_H
#include <malloc.h>
#include <iostream>

#include "pardhp.h"


#define DEFAULT -1

using namespace std;


class Itemset {
public:
   TM_CALLABLE
   Itemset (int = 1);

   TM_CALLABLE
   ~Itemset();
   
   TM_CALLABLE
   void * operator new(size_t size);

   TM_CALLABLE
   void operator delete(void *);

   TM_CALLABLE  
   void *operator new[](size_t);

   TM_CALLABLE
   void operator delete[](void *);

   TM_CALLABLE
   void clear();
   void copy(Itemset *);
   int subsequence(Itemset&, int *);
   int subsequence(Itemset&);
   int subsequence(char *, int);
   int compare(Itemset&);
   int compare(Itemset&, int);

   friend ostream& operator << (ostream&, Itemset&);

   TM_CALLABLE
   int item(int);
 
   TM_CALLABLE
   int numitems();

   inline void add_item(int pos, int val)
   {
      theItemset[pos] = val;
   }
   
   inline void set_numitems(int val)
   {
      theNumel = val;
   }
   
   inline int maxsize(){
      return theSize;
   }
   inline int sup(){
      return support;
   }
   inline float get_t_utility(){
      return t_utility;
   }
   inline void incr_sup()
   {
      support++;
   }

   TM_CALLABLE
   void incr_t_utility(float);

#ifdef OPTIMAL
   inline float get_utility(){
      return utility;
   }
            
   TM_CALLABLE
   void incr_utility(float);
 
#endif               
   inline void set_sup(int val)
   {
      support = val;
   }

   inline void set_t_utility(float val)
   {
      t_utility = val;
   }

   TM_CALLABLE
   int tid();
 
   inline void set_tid(int val)
   {
      Tid = val;
   }
   
private:
   int *theItemset;
   unsigned int theNumel;
   unsigned int theSize;
   int support;
   float t_utility;
   float utility;
   int Tid;
};

#endif //__ITEMSET_H

