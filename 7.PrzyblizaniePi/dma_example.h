/* --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                            */
/* 5724-S84                                                        */
/* (C) Copyright IBM Corp. 2008       All Rights Reserved          */
/* US Government Users Restricted Rights - Use, duplication or     */
/* disclosure restricted by GSA ADP Schedule Contract with         */
/* IBM Corp.                                                       */
/* --------------------------------------------------------------- */
/* PROLOG END TAG zYx                                              */
#ifndef _dma_example_h_
#define _dma_example_h_

#define MAX_NUM_SPE_THREADS     4                                         //Number of SPEs to be used

/* Here we define a control block data structure that contains 
 * all the data the SPE needed to get the large array of data into
 * local store.
 *
 * This data structure has a size that's a multiple of 16 bytes */
typedef struct _control_block
{
  double pi;
  int Nstart;
  int Nend;
  int N;
  char dop[12];
} control_block_t;

#endif /* _dma_example_h_ */
