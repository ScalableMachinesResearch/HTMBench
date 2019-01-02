#include <stddef.h>
#include "ListItemset.h"
#include <stdlib.h>

TM_CALLABLE
ListElement::ListElement(Itemset *itemPtr)
{
     Item = itemPtr;
     Next = NULL;	// assume we'll put it at the end of the list 
}

TM_CALLABLE
ListElement::~ListElement()
{
   Item = NULL;
   Next = NULL;
}

TM_CALLABLE
void *ListElement::operator new(size_t size)
{
   ListElement * list_element = (ListElement*)malloc(size);
   return (void *)list_element;
}

TM_CALLABLE
void ListElement::operator delete(void * ptr_listelement)
{
  ListElement * ptr = (ListElement*) ptr_listelement;
  free(ptr);
}

TM_CALLABLE
void ListElement::set_next(ListElement *n)
{
      Next = n;
}
   
TM_CALLABLE
ListElement * ListElement::next()
{
    return Next;
}

TM_CALLABLE
Itemset * ListElement::item()
{
    return Item;
}

TM_CALLABLE
ListItemset::ListItemset()
{ 
    First = Last = NULL;
    numitem =0;
}

TM_CALLABLE
ListItemset::~ListItemset()
{
   Itemset *tmp;
   while ((tmp = remove()) != NULL)
      delete tmp;	 // delete all the list elements
   numitem = 0;
}

TM_CALLABLE
void *ListItemset::operator new(size_t size)
{
   ListItemset * list_itemset = (ListItemset*)malloc(size);
   return (void *)list_itemset;
}

TM_CALLABLE
void ListItemset::operator delete(void * ptr_itemset)
{
  ListItemset * ptr = (ListItemset*) ptr_itemset;
  free(ptr);
}

TM_CALLABLE
ListElement * ListItemset::first()
{
      return First;
}

TM_CALLABLE
int ListItemset::numitems()
{
      return numitem;
}

//      Append an "item" to the end of the list.
TM_CALLABLE
void ListItemset::append(Itemset &item)
{
    ListElement *element = new ListElement(&item);

    if (First == NULL) {		// list is empty
	First = element;
	Last = element;
    } else {			// else put it after Last
	Last->set_next(element);
	Last = element;
    }
    numitem++;
}

void ListItemset::sortedInsert(Itemset *item)
{
   //cout << item << flush;
   ListElement *element = new ListElement(item);
   ListElement *ptr;		// keep track
   
   numitem++;
   if (First == NULL) {	// if list is empty, put
      First = element;
      Last = element;
   }
   else if (item->compare(*First->item()) <= 0) {	
      // item goes on front of list
      element->set_next(First);
      First = element;
   }
   else {		// look for First elt in list bigger than item
      for (ptr = First; ptr->next() != NULL; ptr = ptr->next()) {
         if (item->compare(*ptr->next()->item())<=0) {
            element->set_next(ptr->next());
            ptr->set_next(element);
            return;
         }
      }
      Last->set_next(element);		// item goes at end of list
      Last = element;
   }
}

ListElement * ListItemset::sortedInsert(Itemset *item, ListElement *cpos)
{
   //cout << item << flush;
   ListElement *element = new ListElement(item);
   ListElement *ptr;		// keep track
   
   numitem++;
   if (cpos == NULL) {	
      First = element;
      Last = element;
      return First;
   }
   else if (cpos == First && item->compare(*First->item()) <= 0) {	
      // item goes on front of list
      element->set_next(First);
      First = element;
      return First;
   }
   else {		// look for First elt in list bigger than item
      for (ptr = cpos; ptr->next() != NULL; ptr = ptr->next()) {
         if (item->compare(*ptr->next()->item())<=0) {
            element->set_next(ptr->next());
            ptr->set_next(element);
            return ptr->next();
         }
      }
      Last->set_next(element);		// item goes at end of list
      Last = element;
      return Last;
   }
}


//----------------------------------------------------------------------
// ListItemset::Remove
//      Remove the First "item" from the front of a list.
// 
//----------------------------------------------------------------------
TM_CALLABLE
Itemset * ListItemset::remove()
{
    ListElement *element = First;
    Itemset*thing;

    if (First == NULL) 
	return NULL;

    thing = First->item();
    if (First == Last) {	// list had one item, now has none 
        First = NULL;
	Last = NULL;
    } else {
        First = element->next();
    }
    delete element;
    numitem--;
    return thing;
}


ListElement *ListItemset::node(int pos){
   ListElement *head = First;
   for (int i=0; i < pos && head; head = head->next(),i++);
   return head;
}


void  ListItemset::clearlist()
{
   while (remove() != NULL);
   numitem = 0;
   First = Last = NULL;
}

ostream& operator << (ostream& outputStream, ListItemset& list){
   int cnt = 0;
   outputStream << "LIST:\n";
   if (!(list.First == NULL)){
      ListElement *head = list.First;
      for (; head; head = head->next(), cnt++)
         outputStream << "EL " << cnt << ":" << *head->item();
      outputStream << "-- numel = " << list.numitem << "\n";
   }
   return outputStream;
}




