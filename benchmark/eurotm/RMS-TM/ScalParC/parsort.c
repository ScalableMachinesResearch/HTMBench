#include <math.h>
#include "pclass.h"

#define itag	1

void FixHeap(int IndexHeap[],int ListHeap[],int HeapSize,
	VR ValsRids[],int vacant,int keyIndex,int listId) {

int child,SmallerKidRid,RightKidRid,keyRid;
float SmallerKidVal,RightKidVal,keyVal;

  keyVal = ValsRids[keyIndex].val;
  keyRid = ValsRids[keyIndex].rid;
  child = 2*vacant+1;
  while(child <= HeapSize-1) {
    SmallerKidVal = ValsRids[IndexHeap[child]].val;
    SmallerKidRid = ValsRids[IndexHeap[child]].rid;

    if(child < HeapSize-1) {
      RightKidVal = ValsRids[IndexHeap[child+1]].val;
      RightKidRid = ValsRids[IndexHeap[child+1]].rid;
      if(fabs(RightKidVal-SmallerKidVal) < EPSILON) {
        if(RightKidRid < SmallerKidRid) {
          child++;
          SmallerKidVal = RightKidVal;
          SmallerKidRid = RightKidRid;
        }
      }
      else {
        if(RightKidVal < SmallerKidVal) {
          child++;
          SmallerKidVal = RightKidVal;
          SmallerKidRid = RightKidRid;
        }
      }
    }

    if(fabs(keyVal-SmallerKidVal) < EPSILON) {
      if(keyRid > SmallerKidRid) {
        IndexHeap[vacant] = IndexHeap[child];
        ListHeap[vacant] = ListHeap[child];
        vacant = child;
        child = 2*vacant+1;
      }
      else
        break;
    }
    else {
      if(keyVal > SmallerKidVal) {
        IndexHeap[vacant] = IndexHeap[child];
        ListHeap[vacant] = ListHeap[child];
        vacant = child;
        child = 2*vacant+1;
      }
      else
        break;
    }
  }
  IndexHeap[vacant] = keyIndex;
  ListHeap[vacant] = listId;
}

int VRCompare(VR *p,VR *q) {
float v1,v2;
int r1,r2;
int retval;
v1 = (*p).val;
v2 = (*q).val;
r1 = (*p).rid;
r2 = (*q).rid;
  if(fabs(v1-v2) < EPSILON) {
    if(r1 > r2) {
      retval = 1;
    }
    else {
      retval = -1;
    }
  }
  else {
    if(v1 > v2) {
      retval = 1;
    }
    else {
      retval = -1;
    }
  }

return retval;
}

int floatCompare(float *p,float *q) {
  if(*p > *q) return(1);
  else 
    if(*p < *q) return(-1);
    else return(0);
}

int search_value(VR *A,float v,int n) {
int i,j,size,left,right;

if(!n) return 0;
if(v - A[0].val   < -EPSILON) return 0;
if(v - A[n-1].val >  EPSILON) return n;

size = n;
i = n/2;
j = i;
while(size>1) {
  left = j;
  right = size-j-1;
  if(v - A[i].val > EPSILON) {
    if(v - A[i+1].val < -EPSILON) return ++i;
    size = right;
    j = right/2;
    i += j+1;
  }
  else 
    if(v - A[i].val < -EPSILON)  {
      if(v - A[i-1].val > EPSILON) return i;
      size = left;
      j = left/2;
      i -= size-j;
    }
    else {
      return ++i;
    }
}
return ++i;
}


