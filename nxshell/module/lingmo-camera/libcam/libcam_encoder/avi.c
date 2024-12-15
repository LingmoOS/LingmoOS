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

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
/* support for internationalization - i18n */
#include <locale.h>
#include <libintl.h>

#include "gviewencoder.h"
#include "encoder.h"
#include "stream_io.h"
#include "file_io.h"
#include "avi.h"
#include "gview.h"
#include "load_libs.h"

#ifndef O_BINARY
/* win32 wants a binary flag to open(); this sets it to null
   on platforms that don't have it. */
#define O_BINARY 0
#endif

#define INFO_LIST

//#define MAX_INFO_STRLEN 64
//static char id_str[MAX_INFO_STRLEN];

#ifndef PACKAGE
#define PACKAGE "guvcview"
#endif
//#ifndef VERSION
//#define VERSION "1.0"
//#endif

#define AVI_INDEX_CLUSTER_SIZE 16384

#define AVIF_HASINDEX           0x00000010      /* Index at end of file */
#define AVIF_MUSTUSEINDEX       0x00000020
#define AVIF_ISINTERLEAVED      0x00000100
#define AVIF_TRUSTCKTYPE        0x00000800      /* Use CKType to find key frames */
#define AVIF_WASCAPTUREFILE     0x00010000
#define AVIF_COPYRIGHTED        0x00020000

#define AVI_MAX_RIFF_SIZE       0x40000000LL    /*1Gb = 0x40000000LL*/
#define AVI_MASTER_INDEX_SIZE   256
#define AVI_MAX_STREAM_COUNT    10

/* index flags */
#define AVIF_INDEX             0x10

// bIndexType codes
//
#define AVI_INDEX_OF_INDEXES 0x00 	// when each entry in aIndex
									// array points to an index chunk

#define AVI_INDEX_OF_CHUNKS 0x01 	// when each entry in aIndex
									// array points to a chunk in the file

#define AVI_INDEX_IS_DATA 0x80 		// when each entry is aIndex is
									// really the data
									// bIndexSubtype codes for INDEX_OF_CHUNKS

#define AVI_INDEX_2FIELD 0x01 		// when fields within frames
									// are also indexed

extern int verbosity;

int64_t avi_open_tag (avi_context_t *avi_ctx, const char *tag)
{
	io_write_4cc(avi_ctx->writer, tag);
	io_write_wl32(avi_ctx->writer, 0);
	return io_get_offset(avi_ctx->writer);
}

static void avi_close_tag(avi_context_t *avi_ctx, int64_t start_pos)
{
	int64_t current_offset = io_get_offset(avi_ctx->writer);
	int32_t size = (int32_t) (current_offset - start_pos);
	io_seek(avi_ctx->writer, start_pos-4);
    io_write_wl32(avi_ctx->writer, (uint32_t)size);
	io_seek(avi_ctx->writer, current_offset);

	if(verbosity > 0)
		printf("ENCODER: (avi) %" PRIu64 " closing tag at %" PRIu64 " with size %i\n",
			current_offset, start_pos-4, size);

}

/*
 * Calculate audio sample size from number of bits and number of channels.
 *    This may have to be adjusted for eg. 12 bits and stereo
 */
static int avi_audio_sample_size(stream_io_t *stream)
{
	if(stream->type != STREAM_TYPE_AUDIO)
		return -1;

	int s;
	if (stream->a_fmt != WAVE_FORMAT_PCM)
	{
		s = 4;
	}
	else
	{
		s = ((stream->a_bits+7)/8)*stream->a_chans;
		if(s<4) s=4; /* avoid possible zero divisions */
	}
	return s;
}

static char* avi_stream2fourcc(char* tag, stream_io_t *stream)
{
    tag[0] = '0' + (char)((stream->id)/10);
    tag[1] = '0' + (stream->id)%10;
    switch(stream->type)
    {
		case STREAM_TYPE_VIDEO:
			tag[2] = 'd';
			tag[3] = 'c';
			break;
		case STREAM_TYPE_SUB:
			// note: this is not an official code
			tag[2] = 's';
			tag[3] = 'b';
			break;
		default: //audio
			tag[2] = 'w';
			tag[3] = 'b';
			break;
	}
    tag[4] = '\0';
    return tag;
}

