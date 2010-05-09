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
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include "particle.h"

#define PARTICLES_PER_BLOCK		1024	/* # of particles to process at a time */

// Local store structures and buffers.
volatile parm_context ctx;
volatile vector float pos[PARTICLES_PER_BLOCK] __attribute__ ((aligned (128)));
volatile vector float vel[PARTICLES_PER_BLOCK] __attribute__ ((aligned (128)));
volatile float inv_mass[PARTICLES_PER_BLOCK] __attribute__ ((aligned (128)));

int main(unsigned long long spu_id __attribute__ ((unused)), unsigned long long parm)
{
  int i, j;
  int left, cnt;
  float time;
  unsigned int tag_id;
  vector float dt_v, dt_inv_mass_v;

  // Reserve a tag ID
  tag_id = mfc_tag_reserve();

  spu_writech(MFC_WrTagMask, -1);

  // Input parameter parm is a pointer to the particle parameter context.
  // Fetch the context, waiting for it to complete.
  
  spu_mfcdma32((void *)(&ctx), (unsigned int)parm, sizeof(parm_context), tag_id, MFC_GET_CMD);
  (void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

  dt_v = spu_splats(ctx.dt);

  // For each step in time
  for (time=0; time<END_OF_TIME; time += ctx.dt) {
    // For each block of particles
    for (i=0; i<ctx.particles; i+=PARTICLES_PER_BLOCK) {
      // Determine the number of particles in this block.
      left = ctx.particles - i;
      cnt = (left < PARTICLES_PER_BLOCK) ? left : PARTICLES_PER_BLOCK;

      // Fetch the data - position, velocity and inverse_mass. Wait for the DMA to complete 
      // before performing computation.
      spu_mfcdma32((void *)(pos), (unsigned int)(ctx.pos_v+i), cnt * sizeof(vector float), tag_id, MFC_GETB_CMD);
      spu_mfcdma32((void *)(vel), (unsigned int)(ctx.vel_v+i), cnt * sizeof(vector float), tag_id, MFC_GET_CMD);
      spu_mfcdma32((void *)(inv_mass), (unsigned int)(ctx.inv_mass+i), cnt * sizeof(float), tag_id, MFC_GET_CMD);
      (void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

      // Compute the step in time for the block of particles
      for (j=0; j<cnt; j++) {
	pos[j] = spu_madd(vel[j], dt_v, pos[j]);
	dt_inv_mass_v = spu_mul(dt_v, spu_splats(inv_mass[j]));
	vel[j] = spu_madd(dt_inv_mass_v, ctx.force_v, vel[j]);
      }

      // Put the position and velocity data back into system memory
      spu_mfcdma32((void *)(pos), (unsigned int)(ctx.pos_v+i), cnt * sizeof(vector float), tag_id, MFC_PUT_CMD);
      spu_mfcdma32((void *)(vel), (unsigned int)(ctx.vel_v+i), cnt * sizeof(vector float), tag_id, MFC_PUT_CMD);
    }
  }
  // Wait for final DMAs to complete before terminating SPU thread.
  (void)spu_mfcstat(MFC_TAG_UPDATE_ALL);
  return (0);
}
