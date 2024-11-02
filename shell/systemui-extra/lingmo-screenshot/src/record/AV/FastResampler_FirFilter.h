/*
 * Copyright (c) 2012-2020 Maarten Baert <maarten-baert@hotmail.com>

 * This file is part of Lingmo-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef FASTRESAMPLER_FIRFILTER_H
#define FASTRESAMPLER_FIRFILTER_H

#include "Global.h"

typedef void (*FirFilter2Ptr)(unsigned int, unsigned int, float*, float*, float, float*, float*);

void FastResampler_FirFilter2_C1_Fallback(unsigned int channels, unsigned int filter_length, float* coef1, float* coef2, float frac, float* input, float* output);
void FastResampler_FirFilter2_C2_Fallback(unsigned int channels, unsigned int filter_length, float* coef1, float* coef2, float frac, float* input, float* output);
void FastResampler_FirFilter2_Cn_Fallback(unsigned int channels, unsigned int filter_length, float* coef1, float* coef2, float frac, float* input, float* output);

#if SSR_USE_X86_ASM
void FastResampler_FirFilter2_C1_SSE2(unsigned int channels, unsigned int filter_length, float* coef1, float* coef2, float frac, float* input, float* output);
void FastResampler_FirFilter2_C2_SSE2(unsigned int channels, unsigned int filter_length, float* coef1, float* coef2, float frac, float* input, float* output);
void FastResampler_FirFilter2_Cn_SSE2(unsigned int channels, unsigned int filter_length, float* coef1, float* coef2, float frac, float* input, float* output);
#endif


#endif // FASTRESAMPLER_FIRFILTER_H
