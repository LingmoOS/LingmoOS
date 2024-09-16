/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#                                                                               #
# This program is free software; you can redistribute it and/or modify          #
# it under the terms of the GNU General Public License as published by          #
# the Free Software Foundation; either version 2 of the License, or             #
# (at your option) any later version.                                           #
#                                                                               #
# This program is distributed in the hope that it will be useful,               #
# but WITHOUT ANY WARRANTY; without even the implied warranty of                #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 #
# GNU General Public License for more details.                                  #
#                                                                               #
# You should have received a copy of the GNU General Public License             #
# along with this program; if not, write to the Free Software                   #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA     #
#                                                                               #
********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
/* support for internationalization - i18n */
#include <locale.h>
#include <libintl.h>

#include "cameraconfig.h"
#include "gviewaudio.h"
#include "audio.h"
#include "gview.h"

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

extern int verbosity;

/*----------- structs for audio effects ------------*/

/*data for Butterworth filter (LP or HP)*/
typedef struct _fx_filt_data_t
{
	sample_t buff_in1[2];
	sample_t buff_in2[2];
	sample_t buff_out1[2];
	sample_t buff_out2[2];
	float c;
	float a1;
	float a2;
	float a3;
	float b1;
	float b2;
} fx_filt_data_t;

/*data for Comb4 filter*/
typedef struct _fx_comb4_data_t
{
	int buff_size1;
	int buff_size2;
	int buff_size3;
	int buff_size4;

	sample_t *CombBuff10; // four parallel  comb filters - first channel
	sample_t *CombBuff11; // four parallel comb filters - second channel
	sample_t *CombBuff20; // four parallel  comb filters - first channel
	sample_t *CombBuff21; // four parallel comb filters - second channel
	sample_t *CombBuff30; // four parallel  comb filters - first channel
	sample_t *CombBuff31; // four parallel comb filters - second channel
	sample_t *CombBuff40; // four parallel  comb filters - first channel
	sample_t *CombBuff41; // four parallel comb filters - second channel

	int CombIndex1; //comb filter 1 index
	int CombIndex2; //comb filter 2 index
	int CombIndex3; //comb filter 3 index
	int CombIndex4; //comb filter 4 index
} fx_comb4_data_t;

/* data for delay*/
typedef struct _fx_delay_data_t
{
	int buff_size;
	sample_t *delayBuff1; // delay buffer 1 - first channel
	sample_t *delayBuff2; // delay buffer 2 - second channel (stereo)
	int delayIndex; // delay buffer index
} fx_delay_data_t;

/* data for WahWah effect*/
typedef struct _fx_wah_data_t
{
	float lfoskip;
	unsigned long skipcount;
	float xn1;
	float xn2;
	float yn1;
	float yn2;
	float b0;
	float b1;
	float b2;
	float a0;
	float a1;
	float a2;
	float phase;
} fx_wah_data_t;

typedef struct _fx_rate_data_t
{
	sample_t *rBuff1;
	sample_t *rBuff2;
	sample_t *wBuff1;
	sample_t *wBuff2;
	int wSize;
	int numsamples;
} fx_rate_data_t;

typedef struct _audio_fx_t
{
	fx_delay_data_t *ECHO;
	fx_delay_data_t *AP1;
	fx_comb4_data_t *COMB4;
	fx_filt_data_t  *HPF;
	fx_filt_data_t  *LPF1;
	fx_rate_data_t  *RT1;
	fx_wah_data_t   *wahData;
} audio_fx_t;

/*audio fx data*/
static audio_fx_t *aud_fx = NULL;

/*
 * initialize audio fx data
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
static void audio_fx_init ()
{
	aud_fx = calloc(1, sizeof(audio_fx_t));
	if(aud_fx == NULL)
	{
		fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_fx_init): %s\n", strerror(errno));
		exit(-1);
	}
	/*Echo effect data */
	aud_fx->ECHO = NULL;
	/* 4 parallel comb filters data*/
	aud_fx->COMB4 = NULL;
	/*all pass 1 filter data*/
	aud_fx->AP1 = NULL;
	/*WahWah effect data*/
	aud_fx->wahData = NULL;
	/*high pass filter data*/
	aud_fx->HPF = NULL;
	/*rate transposer*/
	aud_fx->RT1 = NULL;
	/*low pass filter*/
	aud_fx->LPF1 = NULL;
}

/*
 * clip float samples [-1.0 ; 1.0]
 * args:
 *   in - float sample
 *
 * asserts:
 *   none
 *
 * returns: float sample
 */
static float clip_float (float in)
{
	in = (in < -1.0) ? -1.0 : (in > 1.0) ? 1.0 : in;

	return in;
}

