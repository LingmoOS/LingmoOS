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
#  Audio library                                                                #
#                                                                               #
********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
/* support for internationalization - i18n */
#include <locale.h>
#include <libintl.h>

#include "cameraconfig.h"
#include "gviewaudio.h"
#include "audio.h"
#include "gview.h"
#include "audio_portaudio.h"
#if HAS_PULSEAUDIO
  #include "audio_pulseaudio.h"
#endif

#define AUDBUFF_NUM     80    /*number of audio buffers*/
#define AUDBUFF_FRAMES  1152  /*number of audio frames per buffer*/
static audio_buff_t *audio_buffers = NULL; /*pointer to buffers list*/
static int buffer_read_index = 0; /*current read index of buffer list*/
static int buffer_write_index = 0;/*current write index of buffer list*/

extern int verbosity;

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
void audio_set_verbosity(int value)
{
	verbosity = value;
}

/*
 * Lock the mutex
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void audio_lock_mutex(audio_context_t *audio_ctx)
{
	__LOCK_MUTEX(&(audio_ctx->mutex));
}

/*
 * Unlock the mutex
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void audio_unlock_mutex(audio_context_t *audio_ctx)
{
	__UNLOCK_MUTEX(&(audio_ctx->mutex));
}

/*
 * free audio buffers
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
static void audio_free_buffers()
{
	buffer_read_index = 0;
	buffer_write_index = 0;

	/*return if no buffers set*/
	if(!audio_buffers)
	{
		if(verbosity > 0)
			fprintf(stderr,"AUDIO: can't free audio buffers (audio_free_buffers): audio_buffers is null\n");
		return;
	}

	int i = 0;

	for(i = 0; i < AUDBUFF_NUM; ++i)
	{
		free(audio_buffers[i].data);
	}

	free(audio_buffers);
	audio_buffers = NULL;
}

/*
 * alloc a single audio buffer
 * args:
 *    audio_ctx - pointer to audio context data
 *
 * asserts:
 *    none
 *
 * returns: pointer to newly allocate audio buffer or NULL on error
 *   must be freed with audio_delete_buffer
 * data is allocated for float(32 bit) samples but can also store
 * int16 (16 bit) samples
 */
audio_buff_t *audio_get_buffer(audio_context_t *audio_ctx)
{
	if(audio_ctx->capture_buff_size <= 0)
	{
		fprintf(stderr, "AUDIO: (get_buffer) invalid capture_buff_size(%i)\n",
			audio_ctx->capture_buff_size);
		return NULL;
	}

	audio_buff_t *audio_buff = calloc(1, sizeof(audio_buff_t));
	if(audio_buff == NULL)
	{
		fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_get_buffer): %s\n", strerror(errno));
		exit(-1);
	}
	audio_buff->data = calloc(audio_ctx->capture_buff_size, sizeof(sample_t));
	if(audio_buff->data == NULL)
	{
		fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_get_buffer): %s\n", strerror(errno));
		exit(-1);
	}

	return audio_buff;
}

/*
 * deletes a single audio buffer
 * args:
 *    audio_buff - pointer to audio_buff_t data
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void audio_delete_buffer(audio_buff_t *audio_buff)
{
	if(!audio_buff)
		return;

	if(audio_buff->data)
		free(audio_buff->data);

	free(audio_buff);
}

/*
 * alloc audio buffers
 * args:
 *    audio_ctx - pointer to audio context data
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
static int audio_init_buffers(audio_context_t *audio_ctx)
{
	if(!audio_ctx)
		return -1;

	/*don't allocate if no audio*/
	if(audio_ctx->api == AUDIO_NONE)
	{
		audio_buffers = NULL;
		return 0;
	}
	
	int i = 0;

	/*set the buffers size*/
	if(!audio_ctx->capture_buff_size)
		audio_ctx->capture_buff_size = audio_ctx->channels * AUDBUFF_FRAMES;

	if(audio_ctx->capture_buff)
		free(audio_ctx->capture_buff);

	audio_ctx->capture_buff = calloc(
		audio_ctx->capture_buff_size, sizeof(sample_t));
	if(audio_ctx->capture_buff == NULL)
	{
		fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_init_buffers): %s\n", strerror(errno));
		exit(-1);
	}
	
	/*free audio_buffers (if any)*/
	audio_free_buffers();

	audio_buffers = calloc(AUDBUFF_NUM, sizeof(audio_buff_t));
	if(audio_buffers == NULL)
	{
		fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_init_buffers): %s\n", strerror(errno));
		exit(-1);
	}

	for(i = 0; i < AUDBUFF_NUM; ++i)
	{
		audio_buffers[i].data = calloc(
			audio_ctx->capture_buff_size, sizeof(sample_t));
		if(audio_buffers[i].data == NULL)
		{
			fprintf(stderr,"AUDIO: FATAL memory allocation failure (audio_init_buffers): %s\n", strerror(errno));
			exit(-1);
		}
		audio_buffers[i].flag = AUDIO_BUFF_FREE;
	}

	return 0;
}

