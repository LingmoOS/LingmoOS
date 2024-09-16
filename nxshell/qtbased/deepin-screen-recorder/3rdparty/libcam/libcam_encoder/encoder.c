/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
#                             Add UYVY color support(Macbook iSight)            #
#           Flemming Frandsen <dren.dk@gmail.com>                               #
#                             Add VU meter OSD                                  #
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

/*******************************************************************************#
#                                                                               #
#  Encoder library                                                                #
#                                                                               #
********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/videodev2.h>
#include <errno.h>
#include <assert.h>
/* support for internationalization - i18n */
#include <locale.h>
#include <libintl.h>

#include "cameraconfig.h"
#include "gviewencoder.h"
#include "encoder.h"
#include "stream_io.h"
#include "gview.h"
#include "camview.h"
#include "load_libs.h"

#if LIBAVUTIL_VER_AT_LEAST(52,2)
#include <libavutil/channel_layout.h>
#endif

extern int verbosity;
extern int encodeenv;

/*video buffer data mutex*/
static __MUTEX_TYPE mutex = __STATIC_MUTEX_INIT;
#define __PMUTEX &mutex

static int valid_video_codecs = 0;
static int valid_audio_codecs = 0;

static int64_t last_video_pts = 0;
static int64_t last_audio_pts = 0;
static int64_t reference_pts  = 0;

static int video_frame_max_size = 0;

static int video_ring_buffer_size = 0;
static video_buffer_t *video_ring_buffer = NULL;
static int video_read_index = 0;
static int video_write_index = 0;
static int video_scheduler = 0;

static int64_t video_pause_timestamp = 0;

/*
 * set pause timestamp
 * args:
 *   value - timestamp value
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void set_video_pause_timestamp(int64_t timestamp){
    video_pause_timestamp = timestamp;
}

/*
 * get pause timestamp
 * args:
 *   value: nome
 *
 * asserts:
 *    none
 *
 * returns: pause timestamp
 */
int64_t get_video_pause_timestamp()
{
    return video_pause_timestamp;
}

/*
 * set verbosity
 * args:
 *   value - verbosity value
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void encoder_set_verbosity(int value)
{
	verbosity = value;
}

/*
 * allocate video ring buffer
 * args:
 *   video_width - video frame width (in pixels)
 *   video_height - video frame height (in pixels)
 *   fps_den - frames per sec (denominator)
 *   fps_num - frames per sec (numerator)
 *   codec_ind - video codec index (0 -raw)
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void encoder_alloc_video_ring_buffer(
	int video_width,
	int video_height,
	int fps_den,
	int fps_num,
	int codec_ind)
{
	video_ring_buffer_size = (fps_den * 3) / (fps_num * 2); /* 1.5 sec */
	if(video_ring_buffer_size < 20)
		video_ring_buffer_size = 20; /*at least 20 frames buffer*/
	video_ring_buffer = calloc(video_ring_buffer_size, sizeof(video_buffer_t));
	if(video_ring_buffer == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_alloc_video_ring_buffer): %s\n", strerror(errno));
		exit(-1);
	}

	if(codec_ind > 0)
		video_frame_max_size = (video_width * video_height * 3) / 2;
	else
		video_frame_max_size = video_width * video_height * 3; //RGB formats

	int i = 0;
	for(i = 0; i < video_ring_buffer_size; ++i)
	{
		video_ring_buffer[i].frame = calloc(video_frame_max_size, sizeof(uint8_t));
		if(video_ring_buffer[i].frame == NULL)
		{
			fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_alloc_video_ring_buffer): %s\n", strerror(errno));
			exit(-1);
		}
		video_ring_buffer[i].flag = VIDEO_BUFF_FREE;
	}
}

/*
 * clean video ring buffer
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void encoder_clean_video_ring_buffer()
{
	if(!video_ring_buffer)
		return;

	int i = 0;
	for(i = 0; i < video_ring_buffer_size; ++i)
	{
		/*Max: (yuyv) 2 bytes per pixel*/
		free(video_ring_buffer[i].frame);
	}
	free(video_ring_buffer);
	video_ring_buffer = NULL;
}

/*
 * gviewencoder constructor (called before dlopen or main)
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void /*__attribute__ ((constructor)) */gviewencoder_init()
{
    if (encodeenv != 0)
        return;

#if !LIBAVCODEC_VER_AT_LEAST(57,16)
	fprintf(stderr, "ENCODER: Error - libavcodec version not supported (minimum 57.16)\n");
	return;
#endif
#if !LIBAVCODEC_VER_AT_LEAST(58,9)
	//printf("ENCODER: constructor function called\n");
	/* register all the codecs (you can also register only the codec
	 * you wish to have smaller code)
	 */
	avcodec_register_all();
#endif
	valid_video_codecs = encoder_set_valid_video_codec_list ();
	valid_audio_codecs = encoder_set_valid_audio_codec_list ();

}

/*
 * gviewencoder destructor (called before dlclose os exit)
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void /*__attribute__ ((destructor))*/ gviewencoder_fini()
{
	if(verbosity > 1)
		printf("ENCODER: destructor function called\n");
	//make sure to clean the ring buffer
	encoder_clean_video_ring_buffer();
}

/*
 * check that a given sample format is supported by the encoder
 * args:
 *    codec - pointer to AVCodec
 *    sample_fmt - audio sample format
 *
 * assertions:
 *    none
 *
 * returns: 1 - sample format is supported; 0 - is not supported
 */
static int encoder_check_audio_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt)
{
	const enum AVSampleFormat *p = codec->sample_fmts;

	while (*p != AV_SAMPLE_FMT_NONE)
	{
		if (*p == sample_fmt)
			return 1;
		p++;
	}
	return 0;
}

/*
 * video encoder initialization for raw input
 *  (don't set a codec but set the proper codec 4cc)
 * args:
 *   enc_video_ctx - pointer to video context
 *   video_defaults - pointer to video codec default data
 *   video_width - video frame width
 *   video_height - video frame height
 *
 * asserts:
 *   encoder_ctx is not null
 *   encoder_ctx->enc_video_ctx is not null
 *
 * returns: none
 */
static void encoder_set_raw_video_input(
    encoder_context_t *encoder_ctx,
    video_codec_t *video_defaults
    )
{
	//assertions
	assert(encoder_ctx != NULL);
	assert(encoder_ctx->enc_video_ctx != NULL);

	encoder_ctx->video_codec_ind = 0;

	switch(encoder_ctx->input_format)
	{
		case V4L2_PIX_FMT_MJPEG:
			strncpy(video_defaults->compressor, "MJPG", 5);
			video_defaults->mkv_4cc = v4l2_fourcc('M','J','P','G');
			strncpy(video_defaults->mkv_codec, "V_MS/VFW/FOURCC", 25);
			encoder_ctx->enc_video_ctx->outbuf_size =  (encoder_ctx->video_width * encoder_ctx->video_height) / 2;
			encoder_ctx->enc_video_ctx->outbuf = calloc(encoder_ctx->enc_video_ctx->outbuf_size, sizeof(uint8_t));
			if(encoder_ctx->enc_video_ctx->outbuf == NULL)
			{
				fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_video_init): %s\n", strerror(errno));
				exit(-1);
			}
			break;

		case V4L2_PIX_FMT_H264:
			strncpy(video_defaults->compressor, "H264", 5);
			video_defaults->mkv_4cc = v4l2_fourcc('H','2','6','4');
			strncpy(video_defaults->mkv_codec, "V_MPEG4/ISO/AVC", 25);
			encoder_ctx->enc_video_ctx->outbuf_size = (encoder_ctx->video_width * encoder_ctx->video_height) / 2;
			encoder_ctx->enc_video_ctx->outbuf = calloc(encoder_ctx->enc_video_ctx->outbuf_size, sizeof(uint8_t));
			if(encoder_ctx->enc_video_ctx->outbuf == NULL)
			{
				fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_video_init): %s\n", strerror(errno));
				exit(-1);
			}
			break;

		default:
		{
			char fourcc[5];
			fourcc[0]= (char) encoder_ctx->input_format & 0xFF;
			fourcc[1]= (char) encoder_ctx->input_format & 0xFF00 >> 8;
			fourcc[2]= (char) encoder_ctx->input_format & 0xFF0000 >> 16;
			fourcc[3]= (char) encoder_ctx->input_format & 0xFF000000 >> 24;
			fourcc[4]='\0';

			strncpy(video_defaults->compressor, fourcc, 5);
			video_defaults->mkv_4cc = encoder_ctx->input_format; //v4l2_fourcc('Y','U','Y','2')
			strncpy(video_defaults->mkv_codec, "V_MS/VFW/FOURCC", 25);
			encoder_ctx->enc_video_ctx->outbuf_size = encoder_ctx->video_width * encoder_ctx->video_height * 3; //max of 3 bytes per pixel
			encoder_ctx->enc_video_ctx->outbuf = calloc(encoder_ctx->enc_video_ctx->outbuf_size, sizeof(uint8_t));
			if(encoder_ctx->enc_video_ctx->outbuf == NULL)
			{
				fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_video_init): %s\n", strerror(errno));
				exit(-1);
			}
			break;
		}
	}
}