/*
 * Butterworth Filter for HP or LP
 * out(n) = a1 * in + a2 * in(n-1) + a3 * in(n-2) - b1*out(n-1) - b2*out(n-2)
 * args:
 *   FILT - pointer to fx_filt_data_t
 *   Buff - sampe buffer
 *   NumSamples - samples in buffer
 *   channels - number of audio channels
 */
static void Butt(fx_filt_data_t *FILT,
	sample_t *Buff,
	int NumSamples,
	int channels)
{
	int index = 0;

	for (index = 0; index < NumSamples; index = index + channels)
	{
		sample_t out = FILT->a1 * Buff[index] + FILT->a2 * FILT->buff_in1[0] +
			FILT->a3 * FILT->buff_in1[1] - FILT->b1 * FILT->buff_out1[0] -
			FILT->b2 * FILT->buff_out1[1];
		FILT->buff_in1[1] = FILT->buff_in1[0]; //in(n-2) = in(n-1)
		FILT->buff_in1[0] = Buff[index]; // in(n-1) = in
		FILT->buff_out1[1] = FILT->buff_out1[0]; //out(n-2) = out(n-1)
		FILT->buff_out1[0] = out; //out(n-1) = out

		Buff[index] = clip_float(out);
		/*process second channel*/
		if(channels > 1)
		{
			out = FILT->a1 * Buff[index+1] + FILT->a2 * FILT->buff_in2[0] +
				FILT->a3 * FILT->buff_in2[1] - FILT->b1 * FILT->buff_out2[0] -
				FILT->b2 * FILT->buff_out2[1];
			FILT->buff_in2[1] = FILT->buff_in2[0]; //in(n-2) = in(n-1)
			FILT->buff_in2[0] = Buff[index+1]; // in(n-1) = in
			FILT->buff_out2[1] = FILT->buff_out2[0]; //out(n-2) = out(n-1)
			FILT->buff_out2[0] = out; //out(n-1) = out

			Buff[index+1] = clip_float(out);
		}
	}
}

/*
 * HP Filter: out(n) = a1 * in + a2 * in(n-1) + a3 * in(n-2) - b1*out(n-1) - b2*out(n-2)
 * f - cuttof freq., from ~0 Hz to SampleRate/2 - though many synths seem to filter only  up to SampleRate/4
 * r  = rez amount, from sqrt(2) to ~ 0.1
 *
 *  c = tan(pi * f / sample_rate);
 *  a1 = 1.0 / ( 1.0 + r * c + c * c);
 *  a2 = -2*a1;
 *  a3 = a1;
 *  b1 = 2.0 * ( c*c - 1.0) * a1;
 *  b2 = ( 1.0 - r * c + c * c) * a1;
 * args:
 *   audio_ctx - pointer to audio context
 *   data -pointer to audio buffer to be processed
 *   cutoff_freq - filter cut off frequency
 *   res - rez amount
 *
 * asserts:
 *    none
 *
 * returns: none
 */