void avi_put_main_header(avi_context_t *avi_ctx, avi_riff_t *riff)
{
	avi_ctx->fps = get_first_video_stream(avi_ctx->stream_list)->fps;
	int width = get_first_video_stream(avi_ctx->stream_list)->width;
	int height = get_first_video_stream(avi_ctx->stream_list)->height;
	int time_base_num = avi_ctx->time_base_num;
	int time_base_den = avi_ctx->time_base_den;

	uint32_t data_rate = 0;
	if(time_base_den > 0 || time_base_num > 0) //these are not set yet so it's always false
		data_rate = (uint32_t) (INT64_C(1000000) * time_base_num/time_base_den);
	else
		fprintf(stderr, "ENCODER: (avi) bad time base (%i/%i): set it later", time_base_num, time_base_den);

	/*do not force index yet -only when closing*/
	/*this should prevent bad avi files even if it is not closed properly*/
	//if(hasIndex) flag |= AVIF_HASINDEX;
	//if(hasIndex && avi_ctx->must_use_index) flag |= AVIF_MUSTUSEINDEX;
	avi_ctx->avi_flags = AVIF_WASCAPTUREFILE;

	int64_t avih = avi_open_tag(avi_ctx, "avih");      // main avi header
	riff->time_delay_off = io_get_offset(avi_ctx->writer);
	io_write_wl32(avi_ctx->writer, 1000000 / FRAME_RATE_SCALE); // time per frame (milisec)
	io_write_wl32(avi_ctx->writer, data_rate);         // data rate
	io_write_wl32(avi_ctx->writer, 0);                 // Padding multiple size (2048)
	io_write_wl32(avi_ctx->writer, avi_ctx->avi_flags);    // parameter Flags
	//riff->frames_hdr_all = io_get_offset(avi_ctx->writer);
	io_write_wl32(avi_ctx->writer, 0);			       // number of video frames
	io_write_wl32(avi_ctx->writer, 0);			       // number of preview frames
    io_write_wl32(avi_ctx->writer, (uint32_t)avi_ctx->stream_list_size); // number of data streams (audio + video)*/
	io_write_wl32(avi_ctx->writer, 1024*1024);         // suggested playback buffer size (bytes)
    io_write_wl32(avi_ctx->writer, (uint32_t)width);		       // width
    io_write_wl32(avi_ctx->writer, (uint32_t)height);	    	   // height
	io_write_wl32(avi_ctx->writer, 0);                 // time scale:  unit used to measure time (30)
	io_write_wl32(avi_ctx->writer, 0);			       // data rate (frame rate * time scale)
	io_write_wl32(avi_ctx->writer, 0);			       // start time (0)
	io_write_wl32(avi_ctx->writer, 0);			       // size of avi data chunk (in scale units)
	avi_close_tag(avi_ctx, avih);     //write the chunk size
}

int64_t avi_put_bmp_header(avi_context_t *avi_ctx, stream_io_t *stream)
{
	int frate = 15 * FRAME_RATE_SCALE;
	if(stream->fps > 0.001)
		frate = (int) ((FRAME_RATE_SCALE * (stream->fps)) + 0.5);

	int64_t strh = avi_open_tag(avi_ctx, "strh");// video stream header
	io_write_4cc(avi_ctx->writer, "vids");              // stream type
	io_write_4cc(avi_ctx->writer, stream->compressor);  // Handler (VIDEO CODEC)
	io_write_wl32(avi_ctx->writer, 0);                  // Flags
	io_write_wl16(avi_ctx->writer, 0);                  // stream priority
	io_write_wl16(avi_ctx->writer, 0);                  // language tag
	io_write_wl32(avi_ctx->writer, 0);                  // initial frames
	io_write_wl32(avi_ctx->writer, FRAME_RATE_SCALE);   // Scale
	stream->rate_hdr_strm = io_get_offset(avi_ctx->writer); //store this to set proper fps
    io_write_wl32(avi_ctx->writer, (uint32_t)frate);              // Rate: Rate/Scale == sample/second (fps) */
	io_write_wl32(avi_ctx->writer, 0);                  // start time
	stream->frames_hdr_strm = io_get_offset(avi_ctx->writer);
	io_write_wl32(avi_ctx->writer, 0);                  // lenght of stream
	io_write_wl32(avi_ctx->writer, 1024*1024);          // suggested playback buffer size
    io_write_wl32(avi_ctx->writer, (uint32_t)(-1));                 // Quality
	io_write_wl32(avi_ctx->writer, 0);                  // SampleSize
	io_write_wl16(avi_ctx->writer, 0);                  // rFrame (left)
	io_write_wl16(avi_ctx->writer, 0);                  // rFrame (top)
    io_write_wl16(avi_ctx->writer, (uint16_t)stream->width);      // rFrame (right)
    io_write_wl16(avi_ctx->writer, (uint16_t)stream->height);     // rFrame (bottom)
	avi_close_tag(avi_ctx, strh); //write the chunk size

	return strh;
}

int64_t avi_put_wav_header(avi_context_t *avi_ctx, stream_io_t *stream)
{
	int sampsize = avi_audio_sample_size(stream);

	int64_t strh = avi_open_tag(avi_ctx, "strh");// audio stream header
	io_write_4cc(avi_ctx->writer, "auds");
	io_write_wl32(avi_ctx->writer, 1);                  // codec tag on strf
	io_write_wl32(avi_ctx->writer, 0);                  // Flags
	io_write_wl16(avi_ctx->writer, 0);                  // stream priority
	io_write_wl16(avi_ctx->writer, 0);                  // language tag
	io_write_wl32(avi_ctx->writer, 0);                  // initial frames
	stream->rate_hdr_strm = io_get_offset(avi_ctx->writer);
    io_write_wl32(avi_ctx->writer, (uint32_t)(sampsize/4));         // Scale
    io_write_wl32(avi_ctx->writer, (uint32_t)(stream->mpgrate/8));  // Rate: Rate/Scale == sample/second (fps) */
	io_write_wl32(avi_ctx->writer, 0);                  // start time
	stream->frames_hdr_strm = io_get_offset(avi_ctx->writer);
	io_write_wl32(avi_ctx->writer, 0);                  // lenght of stream
	io_write_wl32(avi_ctx->writer, 12*1024);            // suggested playback buffer size
    io_write_wl32(avi_ctx->writer, (uint32_t)(-1));                 // Quality
    io_write_wl32(avi_ctx->writer,(uint32_t)( sampsize/4));         // SampleSize
	io_write_wl16(avi_ctx->writer, 0);                  // rFrame (left)
	io_write_wl16(avi_ctx->writer, 0);                  // rFrame (top)
	io_write_wl16(avi_ctx->writer, 0);                  // rFrame (right)
	io_write_wl16(avi_ctx->writer, 0);                  // rFrame (bottom)
	avi_close_tag(avi_ctx, strh); //write the chunk size

	return strh;
}

