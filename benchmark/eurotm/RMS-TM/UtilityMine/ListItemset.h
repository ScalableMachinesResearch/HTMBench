#ifndef LISTITEMSET_H
#define LISTITEMSET_H

#include "Itemset.h"

class ListElement {
public:
   TM_CALLABLE
   ListElement(Itemset *itemPtr);	// initialize a list element

   TM_CALLABLE
   ~ListElement();

   TM_CALLABLE
   void * operator new(size_t size);

   TM_CALLABLE
   void operator delete(void *);

   TM_CALLABLE
   ListElement * next();
//    inline ListElement *next()
//    {
//       return Next;
//    }

   TM_CALLABLE
   void set_next(ListElement*);

//    inline void set_next(ListElement *n)
//    {
//       Next = n;
//    }
   
   TM_CALLABLE
   Itemset * item();
//    inline Itemset *item()
//    {
//       return Item;
//    }
   inline void set_item(Itemset *it)
   {
      Item = it;
   }
   
private:
   ListElement *Next;		// next element on list, 
				// NULL if this is the last
   Itemset *Item; 	    	// pointer to item on the list
};

class ListItemset {
public:
   TM_CALLABLE
   ListItemset();			// initialize the list

   TM_CALLABLE
   ~ListItemset();			// de-allocate the list

   TM_CALLABLE
   void * operator new(size_t size);

   TM_CALLABLE
   void operator delete(void *);

   TM_CALLABLE
   void append(Itemset &item); 	// Put item at the end of the list
  
   TM_CALLABLE
   Itemset *remove(); 	 	// Take item off the front of the list

   ListElement *node(int);
   void sortedInsert(Itemset *);// Put item into list
   ListElement * sortedInsert(Itemset *, ListElement *);
   void clearlist();
   friend ostream& operator << (ostream&, ListItemset&);

//    inline ListElement *first()
//    {
//       return First;
//    }

   TM_CALLABLE
   ListElement *first();

   TM_CALLABLE
   int numitems();

   inline ListElement *last()
   {
      return Last;
   }

//    inline int numitems()
//    {
//       return numitem;
//    }
   
private:
   ListElement *First;  	// Head of the list, NULL if list is empty
   ListElement *Last;		// Last element of list
   int numitem;
};

#endif // LISTITEMSET_H





