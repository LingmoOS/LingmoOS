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
#include <string.h>
#include <errno.h>
#include <assert.h>
/* support for internationalization - i18n */
#include <locale.h>
#include <libintl.h>

#include "gviewencoder.h"
#include "gview.h"
#include "encoder.h"
#include "load_libs.h"

extern int verbosity;

/* AAC object types index: MAIN = 1; LOW = 2; SSR = 3; LTP = 4*/
static int AAC_OBJ_TYPE[5] =
	{ FF_PROFILE_UNKNOWN, FF_PROFILE_AAC_MAIN, FF_PROFILE_AAC_LOW, FF_PROFILE_AAC_SSR, FF_PROFILE_AAC_LTP };
/*-1 = reserved; 0 = freq. is writen explictly (increases header by 24 bits)*/
static int AAC_SAMP_FREQ[16] =
	{ 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, -1, -1, 0};

/*NORMAL AAC HEADER*/
/*2 bytes: object type index(5 bits) + sample frequency index(4bits) + channels(4 bits) + flags(3 bit) */
/*default = MAIN(1)+44100(4)+stereo(2)+flags(0) = 0x0A10*/
static uint8_t AAC_ESDS[2] = {0x0A,0x10};
/* if samprate index == 15 AAC_ESDS[5]:
 * object type index(5 bits) + sample frequency index(4bits) + samprate(24bits) + channels(4 bits) + flags(3 bit)
 */


static audio_codec_t listSupCodecs[] = //list of software supported formats
{
	{
		.valid        = 1,
		.bits         = 32,
		.avi_4cc      = WAVE_FORMAT_IEEE_FLOAT,
		.mkv_codec    = "A_PCM/FLOAT/IEEE",
		.description  = N_("PCM - uncompressed (float 32 bit)"),
		.bit_rate     = 0,
		.codec_id     = AV_CODEC_ID_PCM_F32LE,
		.codec_name   = "pcm_f32le",
		.sample_format = AV_SAMPLE_FMT_FLT,
		.profile      = FF_PROFILE_UNKNOWN,
		.mkv_codpriv  = NULL,
		.codpriv_size = 0,
		.flags        = 0,
		.name         = "pcm"
	},
	{
		.valid        = 1,
		.bits         = 0,
		.monotonic_pts= 1,
		.avi_4cc      = WAVE_FORMAT_MPEG12,
		.mkv_codec    = "A_MPEG/L2",
		.description  = N_("MP2 (MPEG audio layer 2)"),
		.bit_rate     = 160000,
		.codec_id     = AV_CODEC_ID_MP2,
		.codec_name   = "mp2",
		.sample_format = AV_SAMPLE_FMT_S16,
		.profile      = FF_PROFILE_UNKNOWN,
		.mkv_codpriv  = NULL,
		.codpriv_size = 0,
		.flags        = 0,
		.name         = "mp2"
	},
	{
		.valid        = 1,
		.bits         = 0,
		.monotonic_pts= 1,
		.avi_4cc      = WAVE_FORMAT_MP3,
		.mkv_codec    = "A_MPEG/L3",
		.description  = N_("MP3 (MPEG audio layer 3)"),
		.bit_rate     = 160000,
		.codec_id     = AV_CODEC_ID_MP3,
		.codec_name   = "libmp3lame",
#if LIBAVCODEC_VER_AT_LEAST(54,31)
		.sample_format = AV_SAMPLE_FMT_FLTP,
#else
		.sample_format = AV_SAMPLE_FMT_S16,
#endif
		.profile      = FF_PROFILE_UNKNOWN,
		.mkv_codpriv  = NULL,
		.codpriv_size = 0,
		.flags        = 0,
		.name         = "mp3"
	},
	{
		.valid        = 1,
		.bits         = 0,
		.monotonic_pts= 1,
		.avi_4cc      = WAVE_FORMAT_AC3,
		.mkv_codec    = "A_AC3",
		.description  = N_("AC-3 (ATSC A/52A)"),
		.bit_rate     = 160000,
		.codec_id     = AV_CODEC_ID_AC3,
		.codec_name   = "ac3",
#if LIBAVCODEC_VER_AT_LEAST(54,31)
		.sample_format = AV_SAMPLE_FMT_FLTP,
#else
		.sample_format = AV_SAMPLE_FMT_FLT,
#endif
		.profile      = FF_PROFILE_UNKNOWN,
		.mkv_codpriv  = NULL,
		.codpriv_size = 0,
		.flags        = 0,
		.name         = "ac3"
	},
	{
		.valid        = 1,
		.bits         = 16,
		.monotonic_pts= 1,
		.avi_4cc      = WAVE_FORMAT_AAC,
		.mkv_codec    = "A_AAC",
		.description  = N_("AAC (Advanced Audio Coding)"),
		.bit_rate     = 64000,
		.codec_id     = AV_CODEC_ID_AAC,
		.codec_name   = "libvo_aacenc",
#if LIBAVCODEC_VER_AT_LEAST(54,31)
		.sample_format = AV_SAMPLE_FMT_FLTP,
#else
		.sample_format = AV_SAMPLE_FMT_S16,
#endif
		.profile      = FF_PROFILE_AAC_LOW,
		.mkv_codpriv  = AAC_ESDS,
		.codpriv_size = 2,
		.flags        = 0,
		.name         = "aac"
	},
	{
		.valid        = 1,
		.bits         = 16,
		.monotonic_pts= 1,
		.avi_4cc      = OGG_FORMAT_VORBIS,
		.mkv_codec    = "A_VORBIS",
		.description  = N_("Vorbis"),
		.bit_rate     = 64000,
		.codec_id     = AV_CODEC_ID_VORBIS,
		.codec_name   = "libvorbis",
#if LIBAVCODEC_VER_AT_LEAST(54,31)
		.sample_format = AV_SAMPLE_FMT_FLTP,
#else
		.sample_format = AV_SAMPLE_FMT_S16,
#endif
		.profile      = FF_PROFILE_UNKNOWN,
		.mkv_codpriv  =  NULL,
		.codpriv_size =  0,
		.flags        = 0,
		.name         = "vorb"
	}
};

