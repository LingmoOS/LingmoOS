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

#ifndef STREAM_IO_H
#define STREAM_IO_H

#include <inttypes.h>
#include <sys/types.h>

enum stream_t
{
	STREAM_TYPE_VIDEO = 0,
	STREAM_TYPE_AUDIO = 1,
	STREAM_TYPE_SUB = 2 //not supported
};

typedef enum stream_t stream_t;

typedef struct _stream_io_t
{
	stream_t type;               /*stream type*/

	int32_t  id;

	uint32_t packet_count;

	/** AVI specific data */
	void*    indexes;            /*pointer to avi_index struct*/
	int32_t  entry;
	int64_t  rate_hdr_strm;
	int64_t  frames_hdr_strm;
	char     compressor[8];      /* Type of compressor, 4 bytes + padding for 0 byte */

	int32_t  codec_id;
	int32_t  h264_process;		 /* Set to 1 if codec private data used (NALU marker needs to be processed)*/

	/*video*/
	int32_t  width;              /* Width  of a video frame */
	int32_t  height;             /* Height of a video frame */
	double   fps;                /* Frames per second */

	/*audio*/
	int32_t  a_fmt;              /* Audio format, see #defines below */
	int32_t  a_chans;            /* Audio channels, 0 for no audio */
	int32_t  a_rate;             /* Rate in Hz */
	int32_t  a_bits;             /* bits per audio sample */
	int32_t  mpgrate;            /* mpg bitrate kbs*/
	int32_t  a_vbr;              /* 0 == no Variable BitRate */
	uint64_t audio_strm_length;  /* Total number of bytes of audio data */

	/*stream private data (codec private data)*/
	uint8_t *extra_data;
	int32_t extra_data_size;

	struct _stream_io_t *previous, *next;
} stream_io_t;

/*
 * get last stream of the list (list tail)
 * args:
 *   stream_list - pointer to stream list
 *
 * asserts:
 *   none
 *
 * returns: pointer to last stream of the list
 *          or NULL if none
 */
stream_io_t *get_last_stream(stream_io_t *stream_list);

/*
 * add a new stream to the list
 * args:
 *   stream_list - pointer to pointer of stream_list
 *   list_size - pointer to list size
 *
 * asserts:
 *   none
 *
 * returns: pointer to newly allocated stream
 */
stream_io_t *add_new_stream(stream_io_t **stream_list, int *list_size);

/*
 * destroy the sream list (free all streams)
 * args:
 *   stream_list - pointer to stream list
 *   list_size - pointer to list size
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void destroy_stream_list(stream_io_t *stream_list, int *list_size);

/*
 * get stream with index from list
 * args:
 *   stream_list - pointer to pointer of stream_list
 *   index - stream index in the list
 *
 * asserts:
 *   none
 *
 * returns: pointer to stream
 */
stream_io_t *get_stream(stream_io_t *stream_list, int index);

/*
 * get first video stream
 * args:
 *   stream_list - pointer to  stream list
 *
 * asserts:
 *   none
 *
 * returns: pointer to stream
 */
stream_io_t *get_first_video_stream(stream_io_t *stream_list);

/*
 * get first audio stream
 * args:
 *   stream_list - pointer to  stream list
 *
 * asserts:
 *   none
 *
 * returns: pointer to stream
 */
//stream_io_t *get_first_audio_stream(stream_io_t *stream_list);

#endif
