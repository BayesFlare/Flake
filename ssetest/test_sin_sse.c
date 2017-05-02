/* some tests of SSE versions of functions                                 
 * These are copied from the LAL library VectorMath functions with copyright assigned to
 * Karl Wette and Reinhard Prix, and distributed under GPL - further copyright notices are below */

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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <xmmintrin.h>

#define UNUSED __attribute__ ((unused))

# define ALIGN16_BEG
# define ALIGN16_END __attribute__((aligned(16)))

/* __m128 is ugly to write */
typedef __m128 v4sf;  // vector of 4 float (sse1)

typedef __m128i v4si; // vector of 4 int (sse2)

typedef ALIGN16_BEG union {
  float f[4];
  int i[4];
  v4sf  v;
  v4si vi;
} ALIGN16_END V4SF;

static v4sf sin_ps(v4sf x);
static v4sf exp_ps(v4sf x);
static inline int VectorMathSin( float *out, const float *in, const unsigned int len );
static inline int VectorMathSin( float *out, const float *in, const unsigned int len );
static inline int VectorMathAdd( float *out, const float *in1, const float *in2, unsigned int len );


/* declare some SSE constants -- why can't I figure a better way to do that? */
#define _PS_CONST(Name, Val)                                    \
  static const V4SF _ps_##Name = { .f={Val, Val, Val, Val} }
#define _PI32_CONST(Name, Val)                                  \
  static const V4SF _pi32_##Name = { .i={Val, Val, Val, Val} }
#define _PS_CONST_INT(Name, Val)                                \
  static const V4SF _ps_##Name = { .i={Val, Val, Val, Val} }

_PS_CONST(1  , 1.0f);
_PS_CONST(0p5, 0.5f);
/* the smallest non denormalized float number */
_PS_CONST_INT(min_norm_pos, 0x00800000);
_PS_CONST_INT(mant_mask, 0x7f800000);
_PS_CONST_INT(inv_mant_mask, ~0x7f800000);

_PS_CONST_INT(sign_mask, (int)0x80000000);
_PS_CONST_INT(inv_sign_mask, ~0x80000000);

_PI32_CONST(1, 1);
_PI32_CONST(inv1, ~1);
_PI32_CONST(2, 2);
_PI32_CONST(4, 4);
_PI32_CONST(0x7f, 0x7f);

_PS_CONST(minus_cephes_DP1, -0.78515625);
_PS_CONST(minus_cephes_DP2, -2.4187564849853515625e-4);
_PS_CONST(minus_cephes_DP3, -3.77489497744594108e-8);
_PS_CONST(sincof_p0, -1.9515295891E-4);
_PS_CONST(sincof_p1,  8.3321608736E-3);
_PS_CONST(sincof_p2, -1.6666654611E-1);
_PS_CONST(coscof_p0,  2.443315711809948E-005);
_PS_CONST(coscof_p1, -1.388731625493765E-003);
_PS_CONST(coscof_p2,  4.166664568298827E-002);
_PS_CONST(cephes_FOPI, 1.27323954473516); // 4 / M_PI

UNUSED static inline __m128
local_add_ps ( __m128 in1, __m128 in2 )
{
  return _mm_add_ps ( in1, in2 );
}

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

_PS_CONST(exp_hi,	88.3762626647949f);
_PS_CONST(exp_lo,	-88.3762626647949f);

_PS_CONST(cephes_LOG2EF, 1.44269504088896341);
_PS_CONST(cephes_exp_C1, 0.693359375);
_PS_CONST(cephes_exp_C2, -2.12194440e-4);

_PS_CONST(cephes_exp_p0, 1.9875691500E-4);
_PS_CONST(cephes_exp_p1, 1.3981999507E-3);
_PS_CONST(cephes_exp_p2, 8.3334519073E-3);
_PS_CONST(cephes_exp_p3, 4.1665795894E-2);
_PS_CONST(cephes_exp_p4, 1.6666665459E-1);
_PS_CONST(cephes_exp_p5, 5.0000001201E-1);

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

static inline int VectorMathSin( float *out, const float *in, const unsigned int len ){
  // walk through vector in blocks of 4
  unsigned int i4Max = len - ( len % 4 );
  for ( unsigned int i4 = 0; i4 < i4Max; i4 += 4 )
    {
      __m128 in4p = _mm_loadu_ps(&in[i4]);
      __m128 out4p = (*sin_ps)( in4p );
      _mm_storeu_ps(&out[i4], out4p);
    }

  // deal with the remaining (<=3) terms separately
  V4SF in4 = {.f = {0,0,0,0}}, out4;
  for ( unsigned int i = i4Max, j=0; i < len; i ++, j++ ) {
    in4.f[j] = in[i];
  }
  out4.v = (*sin_ps)( in4.v );
  for ( unsigned int i = i4Max,j=0; i < len; i ++, j++ ) {
    out[i] = out4.f[j];
  }

  return 0;
}