static int get_aac_obj_ind(int profile)
{
	int i = 0;

    for (i=0; i<5; i++)
    {
        if(AAC_OBJ_TYPE[i] == profile)
        {
            break;
        }
    }
	 return i;
}

static int get_aac_samp_ind(int samprate)
{
	int i = 0;

    for (i=0; i<13; i++)
    {
        if(AAC_SAMP_FREQ[i] == samprate)
        {
            break;
        }
    }
	 if (i>12)
	 {
		printf("WARNING: invalid sample rate for AAC encoding\n");
		printf("valid(96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350)\n");
		i=4; /*default 44100*/
	 }
	 return i;
}

/*
 * get audio codec list size
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: listSupCodecs size (number of elements)
 */
int encoder_get_audio_codec_list_size()
{
	int size = sizeof(listSupCodecs)/sizeof(audio_codec_t);

	if(verbosity > 3)
		printf("ENCODER: audio codec list size:%i\n", size);

	return size;
}

/*
 * get audio codec valid list size
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: listSupCodecs valid number of elements
 */
//int encoder_get_audio_codec_valid_list_size()
//{
//	int valid_size = 0;
//
//	int i = 0;
//	for(i = 0;  i < encoder_get_audio_codec_list_size(); ++i)
//		if(listSupCodecs[i].valid)
//			valid_size++;
//
//	if(verbosity > 3)
//		printf("ENCODER: audio codec valid list size:%i\n", valid_size);
//
//	return valid_size;
//}

/*
 * return the real (valid only) codec index
 * args:
 *   codec_ind - codec list index (with non valid removed)
 *
 * asserts:
 *   none
 *
 * returns: matching listSupCodecs index
 */
static int get_real_index (int codec_ind)
{
	int i = 0;
	int ind = -1;
	for (i = 0; i < encoder_get_audio_codec_list_size(); ++i)
	{
		if(listSupCodecs[i].valid)
			ind++;
		if(ind == codec_ind)
			return i;
	}
	return (codec_ind); //should never arrive
}

/*
 * return the list codec index
 * args:
 *   real_ind - listSupCodecs index
 *
 * asserts:
 *   none
 *
 * returns: matching list index (with non valid removed)
 */
static int get_list_index (int real_index)
{
	if( real_index < 0 ||
		real_index >= encoder_get_audio_codec_list_size() ||
		!listSupCodecs[real_index].valid )
		return -1; //error: real index is not valid

	int i = 0;
	int ind = -1;
	for (i = 0; i<= real_index; ++i)
	{
		if(listSupCodecs[i].valid)
			ind++;
	}

	return (ind);
}

/*
 * returns the real codec array index
 * args:
 *   codec_id - codec id
 *
 * asserts:
 *   none
 *
 * returns: real index or -1 if none
 */
int get_audio_codec_index(int codec_id)
{
	int i = 0;
	for(i = 0; i < encoder_get_audio_codec_list_size(); ++i )
	{
		if(codec_id == listSupCodecs[i].codec_id)
			return i;
	}

	return -1;
}

/*
 * returns the list codec index
 * args:
 *   codec_id - codec id
 *
 * asserts:
 *   none
 *
 * returns: real index or -1 if none
 */
int get_audio_codec_list_index(int codec_id)
{
	return get_list_index(get_audio_codec_index(codec_id));
}

/*
 * get audio list codec entry for codec index
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: list codec entry or NULL if none
 */
