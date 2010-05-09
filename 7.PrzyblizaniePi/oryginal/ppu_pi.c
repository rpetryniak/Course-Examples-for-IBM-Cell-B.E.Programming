#include <stdio.h>
#include "globals.h"
#include <libspe2.h>
#include <pthread.h>
#include <libmisc.h>

extern spe_program_handle_t spu_pi;

#define SPU_THREADS 8

void *pthread_run_spe(void *arg){
  spe_context_ptr_t spe_ctx;
  context *data = (context *)arg;
  void *argp;
  unsigned int entry;

  spe_ctx = spe_context_create(0, NULL);
  spe_program_load (spe_ctx, &spu_pi);

  entry=SPE_DEFAULT_ENTRY;
  argp=data;

  spe_context_run(spe_ctx, &entry,0,argp,NULL,NULL);
  spe_context_destroy(spe_ctx);

  pthread_exit(NULL);
}


int main(){
  int i;
  int N=1024;
  float pi=0.0;
  pthread_t pthreads[SPU_THREADS];
  context ctxs[SPU_THREADS] __attribute__ ((aligned(16)));

  for(i=0;i<SPU_THREADS;i++){
    ctxs[i].N=N;
    ctxs[i].Nstart=(N/SPU_THREADS)*i;
    ctxs[i].Nend=(N/SPU_THREADS)*(i+1);
    ctxs[i].pi=(float*) malloc_align(sizeof(float),7);
    pthread_create(&pthreads[i], NULL, &pthread_run_spe, &ctxs[i]);
  }

  for (i=0; i<SPU_THREADS; i++)
    pthread_join (pthreads[i], NULL);

  for(i=0;i<SPU_THREADS;i++)
    pi+=*(ctxs[i].pi);

  for(i=0;i<SPU_THREADS;i++)
    free_align(ctxs[i].pi);

  printf("PI = %f\n",pi);

  return (0);
}