/*
 * video encoder initialization
 * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: pointer to encoder video context (NULL on none)
 */
static encoder_video_context_t *encoder_video_init(encoder_context_t *encoder_ctx)
{
	//assertions
	assert(encoder_ctx != NULL);

#if !LIBAVCODEC_VER_AT_LEAST(57,16)
	fprintf(stderr, "ENCODER: Error - libavcodec version not supported (minimum 57.16)\n");
	return NULL;
#else
	if(encoder_ctx->video_codec_ind < 0)
	{
		if(verbosity > 0)
			printf("ENCODER: no video codec set - using raw (direct input)\n");

		encoder_ctx->video_codec_ind = 0;
	}

	video_codec_t *video_defaults = encoder_get_video_codec_defaults(encoder_ctx->video_codec_ind);

	if(!video_defaults)
	{
		fprintf(stderr, "ENCODER: defaults for video codec index %i not found: using raw (direct input)\n",
			encoder_ctx->video_codec_ind);
		encoder_ctx->video_codec_ind = 0;
		video_defaults = encoder_get_video_codec_defaults(encoder_ctx->video_codec_ind);
		if(!video_defaults)
		{
			/*should never happen*/
			fprintf(stderr, "ENCODER: defaults for raw video not found\n");
			return NULL;
		}
	}

	encoder_video_context_t *enc_video_ctx = calloc(1, sizeof(encoder_video_context_t));
	if(enc_video_ctx == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_video_init): %s\n", strerror(errno));
		exit(-1);
	}

	/* make sure enc_video_ctx is set in encoder_ctx */
	encoder_ctx->enc_video_ctx = enc_video_ctx;

	if(encoder_ctx->video_codec_ind == 0)
	{
		encoder_set_raw_video_input(encoder_ctx, video_defaults);
		return (enc_video_ctx);
	}

	/*
	 * alloc the video codec data
	 */
	encoder_codec_data_t *video_codec_data = calloc(1, sizeof(encoder_codec_data_t));
	if(video_codec_data == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_video_init): %s\n", strerror(errno));
		exit(-1);
	}
	/*
	 * find the video encoder
	 *   try specific codec (by name)
	 */
    video_codec_data->codec = getLoadLibsInstance()->m_avcodec_find_encoder_by_name(video_defaults->codec_name);
	/*if it fails try any codec with matching AV_CODEC_ID*/
	if(!video_codec_data->codec)
        video_codec_data->codec = getLoadLibsInstance()->m_avcodec_find_encoder(video_defaults->codec_id);

	if(!video_codec_data->codec)
	{
		/*we will use raw data so free the codec data*/
		free(video_codec_data);
		fprintf(stderr, "ENCODER: libav video codec (%i) not found - using raw input\n",video_defaults->codec_id);
		video_defaults = encoder_get_video_codec_defaults(0);
		encoder_set_raw_video_input(encoder_ctx, video_defaults);

		//alloc outbuf
		if(enc_video_ctx->outbuf_size <= 0)
			enc_video_ctx->outbuf_size = 240000;//1792
		if(enc_video_ctx->outbuf != NULL)
			free(enc_video_ctx->outbuf);
		enc_video_ctx->outbuf = calloc(enc_video_ctx->outbuf_size, sizeof(uint8_t));
		if(enc_video_ctx->outbuf == NULL)
		{
			fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_video_init): %s\n", strerror(errno));
			exit(-1);
		}

		return (enc_video_ctx);
	}

#if LIBAVCODEC_VER_AT_LEAST(53,6)
    video_codec_data->codec_context = getLoadLibsInstance()->m_avcodec_alloc_context3(video_codec_data->codec);
    #if !LIBAVCODEC_VER_AT_LEAST(60,0)
        getLoadLibsInstance()->m_avcodec_get_context_defaults3(video_codec_data->codec_context, video_codec_data->codec);
    #endif
#else
    video_codec_data->codec_context = getLoadLibsInstance()->m_avcodec_alloc_context();
    getLoadLibsInstance()->m_avcodec_get_context_defaults(video_codec_data->codec_context);