void avi_put_vstream_format_header(avi_context_t *avi_ctx, stream_io_t *stream)
{
	int vxd_size        = stream->extra_data_size;
	int vxd_size_align  = (stream->extra_data_size+1) & ~1;

	int64_t strf = avi_open_tag(avi_ctx, "strf");   // stream format header
    io_write_wl32(avi_ctx->writer, (uint32_t)(40 + vxd_size));  // sruct Size
    io_write_wl32(avi_ctx->writer,(uint32_t) stream->width);  // Width
    io_write_wl32(avi_ctx->writer, (uint32_t)stream->height); // Height
	io_write_wl16(avi_ctx->writer, 1);              // Planes
	io_write_wl16(avi_ctx->writer, 24);             // Count - bitsperpixel - 1,4,8 or 24  32
	if(strncmp(stream->compressor,"DIB",3)==0)
		io_write_wl32(avi_ctx->writer, 0);          // Compression
	else
		io_write_4cc(avi_ctx->writer, stream->compressor);
    io_write_wl32(avi_ctx->writer, (uint32_t)(stream->width*stream->height*3));// image size (in bytes?)
	io_write_wl32(avi_ctx->writer, 0);              // XPelsPerMeter
	io_write_wl32(avi_ctx->writer, 0);              // YPelsPerMeter
	io_write_wl32(avi_ctx->writer, 0);              // ClrUsed: Number of colors used
	io_write_wl32(avi_ctx->writer, 0);              // ClrImportant: Number of colors important
	// write extradata (codec private)
	if (vxd_size > 0 && stream->extra_data)
	{
		io_write_buf(avi_ctx->writer, stream->extra_data, vxd_size);
		if (vxd_size != vxd_size_align)
		{
			io_write_w8(avi_ctx->writer, 0);  //align
		}
	}
	avi_close_tag(avi_ctx, strf); //write the chunk size
}

void avi_put_astream_format_header(avi_context_t *avi_ctx, stream_io_t *stream)
{
	int axd_size        = stream->extra_data_size;
	int axd_size_align  = (stream->extra_data_size+1) & ~1;

	int sampsize = avi_audio_sample_size(stream);

	int64_t strf = avi_open_tag(avi_ctx, "strf");// audio stream format
    io_write_wl16(avi_ctx->writer, (uint16_t)stream->a_fmt);    // Format (codec) tag
    io_write_wl16(avi_ctx->writer, (uint16_t)stream->a_chans);  // Number of channels
    io_write_wl32(avi_ctx->writer, (uint32_t)stream->a_rate);   // SamplesPerSec
    io_write_wl32(avi_ctx->writer, (uint32_t)stream->mpgrate/8);// Average Bytes per sec
    io_write_wl16(avi_ctx->writer, (uint16_t)sampsize/4);       // BlockAlign
    io_write_wl16(avi_ctx->writer, (uint16_t)stream->a_bits);   //BitsPerSample
    io_write_wl16(avi_ctx->writer, (uint16_t)axd_size);         //size of extra data
	// write extradata (codec private)
	if (axd_size > 0 && stream->extra_data)
	{
		io_write_buf(avi_ctx->writer, stream->extra_data, axd_size);
		if (axd_size != axd_size_align)
		{
			io_write_w8(avi_ctx->writer, 0);  //align
		}
	}
	avi_close_tag(avi_ctx, strf); //write the chunk size
}