audio_codec_t *encoder_get_audio_codec_defaults(int codec_ind)
{
	int real_index = get_real_index (codec_ind);

	if(real_index >= 0 && real_index < encoder_get_audio_codec_list_size())
		return (&(listSupCodecs[real_index]));
	else
	{
		fprintf(stderr, "ENCODER: (audio codec defaults) bad codec index (%i)\n", codec_ind);
		return NULL;
	}
}

/*
 * checks if the audio codec index corresponds to Vorbis (webm) codec
 * args:
 *    codec_ind - audio codec list index
 *
 * asserts:
 *    none
 *
 * returns: 1 true; 0 false
 */
int encoder_check_webm_audio_codec(int codec_ind)
{
	int real_index = get_real_index (codec_ind);

	int ret = 0;
	if(real_index >= 0 && real_index < encoder_get_audio_codec_list_size())
		ret = (listSupCodecs[real_index].codec_id == AV_CODEC_ID_VORBIS) ? 1: 0;

	return ret;
}

/*
 * get the audio codec index for Vorbis (webm) codec
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: index for Vorbis codec or -1 if error
 */
int encoder_get_webm_audio_codec_index()
{
    return get_audio_codec_list_index(AV_CODEC_ID_VORBIS);
}

/*
 * sets the valid flag in the audio codecs list
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: number of valid audio codecs in list
 */
int encoder_set_valid_audio_codec_list ()
{
	int ind = 0;
	int num_codecs = 0;
	for ( ind = 0; ind < encoder_get_audio_codec_list_size(); ++ind)
	{
        AVCodec *codec = getLoadLibsInstance()->m_avcodec_find_encoder(listSupCodecs[ind].codec_id);
		if (!codec)
		{
			printf("ENCODER: no audio codec detected for %s\n", listSupCodecs[ind].description);
			listSupCodecs[ind].valid = 0;
		}
		else num_codecs++;
	}

	return num_codecs;
}

/*
 * get audio list codec description
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: list codec entry or NULL if none
 */
const char *encoder_get_audio_codec_description(int codec_ind)
{
	int real_index = get_real_index (codec_ind);
	if(real_index >= 0 && real_index < encoder_get_audio_codec_list_size())
		return (listSupCodecs[real_index].description);
	else
	{
		fprintf(stderr, "ENCODER: (audio codec description) bad codec index (%i)\n", codec_ind);
		return NULL;
	}
}

/*
 * get audio mkv codec
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: mkv codec entry or NULL if none
 */
const char *encoder_get_audio_mkv_codec(int codec_ind)
{
	int real_index = get_real_index (codec_ind);
	if(real_index >= 0 && real_index < encoder_get_audio_codec_list_size())
		return (listSupCodecs[real_index].mkv_codec);
	else
	{
		fprintf(stderr, "ENCODER: (audio mkv codec) bad codec index (%i)\n", codec_ind);
		return NULL;
	}
}

/*
 * get audio codec bits
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: bits entry from audio codec list
 */
int encoder_get_audio_bits(int codec_ind)
{
	int real_index = get_real_index (codec_ind);
	if(real_index >= 0 && real_index < encoder_get_audio_codec_list_size())
		return (listSupCodecs[real_index].bits);
	else
	{
		fprintf(stderr, "ENCODER: (get_audio_bits) bad codec index (%i)\n", codec_ind);
		return 0;
	}
}

/*
 * get audio codec bit rate
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: bit_rate entry from audio codec list
 */
int encoder_get_audio_bit_rate(int codec_ind)
{
	int real_index = get_real_index (codec_ind);
	if(real_index >= 0 && real_index < encoder_get_audio_codec_list_size())
		return (listSupCodecs[real_index].bit_rate);
	else
	{
		fprintf(stderr, "ENCODER: (get_audio_bit_rate) bad codec index (%i)\n", codec_ind);
		return 0;
	}
}

/*
 * get the mkv codec private data
 * args:
 *    codec_ind - codec list index
 *
 * asserts:
 *    none
 *
 * returns: pointer to mkvCodecPriv data
 */
void *encoder_get_audio_mkvCodecPriv(int codec_ind)
{
	int real_index = get_real_index (codec_ind);
	if(real_index >= 0 && real_index < encoder_get_audio_codec_list_size())
		return ((void *) listSupCodecs[real_index].mkv_codpriv);
	else
	{
		fprintf(stderr, "ENCODER: (mkvCodecPriv) bad codec index (%i)\n", codec_ind);
		return NULL;
	}
}

/*
 * set the audio codec mkv private data
 * args:
 *    encoder_ctx - pointer to encoder context
 *
 * asserts:
 *    encoder_ctx is not null
 *
 * returns: mkvCodecPriv size
 */