#endif

	if(video_codec_data->codec_context == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_video_init): %s\n", strerror(errno));
		exit(-1);
	}

	/*set codec defaults*/
	video_codec_data->codec_context->bit_rate = video_defaults->bit_rate;
	video_codec_data->codec_context->width = encoder_ctx->video_width;
	video_codec_data->codec_context->height = encoder_ctx->video_height;

	video_codec_data->codec_context->flags |= video_defaults->flags;
	if (video_defaults->num_threads > 0)
		video_codec_data->codec_context->thread_count = video_defaults->num_threads;
	/*
	 * mb_decision:
	 * 0 (FF_MB_DECISION_SIMPLE) Use mbcmp (default).
	 * 1 (FF_MB_DECISION_BITS)   Select the MB mode which needs the fewest bits (=vhq).
	 * 2 (FF_MB_DECISION_RD)     Select the MB mode which has the best rate distortion.
	 */
	video_codec_data->codec_context->mb_decision = video_defaults->mb_decision;
	/*use trellis quantization*/
	video_codec_data->codec_context->trellis = video_defaults->trellis;

	/*motion estimation method */
	if( video_defaults->codec_id == AV_CODEC_ID_H264 && video_defaults->me_method > 4)
		video_defaults->me_method = X264_ME_HEX;

    getAvutil()->m_av_dict_set_int(&video_codec_data->private_options, "motion-est", video_defaults->me_method, 0);
    getAvutil()->m_av_dict_set_int(&video_codec_data->private_options, "mpeg_quant", video_defaults->mpeg_quant, 0);
    getAvutil()->m_av_dict_set_int(&video_codec_data->private_options, "mepre", video_defaults->pre_me, 0);


	video_codec_data->codec_context->dia_size = video_defaults->dia;
	video_codec_data->codec_context->pre_dia_size = video_defaults->pre_dia;

	video_codec_data->codec_context->me_pre_cmp = video_defaults->me_pre_cmp;
	video_codec_data->codec_context->me_cmp = video_defaults->me_cmp;
	video_codec_data->codec_context->me_sub_cmp = video_defaults->me_sub_cmp;
	video_codec_data->codec_context->me_subpel_quality = video_defaults->subq; //NEW
	video_codec_data->codec_context->refs = video_defaults->framerefs;         //NEW
	video_codec_data->codec_context->last_predictor_count = video_defaults->last_pred;


	video_codec_data->codec_context->qmin = video_defaults->qmin;             // best detail allowed - worst compression
	video_codec_data->codec_context->qmax = video_defaults->qmax;             // worst detail allowed - best compression
	video_codec_data->codec_context->max_qdiff = video_defaults->max_qdiff;
	video_codec_data->codec_context->max_b_frames = video_defaults->max_b_frames;

	video_codec_data->codec_context->qcompress = video_defaults->qcompress;
	video_codec_data->codec_context->qblur = video_defaults->qblur;
	video_codec_data->codec_context->strict_std_compliance = FF_COMPLIANCE_NORMAL;
	video_codec_data->codec_context->codec_id = video_defaults->codec_id;

	video_codec_data->codec_context->codec_type = AVMEDIA_TYPE_VIDEO;

	video_codec_data->codec_context->pix_fmt =  video_defaults->pix_fmt; //only yuv420p available for mpeg
	if(video_defaults->fps)
		video_codec_data->codec_context->time_base = (AVRational){1, video_defaults->fps}; //use codec properties fps
	else if (encoder_ctx->fps_den >= 5)
		video_codec_data->codec_context->time_base = (AVRational){encoder_ctx->fps_num, encoder_ctx->fps_den}; //default fps (for gspca this is 1/1)
	else
		video_codec_data->codec_context->time_base = (AVRational){1,15}; //fallback to 15 fps (e.g gspca)


    if(video_defaults->gop_size > 0)
    {
        video_codec_data->codec_context->gop_size = video_defaults->gop_size;
    }
    else
    {
        video_codec_data->codec_context->gop_size = video_codec_data->codec_context->time_base.den;
    }
    switch (video_defaults->codec_id)
    {
        case AV_CODEC_ID_H264:
        {
           video_codec_data->codec_context->me_range = 16;
           //av_dict_set(&video_codec_data->private_options, "rc_lookahead", "1", 0);
           getAvutil()->m_av_dict_set(&video_codec_data->private_options, "crf", "23", 0);
           getAvutil()->m_av_dict_set(&video_codec_data->private_options, "preset", "ultrafast", 0);
           getAvutil()->m_av_dict_set(&video_codec_data->private_options,"tune", "zerolatency", 0);
        }
        break;
    case AV_CODEC_ID_HEVC:
        {
           video_codec_data->codec_context->me_range = 16;
           getAvutil()->m_av_dict_set(&video_codec_data->private_options, "crf", "28", 0);
           getAvutil()->m_av_dict_set(&video_codec_data->private_options, "preset", "ultrafast", 0);
        }
        break;
    case AV_CODEC_ID_VP8:
        {
            getAvutil()->m_av_dict_set(&video_codec_data->private_options, "quality", "good", 0);
            getAvutil()->m_av_dict_set(&video_codec_data->private_options, "cpu-used","-10",0);
            getAvutil()->m_av_dict_set(&video_codec_data->private_options, "speed","10",0);
        }
        break;
    default:
        break;
    }

	int ret = 0;
	/* open codec*/
    if ((ret = getLoadLibsInstance()->m_avcodec_open2(
		video_codec_data->codec_context,
		video_codec_data->codec,
		&video_codec_data->private_options)) < 0)
	{
		fprintf(stderr, "ENCODER: could not open video codec (%s): %i - using raw input\n", video_defaults->codec_name, ret);
		free(video_codec_data->codec_context);
		video_codec_data->codec_context = NULL;
		video_codec_data->codec = 0;
		/*we will use raw data so free the codec data*/
		free(video_codec_data);
		video_defaults = encoder_get_video_codec_defaults(0);
		encoder_set_raw_video_input(encoder_ctx, video_defaults);

		//alloc outbuf
		if(enc_video_ctx->outbuf_size <= 0)
			enc_video_ctx->outbuf_size = 240000;//1792
		if(enc_video_ctx->outbuf != NULL)
			free(enc_video_ctx->outbuf);
		enc_video_ctx->outbuf = calloc(enc_video_ctx->outbuf_size, sizeof(uint8_t));
		if(enc_video_ctx->outbuf == NULL)
		{
			fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_video_init): %s\n", strerror(errno));
			exit(-1);
		}

		return (enc_video_ctx);
	}

    video_codec_data->frame = getAvutil()->m_av_frame_alloc();

	if(video_codec_data->frame == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_video_init): %s\n", strerror(errno));
		exit(-1);
	}
	video_codec_data->frame->pts = 0;

    video_codec_data->outpkt = getLoadLibsInstance()->m_av_packet_alloc();

	if(video_codec_data->outpkt == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_video_init): %s\n", strerror(errno));
		exit(-1);
	}

	/*set the codec data in codec context*/
	enc_video_ctx->codec_data = (void *) video_codec_data;

	enc_video_ctx->tmpbuf = NULL; //no need to temp buffer input already in yu12 (yuv420p)

	enc_video_ctx->monotonic_pts = video_defaults->monotonic_pts;

	//alloc outbuf
	enc_video_ctx->outbuf_size = (encoder_ctx->video_width * encoder_ctx->video_height) / 2;
	if(enc_video_ctx->outbuf_size <= 0)
		enc_video_ctx->outbuf_size = 240000;//1792
	if(enc_video_ctx->outbuf != NULL)
		free(enc_video_ctx->outbuf);
	enc_video_ctx->outbuf = calloc(enc_video_ctx->outbuf_size, sizeof(uint8_t));
	if(enc_video_ctx->outbuf == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_video_init): %s\n", strerror(errno));
		exit(-1);
	}

	enc_video_ctx->read_df = -1;
	enc_video_ctx->write_df = -1;

	enc_video_ctx->flushed_buffers = 0;
	enc_video_ctx->flush_delayed_frames = 0;
	enc_video_ctx->flush_done = 0;

	return (enc_video_ctx);
#endif
}

/*
 * audio encoder initialization
 * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: pointer to encoder audio context (NULL on none)
 */
static encoder_audio_context_t *encoder_audio_init(encoder_context_t *encoder_ctx)
{
	//assertions
	assert(encoder_ctx != NULL);

#if !LIBAVCODEC_VER_AT_LEAST(57,16)
	fprintf(stderr, "ENCODER: Error - libavcodec version not supported (minimum 57.16)");
	return NULL;
#else

	if(encoder_ctx->audio_codec_ind < 0)
	{
		if(verbosity > 0)
			printf("ENCODER: no audio codec set\n");

		return NULL;
	}

	if(encoder_ctx->audio_channels <= 0)
	{
		if(verbosity > 0)
			printf("ENCODER: no audio channels set\n");

		return NULL;
	}

	audio_codec_t *audio_defaults = encoder_get_audio_codec_defaults(encoder_ctx->audio_codec_ind);

	if(!audio_defaults)
	{
		fprintf(stderr, "ENCODER: defaults for audio codec index %i not found\n", encoder_ctx->audio_codec_ind);
		return NULL;
	}

	/*alloc the encoder audio context*/
	encoder_audio_context_t *enc_audio_ctx = calloc(1, sizeof(encoder_audio_context_t));
	if(enc_audio_ctx == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_audio_init): %s\n", strerror(errno));
		exit(-1);
	}

	encoder_ctx->enc_audio_ctx = enc_audio_ctx;

	enc_audio_ctx->flushed_buffers = 0;
	enc_audio_ctx->flush_delayed_frames = 0;
	enc_audio_ctx->flush_done = 0;

	/*
	 * alloc the audio codec data
	 */
	encoder_codec_data_t *audio_codec_data = calloc(1, sizeof(encoder_codec_data_t));

	/*
	 * find the audio encoder
	 *   try specific codec (by name)
	 */
    audio_codec_data->codec = getLoadLibsInstance()->m_avcodec_find_encoder_by_name(audio_defaults->codec_name);
	/*if it fails try any codec with matching AV_CODEC_ID*/
	if(!audio_codec_data->codec)
        audio_codec_data->codec = getLoadLibsInstance()->m_avcodec_find_encoder(audio_defaults->codec_id);

	if(!audio_codec_data->codec)
	{
		fprintf(stderr, "ENCODER: audio codec (%i) not found\n",audio_defaults->codec_id);
		free(audio_codec_data);
		free(enc_audio_ctx);
		encoder_ctx->enc_audio_ctx = NULL;
		return NULL;
	}

#if LIBAVCODEC_VER_AT_LEAST(53,6)
    audio_codec_data->codec_context = getLoadLibsInstance()->m_avcodec_alloc_context3(audio_codec_data->codec);
    #if !LIBAVCODEC_VER_AT_LEAST(60,0)
        getLoadLibsInstance()->m_avcodec_get_context_defaults3(audio_codec_data->codec_context, audio_codec_data->codec);
    #endif
#else
    audio_codec_data->codec_context = getLoadLibsInstance()->m_avcodec_alloc_context();
    getLoadLibsInstance()->m_avcodec_get_context_defaults(audio_codec_data->codec_context);
