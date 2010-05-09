#include <libspe2.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

spe_program_handle_t *handle_spe;
spe_context_ptr_t spe_ctx;

void *spe_pthread(void *arg)
{
  uint32_t entry = SPE_DEFAULT_ENTRY;
  spe_context_run(spe_ctx,&entry,0,NULL,NULL,NULL);
  pthread_exit(NULL);
}

int main()
{
  pthread_t pthread;
  
  spe_ctx = spe_context_create(0,NULL);
  handle_spe = spe_image_open("spu/spu_hello");
  spe_program_load(spe_ctx, handle_spe);

  pthread_create(&pthread, NULL, spe_pthread, NULL);
  pthread_join(pthread, NULL);

  spe_context_destroy(spe_ctx);
  spe_image_close(handle_spe);

  return 0;
}
