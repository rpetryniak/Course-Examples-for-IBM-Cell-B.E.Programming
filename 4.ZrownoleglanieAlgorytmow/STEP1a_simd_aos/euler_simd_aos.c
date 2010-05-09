/* --------------------------------------------------------------  */
/* (C)Copyright 2001,2007,                                         */
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
/* STEP1a
 *
 * Euler integration particle simulator SIMDized to run on the 
 * Vector/SIMD Multimedia Extension (aka VMX). This implementation
 * has been SIMDized as an Array of Structures (aos, also known as
 * a vec-across form), in that the 4-D particle vectors are maintained
 * naturally in the 128-bit VMX vector registers.
 */
#include <altivec.h>
#include "euler.h"
//#include <time.h> 

vec4D pos[PARTICLES] __attribute__ ((aligned (16)));
vec4D vel[PARTICLES] __attribute__ ((aligned (16)));
vec4D force __attribute__ ((aligned (16)));
float inv_mass[PARTICLES] __attribute__ ((aligned (16)));
float dt __attribute__ ((aligned (16))) = 1.0f;

int main()
{
  //clock_t start_time, end_time;
  //start_time = clock(); 
  
  int i;
  float time;
  float dt_inv_mass __attribute__ ((aligned (16)));
  vector float dt_v, dt_inv_mass_v;
  vector float *pos_v, *vel_v, force_v;
  vector float zero = (vector float){0.0f, 0.0f, 0.0f, 0.0f};

  pos_v = (vector float *)pos;
  vel_v = (vector float *)vel;
  force_v = *((vector float *)&force);

  // Replicate the variable time step across elements 0-2 of
  // a floating point vector. Force the last element (3) to zero.
  dt_v = vec_sld(vec_splat(vec_lde(0, &dt), 0), zero, 4);

  // For each step in time
  for (time=0; time<END_OF_TIME; time += dt) {
    // For each particle
    for (i=0; i<PARTICLES; i++) {

      pos_v[i] = vec_madd(vel_v[i], dt_v, pos_v[i]);

      dt_inv_mass = dt * inv_mass[i];
      dt_inv_mass_v = vec_splat(vec_lde(0, &dt_inv_mass), 0);

      vel_v[i] = vec_madd(dt_inv_mass_v, force_v, vel_v[i]);
    }
  }
  
  //end_time = clock(); 
  //printf("Total seconds elapsed %.9f\n", (float)(end_time - start_time) / (float)CLOCKS_PER_SEC); 
  
  return (0);
}
