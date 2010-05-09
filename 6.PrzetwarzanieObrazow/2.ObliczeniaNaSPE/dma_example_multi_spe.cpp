/* --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                            */
/* 5724-S84                                                        */
/* (C) Copyright IBM Corp. 2008       All Rights Reserved          */
/* US Government Users Restricted Rights - Use, duplication or     */
/* disclosure restricted by GSA ADP Schedule Contract with         */
/* IBM Corp.                                                       */
/* --------------------------------------------------------------- */
/* PROLOG END TAG zYx                                              */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <libspe2.h>
#include <pthread.h>

#include <dma_example.h>

#include "../lib/EasyBMP.h"

/* Here we define a data structure to store the data that need to be
 * passed into the pthread function that starts the spe thread */
typedef struct _ppu_thread_data 
{
  spe_context_ptr_t spe_context;        //spe context
  void* control_block_ptr;              //pointer to the control block
} ppu_thread_data_t;

/* this is the pointer to the SPE code */
extern spe_program_handle_t dma_example_spu;


/* PPE pthread function that starts the SPE thread */
void *ppu_pthread_function(void *argp) {
  ppu_thread_data_t *thread_data = (ppu_thread_data_t *)argp;
  spe_context_ptr_t ctx;
  unsigned int entry = SPE_DEFAULT_ENTRY;
  
  ctx = thread_data->spe_context;
  /* start the SPE thread. We want to pass the pointer to the control block to the SPE */
  if (spe_context_run(ctx, &entry, 0, thread_data->control_block_ptr, NULL, NULL) < 0) {
    perror ("Failed running context");
    exit (1);
  }
  pthread_exit(NULL);
}


int main()
{
  int i, num_spe_threads, num_elements_per_spe, rc;
  spe_context_ptr_t spe_contexts[MAX_NUM_SPE_THREADS];
  pthread_t spe_threads[MAX_NUM_SPE_THREADS];
  control_block_t* control_blocks[MAX_NUM_SPE_THREADS];
  ppu_thread_data_t ppu_thread_data[MAX_NUM_SPE_THREADS];
  union {
    unsigned long long ull;
    unsigned int ui[2];
  } long_addr;

  //Odczyt z pliku
  BMP img_orginal;
  img_orginal.ReadFromFile("../img/lenna.bmp");
  int height = img_orginal.TellHeight();
  int width  = img_orginal.TellWidth();

  unsigned char *in_data = (unsigned char*)img_orginal.Pixels[0];
/*
  //unsigned char *in_data = new unsigned char [ ARRAY_SIZE * sizeof(unsigned char) ];
  unsigned char *in_data;

  //create the big array of floating point numbers
  rc = posix_memalign ((void**)(&in_data), 16, ARRAY_SIZE * sizeof(unsigned char));
  //in_data = (unsigned char*)malloc(ARRAY_SIZE * sizeof(unsigned char));

  for (i = 0; i < ARRAY_SIZE; i++)
  {
    in_data[i] = 100;
  }*/

  /* Determine the number of SPE threads to create. */
  num_spe_threads = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1);
  if (num_spe_threads > MAX_NUM_SPE_THREADS) num_spe_threads = MAX_NUM_SPE_THREADS;

  /* calculate the number of elements in the array each SPE needs to process */
  num_elements_per_spe = (ARRAY_SIZE/num_spe_threads);

  for (i = 0; i < num_spe_threads; i++)
  {
    /* create control block */
    rc = posix_memalign ((void**)(&control_blocks[i]), 128, sizeof (control_block_t));
    if (rc != 0)
    {
      fprintf (stderr, "Failed allocating space for control block\n");
      exit (1);
    }
 
    /* load the control block with address of the portion of the input array for this SPE */
    long_addr.ui[0] = 0;
    long_addr.ui[1] = (unsigned int)in_data + i* num_elements_per_spe * sizeof(unsigned char);
    control_blocks[i]->in_addr = long_addr.ull;
    control_blocks[i]->num_elements_per_spe = num_elements_per_spe;
    control_blocks[i]->id = i;

    /* Create SPE threads to execute 'dma_example_spu'. */
    spe_contexts[i]= spe_context_create (0, NULL);
    if (spe_contexts[i] == NULL)
    {
      perror ("Failed creating SPE context");
      exit (1);
    }

    /* Load SPE program into context */
    if (spe_program_load (spe_contexts[i], &dma_example_spu))
    {
      perror ("Failed loading SPE program");
      exit (1);
    }

    /* load ppu_thread_data */
    ppu_thread_data[i].spe_context = spe_contexts[i];
    ppu_thread_data[i].control_block_ptr = control_blocks[i];

    /* Create SPE thread */
    if (pthread_create (&spe_threads[i], NULL, &ppu_pthread_function, &ppu_thread_data[i]))
    {
      perror ("Failed creating SPE thread");
      exit (1);
    }

    /*** SPE is executing ***/

    /* Wait for SPE thread to complete execution */
    if (pthread_join (spe_threads[i], NULL))
    {
      perror ("Failed pthread_join");
      exit (1);
    }

    /* Destroy context */
    if (spe_context_destroy (spe_contexts[i]) != 0)
    {
      perror ("Failed destroying SPE context");
      exit (1);
    }
    free (control_blocks[i]);
  }

  img_orginal.WriteToFile("../img/out2.bmp");
  
  //free (in_data);
 
  printf("\nThe program has successfully executed.\n");

  return (0);
}