#endif
	if(audio_codec_data->codec_context == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_audio_init): %s\n", strerror(errno));
		exit(-1);
	}

	/*defaults*/
	enc_audio_ctx->avi_4cc = audio_defaults->avi_4cc;

	audio_codec_data->codec_context->bit_rate = audio_defaults->bit_rate;
	audio_codec_data->codec_context->profile = audio_defaults->profile; /*for AAC*/

	audio_codec_data->codec_context->flags |= audio_defaults->flags;

	audio_codec_data->codec_context->sample_rate = encoder_ctx->audio_samprate;
	audio_codec_data->codec_context->channels = encoder_ctx->audio_channels;

	if(encoder_ctx->audio_channels < 2)
		audio_codec_data->codec_context->channel_layout = AV_CH_LAYOUT_MONO;
	else
		audio_codec_data->codec_context->channel_layout = AV_CH_LAYOUT_STEREO;

	audio_codec_data->codec_context->cutoff = 0; /*automatic*/

    audio_codec_data->codec_context->codec_id = audio_defaults->codec_id;

	audio_codec_data->codec_context->codec_type = AVMEDIA_TYPE_AUDIO;

	audio_codec_data->codec_context->time_base = (AVRational){1, encoder_ctx->audio_samprate};

	/*check if codec supports sample format*/
	if (!encoder_check_audio_sample_fmt(audio_codec_data->codec, audio_defaults->sample_format))
	{
		switch(audio_defaults->sample_format)
		{
			case AV_SAMPLE_FMT_S16:
				if (encoder_check_audio_sample_fmt(audio_codec_data->codec, AV_SAMPLE_FMT_S16P))
				{
					fprintf(stderr, "ENCODER: changing sample format (S16 -> S16P)\n");
					audio_defaults->sample_format = AV_SAMPLE_FMT_S16P;
				}
				else if (encoder_check_audio_sample_fmt(audio_codec_data->codec, AV_SAMPLE_FMT_FLT))
				{
					fprintf(stderr, "ENCODER: changing sample format (S16 -> FLT)\n");
					audio_defaults->sample_format = AV_SAMPLE_FMT_FLT;
				}
				else if (encoder_check_audio_sample_fmt(audio_codec_data->codec, AV_SAMPLE_FMT_FLTP))
				{
					fprintf(stderr, "ENCODER: changing sample format (S16 -> FLTP)\n");
					audio_defaults->sample_format = AV_SAMPLE_FMT_FLTP;
				}
				else
				{
					fprintf(stderr, "ENCODER: could not open audio codec: no supported sample format\n");
					free(audio_codec_data->codec_context);
					free(audio_codec_data);
					free(enc_audio_ctx);
					encoder_ctx->enc_audio_ctx = NULL;
					return NULL;
				}
				break;

			case AV_SAMPLE_FMT_FLT:
				if (encoder_check_audio_sample_fmt(audio_codec_data->codec, AV_SAMPLE_FMT_S16))
				{
					fprintf(stderr, "ENCODER: changing sample format (FLT -> S16)\n");
					audio_defaults->sample_format = AV_SAMPLE_FMT_S16;
				}
				else if (encoder_check_audio_sample_fmt(audio_codec_data->codec, AV_SAMPLE_FMT_S16P))
				{
					fprintf(stderr, "ENCODER: changing sample format (FLT -> S16P)\n");
					audio_defaults->sample_format = AV_SAMPLE_FMT_S16P;
				}
				else if (encoder_check_audio_sample_fmt(audio_codec_data->codec, AV_SAMPLE_FMT_FLTP))
				{
					fprintf(stderr, "ENCODER: changing sample format (FLT -> FLTP)\n");
					audio_defaults->sample_format = AV_SAMPLE_FMT_FLTP;
				}
				else
				{
					fprintf(stderr, "ENCODER: could not open audio codec: no supported sample format\n");
					free(audio_codec_data->codec_context);
					free(audio_codec_data);
					free(enc_audio_ctx);
					encoder_ctx->enc_audio_ctx = NULL;
					return NULL;
				}
				break;

			case AV_SAMPLE_FMT_FLTP:
				if (encoder_check_audio_sample_fmt(audio_codec_data->codec, AV_SAMPLE_FMT_S16))
				{
					fprintf(stderr, "ENCODER: changing sample format (FLTP -> S16)\n");
					audio_defaults->sample_format = AV_SAMPLE_FMT_S16;
				}
				else if (encoder_check_audio_sample_fmt(audio_codec_data->codec, AV_SAMPLE_FMT_S16P))
				{
					fprintf(stderr, "ENCODER: changing sample format (FLTP -> S16P)\n");
					audio_defaults->sample_format = AV_SAMPLE_FMT_S16P;
				}
				else if (encoder_check_audio_sample_fmt(audio_codec_data->codec, AV_SAMPLE_FMT_FLT))
				{
					fprintf(stderr, "ENCODER: changing sample format (FLTP -> FLT)\n");
					audio_defaults->sample_format = AV_SAMPLE_FMT_FLT;
				}
				else
				{
					fprintf(stderr, "ENCODER: could not open audio codec: no supported sample format\n");
					free(audio_codec_data->codec_context);
					free(audio_codec_data);
					free(enc_audio_ctx);
					encoder_ctx->enc_audio_ctx = NULL;
					return NULL;
				}
				break;
		}
	}

	audio_codec_data->codec_context->sample_fmt = audio_defaults->sample_format;

	/* open codec*/
    if (getLoadLibsInstance()->m_avcodec_open2(
		audio_codec_data->codec_context,
		audio_codec_data->codec, NULL) < 0)
	{
		fprintf(stderr, "ENCODER: could not open audio codec\n");
		free(audio_codec_data->codec_context);
		free(audio_codec_data);
		free(enc_audio_ctx);
		encoder_ctx->enc_audio_ctx = NULL;
		return NULL;
	}

	/* the codec gives us the frame size, in samples */
	int frame_size = audio_codec_data->codec_context->frame_size;
	if(frame_size <= 0)
	{
		frame_size = 1152; /*default value*/
		audio_codec_data->codec_context->frame_size = frame_size;
	}
	if(verbosity > 0)
		printf("ENCODER: Audio frame size is %d frames for selected codec\n", frame_size);

	enc_audio_ctx->monotonic_pts = audio_defaults->monotonic_pts;

	/*alloc outbuf*/
	enc_audio_ctx->outbuf_size = 240000;
	enc_audio_ctx->outbuf = calloc(enc_audio_ctx->outbuf_size, sizeof(uint8_t));
	if(enc_audio_ctx->outbuf == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_audio_init): %s\n", strerror(errno));
		exit(-1);
	}

    audio_codec_data->frame = getAvutil()->m_av_frame_alloc();

	if(audio_codec_data->frame == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_audio_init): %s\n", strerror(errno));
		exit(-1);
	}

    getAvutil()->m_av_frame_unref(audio_codec_data->frame);

    audio_codec_data->outpkt = getLoadLibsInstance()->m_av_packet_alloc();

	if(audio_codec_data->outpkt == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_audio_init): %s\n", strerror(errno));
		exit(-1);
	}

	audio_codec_data->frame->nb_samples = frame_size;
	audio_codec_data->frame->format = audio_defaults->sample_format;

	audio_codec_data->frame->channel_layout = audio_codec_data->codec_context->channel_layout;

	/*set codec data in encoder context*/
	enc_audio_ctx->codec_data = (void *) audio_codec_data;

	return (enc_audio_ctx);
#endif
}

/*
 * get the audio encoder frame size
 * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: audio encoder frame size
 *    or < 0 on error
 */
int encoder_get_audio_frame_size(encoder_context_t *encoder_ctx)
{
	/*assertions*/
	assert(encoder_ctx);
	if(encoder_ctx->enc_audio_ctx == NULL)
		return -1;

	encoder_codec_data_t *audio_codec_data = (encoder_codec_data_t *) encoder_ctx->enc_audio_ctx->codec_data;

	if(audio_codec_data == NULL)
		return -1;

	return audio_codec_data->codec_context->frame_size;

}

