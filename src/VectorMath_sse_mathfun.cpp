/* SIMD (SSE2) implementation of sin and exp

   Inspired by Intel Approximate Math library, and based on the
   corresponding algorithms of the cephes math library
*/

/* Copyright (C) 2007  Julien Pommier

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  (this is the zlib license)
*/

#include "VectorMath_sse_mathfun.h"

#ifdef USE_SSE2

v4sf sin_ps(v4sf x) { // any x
  v4sf xmm1, xmm2 = _mm_setzero_ps(), xmm3, sign_bit, y;

  v4si emm0, emm2;

  sign_bit = x;
  /* take the absolute value */
  x = _mm_and_ps(x, _ps_inv_sign_mask.v);
  /* extract the sign bit (upper one) */
  sign_bit = _mm_and_ps(sign_bit, _ps_sign_mask.v);

  /* scale by 4/Pi */
  y = _mm_mul_ps(x, _ps_cephes_FOPI.v);

  /* store the integer part of y in mm0 */
  emm2 = _mm_cvttps_epi32(y);
  /* j=(j+1) & (~1) (see the cephes sources) */
  emm2 = _mm_add_epi32(emm2, _pi32_1.vi);
  emm2 = _mm_and_si128(emm2, _pi32_inv1.vi);
  y = _mm_cvtepi32_ps(emm2);

  /* get the swap sign flag */
  emm0 = _mm_and_si128(emm2, _pi32_4.vi);
  emm0 = _mm_slli_epi32(emm0, 29);
  /* get the polynom selection mask
     there is one polynom for 0 <= x <= Pi/4
     and another one for Pi/4<x<=Pi/2

     Both branches will be computed.
  */
  emm2 = _mm_and_si128(emm2, _pi32_2.vi);
  emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());

  v4sf swap_sign_bit = _mm_castsi128_ps(emm0);
  v4sf poly_mask = _mm_castsi128_ps(emm2);
  sign_bit = _mm_xor_ps(sign_bit, swap_sign_bit);

  /* The magic pass: "Extended precision modular arithmetic"
     x = ((x - y * DP1) - y * DP2) - y * DP3; */
  xmm1 = _ps_minus_cephes_DP1.v;
  xmm2 = _ps_minus_cephes_DP2.v;
  xmm3 = _ps_minus_cephes_DP3.v;
  xmm1 = _mm_mul_ps(y, xmm1);
  xmm2 = _mm_mul_ps(y, xmm2);
  xmm3 = _mm_mul_ps(y, xmm3);
  x = _mm_add_ps(x, xmm1);
  x = _mm_add_ps(x, xmm2);
  x = _mm_add_ps(x, xmm3);

  /* Evaluate the first polynom  (0 <= x <= Pi/4) */
  y = _ps_coscof_p0.v;
  v4sf z = _mm_mul_ps(x,x);

  y = _mm_mul_ps(y, z);
  y = _mm_add_ps(y, _ps_coscof_p1.v);
  y = _mm_mul_ps(y, z);
  y = _mm_add_ps(y, _ps_coscof_p2.v);
  y = _mm_mul_ps(y, z);
  y = _mm_mul_ps(y, z);
  v4sf tmp = _mm_mul_ps(z, _ps_0p5.v);
  y = _mm_sub_ps(y, tmp);
  y = _mm_add_ps(y, _ps_1.v);

  /* Evaluate the second polynom  (Pi/4 <= x <= 0) */

  v4sf y2 = _ps_sincof_p0.v;
  y2 = _mm_mul_ps(y2, z);
  y2 = _mm_add_ps(y2, _ps_sincof_p1.v);
  y2 = _mm_mul_ps(y2, z);
  y2 = _mm_add_ps(y2, _ps_sincof_p2.v);
  y2 = _mm_mul_ps(y2, z);
  y2 = _mm_mul_ps(y2, x);
  y2 = _mm_add_ps(y2, x);

  /* select the correct result from the two polynoms */
  xmm3 = poly_mask;
  y2 = _mm_and_ps(xmm3, y2); //, xmm3);
  y = _mm_andnot_ps(xmm3, y);
  y = _mm_add_ps(y,y2);
  /* update the sign */
  y = _mm_xor_ps(y, sign_bit);
  return y;
}

v4sf exp_ps(v4sf x) {
  v4sf tmp = _mm_setzero_ps(), fx;

  v4si emm0;

  v4sf one = _ps_1.v;

  x = _mm_min_ps(x, _ps_exp_hi.v);
  x = _mm_max_ps(x, _ps_exp_lo.v);

  /* express exp(x) as exp(g + n*log(2)) */
  fx = _mm_mul_ps(x, _ps_cephes_LOG2EF.v);
  fx = _mm_add_ps(fx, _ps_0p5.v);

  emm0 = _mm_cvttps_epi32(fx);
  tmp  = _mm_cvtepi32_ps(emm0);

  /* if greater, substract 1 */
  v4sf mask = _mm_cmpgt_ps(tmp, fx);
  mask = _mm_and_ps(mask, one);
  fx = _mm_sub_ps(tmp, mask);

  tmp = _mm_mul_ps(fx, _ps_cephes_exp_C1.v);
  v4sf z = _mm_mul_ps(fx, _ps_cephes_exp_C2.v);
  x = _mm_sub_ps(x, tmp);
  x = _mm_sub_ps(x, z);

  z = _mm_mul_ps(x,x);

  v4sf y = _ps_cephes_exp_p0.v;
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, _ps_cephes_exp_p1.v);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, _ps_cephes_exp_p2.v);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, _ps_cephes_exp_p3.v);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, _ps_cephes_exp_p4.v);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, _ps_cephes_exp_p5.v);
  y = _mm_mul_ps(y, z);
  y = _mm_add_ps(y, x);
  y = _mm_add_ps(y, one);

  /* build 2^n */
  emm0 = _mm_cvttps_epi32(fx);
  emm0 = _mm_add_epi32(emm0, _pi32_0x7f.vi);
  emm0 = _mm_slli_epi32(emm0, 23);
  v4sf pow2n = _mm_castsi128_ps(emm0);

  y = _mm_mul_ps(y, pow2n);
  return y;
}

#endif
