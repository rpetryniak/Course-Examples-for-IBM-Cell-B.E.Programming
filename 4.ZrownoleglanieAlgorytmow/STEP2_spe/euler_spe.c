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
/* STEP2
 *
 * Port STEP1 to perform the computation in a single SPE. In this case
 * we have ported the Array of Structures implementation (STEP1a) the SPE.
 *
 * To move the code from the PPE/VMX to the SPE required:
 * a) Creating a control structure (called parameter context), that defines 
 *    parameters of the computation to be performed on the SPE. This includes, 
 *    pointers to the particle array data, current force information, etc... 
 *    We pass the pointer to the context by using the parameter passing 
 *    mechanism to the SPE thread. Alternatively, this information could have 
 *    been passed via the mailbox. 
 * b) Porting the computation for execution on the SPE. The complexity of this 
 *    operation depends upon the types of data and types of intrinsics used. 
 *    For this case, some of the intrinsic require just a simple name 
 *    translation (vec_madd -> spu_madd). The translation of the scalar values 
 *    is a little more extensive.
 * c) Adding an additional looping construct to partition the data arrays into 
 *    samller blocks. This was required because all the data will not fit 
 *    within the SPE's local store.
 * d) Adding DMAs to move data in and out of the SPU's local store. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <libspe2.h>
#include <pthread.h>
#include "particle.h"
//#include <time.h>

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
  
  ppu_pthread_data_t data;
  parm_context ctx __attribute__ ((aligned (16)));

  ctx.particles = PARTICLES;
  ctx.pos_v = (vector float *)pos;
  ctx.vel_v = (vector float *)vel;
  ctx.force_v = *((vector float *)&force);
  ctx.inv_mass = inv_mass;
  ctx.dt = dt;

  /* Create a SPE context */
  if ((data.spe_ctx = spe_context_create (0, NULL)) == NULL) {
    perror ("Failed creating context");
    exit (1);
  }
  /* Load SPE program into the SPE context*/
  if (spe_program_load (data.spe_ctx, &particle))  {
    perror ("Failed loading program");
    exit (1);
  }
  /* Initialize context run data */
  data.argp = &ctx;
  /* Create pthread for each of the SPE contexts */
  if (pthread_create (&data.pthread, NULL, &ppu_pthread_function, &data)) {
    perror ("Failed creating thread");
    exit (1);
  }
  /* Wait for the threads to complete */
  if (pthread_join (data.pthread, NULL)) {
    perror ("Failed joining thread\n");
    exit (1);
  }
  /* Destroy SPE context */
  if (spe_context_destroy (data.spe_ctx) != 0) {
    perror("Failed destroying context");
    exit (1);
  }

//  end_time = clock(); 
//  printf("Total seconds elapsed %.9f\n", (float)(end_time - start_time) / (float)CLOCKS_PER_SEC); 

  return (0);
}
