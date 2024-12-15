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

#ifndef AUDIO_H
#define AUDIO_H

#include <inttypes.h>
#include <sys/types.h>

#include "gviewaudio.h"

struct _audio_context_t
{
	int api;                      /*audio api for this context*/
	int num_input_dev;            /*number of audio input devices in list*/
	audio_device_t *list_devices; /*audio input devices list*/
	int device;                   /*current device list index*/
	int channels;                 /*channels*/
	int samprate;                 /*sample rate*/
	double latency;               /*current sugested latency*/

	/*all ts are monotonic based: both real and generated*/
	int64_t current_ts;           /*current buffer generated timestamp*/
	int64_t last_ts;              /*last real timestamp (in nanosec)*/
	int64_t snd_begintime;        /*sound capture start ref time*/
	int64_t ts_drift;             /*drift between real and generated ts*/

	sample_t *capture_buff;       /*pointer to capture data*/
	int capture_buff_size;        /*capture buffer size (bytes)*/
	float capture_buff_level[2];  /*capture buffer channels level*/

	void *stream;                 /*pointer to audio stream (portaudio)*/

	int stream_flag;              /*stream flag*/
	
	pthread_mutex_t mutex;       /*audio mutex*/

};

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
void audio_fill_buffer(audio_context_t *audio_ctx, int64_t ts);

#endif