static void HPF(audio_context_t *audio_ctx,
	sample_t *data,
	int cutoff_freq,
	float res)
{
	if(aud_fx->HPF == NULL)
	{
		float inv_samprate = 1.0 / audio_ctx->samprate;
		aud_fx->HPF = calloc(1, sizeof(fx_filt_data_t));
		if(aud_fx->HPF == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (HPF): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->HPF->c = tan(M_PI * cutoff_freq * inv_samprate);
		aud_fx->HPF->a1 = 1.0 / (1.0 + (res * aud_fx->HPF->c) + (aud_fx->HPF->c * aud_fx->HPF->c));
		aud_fx->HPF->a2 = -2.0 * aud_fx->HPF->a1;
		aud_fx->HPF->a3 = aud_fx->HPF->a1;
		aud_fx->HPF->b1 = 2.0 * ((aud_fx->HPF->c * aud_fx->HPF->c) - 1.0) * aud_fx->HPF->a1;
		aud_fx->HPF->b2 = (1.0 - (res * aud_fx->HPF->c) + (aud_fx->HPF->c * aud_fx->HPF->c)) * aud_fx->HPF->a1;
	}

	Butt(aud_fx->HPF, data, audio_ctx->capture_buff_size, audio_ctx->channels);
}

/*
 * LP Filter: out(n) = a1 * in + a2 * in(n-1) + a3 * in(n-2) - b1*out(n-1) - b2*out(n-2)
 * f - cuttof freq., from ~0 Hz to SampleRate/2 -
 *     though many synths seem to filter only  up to SampleRate/4
 * r  = rez amount, from sqrt(2) to ~ 0.1
 *
 * c = 1.0 / tan(pi * f / sample_rate);
 * a1 = 1.0 / ( 1.0 + r * c + c * c);
 * a2 = 2* a1;
 * a3 = a1;
 * b1 = 2.0 * ( 1.0 - c*c) * a1;
 * b2 = ( 1.0 - r * c + c * c) * a1;
 *
 * args:
 *   audio_ctx - pointer to audio context
 *   data -pointer to audio buffer to be processed
 *   cutoff_freq - filter cut off frequency
 *   res - rez amount
 *
 * asserts:
 *    none
 *
 * returns: none
 */
static void LPF(audio_context_t *audio_ctx,
	sample_t *data,
	float cutoff_freq,
	float res)
{
	if(aud_fx->LPF1 == NULL)
	{
		aud_fx->LPF1 = calloc(1, sizeof(fx_filt_data_t));
		if(aud_fx->LPF1 == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (LPF): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->LPF1->c = 1.0 / tan(M_PI * cutoff_freq / audio_ctx->samprate);
		aud_fx->LPF1->a1 = 1.0 / (1.0 + (res * aud_fx->LPF1->c) + (aud_fx->LPF1->c * aud_fx->LPF1->c));
		aud_fx->LPF1->a2 = 2.0 * aud_fx->LPF1->a1;
		aud_fx->LPF1->a3 = aud_fx->LPF1->a1;
		aud_fx->LPF1->b1 = 2.0 * (1.0 - (aud_fx->LPF1->c * aud_fx->LPF1->c)) * aud_fx->LPF1->a1;
		aud_fx->LPF1->b2 = (1.0 - (res * aud_fx->LPF1->c) + (aud_fx->LPF1->c * aud_fx->LPF1->c)) * aud_fx->LPF1->a1;
	}

	Butt(aud_fx->LPF1, data, audio_ctx->capture_buff_size, audio_ctx->channels);
}

/* Non-linear amplifier with soft distortion curve.
 * args:
 *   input - sample input
 *
 * asserts:
 *   none
 *
 * returns: processed sample
 */
static sample_t CubicAmplifier( sample_t input )
{
	sample_t out;
	float temp;
	if( input < 0 ) /*silence*/
	{

		temp = input + 1.0f;
		out = (temp * temp * temp) - 1.0f;
	}
	else
	{
		temp = input - 1.0f;
		out = (temp * temp * temp) + 1.0f;
	}
	return clip_float(out);
}

/*
 * four paralell Comb filters for reverb
 * args:
 *   audio_ctx - audio context
 *   data - audio buffer to be processed
 *   delay1_ms - delay for filter 1
 *   delay2_ms - delay for filter 2
 *   delay3_ms - delay for filter 3
 *   delay4_ms - delay for filter 4
 *   gain1 - feed gain for filter 1
 *   gain2 - feed gain for filter 2
 *   gain3 - feed gain for filter 3
 *   gain4 - feed gain for filter 4
 *   in_gain - input line gain
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void CombFilter4 (audio_context_t *audio_ctx,
	sample_t *data,
	int delay1_ms,
	int delay2_ms,
	int delay3_ms,
	int delay4_ms,
	float gain1,
	float gain2,
	float gain3,
	float gain4,
	float in_gain)
{
	int samp=0;
	/*buff_size in samples*/

	if (aud_fx->COMB4 == NULL)
	{
		aud_fx->COMB4 = calloc(1, sizeof(fx_comb4_data_t));
		if(aud_fx->COMB4 == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (CombFilter4): %s\n", strerror(errno));
			exit(-1);
		}
		/*buff_size in samples*/
		aud_fx->COMB4->buff_size1 = (int) delay1_ms * (audio_ctx->samprate * 0.001);
		aud_fx->COMB4->buff_size2 = (int) delay2_ms * (audio_ctx->samprate * 0.001);
		aud_fx->COMB4->buff_size3 = (int) delay3_ms * (audio_ctx->samprate * 0.001);
		aud_fx->COMB4->buff_size4 = (int) delay4_ms * (audio_ctx->samprate * 0.001);

		aud_fx->COMB4->CombBuff10 = calloc(aud_fx->COMB4->buff_size1, sizeof(sample_t));
		if(aud_fx->COMB4->CombBuff10 == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (CombFilter4): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->COMB4->CombBuff20 = calloc(aud_fx->COMB4->buff_size2, sizeof(sample_t));
		if(aud_fx->COMB4->CombBuff20 == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (CombFilter4): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->COMB4->CombBuff30 = calloc(aud_fx->COMB4->buff_size3, sizeof(sample_t));
		if(aud_fx->COMB4->CombBuff30 == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (CombFilter4): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->COMB4->CombBuff40 = calloc(aud_fx->COMB4->buff_size4, sizeof(sample_t));
		if(aud_fx->COMB4->CombBuff40 == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (CombFilter4): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->COMB4->CombBuff11 = NULL;
		aud_fx->COMB4->CombBuff21 = NULL;
		aud_fx->COMB4->CombBuff31 = NULL;
		aud_fx->COMB4->CombBuff41 = NULL;
		if(audio_ctx->channels > 1)
		{
			aud_fx->COMB4->CombBuff11 = calloc(aud_fx->COMB4->buff_size1, sizeof(sample_t));
			if(aud_fx->COMB4->CombBuff11 == NULL)
			{
				fprintf(stderr,"AUDIO: FATAL memory allocation failure (CombFilter4): %s\n", strerror(errno));
				exit(-1);
			}
			aud_fx->COMB4->CombBuff21 = calloc(aud_fx->COMB4->buff_size2, sizeof(sample_t));
			if(aud_fx->COMB4->CombBuff21 == NULL)
			{
				fprintf(stderr,"AUDIO: FATAL memory allocation failure (CombFilter4): %s\n", strerror(errno));
				exit(-1);
			}
			aud_fx->COMB4->CombBuff31 = calloc(aud_fx->COMB4->buff_size3, sizeof(sample_t));
			if(aud_fx->COMB4->CombBuff31 == NULL)
			{
				fprintf(stderr,"AUDIO: FATAL memory allocation failure (CombFilter4): %s\n", strerror(errno));
				exit(-1);
			}
			aud_fx->COMB4->CombBuff41 = calloc(aud_fx->COMB4->buff_size4, sizeof(sample_t));
			if(aud_fx->COMB4->CombBuff41 == NULL)
			{
				fprintf(stderr,"AUDIO: FATAL memory allocation failure (CombFilter4): %s\n", strerror(errno));
				exit(-1);
			}
		}
	}

	for(samp = 0; samp < audio_ctx->capture_buff_size; samp = samp + audio_ctx->channels)
	{
		sample_t out1 = in_gain * data[samp] +
			gain1 * aud_fx->COMB4->CombBuff10[aud_fx->COMB4->CombIndex1];
		sample_t out2 = in_gain * data[samp] +
			gain2 * aud_fx->COMB4->CombBuff20[aud_fx->COMB4->CombIndex2];
		sample_t out3 = in_gain * data[samp] +
			gain3 * aud_fx->COMB4->CombBuff30[aud_fx->COMB4->CombIndex3];
		sample_t out4 = in_gain * data[samp] +
			gain4 * aud_fx->COMB4->CombBuff40[aud_fx->COMB4->CombIndex4];

		aud_fx->COMB4->CombBuff10[aud_fx->COMB4->CombIndex1] = data[samp] +
			gain1 * aud_fx->COMB4->CombBuff10[aud_fx->COMB4->CombIndex1];
		aud_fx->COMB4->CombBuff20[aud_fx->COMB4->CombIndex2] = data[samp] +
			gain2 * aud_fx->COMB4->CombBuff20[aud_fx->COMB4->CombIndex2];
		aud_fx->COMB4->CombBuff30[aud_fx->COMB4->CombIndex3] = data[samp] +
			gain3 * aud_fx->COMB4->CombBuff30[aud_fx->COMB4->CombIndex3];
		aud_fx->COMB4->CombBuff40[aud_fx->COMB4->CombIndex4] = data[samp] +
			gain4 * aud_fx->COMB4->CombBuff40[aud_fx->COMB4->CombIndex4];

		data[samp] = clip_float(out1 + out2 + out3 + out4);

		/*if stereo process second channel */
		if(audio_ctx->channels > 1)
		{
			out1 = in_gain * data[samp+1] +
				gain1 * aud_fx->COMB4->CombBuff11[aud_fx->COMB4->CombIndex1];
			out2 = in_gain * data[samp+1] +
				gain2 * aud_fx->COMB4->CombBuff21[aud_fx->COMB4->CombIndex2];
			out3 = in_gain * data[samp+1] +
				gain3 * aud_fx->COMB4->CombBuff31[aud_fx->COMB4->CombIndex3];
			out4 = in_gain * data[samp+1] +
				gain4 * aud_fx->COMB4->CombBuff41[aud_fx->COMB4->CombIndex4];

			aud_fx->COMB4->CombBuff11[aud_fx->COMB4->CombIndex1] = data[samp+1] +
				gain1 * aud_fx->COMB4->CombBuff11[aud_fx->COMB4->CombIndex1];
			aud_fx->COMB4->CombBuff21[aud_fx->COMB4->CombIndex2] = data[samp+1] +
				gain2 * aud_fx->COMB4->CombBuff21[aud_fx->COMB4->CombIndex2];
			aud_fx->COMB4->CombBuff31[aud_fx->COMB4->CombIndex3] = data[samp+1] +
				gain3 * aud_fx->COMB4->CombBuff31[aud_fx->COMB4->CombIndex3];
			aud_fx->COMB4->CombBuff41[aud_fx->COMB4->CombIndex4] = data[samp+1] +
				gain4 * aud_fx->COMB4->CombBuff41[aud_fx->COMB4->CombIndex4];

			data[samp+1] = clip_float(out1 + out2 + out3 + out4);
		}

		if(++(aud_fx->COMB4->CombIndex1) >= aud_fx->COMB4->buff_size1) aud_fx->COMB4->CombIndex1=0;
		if(++(aud_fx->COMB4->CombIndex2) >= aud_fx->COMB4->buff_size2) aud_fx->COMB4->CombIndex2=0;
		if(++(aud_fx->COMB4->CombIndex3) >= aud_fx->COMB4->buff_size3) aud_fx->COMB4->CombIndex3=0;
		if(++(aud_fx->COMB4->CombIndex4) >= aud_fx->COMB4->buff_size4) aud_fx->COMB4->CombIndex4=0;
	}
}

/*
 * All pass filter
 * args:
 *   AP - pointer to fx_delay_data_t
 *   Buff -pointer to sample buffer
 *   NumSamples - number of samples in buffer
 *   channels -number of audio channels
 *   gain- filter gain
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void all_pass (fx_delay_data_t *AP,
	sample_t *Buff,
	int NumSamples,
	int channels,
	float gain)
{
	int samp = 0;
	float inv_gain = 1.0 / gain;

	for(samp = 0; samp < NumSamples; samp += channels)
	{
		AP->delayBuff1[AP->delayIndex] = Buff[samp] +
			(gain * AP->delayBuff1[AP->delayIndex]);
		Buff[samp] = ((AP->delayBuff1[AP->delayIndex] * (1 - gain*gain)) -
			Buff[samp]) * inv_gain;
		if(channels > 1)
		{
			AP->delayBuff2[AP->delayIndex] = Buff[samp+1] +
				(gain * AP->delayBuff2[AP->delayIndex]);
			Buff[samp+1] = ((AP->delayBuff2[AP->delayIndex] * (1 - gain*gain)) -
				Buff[samp+1]) * inv_gain;
		}

		if(++(AP->delayIndex) >= AP->buff_size) AP->delayIndex=0;
	}
}

/*
 * All pass for reverb
 * args:
 * 	 audio_ctx - audio context
 *   data - audio buffer to be processed
 *   delay_ms - delay in ms
 *   gain - filter gain
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void all_pass1 (audio_context_t *audio_ctx,
	sample_t *data,
	int delay_ms,
	float gain)
{
	if(aud_fx->AP1 == NULL)
	{
		aud_fx->AP1 = calloc(1, sizeof(fx_delay_data_t));
		if(aud_fx->AP1 == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (all_pass1): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->AP1->buff_size = (int) delay_ms  * (audio_ctx->samprate * 0.001);
		aud_fx->AP1->delayBuff1 = calloc(aud_fx->AP1->buff_size, sizeof(sample_t));
		if(aud_fx->AP1->delayBuff1 == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (all_pass1): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->AP1->delayBuff2 = NULL;
		if(audio_ctx->channels > 1)
		{
			aud_fx->AP1->delayBuff2 = calloc(aud_fx->AP1->buff_size, sizeof(sample_t));
			if(aud_fx->AP1->delayBuff2 == NULL)
			{
				fprintf(stderr,"AUDIO: FATAL memory allocation failure (all_pass1): %s\n", strerror(errno));
				exit(-1);
			}
		}
	}

	all_pass (aud_fx->AP1, data,
		audio_ctx->capture_buff_size, audio_ctx->channels, gain);
}

/*
 * reduce number of samples with linear interpolation
 *    rate - rate of samples to remove [1,...[
 *    rate = 1-> XXX (splits channels) 2 -> X0X0X  3 -> X00X00X 4 -> X000X000X
 * args:
 *   RT - pointer to fx_rate_data_t
 *   Buff - pointer to sample buffer
 *   rate - rate of samples to remove
 *   NumSamples - samples in buffer
 *   channels - audio channels
 *
 * asserts:
 *
 * returns: none
 */
static void change_rate_less(fx_rate_data_t *RT,
	sample_t *Buff,
	int rate,
	int NumSamples,
	int channels)
{
	int samp = 0;
	int n = 0, i = 0;

	for (samp = 0; samp < NumSamples; samp += channels)
	{
		if (n==0)
		{
			RT->rBuff1[i] = Buff[samp];
			if(channels > 1)
				RT->rBuff2[i] = Buff[samp + 1];

			i++;
		}
		if(++n >= rate) n=0;
	}
	RT->numsamples = i;
}

/*
 * increase audio tempo by adding audio windows of wtime_ms in given rate
 *   rate: 2 -> [w1..w2][w1..w2][w2..w3][w2..w3]  3-> [w1..w2][w1..w2][w1..w2][w2..w3][w2..w3][w2..w3]
 * args:
 *   audio_ctx - audio context
 *   data - audio buffer to be processed
 *   rate -rate of added windows
 *   wtime_ms - window time in ms
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void change_tempo_more(audio_context_t *audio_ctx,
	sample_t *data,
	int rate,
	int	wtime_ms)
{
	int samp = 0;
	int i = 0;
	int r = 0;
	int index = 0;

	if(aud_fx->RT1->wBuff1 == NULL)
	{
		aud_fx->RT1->wSize  = wtime_ms * audio_ctx->samprate * 0.001;
		aud_fx->RT1->wBuff1 = calloc(aud_fx->RT1->wSize, sizeof(sample_t));
		if(aud_fx->RT1->wBuff1 == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (change_tempo_more): %s\n", strerror(errno));
			exit(-1);
		}
		if (audio_ctx->channels >1)
		{
			aud_fx->RT1->wBuff2 = calloc(aud_fx->RT1->wSize, sizeof(sample_t));
			if(aud_fx->RT1->wBuff2 == NULL)
			{
				fprintf(stderr,"AUDIO: FATAL memory allocation failure (change_tempo_more): %s\n", strerror(errno));
				exit(-1);
			}
		}
	}

	//printf("samples  = %i\n", data->RT1->numsamples);
	for(samp = 0; samp < aud_fx->RT1->numsamples; samp++)
	{
		aud_fx->RT1->wBuff1[i] = aud_fx->RT1->rBuff1[samp];
		if(audio_ctx->channels > 1)
			aud_fx->RT1->wBuff2[i] = aud_fx->RT1->rBuff2[samp];

		if((++i) > aud_fx->RT1->wSize)
		{
			for (r = 0; r < rate; r++)
			{
				for(i = 0; i < aud_fx->RT1->wSize; i++)
				{
					data[index] = aud_fx->RT1->wBuff1[i];
					if (audio_ctx->channels > 1)
						data[index +1] = aud_fx->RT1->wBuff2[i];
					index += audio_ctx->channels;
				}
			}
			i = 0;
		}
	}
}

#define FUZZ(x) CubicAmplifier(CubicAmplifier(CubicAmplifier(CubicAmplifier(x))))

/*
 * Fuzz distortion
 * args:
 *   audio_ctx - audio context
 *   data - audio buffer to be processed
 *
 * asserts:
 *    audio_ctx is not null
 *
 * returns: none
 */
static void audio_fx_fuzz (audio_context_t *audio_ctx, sample_t *data)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	int samp=0;
	for(samp = 0; samp < audio_ctx->capture_buff_size; samp++)
		data[samp] = FUZZ(data[samp]);
	HPF(audio_ctx, data, 1000, 0.9);
}

/*
 * Echo effect
 * args:
 *   audio_ctx - audio context
 *   data - audio buffer to be processed
 *   delay_ms - echo delay in ms (e.g: 300)
 *   decay - feedback gain (<1)  (e.g: 0.5)
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: none
 */
static void audio_fx_echo(audio_context_t *audio_ctx,
	sample_t *data,
	int delay_ms,
	float decay)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	int samp = 0;

	if(aud_fx->ECHO == NULL)
	{
		aud_fx->ECHO = calloc(1, sizeof(fx_delay_data_t));
		if(aud_fx->ECHO == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_fx_echo): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->ECHO->buff_size = (int) delay_ms * audio_ctx->samprate * 0.001;
		aud_fx->ECHO->delayBuff1 = calloc(aud_fx->ECHO->buff_size, sizeof(sample_t));
		if(aud_fx->ECHO->delayBuff1 == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_fx_echo): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->ECHO->delayBuff2 = NULL;
		if(audio_ctx->channels > 1)
		{
			aud_fx->ECHO->delayBuff2 = calloc(aud_fx->ECHO->buff_size, sizeof(sample_t));
			if(aud_fx->ECHO->delayBuff2 == NULL)
			{
				fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_fx_echo): %s\n", strerror(errno));
				exit(-1);
			}
		}
	}

	for(samp = 0; samp < audio_ctx->capture_buff_size; samp = samp + audio_ctx->channels)
	{
		sample_t out = (0.7 * data[samp]) +
			(0.3 * aud_fx->ECHO->delayBuff1[aud_fx->ECHO->delayIndex]);
		aud_fx->ECHO->delayBuff1[aud_fx->ECHO->delayIndex] = data[samp] +
			(aud_fx->ECHO->delayBuff1[aud_fx->ECHO->delayIndex] * decay);
		data[samp] = clip_float(out);
		/*if stereo process second channel in separate*/
		if (audio_ctx->channels > 1)
		{
			out = (0.7 * data[samp+1]) +
				(0.3 * aud_fx->ECHO->delayBuff2[aud_fx->ECHO->delayIndex]);
			aud_fx->ECHO->delayBuff2[aud_fx->ECHO->delayIndex] = data[samp] +
				(aud_fx->ECHO->delayBuff2[aud_fx->ECHO->delayIndex] * decay);
			data[samp+1] = clip_float(out);
		}

		if(++(aud_fx->ECHO->delayIndex) >= aud_fx->ECHO->buff_size) aud_fx->ECHO->delayIndex=0;
	}
}

/*
 * Reverb effect
 * args:
 *   audio_ctx - audio context
 *   data - audio buffer to be processed
 *   delay_ms - reverb delay in ms
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: none
 */
static void audio_fx_reverb (audio_context_t *audio_ctx,
	sample_t *data,
	int delay_ms)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	/*4 parallel comb filters*/
	CombFilter4 (audio_ctx, data,
		delay_ms, delay_ms - 5, delay_ms -10, delay_ms -15,
		0.55, 0.6, 0.5, 0.45, 0.7);
	
	/*all pass*/
	all_pass1 (audio_ctx, data, delay_ms, 0.75);
}

#define lfoskipsamples 30

/*
 * WahWah effect
 * 	  !!!!!!!!!!!!! IMPORTANT!!!!!!!!! :
 * 	  depth and freqofs should be from 0(min) to 1(max) !
 * 	  res should be greater than 0 !
 * args:
 *   audio_ctx - audio context
 *   data - audio buffer to be processed
 *   freq - LFO frequency (1.5)
 *   startphase - LFO startphase in RADIANS - usefull for stereo WahWah (0)
 *   depth - Wah depth (0.7)
 *   freqofs - Wah frequency offset (0.3)
 *   res - Resonance (2.5)
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: none
 */
static void audio_fx_wahwah (audio_context_t *audio_ctx,
	sample_t *data,
	float freq,
	float startphase,
	float depth,
	float freqofs,
	float res)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	float frequency, omega, sn, cs, alpha;

	if(aud_fx->wahData == NULL)
	{
		aud_fx->wahData = calloc(1, sizeof(fx_wah_data_t));
		if(aud_fx->wahData == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_fx_wahwah): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->wahData->lfoskip = freq * 2 * M_PI / audio_ctx->samprate;
		aud_fx->wahData->phase = startphase;
		/*if right channel set: phase += (float)M_PI;*/
	}

	int samp = 0;
	for(samp = 0; samp < audio_ctx->capture_buff_size; samp++)
	{
		float in = data[samp];

		if ((aud_fx->wahData->skipcount++) % lfoskipsamples == 0)
		{
			frequency = (1 + cos(aud_fx->wahData->skipcount * aud_fx->wahData->lfoskip + aud_fx->wahData->phase)) * 0.5;
			frequency = frequency * depth * (1 - freqofs) + freqofs;
			frequency = exp((frequency - 1) * 6);
			omega = M_PI * frequency;
			sn = sin(omega);
			cs = cos(omega);
			alpha = sn / (2 * res);
			aud_fx->wahData->b0 = (1 - cs) * 0.5;
			aud_fx->wahData->b1 = 1 - cs;
			aud_fx->wahData->b2 = (1 - cs) * 0.5;
			aud_fx->wahData->a0 = 1 + alpha;
			aud_fx->wahData->a1 = -2 * cs;
			aud_fx->wahData->a2 = 1 - alpha;
		}
		float out = (aud_fx->wahData->b0 * in + aud_fx->wahData->b1 * aud_fx->wahData->xn1 +
			aud_fx->wahData->b2 * aud_fx->wahData->xn2 - aud_fx->wahData->a1 * aud_fx->wahData->yn1 -
			aud_fx->wahData->a2 * aud_fx->wahData->yn2) / aud_fx->wahData->a0;
		aud_fx->wahData->xn2 = aud_fx->wahData->xn1;
		aud_fx->wahData->xn1 = in;
		aud_fx->wahData->yn2 = aud_fx->wahData->yn1;
		aud_fx->wahData->yn1 = out;

		data[samp] = clip_float(out);
	}
}