/*
 * fill a audio buffer data and move write index to next one
 * args:
 *   audio_ctx - pointer to audio context data
 *   ts - timestamp for end of data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: none
 */
void audio_fill_buffer(audio_context_t *audio_ctx, int64_t ts)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	if(verbosity > 3)
		printf("AUDIO: filling buffer ts:%" PRId64 "\n", ts);
	/*in nanosec*/
	uint64_t frame_length = NSEC_PER_SEC / audio_ctx->samprate;
	uint64_t buffer_length = frame_length * (audio_ctx->capture_buff_size / audio_ctx->channels);

	audio_ctx->current_ts += buffer_length; /*buffer end time*/

	audio_ctx->ts_drift = audio_ctx->current_ts - ts;

	/*get the current write indexed buffer flag*/
	audio_lock_mutex(audio_ctx);
	int flag = audio_buffers[buffer_write_index].flag;
	audio_unlock_mutex(audio_ctx);

	if(flag == AUDIO_BUFF_USED)
	{
		fprintf(stderr, "AUDIO: write buffer(%i) is still in use - dropping data\n", buffer_write_index);
		return;
	}

	/*write max_frames and fill a buffer*/
	memcpy(audio_buffers[buffer_write_index].data,
		audio_ctx->capture_buff,
		audio_ctx->capture_buff_size * sizeof(sample_t));
	/*buffer begin time*/
	audio_buffers[buffer_write_index].timestamp = audio_ctx->current_ts - buffer_length;
	if(audio_buffers[buffer_write_index].timestamp < 0)
		fprintf(stderr, "AUDIO: write buffer(%i) - invalid timestamp (< 0): cur_ts:%" PRId64 " buf_length:%" PRId64 "\n", 
			buffer_write_index, audio_ctx->current_ts, buffer_length);

	audio_buffers[buffer_write_index].level_meter[0] = audio_ctx->capture_buff_level[0];
	audio_buffers[buffer_write_index].level_meter[1] = audio_ctx->capture_buff_level[1];

	audio_lock_mutex(audio_ctx);
	audio_buffers[buffer_write_index].flag = AUDIO_BUFF_USED;
	NEXT_IND(buffer_write_index, AUDBUFF_NUM);
	audio_unlock_mutex(audio_ctx);

}

/* saturate float samples to int16 limits*/
static int16_t clip_int16 (float in)
{
	//int16_t out = (int16_t) (in < -32768) ? -32768 : (in > 32767) ? 32767 : in;

	long lout =  lroundf(in);
	int16_t out = (lout < INT16_MIN) ? INT16_MIN : (lout > INT16_MAX) ? INT16_MAX: (int16_t) lout;
	return (out);
}

