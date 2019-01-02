#include "pclass.h"
#include "read.h"

/* this getdata assumes that the categorical attribute values have been
   already mapped from names to discrete value numbers by a preprocessing
   routine */

void getdata(int *inp_array, Continuous *catr) {
  int ret,i,j,ic=0,rid=0;
  int cval;
  int dval;
  int arrayptr;

  arrayptr = 0;

  while (rid < nrec) {

      if(ic < natr) {
        cval = inp_array[arrayptr];
        
        Cval(catr,ic,rid) = cval;
        Crid(catr,ic,rid) = rid;
        if(cval > catr[ic].max) catr[ic].max = cval;
        if(cval < catr[ic].min) catr[ic].min = cval;
        ++ic;
        ++arrayptr;

      }
      else {

        dval = (int)inp_array[arrayptr];

        for(ic=0;ic<natr;ic++) Ccid(catr,ic,rid) = dval;
      	ic =0 ;
      	++rid;
        ++arrayptr;
      }

  }

}