void avi_put_vproperties_header(avi_context_t *avi_ctx, stream_io_t *stream)
{
    uint32_t refresh_rate =  (uint32_t) lrintf((float)(2.0 * avi_ctx->fps));
	if(avi_ctx->time_base_den  > 0 || avi_ctx->time_base_num > 0) //these are not set yet so it's always false
	{
		double time_base = avi_ctx->time_base_num / (double) avi_ctx->time_base_den;
        refresh_rate = (uint32_t)lrintf((float)(1.0/time_base));
	}
    int vprp= (int)avi_open_tag(avi_ctx, "vprp");
    io_write_wl32(avi_ctx->writer, 0);              //video format  = unknown
    io_write_wl32(avi_ctx->writer, 0);              //video standard= unknown
    io_write_wl32(avi_ctx->writer, refresh_rate);   // dwVerticalRefreshRate
    io_write_wl32(avi_ctx->writer, (uint32_t)stream->width ); //horizontal pixels
    io_write_wl32(avi_ctx->writer, (uint32_t)stream->height); //vertical lines
    io_write_wl16(avi_ctx->writer, (uint16_t)stream->height); //Active Frame Aspect Ratio (4:3 - 16:9)
    io_write_wl16(avi_ctx->writer, (uint16_t)stream->width);  //Active Frame Aspect Ratio
    io_write_wl32(avi_ctx->writer, (uint32_t)stream->width ); //Active Frame Height in Pixels
    io_write_wl32(avi_ctx->writer, (uint32_t)stream->height); //Active Frame Height in Lines
    io_write_wl32(avi_ctx->writer, 1);              //progressive FIXME
	//Field Framing Information
    io_write_wl32(avi_ctx->writer, (uint32_t)stream->height);
    io_write_wl32(avi_ctx->writer,(uint32_t) stream->width );
    io_write_wl32(avi_ctx->writer, (uint32_t)stream->height);
    io_write_wl32(avi_ctx->writer, (uint32_t)stream->width );
    io_write_wl32(avi_ctx->writer, 0);
    io_write_wl32(avi_ctx->writer, 0);
    io_write_wl32(avi_ctx->writer, 0);
    io_write_wl32(avi_ctx->writer, 0);

    avi_close_tag(avi_ctx, vprp);
}

int64_t avi_create_riff_tags(avi_context_t *avi_ctx, avi_riff_t *riff)
{
	int64_t off = 0;
	riff->riff_start = avi_open_tag(avi_ctx, "RIFF");

	if(riff->id == 1)
	{
		io_write_4cc(avi_ctx->writer, "AVI ");
		off = avi_open_tag(avi_ctx, "LIST");
		io_write_4cc(avi_ctx->writer, "hdrl");
	}
	else
	{
		io_write_4cc(avi_ctx->writer, "AVIX");
		off = avi_open_tag(avi_ctx, "LIST");
		io_write_4cc(avi_ctx->writer, "movi");

		riff->movi_list = off; //update movi list pos for this riff
	}

	return off;
}

//only for riff id = 1
void avi_create_riff_header(avi_context_t *avi_ctx, avi_riff_t *riff)
{
	int64_t list1 = avi_create_riff_tags(avi_ctx, riff);

	avi_put_main_header(avi_ctx, riff);

	int i, j = 0;

	for(j=0; j< avi_ctx->stream_list_size; j++)
	{
		stream_io_t *stream = get_stream(avi_ctx->stream_list, j);

		int64_t list2 = avi_open_tag(avi_ctx, "LIST");
		io_write_4cc(avi_ctx->writer,"strl");              //stream list

		if(stream->type == STREAM_TYPE_VIDEO)
		{
			avi_put_bmp_header(avi_ctx, stream);
			avi_put_vstream_format_header(avi_ctx, stream);
		}
		else
		{
			avi_put_wav_header(avi_ctx, stream);
			avi_put_astream_format_header(avi_ctx, stream);
		}
		/* Starting to lay out AVI OpenDML master index.
		 * We want to make it JUNK entry for now, since we'd
		 * like to get away without making AVI an OpenDML one
		 * for compatibility reasons.
		 */
		char tag[5];
		avi_index_t *indexes = (avi_index_t *) stream->indexes;
		indexes->entry = indexes->ents_allocated = 0;
		indexes->indx_start = io_get_offset(avi_ctx->writer);
		int64_t ix = avi_open_tag(avi_ctx, "JUNK");           // ’ix##’
		io_write_wl16(avi_ctx->writer, 4);               // wLongsPerEntry must be 4 (size of each entry in aIndex array)
		io_write_w8(avi_ctx->writer, 0);                 // bIndexSubType must be 0 (frame index) or AVI_INDEX_2FIELD
		io_write_w8(avi_ctx->writer, AVI_INDEX_OF_INDEXES);  // bIndexType (0 == AVI_INDEX_OF_INDEXES)
		io_write_wl32(avi_ctx->writer, 0);               // nEntriesInUse (will fill out later on)
		io_write_4cc(avi_ctx->writer, avi_stream2fourcc(tag, stream)); // dwChunkId
		io_write_wl32(avi_ctx->writer, 0);               // dwReserved[3] must be 0
		io_write_wl32(avi_ctx->writer, 0);
		io_write_wl32(avi_ctx->writer, 0);
		for (i=0; i < AVI_MASTER_INDEX_SIZE; i++)
		{
			io_write_wl64(avi_ctx->writer, 0);           // absolute file offset, offset 0 is unused entry
			io_write_wl32(avi_ctx->writer, 0);           // dwSize - size of index chunk at this offset
			io_write_wl32(avi_ctx->writer, 0);           // dwDuration - time span in stream ticks
		}
		avi_close_tag(avi_ctx, ix); //write the chunk size

		if(stream->type == STREAM_TYPE_VIDEO)
			avi_put_vproperties_header(avi_ctx, stream);

		avi_close_tag(avi_ctx, list2); //write the chunk size
	}

	avi_ctx->odml_list = avi_open_tag(avi_ctx, "JUNK");
    io_write_4cc(avi_ctx->writer, "odml");
    io_write_4cc(avi_ctx->writer, "dmlh");
    io_write_wl32(avi_ctx->writer, 248);
    for (i = 0; i < 248; i+= 4)
        io_write_wl32(avi_ctx->writer, 0);
    avi_close_tag(avi_ctx, avi_ctx->odml_list);

	avi_close_tag(avi_ctx, list1); //write the chunk size

	/* some padding for easier tag editing */
    int64_t list3 = avi_open_tag(avi_ctx, "JUNK");
    for (i = 0; i < 1016; i += 4)
        io_write_wl32(avi_ctx->writer, 0);
    avi_close_tag(avi_ctx, list3); //write the chunk size

    riff->movi_list = avi_open_tag(avi_ctx, "LIST");
    io_write_4cc(avi_ctx->writer, "movi");
}