/*
 * change pitch effect
 * args:
 *   audio_ctx - audio context
 *   data - audio buffer to be processed
 *   rate - window rate
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: none
 */
static void audio_fx_change_pitch (audio_context_t *audio_ctx,
	sample_t *data,
	int rate)
{
	if(aud_fx->RT1 == NULL)
	{
		aud_fx->RT1 = calloc(1, sizeof(fx_rate_data_t));
		if(aud_fx->RT1 == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_fx_change_pitch): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->RT1->wBuff1 = NULL;
		aud_fx->RT1->wBuff2 = NULL;
		aud_fx->RT1->rBuff1 = calloc(audio_ctx->capture_buff_size/audio_ctx->channels, sizeof(sample_t));
		if(aud_fx->RT1->rBuff1 == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_fx_change_pitch): %s\n", strerror(errno));
			exit(-1);
		}
		aud_fx->RT1->rBuff2 = NULL;
		if(audio_ctx->channels > 1)
		{
			aud_fx->RT1->rBuff2 = calloc(audio_ctx->capture_buff_size/audio_ctx->channels, sizeof(sample_t));
			if(aud_fx->RT1->rBuff2 == NULL)
			{
				fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_fx_change_pitch): %s\n", strerror(errno));
				exit(-1);
			}
		}
	}

	change_rate_less(aud_fx->RT1, data, rate, audio_ctx->capture_buff_size, audio_ctx->channels);
	change_tempo_more(audio_ctx, data, rate, 20);
	LPF(audio_ctx, data, audio_ctx->samprate * 0.25, 0.9);
}

