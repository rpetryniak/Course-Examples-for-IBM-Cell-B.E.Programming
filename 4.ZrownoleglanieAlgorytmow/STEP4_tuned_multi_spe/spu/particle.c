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
/* STEP 4
 * 
 * Tune SPE code for performance.
 *
 * Programmers typically look to tune for performance using algorithmic 
 * methods. This too is important for SPE programming. However, tuning for 
 * the elimination of stalls, is just as important. The two forms of stalls 
 * we wish to consider are instruction dependency stalls and data stalls.
 *
 * Instruction stalls can be analyzed statically or dynamically. 
 * 
 * A static timing file is an annotated assembly listing of compiled source 
 * code. The annotation provides information on instruction issue assuming 
 * linear execution of the code. See the tutorial for a decription on 
 * generating and understanding static timing files. 
 * 
 * Dynamic analysis entails runing the system simulator on the SPE code in 
 * "pipeline" mode. The simulator provides detailed information including:
 *	- Instruction counts
 *	- Cycle counts
 *	- Cylces per instruction (CPI)
 *	- Branch statistics
 *	- Single and Dual issue rates
 *	- Numerous stalls statistics
 *	- Register usage
 *
 * For this euler example: To eliminate stalls and improve the CPI and 
 * ultimately the performance we must give the compiler more instructions in 
 * which to schedule so that we don't stall. The SPU's large register file 
 * allows us to unroll loops. Because there is no inner-loop dependencies in 
 * our computations, the unrolled loops should get interleaved by the 
 * compiler's scheduler. A dynamic analysis show that the register usage if 
 * fairly small so moderately agrressive unrolling will not produce "spilling"
 * (ie, registers having to be written into temporary stack storage). 
 *
 * Most compilers have the capability to automatic loop unrolling. Sometimes 
 * these are affective. Because automatic loop unrolling is not always 
 * affective or the programmer wants explicit control of when to unroll so as 
 * to specially manage the limited local resource, we will manually loop 
 * unroll. 
 *
 * The first pass of optimizations include:
 * 1) loop unroll to give more instructions in which to interleave.
 * 2) load DMA buffers contents into local non-volatile registers to eliminate 
 *    volatile migration constraints.
 * 3) eliminate scalar loads (inv_mass).
 * 4) eliminate extra multiplies of dt*inv_mass and "splat" the products term 
 *    after the SIMD multiply (instead of before).
 * 5) Interleave DMAs with computation by multi-buffering the inputs and 
 *    outputs to eliminate (or reduce) DMA stalls. Note, these stalls are not 
 *    reflected in the above static and dynamic analysis above and below. In 
 *    the process of adding double buffering, the inner-loop was moved into a 
 *    function so that the code need not be repeated.
 */

#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include "particle.h"

#define PARTICLES_PER_BLOCK	1024	/* # of particles to process at a time */

// Local store structures and buffers.
volatile parm_context ctx;
volatile vector float pos[2][PARTICLES_PER_BLOCK] __attribute__ ((aligned (128)));
volatile vector float vel[2][PARTICLES_PER_BLOCK] __attribute__ ((aligned (128)));
volatile vector float inv_mass[2][PARTICLES_PER_BLOCK/4] __attribute__ ((aligned (128)));


void process_buffer(int buffer, int cnt, vector float dt_v)
{
  int i;
  volatile vector float *p_inv_mass_v;
  vector float force_v, inv_mass_v;
  vector float pos0, pos1, pos2, pos3;
  vector float vel0, vel1, vel2, vel3;
  vector float dt_inv_mass_v, dt_inv_mass_v_0, dt_inv_mass_v_1, dt_inv_mass_v_2, dt_inv_mass_v_3;
  vector unsigned char splat_word_0 = (vector unsigned char){0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};
  vector unsigned char splat_word_1 = (vector unsigned char){4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7};
  vector unsigned char splat_word_2 = (vector unsigned char){8, 9,10,11, 8, 9,10,11, 8, 9,10,11, 8, 9,10,11};
  vector unsigned char splat_word_3 = (vector unsigned char){12,13,14,15,12,13,14,15,12,13,14,15,12,13,14,15};

  p_inv_mass_v = (volatile vector float *)&inv_mass[buffer][0]; 
  force_v = ctx.force_v;

  // Compute the step in time for the block of particles, four 
  // particle at a time.
  for (i=0; i<cnt; i+=4) {
    inv_mass_v = *p_inv_mass_v++;
    
    pos0 = pos[buffer][i+0];
    pos1 = pos[buffer][i+1];
    pos2 = pos[buffer][i+2];
    pos3 = pos[buffer][i+3];

    vel0 = vel[buffer][i+0];
    vel1 = vel[buffer][i+1];
    vel2 = vel[buffer][i+2];
    vel3 = vel[buffer][i+3];

    dt_inv_mass_v = spu_mul(dt_v, inv_mass_v);

    pos0 = spu_madd(vel0, dt_v, pos0);
    pos1 = spu_madd(vel1, dt_v, pos1);
    pos2 = spu_madd(vel2, dt_v, pos2);
    pos3 = spu_madd(vel3, dt_v, pos3);

    dt_inv_mass_v_0 = spu_shuffle(dt_inv_mass_v, dt_inv_mass_v, splat_word_0);
    dt_inv_mass_v_1 = spu_shuffle(dt_inv_mass_v, dt_inv_mass_v, splat_word_1);
    dt_inv_mass_v_2 = spu_shuffle(dt_inv_mass_v, dt_inv_mass_v, splat_word_2);
    dt_inv_mass_v_3 = spu_shuffle(dt_inv_mass_v, dt_inv_mass_v, splat_word_3);

    vel0 = spu_madd(dt_inv_mass_v_0, force_v, vel0);
    vel1 = spu_madd(dt_inv_mass_v_1, force_v, vel1);
    vel2 = spu_madd(dt_inv_mass_v_2, force_v, vel2);
    vel3 = spu_madd(dt_inv_mass_v_3, force_v, vel3);

    pos[buffer][i+0] = pos0;
    pos[buffer][i+1] = pos1;
    pos[buffer][i+2] = pos2;
    pos[buffer][i+3] = pos3;

    vel[buffer][i+0] = vel0;
    vel[buffer][i+1] = vel1;
    vel[buffer][i+2] = vel2;
    vel[buffer][i+3] = vel3;
  }
}


