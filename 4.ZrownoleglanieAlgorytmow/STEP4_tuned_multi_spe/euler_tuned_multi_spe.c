/* --------------------------------------------------------------  */
/* (C)Copyright 2001,2008,                                         */
/* International Business Machines Corporation,                    */
/* Sony Computer Entertainment, Incorporated,                      */
/* Toshiba Corporation,                                            */
/*                                                                 */
/* All Rights Reserved.                                            */
/*                                                                 */
/* Redistribution and use in source and binary forms, with or      */
/* without modification, are permitted provided that the           */
/* following conditions are met:                                   */
/*                                                                 */
/* - Redistributions of source code must retain the above copyright*/
/*   notice, this list of conditions and the following disclaimer. */
/*                                                                 */
/* - Redistributions in binary form must reproduce the above       */
/*   copyright notice, this list of conditions and the following   */
/*   disclaimer in the documentation and/or other materials        */
/*   provided with the distribution.                               */
/*                                                                 */
/* - Neither the name of IBM Corporation nor the names of its      */
/*   contributors may be used to endorse or promote products       */
/*   derived from this software without specific prior written     */
/*   permission.                                                   */
/*                                                                 */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND          */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,     */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE        */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR            */
/* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT    */
/* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;    */
/* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)        */
/* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN       */
/* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR    */
/* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  */
/* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              */
/* --------------------------------------------------------------  */
/* PROLOG END TAG zYx                                              */
/* STEP 3
 *
 * Parallelize code for execution across multiple SPEs
 *
 * The most common and practicle method of parallelizing computation across 
 * multiple SPE is to data partition the problem. The works well for 
 * applications with little or no data dependency. In our example, we can 
 * partition the Euler integration of the particle equally amongst the 
 * available SPEs. If there are 4 SPEs, then the first quarter of the
 * particples is processed by the first SPE, the second quarter of the 
 * particle is processed by the second SPE, and so forth.
 *
 * In this step we reuse the SPE program developed in STEP 2 (see 
 * ../STEP2_spe/spu)
 */

#include <stdio.h>
#include <stdlib.h>
#include <libspe2.h>
#include <pthread.h>
#include "particle.h"
//#include <time.h> 

#define MAX_SPE_THREADS		16

vec4D pos[PARTICLES] __attribute__ ((aligned (128)));
vec4D vel[PARTICLES] __attribute__ ((aligned (128)));
vec4D force __attribute__ ((aligned (16)));
float inv_mass[PARTICLES] __attribute__ ((aligned (128)));
float dt = 1.0f;

extern spe_program_handle_t particle;

typedef struct ppu_pthread_data {
  spe_context_ptr_t spe_ctx;
  pthread_t pthread;
  void *argp;
} ppu_pthread_data_t;

void *ppu_pthread_function(void *arg) {
  ppu_pthread_data_t *datap = (ppu_pthread_data_t *)arg;
  unsigned int entry = SPE_DEFAULT_ENTRY;
  if (spe_context_run(datap->spe_ctx, &entry, 0, datap->argp, NULL, NULL) < 0) {
    perror ("Failed running context");
    exit (1);
  }
  pthread_exit(NULL);
}


int main()
{
//  clock_t start_time, end_time;
//  start_time = clock(); 
  
  int i, offset, count, spe_threads;
  ppu_pthread_data_t datas[MAX_SPE_THREADS];
  parm_context ctxs[MAX_SPE_THREADS] __attribute__ ((aligned (16)));

  /* Determine the number of SPE threads to create.
   */
  spe_threads = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1);
  if (spe_threads > MAX_SPE_THREADS) spe_threads = MAX_SPE_THREADS;

  /* Create multiple SPE threads */
  for (i=0, offset=0; i<spe_threads; i++, offset+=count) {
    /* Construct a parameter context for each SPE. Make sure
     * that each SPEs (excluding the last) particle count is a multiple
     * of 4 so that inv_mass context pointer is always quadword aligned.
     */
    count = (PARTICLES / spe_threads + 3) & ~3;
    ctxs[i].particles = (i==(spe_threads-1)) ? PARTICLES - offset : count;
    ctxs[i].pos_v = (vector float *)&pos[offset];
    ctxs[i].vel_v = (vector float *)&vel[offset];
    ctxs[i].force_v = *((vector float *)&force);
    ctxs[i].inv_mass = &inv_mass[offset];
    ctxs[i].dt = dt;
    
    /* Create SPE context */
    if ((datas[i].spe_ctx = spe_context_create (0, NULL)) == NULL) {
        perror ("Failed creating context");
        exit (1);
    }
    /* Load SPE program into the SPE context */
    if (spe_program_load (datas[i].spe_ctx, &particle)) {
      perror ("Failed loading program");
      exit (1);
    }
    /* Initialize context run data */
    datas[i].argp = &ctxs[i];
    /* Create pthread for each of the SPE conexts */
    if (pthread_create (&datas[i].pthread, NULL, &ppu_pthread_function, &datas[i])) {
      perror ("Failed creating thread");
    }
  }

  /* Wait for all the SPE threads to complete.*/
  for (i=0; i<spe_threads; i++) {
    if (pthread_join (datas[i].pthread, NULL)) {
      perror ("Failed joining thread");
      exit (1);
    }

    /* Destroy context */
    if (spe_context_destroy (datas[i].spe_ctx) != 0) {
      perror("Failed destroying context");
      exit (1);
    }
  }

//  end_time = clock(); 
//  printf("Total seconds elapsed %.9f\n", (float)(end_time - start_time) / (float)CLOCKS_PER_SEC); 

  return (0);
}

