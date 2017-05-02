#ifdef __SSE2__
#define USE_SSE2
#endif

#ifdef USE_SSE2

#include <xmmintrin.h>

int VectorMathSin( float *out, const float *in, const unsigned int len );
int VectorMathExp( float *out, const float *in, const unsigned int len );
int VectorMathAdd( float *out, const float *in1, const float *in2, unsigned int len );
int VectorMathScale( float *out, float scalar, const float *in, const unsigned int len );

#define ALIGN16_BEG
#define ALIGN16_END __attribute__((aligned(16)))

/* __m128 is ugly to write */
typedef __m128 v4sf;  // vector of 4 float (sse1)

typedef __m128i v4si; // vector of 4 int (sse2)

typedef ALIGN16_BEG union {
  float f[4];
  int i[4];
  v4sf  v;
  v4si vi;
} ALIGN16_END V4SF;

v4sf sin_ps(v4sf x);
v4sf exp_ps(v4sf x);

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

#endif