/*
 * get the audio encoder input sample format
 * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: audio encoder sample format
 */
int encoder_get_audio_sample_fmt(encoder_context_t *encoder_ctx)
{
	/*assertions*/
	assert(encoder_ctx);

	/*default*/
	int sample_type = GV_SAMPLE_TYPE_INT16;

	if(encoder_ctx->enc_audio_ctx == NULL)
		return sample_type;

	encoder_codec_data_t *audio_codec_data = (encoder_codec_data_t *) encoder_ctx->enc_audio_ctx->codec_data;

	if(audio_codec_data == NULL)
		return sample_type;

	switch(audio_codec_data->codec_context->sample_fmt)
	{
		case AV_SAMPLE_FMT_FLTP:
			sample_type = GV_SAMPLE_TYPE_FLOATP;
			break;
		case AV_SAMPLE_FMT_FLT:
			sample_type = GV_SAMPLE_TYPE_FLOAT;
			break;
		case AV_SAMPLE_FMT_S16P:
			sample_type = GV_SAMPLE_TYPE_INT16P;
			break;
		default:
			sample_type = GV_SAMPLE_TYPE_INT16;
			break;
	}

	return sample_type;
}

/*
 * get audio sample format max value
 * args:
 *   none
 *
 * returns the maximum audio sample format value
 */
int encoder_get_max_audio_sample_fmt()
{
	return AV_SAMPLE_FMT_NB-1;
}

/*
 * get an estimated write loop sleep time to avoid a ring buffer overrun
 * args:
 *   mode: scheduler mode:
 *      0 - linear funtion; 1 - exponencial funtion
 *   thresh: ring buffer threshold in wich scheduler becomes active:
 *      [0.2 (20%) - 0.9 (90%)]
 *   max_time - maximum scheduler time (in ms)
 *
 * asserts:
 *   none
 *
 * returns: estimate sleep time (milisec)
 */
double encoder_buff_scheduler(int mode, double thresh, double max_time)
{
	int diff_ind = 0;
	double sched_time = 0; /*in milisec*/

	__LOCK_MUTEX( __PMUTEX );
	/* try to balance buffer overrun in read/write operations */
	if(video_write_index >= video_read_index)
		diff_ind = video_write_index - video_read_index;
	else
		diff_ind = (video_ring_buffer_size - video_read_index) + video_write_index;
	__UNLOCK_MUTEX( __PMUTEX );

	/*clip ring buffer threshold*/
	if(thresh < 0.2)
		thresh = 0.2; /*20% full*/
	if(thresh > 0.9)
		thresh = 0.9; /*90% full*/

	int th = (int) lround((double) video_ring_buffer_size * thresh);

	if (diff_ind >= th)
	{
		switch(mode)
		{
			case ENCODER_SCHED_LIN: /*linear function*/
				sched_time = (double) (diff_ind - th) * (max_time/(video_ring_buffer_size - th));
				break;

			case ENCODER_SCHED_EXP: /*exponencial*/
			{
				double exp = (double) log10(max_time)/log10(video_ring_buffer_size - th);
				if(exp > 0)
					sched_time = pow(diff_ind - th, exp);
				else /*use linear function*/
					sched_time = (double) (diff_ind - th) * (max_time/(video_ring_buffer_size - th));
				break;
			}

		}
	}

	if(verbosity > 2)
		printf("ENCODER: scheduler %.2f ms (index delta %i)\n", sched_time, diff_ind);

	/*clip*/
	if(sched_time > max_time)
		sched_time = max_time;
	if(sched_time < 0)
		sched_time = 0;

	return (sched_time);
}

/*
 * get valid video codec count
 * args:
 *   none
 *
 * asserts:
 *    none
 *
 * returns: number of valid video codecs
 */
int encoder_get_valid_video_codecs()
{
	return valid_video_codecs;
}

/*
 * get valid audio codec count
 * args:
 *   none
 *
 * asserts:
 *    none
 *
 * returns: number of valid audio codecs
 */
int encoder_get_valid_audio_codecs()
{
	return valid_audio_codecs;
}

/*
 * encoder initialization
 * args:
 *   input_format - input v4l2 format (yuyv for encoding)
 *   video_codec_ind - video codec list index
 *   audio_codec_ind - audio codec list index
 *   muxer_id - file muxer:
 *        ENCODER_MUX_MKV; ENCODER_MUX_WEBM; ENCODER_MUX_AVI
 *   video_width - video frame width
 *   video_height - video frame height
 *   fps_num - fps numerator
 *   fps_den - fps denominator
 *   audio_channels- audio channels
 *   audio_samprate- audio sample rate
 *
 * asserts:
 *   none
 *
 * returns: pointer to encoder context (NULL on error)
 */
encoder_context_t *encoder_init(
	int input_format,
	int video_codec_ind,
	int audio_codec_ind,
	int muxer_id,
	int video_width,
	int video_height,
	int fps_num,
	int fps_den,
	int audio_channels,
	int audio_samprate)
{
	encoder_context_t *encoder_ctx = calloc(1, sizeof(encoder_context_t));

	if(encoder_ctx == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_get_context): %s\n", strerror(errno));
		exit(-1);
	}

	encoder_ctx->input_format = input_format;

	encoder_ctx->video_codec_ind = video_codec_ind;
	encoder_ctx->audio_codec_ind = audio_codec_ind;

	encoder_ctx->muxer_id = muxer_id;

	encoder_ctx->video_width = video_width;
	encoder_ctx->video_height = video_height;

	encoder_ctx->fps_num = fps_num;
	encoder_ctx->fps_den = fps_den;

	encoder_ctx->audio_channels = audio_channels;
	encoder_ctx->audio_samprate = audio_samprate;

	/******************* video **********************/
	encoder_video_init(encoder_ctx);

	/******************* audio **********************/
	encoder_audio_init(encoder_ctx);

	if(!encoder_ctx->enc_audio_ctx)
		encoder_ctx->audio_channels = 0; /*no audio*/

	/****************** ring buffer *****************/
	encoder_alloc_video_ring_buffer(
		video_width,
		video_height,
		fps_den,
		fps_num,
		video_codec_ind);

	return encoder_ctx;
}

/*
 * store unprocessed input video frame in video ring buffer
 * args:
 *   frame - pointer to unprocessed frame data
 *   size - frame size (in bytes)
 *   timestamp - frame timestamp (in nanosec)
 *   isKeyframe - flag if it's a key(IDR) frame
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int encoder_add_video_frame(uint8_t *frame, int size, int64_t timestamp, int isKeyframe)
{
//    cheese_print_log("encoder_add_video_frame");
	if(!video_ring_buffer)
		return -1;

	if (reference_pts == 0)
	{
		reference_pts = timestamp; /*first frame ts*/
		if(verbosity > 0)
			printf("ENCODER: ref ts = %" PRId64 "\n", timestamp);
	}
    int64_t video_pause_timestamp = get_video_pause_timestamp();
    if(video_pause_timestamp != 0)
    {
        reference_pts += video_pause_timestamp;
        set_video_pause_timestamp(0);
    }

    int64_t pts = timestamp - reference_pts;

	__LOCK_MUTEX( __PMUTEX );
	int flag = video_ring_buffer[video_write_index].flag;
	__UNLOCK_MUTEX( __PMUTEX );

	if(flag != VIDEO_BUFF_FREE)
	{
		fprintf(stderr, "ENCODER: video ring buffer full - dropping frame\n");
		return -1;
	}

	/*clip*/
	if(size > video_frame_max_size)
	{
		fprintf(stderr, "ENCODER: frame (%i bytes) larger than buffer (%i bytes): clipping\n",
			size, video_frame_max_size);

		size = video_frame_max_size;
	}
	memcpy(video_ring_buffer[video_write_index].frame, frame, size);
	video_ring_buffer[video_write_index].frame_size = size;
	video_ring_buffer[video_write_index].timestamp = pts;
	video_ring_buffer[video_write_index].keyframe = isKeyframe;

	__LOCK_MUTEX( __PMUTEX );
	video_ring_buffer[video_write_index].flag = VIDEO_BUFF_USED;
	NEXT_IND(video_write_index, video_ring_buffer_size);
	__UNLOCK_MUTEX( __PMUTEX );

	return 0;
}

