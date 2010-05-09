#include <stdio.h> 
#include <stdlib.h>
#include <libspe2.h> 
#include <pthread.h> 
#include <time.h> 

#define SPU_THREADS 100

extern spe_program_handle_t hello_spu; 

typedef struct { 
  spe_context_ptr_t spe; 
  unsigned long long *args; 
} thread_arg_t; 

void *run_hello_spu(void *thread_arg) { 
  int ret; 
  thread_arg_t *arg = (thread_arg_t *) thread_arg; 
  unsigned int entry = SPE_DEFAULT_ENTRY; 
  spe_stop_info_t stop_info; 
  
  ret = spe_context_run(arg->spe, &entry, 0, arg->args, NULL, &stop_info); 
  if (ret < 0) { 
    perror("spe_context_run"); 
    return NULL; 
  } 
  
  printf("status = %d\n", spe_stop_info_read(arg->spe, &stop_info)); 
  return NULL; 
} 
  
int main(int argc, char **argv) { 
  clock_t start_time, end_time; 
  start_time = clock(); 
  int i; 
  int ret; 
  
  spe_context_ptr_t spe[SPU_THREADS]; 
  pthread_t thread[SPU_THREADS]; 
  thread_arg_t arg[SPU_THREADS]; 
  
  for (i = 0; i < SPU_THREADS; i++) { 
    spe[i] = spe_context_create(SPE_EVENTS_ENABLE, NULL); 
    
    if (!spe[i]) { 
      perror("spe_context_create"); 
      exit(1); 
    }
    
    ret = spe_program_load(spe[i], &hello_spu); 
    if (ret) { 
      perror("spe_program_load"); 
      exit(1); 
    }
    
    arg[i].spe = spe[i]; 
    arg[i].args = NULL; 
    ret = pthread_create(&thread[i], NULL, run_hello_spu, &arg[i]); 
    
    if (ret) {
      perror("pthread_create"); 
      exit(1); 
    } 
  } 
  
  for (i = 0; i < SPU_THREADS; i++) { 
    pthread_join(thread[i], NULL); 
    ret = spe_context_destroy(spe[i]); 
    
    if (ret) { 
      perror("spe_context_destroy"); 
      exit(1); 
    } 
  } 
  
  end_time = clock(); 
  printf("Total seconds elapsed %.2f\n", (float)(end_time - start_time) / (float)CLOCKS_PER_SEC); 

  return 0; 
}