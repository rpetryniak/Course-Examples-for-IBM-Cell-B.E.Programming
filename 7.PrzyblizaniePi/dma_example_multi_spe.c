#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <libspe2.h>
#include <pthread.h>

#include <dma_example.h>

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
  int i, num_spe_threads, rc;
  spe_context_ptr_t   spe_contexts    [MAX_NUM_SPE_THREADS];
  pthread_t           spe_threads     [MAX_NUM_SPE_THREADS];
  control_block_t*    control_blocks  [MAX_NUM_SPE_THREADS];
  ppu_thread_data_t   ppu_thread_data [MAX_NUM_SPE_THREADS];

  int N=1024*32;
  double pi=0.0;

  /* Determine the number of SPE threads to create. */
  num_spe_threads = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1);
  if (num_spe_threads > MAX_NUM_SPE_THREADS) num_spe_threads = MAX_NUM_SPE_THREADS;

  for (i = 0; i < num_spe_threads; i++)
  {
    /* create control block */
    rc = posix_memalign ((void**)(&control_blocks[i]), 128, sizeof (control_block_t));
    if (rc != 0)
    {
      fprintf (stderr, "Failed allocating space for control block\n");
      exit (1);
    }

    control_blocks[i]->N=N;
    control_blocks[i]->Nstart=(N/num_spe_threads)*i;
    control_blocks[i]->Nend=(N/num_spe_threads)*(i+1);
    control_blocks[i]->pi = 0.0f;

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

    pi+=control_blocks[i]->pi;

    /* Destroy context */
    if (spe_context_destroy (spe_contexts[i]) != 0)
    {
      perror ("Failed destroying SPE context");
      exit (1);
    }

    free (control_blocks[i]);
  }

  printf("PI = %.50f\n",pi);
  printf("PI = 3.14159265358979323846264338327950288419716939937510 (wikipedia, 50 miejsc)\n");
  printf("\nThe program has successfully executed.\n");

  return (0);
}