/*
 * process next video frame on the ring buffer (encode and mux to file)
 * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: error code
 */
int encoder_process_next_video_buffer(encoder_context_t *encoder_ctx)
{
	/*assertions*/
	assert(encoder_ctx != NULL);

	__LOCK_MUTEX( __PMUTEX );

	int flag = video_ring_buffer[video_read_index].flag;

	__UNLOCK_MUTEX ( __PMUTEX );

	if(flag == VIDEO_BUFF_FREE)
		return 1; /*all done*/

	/*timestamp is zero indexed*/
	encoder_ctx->enc_video_ctx->pts = video_ring_buffer[video_read_index].timestamp;

	/*raw (direct input)*/
	if(encoder_ctx->video_codec_ind == 0)
	{
		/*outbuf_coded_size must already be set*/
		encoder_ctx->enc_video_ctx->outbuf_coded_size = video_ring_buffer[video_read_index].frame_size;
		if(video_ring_buffer[video_read_index].keyframe)
			encoder_ctx->enc_video_ctx->flags |= AV_PKT_FLAG_KEY;
	}

	encoder_encode_video(encoder_ctx, video_ring_buffer[video_read_index].frame);

	/*mux the frame*/
	__LOCK_MUTEX( __PMUTEX );

	video_ring_buffer[video_read_index].flag = VIDEO_BUFF_FREE;
	NEXT_IND(video_read_index, video_ring_buffer_size);

	__UNLOCK_MUTEX ( __PMUTEX );


	encoder_write_video_data(encoder_ctx);

	return 0;
}

/*
 * process all used video frames from buffer
  * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: error code
 */
int encoder_flush_video_buffer(encoder_context_t *encoder_ctx)
{
	/*assertions*/
	assert(encoder_ctx != NULL);

	__LOCK_MUTEX( __PMUTEX );
	int flag = video_ring_buffer[video_read_index].flag;
	__UNLOCK_MUTEX ( __PMUTEX );

	int buffer_count = video_ring_buffer_size;
	int flushed_frame_counter = buffer_count;

	if(verbosity > 1)
		printf("ENCODER: flushing video buffer with %i frames\n", buffer_count);

	while(flag != VIDEO_BUFF_FREE && buffer_count > 0)
	{
		buffer_count--;

		encoder_process_next_video_buffer(encoder_ctx);

		/*get next buffer flag*/
		__LOCK_MUTEX( __PMUTEX );
		flag = video_ring_buffer[video_read_index].flag;
		__UNLOCK_MUTEX ( __PMUTEX );
	}

	if(verbosity > 1)
		printf("ENCODER: processed remaining %i video frames\n", flushed_frame_counter - buffer_count);

	/*flush libav*/
	flushed_frame_counter = 0;
	encoder_ctx->enc_video_ctx->flush_delayed_frames  = 1;

	while(!encoder_ctx->enc_video_ctx->flush_done)
	{
		encoder_encode_video(encoder_ctx, NULL);
		encoder_write_video_data(encoder_ctx);
		flushed_frame_counter++;
	}

	if(verbosity > 1)
		printf("ENCODER: flushed %i delayed video frames\n", flushed_frame_counter);

	if(!buffer_count)
	{
		fprintf(stderr, "ENCODER: (flush video buffer) max processed buffers reached\n");
		return -1;
	}

	return 0;
}

/*
 * process all delayed audio frames from libavcodec
  * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: error code
 */
int encoder_flush_audio_buffer(encoder_context_t *encoder_ctx)
{
	/*assertions*/
	assert(encoder_ctx != NULL);

	/*flush libav*/
	int flushed_frame_counter = 0;
	encoder_ctx->enc_audio_ctx->flush_delayed_frames  = 1;

	while(!encoder_ctx->enc_audio_ctx->flush_done)
	{
		encoder_encode_audio(encoder_ctx, NULL);
		encoder_write_audio_data(encoder_ctx);
		flushed_frame_counter++;
	}

	if(verbosity > 1)
		printf("ENCODER: flushed %i delayed audio frames\n", flushed_frame_counter);

	return 0;
}

/*
 * process audio frame (encode and mux to file)
 * args:
 *   encoder_ctx - pointer to encoder context
 *   data - audio buffer
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: error code
 */
int encoder_process_audio_buffer(encoder_context_t *encoder_ctx, void *data)
{
	/*assertions*/
	assert(encoder_ctx != NULL);

	if(encoder_ctx->enc_audio_ctx == NULL ||
		encoder_ctx->audio_channels <= 0)
		return -1;

	encoder_encode_audio(encoder_ctx, data);

	int ret = encoder_write_audio_data(encoder_ctx);

	return ret;
}

/*
 * store the pts into the delayed frame buffer
 * args:
 *   enc_video_ctx - pointer to encoder video context
 *
 * asserts:
 *   enc_video_ctx is not null
 *
 * returns: delayed frame write index (or <0 if error)
 */
static int store_video_df_pts(encoder_video_context_t *enc_video_ctx)
{
	/*assertions*/
	assert(enc_video_ctx != NULL);

	if(enc_video_ctx->write_df < 0)
		enc_video_ctx->write_df = 0;
	else
		enc_video_ctx->write_df++;

	if(enc_video_ctx->write_df == enc_video_ctx->read_df)
	{
		fprintf(stderr, "ENCODER: Maximum of %i delayed video frames reached...\n", MAX_DELAYED_FRAMES);
		fprintf(stderr, "         write: %i read: %i\n", enc_video_ctx->write_df, enc_video_ctx->read_df);
		return -1;
	}

	if(enc_video_ctx->write_df >= MAX_DELAYED_FRAMES)
	{
		if(enc_video_ctx->read_df > 0)
			enc_video_ctx->write_df = 0; //go to start
		else
		{
			fprintf(stderr, "ENCODER: Maximum of %i delayed video frames reached...\n", MAX_DELAYED_FRAMES);
			fprintf(stderr, "         write: %i read: %i\n", enc_video_ctx->write_df, enc_video_ctx->read_df);
			enc_video_ctx->write_df = MAX_DELAYED_FRAMES -1;
			return -1;
		}
	}

	enc_video_ctx->delayed_pts[enc_video_ctx->write_df] = enc_video_ctx->pts;

	return enc_video_ctx->write_df;
}

/*
 * read the next pts in the delayed frame buffer and stores the current one
 * args:
 *   enc_video_ctx - pointer to encoder video context
 *
 * asserts:
 *   enc_video_ctx is not null
 *
 * returns: delayed frame read index (or <0 if error)
 */
static int read_video_df_pts(encoder_video_context_t *enc_video_ctx)
{
	/*assertions*/
	assert(enc_video_ctx != NULL);

	//store the current frame pts
	if(!enc_video_ctx->flush_delayed_frames)
		store_video_df_pts(enc_video_ctx);

	if(enc_video_ctx->read_df < 0)
	{
		printf("ENCODER: video codec is using %i delayed frames\n", enc_video_ctx->write_df);
		enc_video_ctx->read_df = 0;
	}
	else
		enc_video_ctx->read_df++;

	if(enc_video_ctx->read_df >= MAX_DELAYED_FRAMES)
		enc_video_ctx->read_df = 0;

	//read the delayed frame pts
	enc_video_ctx->pts = enc_video_ctx->delayed_pts[enc_video_ctx->read_df];

	if(enc_video_ctx->flush_delayed_frames && verbosity > 1)
		printf("ENCODER: video codec flushing delayed frame %i ( pts: %" PRId64 " )\n",
			enc_video_ctx->read_df, enc_video_ctx->pts);

	if(enc_video_ctx->read_df == enc_video_ctx->write_df)
	{
		printf("ENCODER: no more delayed video frames\n");
		if(enc_video_ctx->flush_delayed_frames)
			enc_video_ctx->flush_done = 1;
		enc_video_ctx->read_df = -1;
	}

	return enc_video_ctx->read_df;
}


