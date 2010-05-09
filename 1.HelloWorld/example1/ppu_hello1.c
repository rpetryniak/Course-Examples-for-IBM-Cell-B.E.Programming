//Szkolenie na Politechnice Czestochowskiej
//Przykład aplikacji wykorzystującej 1 wątek SPE (1/2)
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <libspe2.h>

extern spe_program_handle_t hello_spu;

int main()
{
  spe_context_ptr_t speid;
  unsigned int flags = 0;
  unsigned int entry = SPE_DEFAULT_ENTRY;
  void *argp = NULL;
  void *envp = NULL;
  spe_stop_info_t stop_info;
  int rc;

  speid = spe_context_create(0,NULL);
  if (speid==NULL)
  {
    perror("spe_context_create");
    return -1;
  }
  
  //Load SPE executable object into the SPE context local store
  if (spe_program_load(speid, &hello_spu))
  {
    perror("spe_program_load");
    return -2;
  }
  
  //Run the SPE context
  rc = spe_context_run(speid, &entry, flags, argp, envp, &stop_info);
  if (rc<0) perror("spe_context_run");
  
  //Destroy the SPE context
  spe_context_destroy(speid);
  
  return 0;
}