static inline int VectorMathExp( float *out, const float *in, const unsigned int len ){
  // walk through vector in blocks of 4
  unsigned int i4Max = len - ( len % 4 );
  for ( unsigned int i4 = 0; i4 < i4Max; i4 += 4 )
    {
      __m128 in4p = _mm_loadu_ps(&in[i4]);
      __m128 out4p = (*exp_ps)( in4p );
      _mm_storeu_ps(&out[i4], out4p);
    }

  // deal with the remaining (<=3) terms separately
  V4SF in4 = {.f = {0,0,0,0}}, out4;
  for ( unsigned int i = i4Max, j=0; i < len; i ++, j++ ) {
    in4.f[j] = in[i];
  }
  out4.v = (*exp_ps)( in4.v );
  for ( unsigned int i = i4Max,j=0; i < len; i ++, j++ ) {
    out[i] = out4.f[j];
  }

  return 0;
}

static inline int VectorMathAdd( float *out, const float *in1, const float *in2, unsigned int len ){

  // walk through vector in blocks of 4
  unsigned int i4Max = len - ( len % 4 );
  for ( unsigned int i4 = 0; i4 < i4Max; i4 += 4 )
    {
      __m128 in4p_1 = _mm_loadu_ps(&in1[i4]);
      __m128 in4p_2 = _mm_loadu_ps(&in2[i4]);
      __m128 out4p = _mm_add_ps( in4p_1, in4p_2 );
      _mm_storeu_ps(&out[i4], out4p);
    }

  // deal with the remaining (<=3) terms separately
  V4SF in4_1 = {.f={0,0,0,0}};
  V4SF in4_2 = {.f={0,0,0,0}};
  V4SF out4;
  for ( unsigned int i = i4Max,j=0; i < len; i ++, j++ ) {
    in4_1.f[j] = in1[i];
    in4_2.f[j] = in2[i];
  }
  out4.v = _mm_add_ps( in4_1.v, in4_2.v );
  for ( unsigned int i = i4Max,j=0; i < len; i ++, j++ ) {
    out[i] = out4.f[j];
  }

  return 0;
}

int main(){
  unsigned int N = 5000, Ntries=10000,  j = 0, i = 0;
  double y = 0.;

  float phases[N], phases2[N], phases3[N], sinphis[N];
  float ssephases[N], ssephases2[N], ssephases3[N], ssesinphis[N];

  float maxerr = 0.;

  for ( j = 0; j < N; j++ ){
    phases[j] = 2.*3.14*(float)j*0.16757247;
    phases2[j] = 2.*3.14*(float)j*0.87523986;
    ssephases[j] = 2.*3.14*(float)j*0.16757247;
    ssephases2[j] = 2.*3.14*(float)j*0.87523986;
  }

  /* time SSE vector math sin function */
  clock_t begin = clock();
  for ( j = 0; j < Ntries; j++ ){ VectorMathSin( ssesinphis, ssephases, N ); }
  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  
  fprintf(stderr, "time \"SSE\" vector math sin = %.5lf\n", time_spent);

  /* time math.h sin function */
  begin = clock();
  for ( j = 0; j < Ntries; j++ ){
    for ( i = 0; i < N; i++ ){ sinphis[i] = sinf(phases[i]); }
  }
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  
  fprintf(stderr, "time \"math.h\" sin = %.5lf\n", time_spent);

  for ( i = 0; i < N; i++ ){
    if ( fabs(sinphis[i] - ssesinphis[i]) > maxerr ){
      maxerr = fabs(sinphis[i] - ssesinphis[i]);
    }
  }

  fprintf(stderr, "maximum sin error = %.12f\n", maxerr);

  /* time SSE vector math exp function */
  begin = clock();
  for ( j = 0; j < Ntries; j++ ){ VectorMathExp( ssephases3, ssesinphis, N ); }
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  
  fprintf(stderr, "time \"SSE\" vector math exp = %.5lf\n", time_spent);

  /* time math.h sin function */
  begin = clock();
  for ( j = 0; j < Ntries; j++ ){
    for ( i = 0; i < N; i++ ){ phases3[i] = expf(sinphis[i]); }
  }
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  
  fprintf(stderr, "time \"math.h\" exp = %.5lf\n", time_spent);

  maxerr = 0.;
  for ( i = 0; i < N; i++ ){
    if ( fabs(phases3[i] - ssephases3[i]) > maxerr ){
      maxerr = fabs(phases3[i] - ssephases3[i]);
    }
  }

  fprintf(stderr, "maximum exp error = %.12f\n", maxerr);

  /* time SSE vector math addition functions */
  begin = clock();
  for ( j = 0; j < Ntries; j++ ){ VectorMathAdd( phases3, phases, phases2, N ); }
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  fprintf(stderr, "time \"SSE\" vector add = %.5lf\n", time_spent);

  /* time looped vector add */
  begin = clock();
  for ( j = 0; j < Ntries; j++ ){
    for ( i = 0; i < N; i++ ){ phases3[i] = phases[i] + phases2[i]; }
  }
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  fprintf(stderr, "time looped vector add = %.5lf\n", time_spent);

  return 0;
}

