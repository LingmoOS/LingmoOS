/* Single-precision (Advanced SIMD) sinpi function

   Copyright (C) 2024 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include "v_math.h"
#include "poly_advsimd_f32.h"

static const struct data
{
  float32x4_t poly[6];
} data = {
  /* Taylor series coefficents for sin(pi * x).  */
  .poly = { V4 (0x1.921fb6p1f), V4 (-0x1.4abbcep2f), V4 (0x1.466bc6p1f),
	    V4 (-0x1.32d2ccp-1f), V4 (0x1.50783p-4f), V4 (-0x1.e30750p-8f) },
};

#if WANT_SIMD_EXCEPT
# define TinyBound v_u32 (0x30000000) /* asuint32(0x1p-31f).  */
# define Thresh v_u32 (0x1f000000)    /* asuint32(0x1p31f) - TinyBound.  */

static float32x4_t VPCS_ATTR NOINLINE
special_case (float32x4_t x, float32x4_t y, uint32x4_t odd, uint32x4_t cmp)
{
  /* Fall back to scalar code.  */
  y = vreinterpretq_f32_u32 (veorq_u32 (vreinterpretq_u32_f32 (y), odd));
  return v_call_f32 (sinpif, x, y, cmp);
}
#endif

/* Approximation for vector single-precision sinpi(x)
    Maximum Error 3.03 ULP:
    _ZGVnN4v_sinpif(0x1.c597ccp-2) got 0x1.f7cd56p-1
				  want 0x1.f7cd5p-1.  */
float32x4_t VPCS_ATTR NOINLINE V_NAME_F1 (sinpi) (float32x4_t x)
{
  const struct data *d = ptr_barrier (&data);

#if WANT_SIMD_EXCEPT
  uint32x4_t ir = vreinterpretq_u32_f32 (vabsq_f32 (x));
  uint32x4_t cmp = vcgeq_u32 (vsubq_u32 (ir, TinyBound), Thresh);

  /* When WANT_SIMD_EXCEPT = 1, special lanes should be set to 0
     to avoid them under/overflowing and throwing exceptions.  */
  float32x4_t r = v_zerofy_f32 (x, cmp);
#else
  float32x4_t r = x;
#endif

  /* If r is odd, the sign of the result should be inverted.  */
  uint32x4_t odd
      = vshlq_n_u32 (vreinterpretq_u32_s32 (vcvtaq_s32_f32 (r)), 31);

  /* r = x - rint(x). Range reduction to -1/2 .. 1/2.  */
  r = vsubq_f32 (r, vrndaq_f32 (r));

  /* Pairwise Horner approximation for y = sin(r * pi).  */
  float32x4_t r2 = vmulq_f32 (r, r);
  float32x4_t r4 = vmulq_f32 (r2, r2);
  float32x4_t y = vmulq_f32 (v_pw_horner_5_f32 (r2, r4, d->poly), r);

#if WANT_SIMD_EXCEPT
  if (__glibc_unlikely (v_any_u32 (cmp)))
    return special_case (x, y, odd, cmp);
#endif

  return vreinterpretq_f32_u32 (veorq_u32 (vreinterpretq_u32_f32 (y), odd));
}

libmvec_hidden_def (V_NAME_F1 (sinpi))
HALF_WIDTH_ALIAS_F1 (sinpi)
