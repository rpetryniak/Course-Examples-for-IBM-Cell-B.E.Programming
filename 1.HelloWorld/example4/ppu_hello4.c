#include <libspe2.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

#define N 4

spe_program_handle_t *handle_spe;
spe_context_ptr_t spe_ctx[N];

void *spe_pthread(void *arg)
{
  uint32_t entry = SPE_DEFAULT_ENTRY;
  int *spe_id = (int*)(arg);
  
  spe_context_run(spe_ctx[*spe_id],&entry,0,NULL,NULL,NULL);
  
  pthread_exit(NULL);
}

int main()
{
  pthread_t pthread[N];
  int id[N];
  
  handle_spe = spe_image_open("spu/spu_hello");

  int i=0;
  for (i=0; i<N; i++) {
    spe_ctx[i] = spe_context_create(SPE_EVENTS_ENABLE,NULL);
    spe_program_load(spe_ctx[i], handle_spe);
  }
  
  for (i=0; i<N; i++) {
    id[i] = i;
    pthread_create(&pthread[i], NULL, spe_pthread, &id[i]);
  }

  for (i=0; i<N; i++)
    pthread_join(&pthread[i], NULL);
  
  for (i=0; i<N; i++)
    spe_context_destroy(spe_ctx[i]);

  spe_image_close(handle_spe);

  return 0;
}
