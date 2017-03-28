// SSE2 vector math funtions copied from LAL as written by Reinhard Prix, Karl Wette
// and Evan Goetz

#include "VectorMath_sse_mathfun.h"

#ifdef USE_SSE2

int VectorMathSin( float *out, const float *in, const unsigned int len ){
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

int VectorMathExp( float *out, const float *in, const unsigned int len ){
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

int VectorMathAdd( float *out, const float *in1, const float *in2, unsigned int len ){
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

int VectorMathScale( float *out, float scalar, const float *in, const unsigned int len ){
  const V4SF scalar4 = {.f={scalar,scalar,scalar,scalar}};

  // walk through vector in blocks of 4
  unsigned int i4Max = len - ( len % 4 );
  for ( unsigned int i4 = 0; i4 < i4Max; i4 += 4 )
    {
      __m128 in4p = _mm_loadu_ps(&in[i4]);
      __m128 out4p = _mm_mul_ps( scalar4.v, in4p );
      _mm_storeu_ps(&out[i4], out4p);
    }

  // deal with the remaining (<=3) terms separately
  V4SF in4 = {.f={0,0,0,0}};
  V4SF out4;
  for ( unsigned int i = i4Max,j=0; i < len; i ++, j++ ) {
    in4.f[j] = in[i];
  }
  out4.v = _mm_mul_ps( scalar4.v, in4.v );
  for ( unsigned int i = i4Max,j=0; i < len; i ++, j++ ) {
    out[i] = out4.f[j];
  }

  return 0;
}

#endif
