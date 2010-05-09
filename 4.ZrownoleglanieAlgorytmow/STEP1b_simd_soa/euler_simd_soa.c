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
/* STEP1b
 *
 * Euler integration particle simulator SIMDized to run on the 
 * Vector/SIMD Multimedia Extension (aka VMX). This implementation
 * has been SIMDized as an Structure of Arrays (soa, also known as
 * a parallel-array form), in that each component of the 4-D particle
 * vectors are maintained in seperate arrays. This form is naturally
 * vectorizes by expressing the code in scalar operations using the 
 * SIMD instructions so that 4 independent results are computed in
 * parallel.
 */
#include <altivec.h>
#include "euler.h"
//#include <time.h> 

// Seperate arrays for each component of the vector.
vector float pos_x[PARTICLES/4], pos_y[PARTICLES/4], pos_z[PARTICLES/4];
vector float vel_x[PARTICLES/4], vel_y[PARTICLES/4], vel_z[PARTICLES/4];
vec4D force __attribute__ ((aligned (16)));
float inv_mass[PARTICLES] __attribute__ ((aligned (16)));
float dt = 1.0f;

int main()
{
  //clock_t start_time, end_time;
  //start_time = clock();
  
  int i;
  float time;
  float dt_inv_mass __attribute__ ((aligned (16)));
  vector float force_v, force_x, force_y, force_z;
  vector float dt_v, dt_inv_mass_v;

  // Create a replicated vector for each component of the force vector.
  force_v = *(vector float *)(&force);
  force_x = vec_splat(force_v, 0);
  force_y = vec_splat(force_v, 1);
  force_z = vec_splat(force_v, 2);

  // Replicate the variable time step across all elements.
  dt_v = vec_splat(vec_lde(0, &dt), 0);

  // For each step in time
  for (time=0; time<END_OF_TIME; time += dt) {
    // For each particle (assume that the number of particles is a multiple
    // of 4.
    for (i=0; i<PARTICLES/4; i++) {
      // Compute the new position and velocity as acted upon by the force f.
      pos_x[i] = vec_madd(vel_x[i], dt_v, pos_x[i]);
      pos_y[i] = vec_madd(vel_y[i], dt_v, pos_y[i]);
      pos_z[i] = vec_madd(vel_z[i], dt_v, pos_z[i]);

      dt_inv_mass = dt * inv_mass[i];
      dt_inv_mass_v = vec_splat(vec_lde(0, &dt_inv_mass), 0);

      vel_x[i] = vec_madd(dt_inv_mass_v, force_x, vel_x[i]);
      vel_y[i] = vec_madd(dt_inv_mass_v, force_y, vel_y[i]);
      vel_z[i] = vec_madd(dt_inv_mass_v, force_z, vel_z[i]);
    }
  }
  
  //end_time = clock(); 
  //printf("Total seconds elapsed %.9f\n", (float)(end_time - start_time) / (float)CLOCKS_PER_SEC); 
  
  return (0);
}