static int libav_encode(AVCodecContext *avctx, AVPacket *pkt, AVFrame *frame, int *got_packet)
{
	int ret;

  *got_packet = 0;

	if(frame)
    {
        ret = getLoadLibsInstance()->m_avcodec_send_frame(avctx, frame);
        if (ret < 0)
            return ret; //if (ret == AVERROR(EAGAIN)) //input buffer is full
    }

  ret = getLoadLibsInstance()->m_avcodec_receive_packet(avctx, pkt);
  char str[50];
  sprintf(str,"avcode_receive_packet of ret=%d",ret);
  //cheese_print_log(str);
  if (!ret)
  	*got_packet = 1;
  if (ret == AVERROR(EAGAIN)) //output buffer is empty
  	return 0;

  return ret;
}

/*
 * encode video frame
 * args:
 *   encoder_ctx - pointer to encoder context
 *   input_frame - pointer to frame data
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: encoded buffer size
 */
int encoder_encode_video(encoder_context_t *encoder_ctx, void *input_frame)
{
	/*assertions*/
	assert(encoder_ctx != NULL);

#if !LIBAVCODEC_VER_AT_LEAST(57, 16)
	fprintf(stderr, "ENCODER: Error - libavcodec version not supported (minimum 57.16)\n");
	return 0;
#else

	encoder_video_context_t *enc_video_ctx = encoder_ctx->enc_video_ctx;

	int outsize = 0;

	if(!enc_video_ctx)
	{
		if(verbosity > 1)
			printf("ENCODER: video encoder not set\n");
		encoder_ctx->enc_video_ctx->outbuf_coded_size = outsize;
		return outsize;
	}

	/*raw - direct input no software encoding*/
	if(encoder_ctx->video_codec_ind == 0)
	{
		if(enc_video_ctx->flush_delayed_frames > 0)
		{
			enc_video_ctx->flush_done = 1;
			encoder_ctx->enc_video_ctx->outbuf_coded_size = outsize;
			return outsize;
		}
		if(input_frame == NULL)
		{
			encoder_ctx->enc_video_ctx->outbuf_coded_size = outsize;
			return outsize;
		}
		/*outbuf_coded_size must already be set*/
		outsize = enc_video_ctx->outbuf_coded_size;
        char str[100];
        sprintf(str,"enc_video_ctx->outbuf_coded_size of outsize is %d\n",outsize);
        //cheese_print_log(str);
		if(outsize > enc_video_ctx->outbuf_size)
		{
			enc_video_ctx->outbuf_size = outsize;
			if(enc_video_ctx->outbuf)
				free(enc_video_ctx->outbuf);
			enc_video_ctx->outbuf = calloc(enc_video_ctx->outbuf_size, sizeof(uint8_t));
		}
		memcpy(enc_video_ctx->outbuf, input_frame, outsize);
		enc_video_ctx->flags = 0;
		/*enc_video_ctx->flags must be set*/
		enc_video_ctx->dts = AV_NOPTS_VALUE;

		if(last_video_pts == 0)
			last_video_pts = enc_video_ctx->pts;

		enc_video_ctx->duration = enc_video_ctx->pts - last_video_pts;
		last_video_pts = enc_video_ctx->pts;
		return (outsize);
	}

	encoder_codec_data_t *video_codec_data = (encoder_codec_data_t *) enc_video_ctx->codec_data;

	if(input_frame != NULL)
		prepare_video_frame(video_codec_data, input_frame, encoder_ctx->video_width, encoder_ctx->video_height);

	if(!enc_video_ctx->monotonic_pts) //generate a real pts based on the frame timestamp
	{
		video_codec_data->frame->pts += ((enc_video_ctx->pts - last_video_pts)/1000) * 90;
		printf("ENCODER: using non-monotonic pts (this can cause encoding to fail)\n");
	}
	else  /*generate a true monotonic pts based on the codec fps*/
	{
		video_codec_data->frame->pts +=
			(video_codec_data->codec_context->time_base.num * 1000 / video_codec_data->codec_context->time_base.den) * 90;
	}

	if(enc_video_ctx->flush_delayed_frames)
	{
		if(!enc_video_ctx->flushed_buffers)
		{
            getLoadLibsInstance()->m_avcodec_flush_buffers(video_codec_data->codec_context);
			enc_video_ctx->flushed_buffers = 1;
		}
 	}

	/* encode the video */
	AVPacket *pkt = video_codec_data->outpkt;
	int got_packet = 0;
	int ret = 0;
    //if(enc_video_ctx->outbuf_size < FF_MIN_BUFFER_SIZE)
    //{
	//	av_log(avctx, AV_LOG_ERROR, "buffer smaller than minimum size\n");
    //    return -1;
    //}
 	if(!enc_video_ctx->flush_delayed_frames)
    	ret = libav_encode(
			video_codec_data->codec_context,
			pkt,
			video_codec_data->frame,
			&got_packet);
   	else
   		ret = libav_encode(
			video_codec_data->codec_context,
			pkt, NULL, /*NULL flushes the encoder buffers*/
			&got_packet);

	if(ret < 0)
	{
		fprintf(stderr, "ENCODER: Error encoding video frame: %i\n", ret);
		return ret;
	}

    char str_gotpacket[30];
    sprintf(str_gotpacket,"ret = %d, gotpacket = %d\n",ret, got_packet);
    //cheese_print_log(str_gotpacket);

	if(got_packet)
	{
		//if(pkt.pts != AV_NOPTS_VALUE)
		//	printf("ENCODER: (video) pts:%" PRId64 " dts:%" PRId64 "\n", pkt.pts, pkt.dts);
		//else
		//	printf("ENCODER: (video) no pts set\n");
		enc_video_ctx->dts = pkt->dts;
		enc_video_ctx->flags = pkt->flags;
		enc_video_ctx->duration = pkt->duration;

		if(pkt->size <= enc_video_ctx->outbuf_size)
			memcpy(enc_video_ctx->outbuf, pkt->data, pkt->size);
		else
			fprintf(stderr, "video packet size is bigger than output buffer(%i>%i)\n",pkt->size, enc_video_ctx->outbuf_size);

    	/* free any side data since we cannot return it */
    	if (pkt->side_data_elems > 0)
    	{
    		int i;
        	for (i = 0; i < pkt->side_data_elems; i++)
                getAvutil()->m_av_free(pkt->side_data[i].data);
            getAvutil()->m_av_freep(&pkt->side_data);
        	pkt->side_data_elems = 0;
    	}
    	outsize = pkt->size;

            getLoadLibsInstance()->m_av_packet_unref(pkt);
    }

	if(enc_video_ctx->flush_delayed_frames && ((outsize == 0) || !got_packet))
    	enc_video_ctx->flush_done = 1;
	else if(outsize == 0 || !got_packet) //the frame was delayed
		store_video_df_pts(enc_video_ctx);
	else if(enc_video_ctx->write_df >= 0) //we have delayed frames
		read_video_df_pts(enc_video_ctx);

	last_video_pts = enc_video_ctx->pts;

	encoder_ctx->enc_video_ctx->outbuf_coded_size = outsize;
	return (outsize);
#endif
}

/*
 * encode audio
 * args:
 *   encoder_ctx - pointer to encoder context
 *   audio_data - pointer to audio pcm data
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: encoded buffer size
 */