avi_riff_t *avi_get_last_riff(avi_context_t *avi_ctx)
{
	avi_riff_t *last_riff = avi_ctx->riff_list;
	while(last_riff->next != NULL)
		last_riff = last_riff->next;

	return last_riff;
}

avi_riff_t *avi_get_riff(avi_context_t *avi_ctx, int index)
{
	avi_riff_t *riff = avi_ctx->riff_list;

	if(!riff)
		return NULL;

	int j = 1;

	while(riff->next != NULL && (j < index))
	{
		riff = riff->next;
		j++;
	}

	if(j != index)
		return NULL;

	return riff;
}

static void clean_indexes(avi_context_t *avi_ctx)
{
	int i=0, j=0;

	for (i=0; i<avi_ctx->stream_list_size; i++)
    {
        stream_io_t *stream = get_stream(avi_ctx->stream_list, i);

		avi_index_t *indexes = (avi_index_t *) stream->indexes;
		for (j=0; j<indexes->ents_allocated/AVI_INDEX_CLUSTER_SIZE; j++)
             free(indexes->cluster[j]);
        getAvutil()->m_av_freep(&indexes->cluster);
        indexes->ents_allocated = indexes->entry = 0;
    }
}

//call this after adding all the streams
avi_riff_t *avi_add_new_riff(avi_context_t *avi_ctx)
{
	avi_riff_t *riff = calloc(1, sizeof(avi_riff_t));

	if(riff == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (avi_add_new_riff): %s\n", strerror(errno));
		exit(-1);
	}

	riff->next = NULL;
	riff->id = avi_ctx->riff_list_size + 1;

	if(riff->id == 1)
	{
		riff->previous = NULL;
		avi_ctx->riff_list = riff;
		avi_create_riff_header(avi_ctx, riff);
	}
	else
	{
		avi_riff_t *last_riff = avi_get_last_riff(avi_ctx);
		riff->previous = last_riff;
		last_riff->next = riff;
		avi_create_riff_tags(avi_ctx, riff);
	}

	avi_ctx->riff_list_size++;

	clean_indexes(avi_ctx);

	if(verbosity > 0)
		printf("ENCODER: (avi) adding new RIFF (%i)\n", riff->id);
	return riff;
}

//second function to get called (add video stream to avi_Context)
stream_io_t *avi_add_video_stream(
		avi_context_t *avi_ctx,
		int32_t width,
		int32_t height,
		int32_t fps,
		int32_t fps_num,
		int32_t codec_id)
{
	stream_io_t *stream = add_new_stream(&avi_ctx->stream_list, &avi_ctx->stream_list_size);
	stream->type = STREAM_TYPE_VIDEO;
    stream->fps = (double) fps/fps_num;
	stream->width = width;
	stream->height = height;
	stream->codec_id = codec_id;

	stream->indexes = (void *) calloc(1, sizeof(avi_index_t));
	if(stream->indexes == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (avi_add_video_stream): %s\n", strerror(errno));
		exit(-1);
	}

	int codec_ind = get_video_codec_list_index(codec_id);
	strncpy(stream->compressor, encoder_get_video_codec_4cc(codec_ind), 8);

	return stream;
}

//third function to get called (add audio stream to avi_Context)
stream_io_t *avi_add_audio_stream(
		avi_context_t *avi_ctx,
		int32_t   channels,
		int32_t   rate,
		int32_t   bits,
		int32_t   mpgrate,
		int32_t   codec_id,
		int32_t   format)
{
	stream_io_t *stream = add_new_stream(&avi_ctx->stream_list, &avi_ctx->stream_list_size);
	stream->type = STREAM_TYPE_AUDIO;

	stream->a_chans = channels;
	stream->a_rate = rate;
	stream->a_bits = bits;
	stream->mpgrate = mpgrate;
	stream->a_vbr = 0;
	stream->codec_id = codec_id;
	stream->a_fmt = format;

	stream->indexes = (void *) calloc(1, sizeof(avi_index_t));
	if(stream->indexes == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (avi_add_audio_stream): %s\n", strerror(errno));
		exit(-1);
	}

	return stream;
}

