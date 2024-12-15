/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#                                                                               #
# This is a heavily modified version of the matroska interface from x264        #
#           Copyright (C) 2005 Mike Matsnev                                     #
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

#ifndef AVI_H
#define AVI_H

#include <inttypes.h>
#include <sys/types.h>

#include "stream_io.h"
#include "file_io.h"

#define AVI_MAX_TRACKS 8
#define FRAME_RATE_SCALE 1000 //1000000

typedef struct _video_index_entry_t
{
	off_t key;
	off_t pos;
	off_t len;
} video_index_entry_t;

typedef struct _audio_index_entry_t
{
	off_t pos;
	off_t len;
	off_t tot;
} audio_index_entry_t;

typedef struct avi_I_entry_t
{
    unsigned int flags, pos, len;
} avi_I_entry_t;

typedef struct avi_index_t
{
    int64_t     indx_start;
    int         entry;
    int         ents_allocated;
    avi_I_entry_t **cluster;
} avi_index_t;

typedef struct _avi_riff_t
{
    int64_t riff_start, movi_list;
    //int64_t frames_hdr_all;
    int64_t time_delay_off;
    int id;

    struct _avi_riff_t *previous, *next;
} avi_riff_t;

typedef struct avi_RIFF avi_RIFF;

typedef struct avi_context_t
{
	io_writer_t  *writer;

	int flags; /* 0 - AVI is recordind;   1 - AVI is not recording*/

	uint32_t avi_flags;

	int32_t time_base_num;       /* video time base numerator */
	int32_t time_base_den;       /* video time base denominator */

	avi_riff_t *riff_list; // avi_riff list (NULL terminated)
	int riff_list_size;

	stream_io_t *stream_list;
	int stream_list_size;

	double fps;

	int64_t odml_list; /*,time_delay_off*/ ; //some file offsets

} avi_context_t;

avi_context_t *avi_create_context(const char *filename);

stream_io_t *avi_add_video_stream(
		avi_context_t *avi_ctx,
		int32_t width,
		int32_t height,
		int32_t fps,
		int32_t fps_num,
		int32_t codec_id);

stream_io_t *avi_add_audio_stream(
		avi_context_t *avi_ctx,
		int32_t   channels,
		int32_t   rate,
		int32_t   bits,
		int32_t   mpgrate,
		int32_t   codec_id,
		int32_t   format);


int avi_write_packet(
		avi_context_t *avi_ctx,
		int stream_index,
		uint8_t *data,
		uint32_t size,
		int64_t dts,
		int block_align,
		int32_t flags);

avi_riff_t *avi_add_new_riff(avi_context_t *avi_ctx);

int avi_close(avi_context_t *avi_ctx);


void avi_destroy_context(avi_context_t *avi_ctx);

#endif