int encoder_encode_audio(encoder_context_t *encoder_ctx, void *audio_data)
{
	/*assertions*/
	assert(encoder_ctx != NULL);

#if !LIBAVCODEC_VER_AT_LEAST(57, 16)
	fprintf(stderr, "ENCODER: Error - libavcodec version not supported (minimum 57.16)\n");
	return outsize;
#else

	encoder_audio_context_t *enc_audio_ctx = encoder_ctx->enc_audio_ctx;

	int outsize = 0;

	if(!enc_audio_ctx)
	{
		if(verbosity > 1)
			printf("ENCODER: audio encoder not set\n");

		return outsize;
	}

	encoder_codec_data_t *audio_codec_data = (encoder_codec_data_t *) enc_audio_ctx->codec_data;

	if(enc_audio_ctx->flush_delayed_frames)
	{
		//pkt.size = 0;
		if(!enc_audio_ctx->flushed_buffers)
		{
			if(audio_codec_data)
                getLoadLibsInstance()->m_avcodec_flush_buffers(audio_codec_data->codec_context);
			enc_audio_ctx->flushed_buffers = 1;
		}
 	}

	/* encode the audio */
	/* encode the video */
	AVPacket *pkt = audio_codec_data->outpkt;
	int got_packet = 0;
	int ret = 0;

	if(!enc_audio_ctx->flush_delayed_frames)
	{
		/*number of samples per channel*/
		audio_codec_data->frame->nb_samples  = audio_codec_data->codec_context->frame_size;
		audio_codec_data->frame->pts = enc_audio_ctx->pts;

		int align = 0;

        int buffer_size = getAvutil()->m_av_samples_get_buffer_size(
			NULL,
			audio_codec_data->codec_context->channels,
			audio_codec_data->frame->nb_samples,
			audio_codec_data->codec_context->sample_fmt,
			align);

		if(buffer_size <= 0)
		{
			fprintf(stderr, "ENCODER: (encoder_encode_audio) av_samples_get_buffer_size error (%d): chan(%d) nb_samp(%d) samp_fmt(%d)\n",
				buffer_size,
				audio_codec_data->codec_context->channels,
				audio_codec_data->frame->nb_samples,
				audio_codec_data->codec_context->sample_fmt);

			return outsize;
		}


		/*set the data pointers in frame*/
        ret = getLoadLibsInstance()->m_avcodec_fill_audio_frame(
			audio_codec_data->frame,
			audio_codec_data->codec_context->channels,
			audio_codec_data->codec_context->sample_fmt,
			(const uint8_t *) audio_data,
			buffer_size,
			align);

		if(ret < 0)
		{
			fprintf(stderr, "ENCODER: (encoder_encode_audio) avcodec_fill_audio_frame error (%d): chan(%d) nb_samp(%d) samp_fmt(%d) buff(%d bytes)\n",
				ret,
				audio_codec_data->codec_context->channels,
				audio_codec_data->frame->nb_samples,
				audio_codec_data->codec_context->sample_fmt,
				buffer_size);

			return outsize;
		}

		if(!enc_audio_ctx->monotonic_pts) /*generate a real pts based on the frame timestamp*/
			audio_codec_data->frame->pts += ((enc_audio_ctx->pts - last_audio_pts)/1000) * 90;
		else  if (audio_codec_data->codec_context->time_base.den > 0) /*generate a true monotonic pts based on the codec fps*/
			audio_codec_data->frame->pts +=
				(audio_codec_data->codec_context->time_base.num*1000/audio_codec_data->codec_context->time_base.den) * 90;
		else
		{
			fprintf(stderr, "ENCODER: (encoder_encode_audio) couldn't generate a monotonic ts: time_base.den(%d)\n",
				 audio_codec_data->codec_context->time_base.den);
		}

		ret = libav_encode(
				audio_codec_data->codec_context,
				pkt,
				audio_codec_data->frame,
				&got_packet);
	}
	else
	{
		ret = libav_encode(
			audio_codec_data->codec_context,
			pkt,
			NULL, /*NULL flushes the encoder buffers*/
			&got_packet);
	}

	if(got_packet)
	{
		//if(pkt.pts != AV_NOPTS_VALUE)
		//	printf("ENCODER: (audio) pts:%" PRId64 " dts:%" PRId64 "\n", pkt.pts, pkt.dts);
		//else
		//	printf("ENCODER: (audio) no pts set\n");

		if(pkt->pts < 0) //avoid negative pts
			pkt->pts = -pkt->pts;
		enc_audio_ctx->pts = pkt->pts;
		enc_audio_ctx->dts = pkt->dts;
		enc_audio_ctx->flags = pkt->flags;
		enc_audio_ctx->duration = pkt->duration;

		if(pkt->size <= enc_audio_ctx->outbuf_size)
			memcpy(enc_audio_ctx->outbuf, pkt->data, pkt->size);
		else
			fprintf(stderr, "audio packet size is bigger than output buffer(%i>%i)\n",pkt->size, enc_audio_ctx->outbuf_size);

		/* free any side data since we cannot return it */
		//ff_packet_free_side_data(&pkt);
		if (audio_codec_data->frame &&
			audio_codec_data->frame->extended_data != audio_codec_data->frame->data)
            getAvutil()->m_av_freep(audio_codec_data->frame->extended_data);

		outsize = pkt->size;
		
        getLoadLibsInstance()->m_av_packet_unref(pkt);
	}

	last_audio_pts = enc_audio_ctx->pts;

	if(enc_audio_ctx->flush_delayed_frames && ((outsize == 0) || !got_packet))
    	enc_audio_ctx->flush_done = 1;

	enc_audio_ctx->outbuf_coded_size = outsize;
	return (outsize);
#endif
}

/*
 * close and clean encoder context
 * args:
 *   encoder_ctx - pointer to encoder context data
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void encoder_close(encoder_context_t *encoder_ctx)
{
	encoder_clean_video_ring_buffer();

	if(!encoder_ctx)
		return;

	encoder_video_context_t *enc_video_ctx = encoder_ctx->enc_video_ctx;
	encoder_audio_context_t *enc_audio_ctx = encoder_ctx->enc_audio_ctx;
	encoder_codec_data_t *video_codec_data = NULL;
	encoder_codec_data_t *audio_codec_data = NULL;

	if(encoder_ctx->h264_pps)
		free(encoder_ctx->h264_pps);

	if(encoder_ctx->h264_sps)
		free(encoder_ctx->h264_sps);

	/*close video codec*/
	if(enc_video_ctx)
	{

		video_codec_data = (encoder_codec_data_t *) enc_video_ctx->codec_data;
		if(video_codec_data)
		{
			if(!(enc_video_ctx->flushed_buffers))
			{
                getLoadLibsInstance()->m_avcodec_flush_buffers(video_codec_data->codec_context);
				enc_video_ctx->flushed_buffers = 1;
			}

            getLoadLibsInstance()->m_avcodec_close(video_codec_data->codec_context);
                free(video_codec_data->codec_context);

            getAvutil()->m_av_dict_free(&(video_codec_data->private_options));

			if(video_codec_data->frame)
                getAvutil()->m_av_frame_free(&video_codec_data->frame);

			if(video_codec_data->outpkt)
                getLoadLibsInstance()->m_av_packet_free(&video_codec_data->outpkt);

			free(video_codec_data);
		}

		if(enc_video_ctx->priv_data)
			free(enc_video_ctx->priv_data);
		if(enc_video_ctx->tmpbuf)
			free(enc_video_ctx->tmpbuf);
		if(enc_video_ctx->outbuf)
			free(enc_video_ctx->outbuf);

		free(enc_video_ctx);
	}

	/*close audio codec*/
	if(enc_audio_ctx)
	{
        //测试video时长是否正常
        printf("video_duration:%d",enc_audio_ctx->duration);
		audio_codec_data = (encoder_codec_data_t *) enc_audio_ctx->codec_data;
		if(audio_codec_data)
		{
            getLoadLibsInstance()->m_avcodec_flush_buffers(audio_codec_data->codec_context);

            getLoadLibsInstance()->m_avcodec_close(audio_codec_data->codec_context);
			free(audio_codec_data->codec_context);

			if(audio_codec_data->frame)
                getAvutil()->m_av_frame_free(&audio_codec_data->frame);

			if(audio_codec_data->outpkt)
                getLoadLibsInstance()->m_av_packet_free(&audio_codec_data->outpkt);

			free(audio_codec_data);
		}

		if(enc_audio_ctx->priv_data)
			free(enc_audio_ctx->priv_data);
		if(enc_audio_ctx->outbuf)
			free(enc_audio_ctx->outbuf);

		free(enc_audio_ctx);
	}

	free(encoder_ctx);

	/*reset static data*/
    set_video_pause_timestamp(0);
    set_video_timestamptmp(0);
	last_video_pts = 0;
	last_audio_pts = 0;
	reference_pts  = 0;

	video_frame_max_size = 0;

	video_ring_buffer_size = 0;
	video_ring_buffer = NULL;
	video_read_index = 0;
	video_write_index = 0;
	video_scheduler = 0;
}