/*
   first function to get called

   avi_create_context:  Open an AVI File and write a bunch
                        of zero bytes as space for the header.
                        Creates a mutex.

   returns a pointer to avi_Context on success, a NULL pointer on error
*/
avi_context_t *avi_create_context(const char *filename)
{
	avi_context_t *avi_ctx = calloc(1, sizeof(avi_context_t));

	if(avi_ctx == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (avi_create_context): %s\n", strerror(errno));
		exit(-1);
	}

	avi_ctx->writer = io_create_writer(filename, 0);

	if (avi_ctx->writer == NULL)
	{
		fprintf(stderr, "ENCODER: (avi) Could not open file (%s) for writing: %s",
			filename, strerror(errno));
		free(avi_ctx);
		return NULL;
	}

	avi_ctx->flags = 0; /*recordind*/

	avi_ctx->riff_list = NULL;
	avi_ctx->riff_list_size = 0;

	avi_ctx->stream_list = NULL;
	avi_ctx->stream_list_size = 0;

	return avi_ctx;
}

void avi_destroy_context(avi_context_t *avi_ctx)
{
	//clean up
	io_destroy_writer(avi_ctx->writer);

	avi_riff_t *riff = avi_get_last_riff(avi_ctx);
	while(riff != NULL) //from end to start
	{
		avi_riff_t *prev_riff = riff->previous;
		free(riff);
		riff = prev_riff;
		avi_ctx->riff_list_size--;
	}

	destroy_stream_list(avi_ctx->stream_list, &avi_ctx->stream_list_size);

	//free avi_Context
	free(avi_ctx);
}

avi_I_entry_t *avi_get_ientry(avi_index_t *idx, int ent_id)
{
	int cl = ent_id / AVI_INDEX_CLUSTER_SIZE;
    int id = ent_id % AVI_INDEX_CLUSTER_SIZE;
    return &idx->cluster[cl][id];
}

static int avi_write_counters(avi_context_t *avi_ctx, __attribute__((unused))avi_riff_t *riff)
{
    int n, nb_frames = 0;
    io_flush_buffer(avi_ctx->writer);

	//int time_base_num = avi_ctx->time_base_num;
	//int time_base_den = avi_ctx->time_base_den;

    int64_t file_size = io_get_offset(avi_ctx->writer);//avi_tell(avi_ctx);
    if(verbosity > 0)
		printf("ENCODER: (avi) file size = %" PRIu64 "\n", file_size);

    for(n = 0; n < avi_ctx->stream_list_size; n++)
    {
        stream_io_t *stream = get_stream(avi_ctx->stream_list, n);

		if(stream->rate_hdr_strm <= 0)
        {
			fprintf(stderr, "ENCODER: (avi) stream rate header pos not valid\n");
		}
		else
		{
			io_seek(avi_ctx->writer, stream->rate_hdr_strm);

			if(stream->type == STREAM_TYPE_VIDEO && avi_ctx->fps > 0.001)
			{
                uint32_t rate =(uint32_t) FRAME_RATE_SCALE * (uint32_t)lrintf((float)avi_ctx->fps);
				if(verbosity > 0)
                    fprintf(stderr,"ENCODER: (avi) storing rate(%u)\n",rate);
				io_write_wl32(avi_ctx->writer, rate);
			}
		}

        if(stream->frames_hdr_strm <= 0)
        {
			fprintf(stderr, "ENCODER: (avi) stream frames header pos not valid\n");
		}
		else
		{
			io_seek(avi_ctx->writer, stream->frames_hdr_strm);

			if(stream->type == STREAM_TYPE_VIDEO)
			{
				io_write_wl32(avi_ctx->writer, stream->packet_count);
                nb_frames = (int)MAX((uint32_t)nb_frames, stream->packet_count);
			}
			else
			{
				int sampsize = avi_audio_sample_size(stream);
                io_write_wl32(avi_ctx->writer, (uint32_t)(4*stream->audio_strm_length/(size_t)sampsize));
			}
		}
    }

    avi_riff_t *riff_1 = avi_get_riff(avi_ctx, 1);
    if(riff_1->id == 1) /*should always be true*/
    {
        if(riff_1->time_delay_off <= 0)
        {
			fprintf(stderr, "ENCODER: (avi) riff main header pos not valid\n");
        }
        else
        {
			uint32_t us_per_frame = 1000; //us
			if(avi_ctx->fps > 0.001)
                us_per_frame=(uint32_t) lrintf((float)(1000000.0 / avi_ctx->fps));

			avi_ctx->avi_flags |= AVIF_HASINDEX;

			io_seek(avi_ctx->writer, riff_1->time_delay_off);
			io_write_wl32(avi_ctx->writer, us_per_frame);      // time_per_frame
			io_write_wl32(avi_ctx->writer, 0);                 // data rate
			io_write_wl32(avi_ctx->writer, 0);                 // Padding multiple size (2048)
			io_write_wl32(avi_ctx->writer, avi_ctx->avi_flags);    // parameter Flags
			//io_seek(avi_ctx->writer, riff_1->frames_hdr_all);
            io_write_wl32(avi_ctx->writer, (uint32_t)nb_frames);
		}
    }

	//return to position (EOF)
    io_seek(avi_ctx->writer, file_size);

    return 0;
}