/*
 * get the next used buffer from the ring buffer
 * args:
 *   audio_ctx - pointer to audio context
 *   buff - pointer to an allocated audio buffer
 *   type - type of data (SAMPLE_TYPE_[INT16|FLOAT])
 *   mask - audio fx mask
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int audio_get_next_buffer(audio_context_t *audio_ctx, audio_buff_t *buff, int type, uint32_t mask)
{
	audio_lock_mutex(audio_ctx);
	int flag = audio_buffers[buffer_read_index].flag;
	audio_unlock_mutex(audio_ctx);

	if(flag == AUDIO_BUFF_FREE)
		return 1; /*all done*/

	/*aplly fx*/
	audio_fx_apply(audio_ctx, (sample_t *) audio_buffers[buffer_read_index].data, mask);

	/*copy data into requested format type*/
	int i = 0;
	switch(type)
	{
		case GV_SAMPLE_TYPE_FLOAT:
		{
			sample_t *my_data = (sample_t *) buff->data;
			memcpy( my_data, audio_buffers[buffer_read_index].data,
				audio_ctx->capture_buff_size * sizeof(sample_t));
			break;
		}
		case GV_SAMPLE_TYPE_INT16:
		{
			int16_t *my_data = (int16_t *) buff->data;
			sample_t *buff_p = (sample_t *) audio_buffers[buffer_read_index].data;
			for(i = 0; i < audio_ctx->capture_buff_size; ++i)
			{
				my_data[i] = clip_int16( (buff_p[i]) * INT16_MAX);
			}
			break;
		}
		case GV_SAMPLE_TYPE_FLOATP:
		{
			int j=0;

			float *my_data[audio_ctx->channels];
			sample_t *buff_p = (sample_t *) audio_buffers[buffer_read_index].data;

			for(j = 0; j < audio_ctx->channels; ++j)
				my_data[j] = (float *) (((float *) buff->data) +
					(j * audio_ctx->capture_buff_size/audio_ctx->channels));

			for(i = 0; i < audio_ctx->capture_buff_size/audio_ctx->channels; ++i)
				for(j = 0; j < audio_ctx->channels; ++j)
				{
					my_data[j][i] = *buff_p++;
				}
			break;
		}
		case GV_SAMPLE_TYPE_INT16P:
		{
			int j=0;

			int16_t *my_data[audio_ctx->channels];
			sample_t *buff_p = (sample_t *) audio_buffers[buffer_read_index].data;

			for(j = 0; j < audio_ctx->channels; ++j)
				my_data[j] = (int16_t *) (((int16_t *) buff->data) +
					(j * audio_ctx->capture_buff_size/audio_ctx->channels));

			for(i = 0; i < audio_ctx->capture_buff_size/audio_ctx->channels; ++i)
				for(j = 0; j < audio_ctx->channels; ++j)
				{
					my_data[j][i] = clip_int16((*buff_p++) * INT16_MAX);
				}
			break;
		}
	}

	buff->timestamp = audio_buffers[buffer_read_index].timestamp;

	buff->level_meter[0] = audio_buffers[buffer_read_index].level_meter[0];
	buff->level_meter[1] = audio_buffers[buffer_read_index].level_meter[1];

	audio_lock_mutex(audio_ctx);
	audio_buffers[buffer_read_index].flag = AUDIO_BUFF_FREE;
	NEXT_IND(buffer_read_index, AUDBUFF_NUM);
	audio_unlock_mutex(audio_ctx);

	return 0;
}

/*
 * audio initialization
 * args:
 *   api - audio API to use
 *           (AUDIO_NONE, AUDIO_PORTAUDIO, AUDIO_PULSE, ...)
 *   device - api device index to use (-1 - use api default)
 *
 * asserts:
 *   none
 *
 * returns: pointer to audio context data (or NULL on error)
 */
audio_context_t *audio_init(int api, int device)
{
	audio_context_t *audio_ctx = calloc(1, sizeof(audio_context_t));

	if(audio_ctx == NULL)
	{
		fprintf(stderr, "AUDIO: (audio_init) couldn't allocate audio context\n");
		return NULL;
	}

	/*initialize the mutex*/
	__INIT_MUTEX(&(audio_ctx->mutex));
	
	int ret = 0;

	switch(api)
	{
		case AUDIO_NONE:
			audio_ctx->api = AUDIO_NONE;
			break;

#if HAS_PULSEAUDIO
		case AUDIO_PULSE:
			ret = audio_init_pulseaudio(audio_ctx);
			break;
#endif
		case AUDIO_PORTAUDIO:
		default:
			ret = audio_init_portaudio(audio_ctx);
			break;
	}

	/*if api couldn't be initialized set audio to none*/
	if (ret != 0)
		audio_ctx->api = AUDIO_NONE;

	/*set default api device*/
	audio_set_device_index(audio_ctx, device);

	/*force a valid number of channels*/
	if(audio_ctx->channels > 2)
		audio_ctx->channels = 2;

	return audio_ctx;
}

/*
 * get audio api
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: audio API
 */
int audio_get_api(audio_context_t *audio_ctx)
{
	/*assertions*/
	assert(audio_ctx != NULL);
	
	return audio_ctx->api;
}

/*
 * set the audio device index to use
 * args:
 *   audio_ctx - pointer to audio context data
 *   index - device index (from device list) to set
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: none
 */
void audio_set_device_index(audio_context_t *audio_ctx, int index)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	switch(audio_ctx->api)
	{
		case AUDIO_NONE:
			break;

#if HAS_PULSEAUDIO
		case AUDIO_PULSE:
			audio_set_pulseaudio_device(audio_ctx, index);
			break;
#endif
		case AUDIO_PORTAUDIO:
		default:
			audio_set_portaudio_device(audio_ctx, index);
			break;
	}
}

/*
 * get the current audio device index
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: current device index (from device list)
 */
int audio_get_device_index(audio_context_t *audio_ctx)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	return audio_ctx->device;
}

/*
 * get the number of available input audio devices
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: number of listed audio devices
 */
