#include <pthread.h>
#include <libspe2.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define NUM_SPES 1

spe_program_handle_t * handle_spu[NUM_SPES];

typedef struct
{
  spe_context_ptr_t spe_ctx;
  pthread_t pthread;
  void * argp;
} spu_data_t;

spu_data_t data[NUM_SPES];

void *spu_pthread(void *arg)
{
  spu_data_t * data_pthread = (spu_data_t *)arg;

  uint32_t entry = SPE_DEFAULT_ENTRY;

  if(spe_context_run( data_pthread->spe_ctx , &entry, 0, data_pthread->argp ,NULL,NULL)<0)
  {
    perror ("Failed running context"); exit (1);
  }
  pthread_exit(NULL);
}

int main()
{
  uint32_t num;

  for(num = 0; num < NUM_SPES; ++num)
  {
    if((data[num].spe_ctx = spe_context_create (0, NULL))==NULL)
    {
      perror("Failed creating context "); exit(1);
    }

    if (!(handle_spu[num] = spe_image_open("./spu/spu_vector")))
    {
      perror("Fail opening image"); exit(1);
    }

    if(spe_program_load (data[num].spe_ctx, handle_spu[num]))
    {
      perror("Failed loading program"); exit(1);
    }
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