/*
 * clean fx_delay_data_t
 * args:
 *   DELAY - pointer to fx_delay_data_t
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void close_DELAY(fx_delay_data_t *DELAY)
{
	if(DELAY != NULL)
	{
		free(DELAY->delayBuff1);
		free(DELAY->delayBuff2);
		free(DELAY);
	}
}

/*
 * clean fx_comb4_data_t
 * args:
 *   COMB4 - pointer to fx_comb4_data_t
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void close_COMB4(fx_comb4_data_t *COMB4)
{
	if(COMB4 != NULL)
	{
		free(COMB4->CombBuff10);
		free(COMB4->CombBuff20);
		free(COMB4->CombBuff30);
		free(COMB4->CombBuff40);

		free(COMB4->CombBuff11);
		free(COMB4->CombBuff21);
		free(COMB4->CombBuff31);
		free(COMB4->CombBuff41);

		free(COMB4);
	}
}

/*
 * clean fx_filt_data_t
 * args:
 *   FILT - pointer to fx_filt_data_t
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void close_FILT(fx_filt_data_t *FILT)
{
	if(FILT != NULL)
	{
		free(FILT);
	}
}

/*
 * clean fx_wah_data_t
 * args:
 *   WAH - pointer to fx_wah_data_t
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void close_WAHWAH(fx_wah_data_t *WAH)
{
	if(WAH != NULL)
	{
		free(WAH);
	}
}

/*
 * clean reverb data
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void close_reverb()
{
	close_DELAY(aud_fx->AP1);
	aud_fx->AP1 = NULL;
	close_COMB4(aud_fx->COMB4);
	aud_fx->COMB4 = NULL;
}

/*
 * clean pitch data
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void close_pitch ()
{
	if(aud_fx->RT1 != NULL)
	{
		free(aud_fx->RT1->rBuff1);
		free(aud_fx->RT1->rBuff2);
		free(aud_fx->RT1->wBuff1);
		free(aud_fx->RT1->wBuff2);
		free(aud_fx->RT1);
		aud_fx->RT1 = NULL;
		close_FILT(aud_fx->LPF1);
		aud_fx->LPF1 = NULL;
	}
}

/*
 * clean audio fx data
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void audio_fx_close()
{
	if(aud_fx == NULL)
		return;

	close_DELAY(aud_fx->ECHO);
	aud_fx->ECHO = NULL;
	close_reverb();
	close_WAHWAH(aud_fx->wahData);
	aud_fx->wahData = NULL;
	close_FILT(aud_fx->HPF);
	aud_fx->HPF = NULL;
	close_pitch();

	free(aud_fx);
	aud_fx = NULL;
}

/*
 * apply audio fx
 * args:
 *   audio_ctx - pointer to audio context
 *   proc_buff - pointer to audio buffer to process
 *   mask - or'ed fx combination
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void audio_fx_apply(audio_context_t *audio_ctx,
	sample_t *data,
	uint32_t mask)
{
	if(mask != AUDIO_FX_NONE)
    {
		if(verbosity > 2)
			printf("AUDIO: Apllying Fx (0x%x)\n", mask);
			
		if(aud_fx == NULL)
			audio_fx_init();
		
		if(mask & AUDIO_FX_ECHO)
			audio_fx_echo(audio_ctx, data, 300, 0.5);
		else
		{
			close_DELAY(aud_fx->ECHO);
			aud_fx->ECHO = NULL;
		}

		if(mask & AUDIO_FX_REVERB)
			audio_fx_reverb(audio_ctx, data, 50);
		else
			close_reverb();

		if(mask & AUDIO_FX_FUZZ)
			audio_fx_fuzz(audio_ctx, data);
		else
		{
			close_FILT(aud_fx->HPF);
			aud_fx->HPF = NULL;
		}

		if(mask & AUDIO_FX_WAHWAH)
			audio_fx_wahwah(audio_ctx, data, 1.5, 0, 0.7, 0.3, 2.5);
		else
		{
			close_WAHWAH(aud_fx->wahData);
			aud_fx->wahData = NULL;
		}

		if(mask & AUDIO_FX_DUCKY)
			audio_fx_change_pitch(audio_ctx, data, 2);
		else
			close_pitch();
	}
	else
		audio_fx_close();
}