int encoder_set_audio_mkvCodecPriv(encoder_context_t *encoder_ctx)
{
	/*assertions*/
	assert(encoder_ctx != NULL);

	/*assert audio encoder context is not null*/
	assert( encoder_ctx->enc_audio_ctx);
	encoder_codec_data_t *audio_codec_data = (encoder_codec_data_t *) encoder_ctx->enc_audio_ctx->codec_data;
	/*assert video codec data is not null*/	
	assert(audio_codec_data);

	int codec_id = audio_codec_data->codec_context->codec_id;
	int real_index = get_audio_codec_index(codec_id);


	if (codec_id == AV_CODEC_ID_AAC)
	{
		int obj_type = get_aac_obj_ind(listSupCodecs[real_index].profile);
		int sampind  = get_aac_samp_ind(encoder_ctx->audio_samprate);
		AAC_ESDS[0] = (uint8_t) ((obj_type & 0x1F) << 3 ) + ((sampind & 0x0F) >> 1);
		AAC_ESDS[1] = (uint8_t) ((sampind & 0x0F) << 7 ) + ((encoder_ctx->audio_channels & 0x0F) << 3);

		return listSupCodecs[real_index].codpriv_size; /*return size = 2 */
	}
	else if(codec_id == AV_CODEC_ID_VORBIS)
	{
		//get the 3 first header packets
		uint8_t *header_start[3];
		int header_len[3];
		int first_header_size;

		first_header_size = 30; //theora = 42
    	if (avpriv_split_xiph_headers(
			audio_codec_data->codec_context->extradata,
			audio_codec_data->codec_context->extradata_size,
				first_header_size, header_start, header_len) < 0)
        {
			fprintf(stderr, "ENCODER: vorbis codec - Extradata corrupt.\n");
			return -1;
		}

		//printf("Vorbis: header1: %i  header2: %i  header3:%i \n", header_len[0], header_len[1], header_len[2]);

		//get the allocation needed for headers size
		int header_lace_size[2];
		header_lace_size[0]=0;
		header_lace_size[1]=0;
		int i;
		for (i = 0; i < header_len[0] / 255; i++)
			header_lace_size[0]++;
		header_lace_size[0]++;
		for (i = 0; i < header_len[1] / 255; i++)
			header_lace_size[1]++;
		header_lace_size[1]++;

		int priv_data_size = 1 + //number of packets -1
						header_lace_size[0] +  //first packet size
						header_lace_size[1] +  //second packet size
						header_len[0] + //first packet header
						header_len[1] + //second packet header
						header_len[2];  //third packet header

		/*should check and clean before allocating ??*/
		encoder_ctx->enc_audio_ctx->priv_data = calloc(priv_data_size, sizeof(uint8_t));
		if(encoder_ctx->enc_audio_ctx->priv_data == NULL)
		{
			fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_set_audio_mkvCodecPriv): %s\n", strerror(errno));
			exit(-1);
		}
		//write header
		uint8_t* tmp = encoder_ctx->enc_audio_ctx->priv_data;
		*tmp++ = 0x02; //number of packets -1
		//size of head 1
		for (i = 0; i < header_len[0] / 0xff; i++)
			*tmp++ = 0xff;
		*tmp++ = header_len[0] % 0xff;
		//size of head 2
		for (i = 0; i < header_len[1] / 0xff; i++)
			*tmp++ = 0xff;
		*tmp++ = header_len[1] % 0xff;
		//add headers
		for(i=0; i<3; i++)
		{
			memcpy(tmp, header_start[i] , header_len[i]);
			tmp += header_len[i];
		}

		listSupCodecs[real_index].mkv_codpriv = encoder_ctx->enc_audio_ctx->priv_data;
		listSupCodecs[real_index].codpriv_size = priv_data_size;
		return listSupCodecs[real_index].codpriv_size;
	}


	return 0;
}

/*
 * get audio codec name
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: codec name entry
 */
const char *encoder_get_audio_codec_name(int codec_ind)
{
	int real_index = get_real_index (codec_ind);
	if(real_index >= 0 && real_index < encoder_get_audio_codec_list_size())
		return (listSupCodecs[real_index].name);
	else
	{
		fprintf(stderr, "ENCODER: (audio codec name) bad codec index (%i)\n", codec_ind);;
		return NULL;
	}
}

/*
 * get audio codec list index for codec name
 * args:
 *   codec_name - codec common name
 *
 * asserts:
 *   none
 *
 * returns: codec index or -1 if error
 */
int encoder_get_audio_codec_ind_name(const char *codec_name)
{
	int real_index = 0;
	int index = -1;
	for(real_index = 0; real_index < encoder_get_audio_codec_list_size(); ++real_index)
	{
		if(listSupCodecs[real_index].valid)
			index++;
		if(strcasecmp(codec_name, listSupCodecs[real_index].name) == 0)
			return index;
	}

	return -1;
}