static int avi_write_ix(avi_context_t *avi_ctx)
{
    char tag[5];
    char ix_tag[] = "ix00";
    int i, j;

	avi_riff_t *riff = avi_get_last_riff(avi_ctx);

    if (riff->id > AVI_MASTER_INDEX_SIZE)
        return -1;

    for (i=0;i<avi_ctx->stream_list_size;i++)
    {
        stream_io_t *stream = get_stream(avi_ctx->stream_list, i);
        int64_t ix, pos;

        avi_stream2fourcc(tag, stream);

        ix_tag[3] = '0' + (char)i; /*only 10 streams supported*/

        /* Writing AVI OpenDML leaf index chunk */
        ix = io_get_offset(avi_ctx->writer);
        io_write_4cc(avi_ctx->writer, ix_tag);     /* ix?? */
        avi_index_t *indexes = (avi_index_t *) stream->indexes;
        io_write_wl32(avi_ctx->writer, (uint32_t)(indexes->entry * 8 + 24));
                                      /* chunk size */
        io_write_wl16(avi_ctx->writer, 2);           /* wLongsPerEntry */
        io_write_w8(avi_ctx->writer, 0);             /* bIndexSubType (0 == frame index) */
        io_write_w8(avi_ctx->writer, AVI_INDEX_OF_CHUNKS); /* bIndexType (1 == AVI_INDEX_OF_CHUNKS) */
        io_write_wl32(avi_ctx->writer, (uint32_t)indexes->entry);
                                      /* nEntriesInUse */
        io_write_4cc(avi_ctx->writer, tag);        /* dwChunkId */
        io_write_wl64(avi_ctx->writer, (uint64_t)riff->movi_list);/* qwBaseOffset */
        io_write_wl32(avi_ctx->writer, 0);             /* dwReserved_3 (must be 0) */

        for (j=0; j< indexes->entry; j++)
        {
             avi_I_entry_t *ie = avi_get_ientry(indexes, j);
             io_write_wl32(avi_ctx->writer, ie->pos + 8);
             io_write_wl32(avi_ctx->writer, ((uint32_t)ie->len & ~0x80000000) |
                          (ie->flags & 0x10 ? 0 : 0x80000000));
         }
         io_flush_buffer(avi_ctx->writer);
         pos = io_get_offset(avi_ctx->writer); //current position
         if(verbosity > 0)
			printf("ENCODER: (avi) wrote ix %s with %i entries\n",
				tag, indexes->entry);

         /* Updating one entry in the AVI OpenDML master index */
         io_seek(avi_ctx->writer, indexes->indx_start);
         io_write_4cc(avi_ctx->writer, "indx");            /* enabling this entry */
         io_skip(avi_ctx->writer, 8);
         io_write_wl32(avi_ctx->writer, (uint32_t)(riff->id));         /* nEntriesInUse */
         io_skip(avi_ctx->writer, 16*(riff->id));
         io_write_wl64(avi_ctx->writer,(uint64_t) ix);               /* qwOffset */
         io_write_wl32(avi_ctx->writer, (uint32_t)(pos - ix));         /* dwSize */
         io_write_wl32(avi_ctx->writer,(uint32_t)(indexes->entry));   /* dwDuration */

		//return to position
         io_seek(avi_ctx->writer, pos);
    }
    return 0;
}

static int avi_write_idx1(avi_context_t *avi_ctx, avi_riff_t *riff)
{

    int64_t idx_chunk;
    int i;
    char tag[5];


    stream_io_t *stream;
    avi_I_entry_t *ie = 0, *tie;
    int empty, stream_id = -1;

    idx_chunk = avi_open_tag(avi_ctx, "idx1");
    for (i=0;i<avi_ctx->stream_list_size;i++)
    {
            stream = get_stream(avi_ctx->stream_list, i);
            stream->entry=0;
    }

    do
    {
        empty = 1;
        for (i=0;i<avi_ctx->stream_list_size;i++)
        {
			stream = get_stream(avi_ctx->stream_list, i);
			avi_index_t *indexes = (avi_index_t *) stream->indexes;
            if (indexes->entry <= stream->entry)
                continue;

            tie = avi_get_ientry(indexes, stream->entry);
            if (empty || tie->pos < ie->pos)
            {
                ie = tie;
                stream_id = i;
            }
            empty = 0;
        }

        if (!empty)
        {
            stream = get_stream(avi_ctx->stream_list, stream_id);
            avi_stream2fourcc(tag, stream);
            io_write_4cc(avi_ctx->writer, tag);
            io_write_wl32(avi_ctx->writer, ie->flags);
            io_write_wl32(avi_ctx->writer, ie->pos);
            io_write_wl32(avi_ctx->writer, ie->len);
            stream->entry++;
        }
    }
    while (!empty);

    avi_close_tag(avi_ctx, idx_chunk);
    if(verbosity > 0)
		printf("ENCODER: (avi) wrote idx1\n");
    avi_write_counters(avi_ctx, riff);

    return 0;
}

