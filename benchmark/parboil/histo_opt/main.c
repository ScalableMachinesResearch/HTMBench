/***************************************************************************
 *
 *            (C) Copyright 2010 The Board of Trustees of the
 *                        University of Illinois
 *                         All Rights Reserved
 *
 ***************************************************************************/

#include "parboil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "tsx.h"

#define UINT8_MAX 255

/******************************************************************************
* Implementation: Reference
* Details:
* This implementations is a scalar, minimally optimized version. The only
* optimization, which reduces the number of pointer chasing operations is the
* use of a temporary pointer for each row.
******************************************************************************/

int cmpfunc (const void * a, const void * b) {
   return ( *(unsigned int*)a - *(unsigned int*)b );
}


int main(int argc, char* argv[]) {
  struct pb_TimerSet timers;
  struct pb_Parameters *parameters;

  printf("Base implementation of histogramming.\n");
  printf("Maintained by Nady Obeid <obeid1@ece.uiuc.edu>\n");

  parameters = pb_ReadParameters(&argc, argv);
  if (!parameters)
    return -1;

  if(!parameters->inpFiles[0]){
    fputs("Input file expected\n", stderr);
    return -1;
  }

  int numIterations;
  if (argc >= 2){
    numIterations = atoi(argv[1]);
  } else {
    fputs("Expected at least one command line argument\n", stderr);
    return -1;
  }

  TM_STARTUP(-1);
  #ifdef RTM
  tm_startup_();
  const char *env_txn_gran=getenv("TXN_GRAN");
  unsigned int txn_gran = 1;
  if (env_txn_gran){
    txn_gran = atoi(env_txn_gran);
  }
  printf("txn_gran = %u\n", txn_gran);
  #endif
  pb_InitializeTimerSet(&timers);

  char *inputStr = "Input";
  char *outputStr = "Output";

  pb_AddSubTimer(&timers, inputStr, pb_TimerID_IO);
  pb_AddSubTimer(&timers, outputStr, pb_TimerID_IO);

  pb_SwitchToSubTimer(&timers, inputStr, pb_TimerID_IO);

  unsigned int img_width, img_height;
  unsigned int histo_width, histo_height;

  FILE* f = fopen(parameters->inpFiles[0],"rb");
  int result = 0;

  result += fread(&img_width,    sizeof(unsigned int), 1, f);
  result += fread(&img_height,   sizeof(unsigned int), 1, f);
  result += fread(&histo_width,  sizeof(unsigned int), 1, f);
  result += fread(&histo_height, sizeof(unsigned int), 1, f);

  if (result != 4){
    fputs("Error reading input and output dimensions from file\n", stderr);
    return -1;
  }

  unsigned int* img = (unsigned int*) malloc (img_width*img_height*sizeof(unsigned int));

  unsigned char* histo = (unsigned char*) calloc (histo_width*histo_height, sizeof(unsigned char));

  pb_SwitchToSubTimer(&timers, "Input", pb_TimerID_IO);

  result = fread(img, sizeof(unsigned int), img_width*img_height, f);


  fclose(f);

  if (result != img_width*img_height){
    fputs("Error reading input array from file\n", stderr);
    return -1;
  }

  pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);
  qsort(img, img_width*img_height, sizeof(unsigned int), cmpfunc);

  printf("img_width * img_height = %u\n", img_width * img_height);
  printf("histo_height * histo_width = %u\n", histo_height * histo_width);
  int iter;
  for (iter = 0; iter < numIterations; iter++){
    memset(histo,0,histo_height*histo_width*sizeof(unsigned char));
    unsigned int i,j;

#ifdef RTM
#ifdef TXN
#pragma omp parallel private(j)
{
#pragma omp for
    for (i = 0; i < img_width*img_height/txn_gran+1 ; ++i) {
	  TM_BEGIN();
	  for(j=0; j<txn_gran && (i*txn_gran+j)<img_width*img_height; j++){
		  const unsigned int value = img[i*txn_gran+j];
		  if (histo[value] < UINT8_MAX) {
			++histo[value];
		  }
	  }
	  TM_END();
    }
    tm_thread_exit_();
}
#else
#pragma omp parallel
{
#pragma omp for
    for (i = 0; i < img_width*img_height; ++i) {
      const unsigned int value = img[i];
      TM_BEGIN();
      if (histo[value] < UINT8_MAX) {
        ++histo[value];
      }
      TM_END();
    }
    tm_thread_exit_();
}
#endif /*TXN*/
#elif ATOMIC
#pragma omp parallel for
    for (i = 0; i < img_width*img_height; ++i) {
      const unsigned int value = img[i];
      if (histo[value] < UINT8_MAX) {
        __sync_fetch_and_add(&(histo[value]),1);
      }
    }

#else

#pragma omp parallel for
    for (i = 0; i < img_width*img_height; ++i) {
      const unsigned int value = img[i];
#pragma omp critical
      if (histo[value] < UINT8_MAX) {
        ++histo[value];
      }
    }
#endif /*RTM*/
}

#ifdef RTM
  tm_shutdown_();
#endif

  pb_SwitchToTimer(&timers, pb_TimerID_IO);
  pb_SwitchToSubTimer(&timers, outputStr, pb_TimerID_IO);

  if (parameters->outFile) {
    dump_histo_img(histo, histo_height, histo_width, parameters->outFile);
  }

  pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);

  free(img);
  free(histo);

  pb_SwitchToTimer(&timers, pb_TimerID_NONE);

  printf("\n");
  pb_PrintTimerSet(&timers);
  pb_FreeParameters(parameters);

  return 0;
}
