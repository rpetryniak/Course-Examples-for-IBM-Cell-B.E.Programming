#include <stdint.h>

#include <pthread.h>
#include <libspe2.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h"

#define NUM_SPES 1

spe_program_handle_t * handle_spu[NUM_SPES];

typedef struct
{
 spe_context_ptr_t spe_ctx;
 pthread_t pthread;
 void * argp;
} spu_data_t;

spu_data_t data[NUM_SPES] __attribute__ ((aligned (128))) ;
COMMON     parm[NUM_SPES] __attribute__ ((aligned (128)));

void *spu_pthread(void *arg)
{
  spu_data_t * data_pthread = (spu_data_t *)arg;
  uint32_t entry = SPE_DEFAULT_ENTRY;

  if(spe_context_run( data_pthread->spe_ctx , &entry,0,data_pthread->argp ,NULL,NULL)<0)
  {
     perror ("Failed running context"); exit (1);
  }
  pthread_exit(NULL);
}


int main()
{
 float tab[8192] __attribute__(( aligned(128) ));
 volatile int SYNC=5;

 int i;
 for(i=0; i<8192;++i)
   tab[i] = (float)i;

 parm[0].tab = (uint64_t)tab;
 parm[0].sync= (uint64_t)&SYNC;

 uint32_t num;

 for(num = 0; num < NUM_SPES; ++num)
 {

   if((data[num].spe_ctx = spe_context_create (0, NULL))==NULL)
   {
     perror("Failed creating context "); exit(1);
   }

   if (!(handle_spu[num] = spe_image_open("./spu/spu_dma")))
   {
     perror("Fail opening image"); exit(1);
   }

   if(spe_program_load (data[num].spe_ctx, handle_spu[num]))
   {
     perror("Failed loading program"); exit(1);
   }

 }

 /*-----*/
 for(num = 0 ; num < NUM_SPES ; ++num)
 {
   data[num].argp = parm +  num;
 }

 for( num=0; num<NUM_SPES; ++num)
 {
    if(pthread_create(&data[num].pthread, NULL,&spu_pthread, &data[num]))
    {
      perror("Failed creating thread"); exit(1);
    }
 }

 for( num=0; num<NUM_SPES; ++num)
 {
   while(1) {
     if (SYNC != 5) {
       printf("DZIALA!!!");
       break;
     }
   }
 }

 for (i=0;i<100;i++)
   printf(" %.0f ",tab[i]);

 for( num=0; num<NUM_SPES; ++num)
 {
   if (pthread_join (data[num].pthread, NULL))
   {
     perror("Failed joining thread"); exit (1);
   }
 }

 for(num = 0 ; num < NUM_SPES; ++num)
 {
   spe_context_destroy(data[num].spe_ctx);
 }

 return 0;
}