int main(unsigned long long spu_id __attribute__ ((unused)), unsigned long long argv)
{
  int buffer, next_buffer;
  int cnt, next_cnt, left;
  float time, dt;
  vector float dt_v;
  volatile vector float *ctx_pos_v, *ctx_vel_v;
  volatile vector float *next_ctx_pos_v, *next_ctx_vel_v;
  volatile float *ctx_inv_mass, *next_ctx_inv_mass;
  unsigned int tags[2];

  // Reserve a pair of DMA tag IDs
  tags[0] = mfc_tag_reserve();
  tags[1] = mfc_tag_reserve();
  
  // Input parameter argv is a pointer to the particle context.
  // Fetch the parameter context, waiting for it to complete.
  spu_writech(MFC_WrTagMask, 1 << tags[0]);
  spu_mfcdma32((void *)(&ctx), (unsigned int)argv, sizeof(parm_context), tags[0], MFC_GET_CMD);
  (void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

  dt = ctx.dt;
  dt_v = spu_splats(dt);

  // For each step in time
  for (time=0; time<END_OF_TIME; time += dt) {
    // For each double buffered block of particles
    left = ctx.particles;

    cnt = (left < PARTICLES_PER_BLOCK) ? left : PARTICLES_PER_BLOCK;

    ctx_pos_v = ctx.pos_v;
    ctx_vel_v = ctx.vel_v;
    ctx_inv_mass = ctx.inv_mass;

    // Prefetch first buffer of input data.
    buffer = 0;
    spu_mfcdma32((void *)(pos), (unsigned int)(ctx_pos_v), cnt * sizeof(vector float), tags[0], MFC_GETB_CMD);
    spu_mfcdma32((void *)(vel), (unsigned int)(ctx_vel_v), cnt * sizeof(vector float), tags[0], MFC_GET_CMD);
    spu_mfcdma32((void *)(inv_mass), (unsigned int)(ctx_inv_mass), cnt * sizeof(float), tags[0], MFC_GET_CMD);

    while (cnt < left) {
      left -= cnt;

      next_ctx_pos_v = ctx_pos_v + cnt;
      next_ctx_vel_v = ctx_vel_v + cnt;
      next_ctx_inv_mass = ctx_inv_mass + cnt;
      next_cnt = (left < PARTICLES_PER_BLOCK) ? left : PARTICLES_PER_BLOCK;

      // Prefetch next buffer so the data is available for computation on next loop iteration.
      // The first DMA is barriered so that we don't GET data before the previous iteration's
      // data is PUT.
      next_buffer = buffer^1;

      spu_mfcdma32((void *)(&pos[next_buffer][0]), (unsigned int)(next_ctx_pos_v), next_cnt * sizeof(vector float), tags[next_buffer], MFC_GETB_CMD);
      spu_mfcdma32((void *)(&vel[next_buffer][0]), (unsigned int)(next_ctx_vel_v), next_cnt * sizeof(vector float), tags[next_buffer], MFC_GET_CMD);
      spu_mfcdma32((void *)(&inv_mass[next_buffer][0]), (unsigned int)(next_ctx_inv_mass), next_cnt * sizeof(float), tags[next_buffer], MFC_GET_CMD);
      
      // Wait for previously prefetched data
      spu_writech(MFC_WrTagMask, 1 << tags[buffer]);
      (void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

      process_buffer(buffer, cnt, dt_v);

      // Put the buffer's position and velocity data back into system memory
      spu_mfcdma32((void *)(&pos[buffer][0]), (unsigned int)(ctx_pos_v), cnt * sizeof(vector float), tags[buffer], MFC_PUT_CMD);
      spu_mfcdma32((void *)(&vel[buffer][0]), (unsigned int)(ctx_vel_v), cnt * sizeof(vector float), tags[buffer], MFC_PUT_CMD);
      
      ctx_pos_v = next_ctx_pos_v;
      ctx_vel_v = next_ctx_vel_v;
      ctx_inv_mass = next_ctx_inv_mass;

      buffer = next_buffer;
      cnt = next_cnt;		  
    }

    // Wait for previously prefetched data
    spu_writech(MFC_WrTagMask, 1 << tags[buffer]);
    (void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

    process_buffer(buffer, cnt, dt_v);

    // Put the buffer's position and velocity data back into system memory
    spu_mfcdma32((void *)(&pos[buffer][0]), (unsigned int)(ctx_pos_v), cnt * sizeof(vector float), tags[buffer], MFC_PUT_CMD);
    spu_mfcdma32((void *)(&vel[buffer][0]), (unsigned int)(ctx_vel_v), cnt * sizeof(vector float), tags[buffer], MFC_PUT_CMD);

    // Wait for DMAs to complete before starting the next step in time.
    spu_writech(MFC_WrTagMask, 1 << tags[buffer]);
    (void)spu_mfcstat(MFC_TAG_UPDATE_ALL);
  }

  return (0);
}