int audio_get_num_inp_devices(audio_context_t *audio_ctx)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	return audio_ctx->num_input_dev;
}

/*
 * get the audio device referenced by index
 * args:
 *   audio_ctx - pointer to audio context data
 *   index - index of audio device
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: audio device referenced by index
 */
audio_device_t* audio_get_device(audio_context_t *audio_ctx, int index)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	if(index >= audio_ctx->num_input_dev)
	{
		fprintf(stderr, "AUDIO: (audio_get_device) bad index %i using %i\n",
			index, audio_ctx->num_input_dev - 1);
		index = audio_ctx->num_input_dev - 1;
	}

	if(index < 0)
	{
		fprintf(stderr, "AUDIO: (audio_get_device) bad index %i using 0\n", index);
		index = 0;
	}

	return &audio_ctx->list_devices[index];
}

/*
 * set the current latency
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: none
 */
void audio_set_latency(audio_context_t *audio_ctx, double latency)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	audio_ctx->latency = latency;
}

/*
 * get the current latency
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: defined lantency
 */
double audio_get_latency(audio_context_t *audio_ctx)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	return audio_ctx->latency;
}

/*
 * set the number of channels
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: none
 */
void audio_set_channels(audio_context_t *audio_ctx, int channels)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	audio_ctx->channels = channels;
}

/*
 * get the number of channels
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: number of channels
 */
int audio_get_channels(audio_context_t *audio_ctx)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	return audio_ctx->channels;
}

/*
 * set the sample rate
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: none
 */
void audio_set_samprate(audio_context_t *audio_ctx, int samprate)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	audio_ctx->samprate = samprate;
}

/*
 * get the sample rate
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: sample rate
 */
int audio_get_samprate(audio_context_t *audio_ctx)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	return audio_ctx->samprate;
}

/*
 * set the capture buffer size
 * args:
 *   audio_ctx - pointer to audio context data
 *   size - capture buffer size in bytes
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: none
 */
void audio_set_cap_buffer_size(audio_context_t *audio_ctx, int size)
{
	/*assertions*/
	assert(audio_ctx != NULL);
	
	if(verbosity > 2)
		printf("AUDIO: set capture buffer size to %i samples\n", size);
	
	audio_ctx->capture_buff_size = size;
}

/*
 * start audio stream capture
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: error code
 */
int audio_start(audio_context_t *audio_ctx)
{
	if(verbosity > 1)
		printf("AUDIO: starting audio capture\n");
	/*assertions*/
	assert(audio_ctx != NULL);

	/*alloc the ring buffer*/
	audio_init_buffers(audio_ctx);
	
	/*reset timestamp values*/
	audio_ctx->current_ts = 0;
	audio_ctx->last_ts = 0;
	audio_ctx->snd_begintime = 0;
	audio_ctx->ts_drift = 0;  

	int err = 0;

	switch(audio_ctx->api)
	{
		case AUDIO_NONE:
			break;

#if HAS_PULSEAUDIO
		case AUDIO_PULSE:
			err = audio_start_pulseaudio(audio_ctx);
			break;
#endif
		case AUDIO_PORTAUDIO:
		default:
			err = audio_start_portaudio(audio_ctx);
			break;
	}

	return err;
}

/*
 * stop audio stream capture
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: error code
 */
int audio_stop(audio_context_t *audio_ctx)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	int err =0;

	switch(audio_ctx->api)
	{
		case AUDIO_NONE:
			break;

#if HAS_PULSEAUDIO
		case AUDIO_PULSE:
			err = audio_stop_pulseaudio(audio_ctx);
			break;
#endif
		case AUDIO_PORTAUDIO:
		default:
			err = audio_stop_portaudio(audio_ctx);
			break;
	}

	/*free the ring buffer (if any)*/
	audio_free_buffers();
		
	return err;
}

/*
 * close and clean audio context
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: none
 */
void audio_close(audio_context_t *audio_ctx)
{
	/*assertions*/
	assert(audio_ctx != NULL);

	audio_fx_close();
	
	/* thread must be joined before destroying the mutex
         * so no need to unlock before destroying it
         */
	
	/*destroy the mutex*/
	__CLOSE_MUTEX(&(audio_ctx->mutex));

	switch(audio_ctx->api)
	{
		case AUDIO_NONE:
			break;

#if HAS_PULSEAUDIO
		case AUDIO_PULSE:
			audio_close_pulseaudio(audio_ctx);
			break;
#endif
		case AUDIO_PORTAUDIO:
		default:
			audio_close_portaudio(audio_ctx);
			break;
	}

	if(audio_buffers != NULL)
		audio_free_buffers();
}