int avi_write_packet(
	avi_context_t *avi_ctx,
	int stream_index,
	uint8_t *data,
	uint32_t size,
__attribute__((unused))	int64_t dts,
__attribute__((unused))	int block_align,
	int32_t flags)
{
    char tag[5];
    unsigned int i_flags=0;

    stream_io_t *stream= get_stream(avi_ctx->stream_list, stream_index);

	avi_riff_t *riff = avi_get_last_riff(avi_ctx);
	//align
    //while(block_align==0 && dts != AV_NOPTS_VALUE && dts > stream->packet_count)
    //    avi_write_packet(avi_ctx, stream_index, NULL, 0, AV_NOPTS_VALUE, 0, 0);

    stream->packet_count++;

    // Make sure to put an OpenDML chunk when the file size exceeds the limits
    if (io_get_offset(avi_ctx->writer) - riff->riff_start > AVI_MAX_RIFF_SIZE)
    {
        avi_write_ix(avi_ctx);
        avi_close_tag(avi_ctx, riff->movi_list);

        if (riff->id == 1)
            avi_write_idx1(avi_ctx, riff);

        avi_close_tag(avi_ctx, riff->riff_start);

        avi_add_new_riff(avi_ctx);

        riff = avi_get_last_riff(avi_ctx); //update riff
    }

    avi_stream2fourcc(tag, stream);

    if(flags & AV_PKT_FLAG_KEY) //key frame
        i_flags = 0x10;

    if (stream->type == STREAM_TYPE_AUDIO)
       stream->audio_strm_length += size;


    avi_index_t *idx = (avi_index_t *) stream->indexes;
    int cl = idx->entry / AVI_INDEX_CLUSTER_SIZE;
    int id = idx->entry % AVI_INDEX_CLUSTER_SIZE;
    if (idx->ents_allocated <= idx->entry)
    {
        idx->cluster = realloc(idx->cluster, (size_t)(cl+1)*sizeof(void*));
        if (idx->cluster == NULL)
        {
			fprintf(stderr, "ENCODER: FATAL memory allocation failure (avi_write_packet): %s\n", strerror(errno));
			exit(-1);
		}
	
        idx->cluster[cl] = calloc(AVI_INDEX_CLUSTER_SIZE, sizeof(avi_I_entry_t));
        if (idx->cluster[cl] == NULL)
		{
			fprintf(stderr, "ENCODER: FATAL memory allocation failure (avi_write_packet): %s\n", strerror(errno));
			exit(-1);
		}
        idx->ents_allocated += AVI_INDEX_CLUSTER_SIZE;
    }

    idx->cluster[cl][id].flags = i_flags;
    idx->cluster[cl][id].pos = (unsigned int)(io_get_offset(avi_ctx->writer) - riff->movi_list);
    idx->cluster[cl][id].len = size;
    idx->entry++;


    io_write_4cc(avi_ctx->writer, tag);
    io_write_wl32(avi_ctx->writer, size);
    io_write_buf(avi_ctx->writer, data, (int)size);
    if (size & 1)
        io_write_w8(avi_ctx->writer, 0);

    io_flush_buffer(avi_ctx->writer);

    return 0;
}

int avi_close(avi_context_t *avi_ctx)
{
    int res = 0;
    int64_t file_size;

    avi_riff_t *riff = avi_get_last_riff(avi_ctx);

    if (riff->id == 1)
    {
        avi_close_tag(avi_ctx, riff->movi_list);
        if(verbosity > 0)
			printf("ENCODER: (avi) %" PRIu64 " close movi tag\n",io_get_offset(avi_ctx->writer));
        res = avi_write_idx1(avi_ctx, riff);
        avi_close_tag(avi_ctx, riff->riff_start);
    }
    else
    {
        avi_write_ix(avi_ctx);
        avi_close_tag(avi_ctx, riff->movi_list);
        avi_close_tag(avi_ctx, riff->riff_start);

        file_size = io_get_offset(avi_ctx->writer);
        io_seek(avi_ctx->writer, avi_ctx->odml_list - 8);
        io_write_4cc(avi_ctx->writer, "LIST"); /* Making this AVI OpenDML one */
        io_skip(avi_ctx->writer, 16);

		int n = 0;
		int nb_frames = 0;

        for (n=nb_frames=0;n<avi_ctx->stream_list_size;n++)
        {
            stream_io_t *stream = get_stream(avi_ctx->stream_list, n);

            if (stream->type == STREAM_TYPE_VIDEO)
            {
                if (nb_frames < (int)stream->packet_count)
                        nb_frames = (int)stream->packet_count;
            }
            else
            {
                if (stream->codec_id == AV_CODEC_ID_MP2 || stream->codec_id == AV_CODEC_ID_MP3)
                        nb_frames += stream->packet_count;
            }
        }
        io_write_wl32(avi_ctx->writer, (uint32_t)nb_frames);
        io_seek(avi_ctx->writer, file_size);

        avi_write_counters(avi_ctx, riff);
    }

	clean_indexes(avi_ctx);

    return res;
}
