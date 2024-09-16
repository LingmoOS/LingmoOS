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

#include "cameraconfig.h"
/*random generator (HAS_GSL is set in ../config.h)*/
#ifdef HAS_GSL
	#include <gsl/gsl_rng.h>
#else
	#include <time.h>
#endif
#include "gviewencoder.h"
#include "encoder.h"
#include "stream_io.h"
#include "file_io.h"
#include "matroska.h"
#include "gview.h"
#include "gui.h"
#include "camview.h"
#include "load_libs.h"
/*
 * default size of pkt ring buffer 
 * for caching audio frames
 * aprox. 4 sec for 44100 samp/sec with
 * each buffer containing 1152 samples
 * vorbis has 64 samples
 */
#define PKT_BUFFER_DEF_SIZE 156

/** 2 bytes * 3 for EBML IDs, 3 1-byte EBML lengths, 8 bytes for 64 bit
 * offset, 4 bytes for target EBML ID */
#define MAX_SEEKENTRY_SIZE 21

/** per-cuepoint-track - 3 1-byte EBML IDs, 3 1-byte EBML sizes, 2
 * 8-byte uint max */
#define MAX_CUETRACKPOS_SIZE 22

/** per-cuepoint - 2 1-byte EBML IDs, 2 1-byte EBML sizes, 8-byte uint max */
#define MAX_CUEPOINT_SIZE(num_tracks) 12 + MAX_CUETRACKPOS_SIZE*num_tracks

/*default audio frames per buffer*/
#define AUDBUFF_FRAMES  1152

extern int verbosity;

/** Some utilities for
 *  float and double conversion to/from int */
union mkv_union_intfloat32
{
	uint32_t i;
	float f;
};

union mkv_union_intfloat64
{
	uint64_t i;
	double f;
};

static uint64_t mkv_double2int(double f)
{
	union mkv_union_intfloat64 v;
	v.f = f;
	return v.i;
}


/** get id size */
static int ebml_id_size(unsigned int id)
{
	int bytes = 4, mask = 0x10;

    while (!(id & (unsigned int)(mask << ((bytes - 1) * 8))) && bytes > 0)
	{
		mask <<= 1;
		bytes--;
	}

    return bytes;
}

/** write an id */
static void mkv_put_ebml_id(mkv_context_t *mkv_ctx, unsigned int id)
{
    int i = ebml_id_size(id);
    while (i--)
        io_write_w8(mkv_ctx->writer, (uint8_t)(id >> (i*8)));
}

/**
 * Write an EBML size meaning "unknown size".
 *
 * @param bytes The number of bytes the size should occupy (maximum: 8).
 */
static void mkv_put_ebml_size_unknown(mkv_context_t *mkv_ctx, int bytes)
{
    if(bytes <= 8) //max is 64 bits
    {
        io_write_w8(mkv_ctx->writer,(uint8_t)( 0x1ff >> bytes));
		while (--bytes)
			io_write_w8(mkv_ctx->writer, 0xff);
	}
	else
		fprintf(stderr, "mkv_ctx: bad unknown size (%i > 8) bytes)\n", bytes);
}

/**
 * Calculate how many bytes are needed to represent a given number in EBML.
 */
static int ebml_num_size(uint64_t num)
{
    int bytes = 1;
    while ((num+1) >> bytes*7)
		bytes++;
    return bytes;
}

/**
 * Write a number in EBML variable length format.
 *
 * @param bytes The number of bytes that need to be used to write the number.
 *              If zero, any number of bytes can be used.
 */
static void mkv_put_ebml_num(mkv_context_t *mkv_ctx, uint64_t num, int bytes)
{
    int i, needed_bytes = ebml_num_size(num);

    // sizes larger than this are currently undefined in EBML
    if(num >= (1ULL<<56)-1)
    {
		fprintf(stderr, "ENCODER: (matroska) ebml number: %" PRIu64 "\n", num);
		return;
	}

    if (bytes == 0)
        // don't care how many bytes are used, so use the min
        bytes = needed_bytes;
    // the bytes needed to write the given size would exceed the bytes
    // that we need to use, so write unknown size. This shouldn't happen.
    if(bytes < needed_bytes)
    {
		fprintf(stderr, "ENCODER: (matroska) bad requested size for ebml number: %" PRIu64 " (%i < %i)\n", num, bytes, needed_bytes);
		return;
	}

    num |= 1ULL << bytes*7;
    for (i = bytes - 1; i >= 0; i--)
        io_write_w8(mkv_ctx->writer,(uint8_t)( num >> i*8));
}

static void mkv_put_ebml_uint(mkv_context_t *mkv_ctx, unsigned int elementid, uint64_t val)
{
    int i, bytes = 1;
    uint64_t tmp = val;
    while (tmp>>=8) bytes++;

    mkv_put_ebml_id(mkv_ctx, elementid);
    mkv_put_ebml_num(mkv_ctx,(uint64_t) bytes, 0);
    for (i = bytes - 1; i >= 0; i--)
        io_write_w8(mkv_ctx->writer,(uint8_t)( val >> i*8));
}

static void mkv_put_ebml_float(mkv_context_t *mkv_ctx, unsigned int elementid, double val)
{
    mkv_put_ebml_id(mkv_ctx, elementid);
    mkv_put_ebml_num(mkv_ctx, 8, 0);
    io_write_wb64(mkv_ctx->writer, mkv_double2int(val));
}

static void mkv_put_ebml_binary(mkv_context_t *mkv_ctx, unsigned int elementid,
                            void *buf, int size)
{
    mkv_put_ebml_id(mkv_ctx, elementid);
    mkv_put_ebml_num(mkv_ctx,(uint64_t) size, 0);
    io_write_buf(mkv_ctx->writer, buf, size);
}

static void mkv_put_ebml_string(mkv_context_t *mkv_ctx, unsigned int elementid, char *str)
{
    mkv_put_ebml_binary(mkv_ctx, elementid, str, (int)strlen(str));
}

/**
 * Write a void element of a given size. Useful for reserving space in
 * the file to be written to later.
 *
 * @param size The number of bytes to reserve, which must be at least 2.
 */
static void mkv_put_ebml_void(mkv_context_t *mkv_ctx, uint64_t size)
{
    int64_t currentpos = io_get_offset(mkv_ctx->writer);

    if(size < 2)
    {
		fprintf(stderr, "ENCODER: (matroska) wrong void size %" PRIu64 " < 2", size);
	}

    mkv_put_ebml_id(mkv_ctx, EBML_ID_VOID);
    // we need to subtract the length needed to store the size from the
    // size we need to reserve so 2 cases, we use 8 bytes to store the
    // size if possible, 1 byte otherwise
    if (size < 10)
        mkv_put_ebml_num(mkv_ctx, size-1, 0);
    else
        mkv_put_ebml_num(mkv_ctx, size-9, 8);
    while(io_get_offset(mkv_ctx->writer) < (int64_t)(currentpos +(int64_t) size))
        io_write_w8(mkv_ctx->writer, 0);
}

static ebml_master_t mkv_start_ebml_master(mkv_context_t *mkv_ctx,
	unsigned int elementid,
	uint64_t expectedsize)
{
	//if 0 reserve max (8 bytes)
    int bytes = expectedsize ? ebml_num_size(expectedsize) : 8;
    mkv_put_ebml_id(mkv_ctx, elementid);
    mkv_put_ebml_size_unknown(mkv_ctx, bytes);
    return (ebml_master_t){ io_get_offset(mkv_ctx->writer), bytes };
}

static void mkv_end_ebml_master(mkv_context_t *mkv_ctx, ebml_master_t master)
{
    int64_t pos = io_get_offset(mkv_ctx->writer);

    if (io_seek(mkv_ctx->writer, master.pos - master.sizebytes) < 0)
        return;
    mkv_put_ebml_num(mkv_ctx, (uint64_t)(pos - master.pos), master.sizebytes);
	io_seek(mkv_ctx->writer, pos);
}

//static void mkv_put_xiph_size(mkv_context_t* mkv_ctx, int size)
//{
//    int i;
//    for (i = 0; i < size / 255; i++)
//		io_write_w8(mkv_ctx->writer, 255);
//    io_write_w8(mkv_ctx->writer, size % 255);
//}

/**
 * Initialize a mkv_seekhead element to be ready to index level 1 Matroska
 * elements. If a maximum number of elements is specified, enough space
 * will be reserved at the current file location to write a seek head of
 * that size.
 *
 * @param segment_offset The absolute offset to the position in the file
 *                       where the segment begins.
 * @param numelements The maximum number of elements that will be indexed
 *                    by this seek head, 0 if unlimited.
 */
static mkv_seekhead_t *mkv_start_seekhead(mkv_context_t *mkv_ctx,
	int64_t segment_offset,
	int numelements)
{
    mkv_seekhead_t *new_seekhead = calloc(1, sizeof(mkv_seekhead_t));
    if (new_seekhead == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (mkv_start_seekhead): %s\n", strerror(errno));
		exit(-1);
	}

    new_seekhead->segment_offset = segment_offset;

    if (numelements > 0)
    {
        new_seekhead->filepos = io_get_offset(mkv_ctx->writer);
        /*
         * 21 bytes max for a seek entry, 10 bytes max for the SeekHead ID
         * and size, and 3 bytes to guarantee that an EBML void element
         * will fit afterwards
         */
        new_seekhead->reserved_size = numelements * MAX_SEEKENTRY_SIZE + 13;
        new_seekhead->max_entries = numelements;
        mkv_put_ebml_void(mkv_ctx, (uint64_t)new_seekhead->reserved_size);
    }
    return new_seekhead;
}

static int mkv_add_seekhead_entry(mkv_seekhead_t *seekhead,
	unsigned int elementid,
	uint64_t filepos)
{
    mkv_seekhead_entry_t *entries = seekhead->entries;
	fprintf(stderr,"ENCODER: (matroska) add seekhead entry %i (max %i)\n", seekhead->num_entries, seekhead->max_entries);
    /* don't store more elements than we reserved space for*/
    if (seekhead->max_entries > 0 && seekhead->max_entries <= seekhead->num_entries)
        return -1;

    entries = realloc(entries, (size_t)(seekhead->num_entries + 1) * sizeof(mkv_seekhead_entry_t));
	if (entries == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (mkv_add_seekhead_entry): %s\n", strerror(errno));
		exit(-1);
	}
	
    entries[seekhead->num_entries].elementid = elementid;
    entries[seekhead->num_entries].segmentpos = filepos - (uint64_t)seekhead->segment_offset;

	seekhead->num_entries++;

    seekhead->entries = entries;
    return 0;
}

/**
 * Write the seek head to the file and free it. If a maximum number of
 * elements was specified to mkv_start_seekhead(), the seek head will
 * be written at the location reserved for it. Otherwise, it is written
 * at the current location in the file.
 *
 * @return The file offset where the seekhead was written,
 * -1 if an error occurred.
 */
static int64_t mkv_write_seekhead(mkv_context_t* mkv_ctx, mkv_seekhead_t *seekhead)
{
    ebml_master_t metaseek, seekentry;
    int64_t currentpos;
    int i;

    currentpos = io_get_offset(mkv_ctx->writer);

    if (seekhead->reserved_size > 0)
    {
        if (io_seek(mkv_ctx->writer, seekhead->filepos) < 0)
        {
			fprintf(stderr, "ENCODER: (matroska) failed to write seekhead at pos %" PRIu64 "\n", seekhead->filepos);
            currentpos = -1;
            goto fail;
        }
    }

    metaseek = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_SEEKHEAD, (uint64_t)seekhead->reserved_size);
    for (i = 0; i < seekhead->num_entries; i++)
    {
        mkv_seekhead_entry_t *entry = &seekhead->entries[i];

        seekentry = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_SEEKENTRY, MAX_SEEKENTRY_SIZE);

        mkv_put_ebml_id(mkv_ctx, MATROSKA_ID_SEEKID);
        mkv_put_ebml_num(mkv_ctx,(uint64_t) ebml_id_size(entry->elementid), 0);
        mkv_put_ebml_id(mkv_ctx, entry->elementid);

        mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_SEEKPOSITION, entry->segmentpos);
        mkv_end_ebml_master(mkv_ctx, seekentry);
    }
    mkv_end_ebml_master(mkv_ctx, metaseek);

    if (seekhead->reserved_size > 0) {
        uint64_t remaining = (uint64_t)(seekhead->filepos + seekhead->reserved_size - io_get_offset(mkv_ctx->writer));
        mkv_put_ebml_void(mkv_ctx, remaining);
        io_seek(mkv_ctx->writer, currentpos);

        currentpos = seekhead->filepos;
    }
fail:
    free(seekhead->entries);
    free(seekhead);

    return currentpos;
}

static mkv_cues_t *mkv_start_cues(int64_t segment_offset)
{
    mkv_cues_t *cues = calloc(1, sizeof(mkv_cues_t));
    if (cues == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (mkv_start_cues): %s\n", strerror(errno));
		exit(-1);
	}

    cues->segment_offset = segment_offset;
    return cues;
}

static int mkv_add_cuepoint(mkv_cues_t *cues, int stream, int64_t ts, int64_t cluster_pos)
{
    mkv_cuepoint_t *entries = cues->entries;

    if (ts < 0)
        return 0;

    entries = realloc(entries, (size_t)(cues->num_entries + 1) * sizeof(mkv_cuepoint_t));

    if (entries == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (mkv_add_cuepoint): %s\n", strerror(errno));
		exit(-1);
	}

    entries[cues->num_entries].pts = (uint64_t)ts;
    entries[cues->num_entries].tracknum = stream + 1;
    entries[cues->num_entries].cluster_pos = cluster_pos - cues->segment_offset;

	cues->num_entries++;

    cues->entries = entries;
    return 0;
}

static int64_t mkv_write_cues(mkv_context_t *mkv_ctx, mkv_cues_t *cues, int num_tracks)
{
    ebml_master_t cues_element;
    int64_t currentpos;
    int i, j;

    currentpos = io_get_offset(mkv_ctx->writer);
    cues_element = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_CUES, 0);

    for (i = 0; i < cues->num_entries; i++)
    {
        ebml_master_t cuepoint, track_positions;
        mkv_cuepoint_t *entry = &cues->entries[i];
        uint64_t pts = entry->pts;

        cuepoint = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_POINTENTRY, (uint64_t)MAX_CUEPOINT_SIZE((uint64_t)num_tracks));
        mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_CUETIME, pts);

        // put all the entries from different tracks that have the exact same
        // timestamp into the same CuePoint
        for (j = 0; j < cues->num_entries - i && entry[j].pts == pts; j++)
        {
            track_positions = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_CUETRACKPOSITION, MAX_CUETRACKPOS_SIZE);
            mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_CUETRACK          , (uint64_t)entry[j].tracknum   );
            mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_CUECLUSTERPOSITION, (uint64_t)entry[j].cluster_pos);
            mkv_end_ebml_master(mkv_ctx, track_positions);
        }
        i += j - 1;
        mkv_end_ebml_master(mkv_ctx, cuepoint);
    }
    mkv_end_ebml_master(mkv_ctx, cues_element);

    return currentpos;
}

static void mkv_write_codecprivate(mkv_context_t *mkv_ctx, stream_io_t *stream)
{
	if (stream->extra_data_size && stream->extra_data != NULL)
		mkv_put_ebml_binary(mkv_ctx, MATROSKA_ID_CODECPRIVATE, stream->extra_data, stream->extra_data_size);
}

static void  mkv_write_trackdefaultduration(mkv_context_t *mkv_ctx, stream_io_t *stream)
{
	if(stream->type == STREAM_TYPE_VIDEO)
	{
        mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_TRACKDEFAULTDURATION, (uint64_t)floor(1E9/stream->fps));
	}
}

static int mkv_write_tracks(mkv_context_t *mkv_ctx)
{
    ebml_master_t tracks;
    int i, ret;

    ret = mkv_add_seekhead_entry(mkv_ctx->main_seekhead, MATROSKA_ID_TRACKS,(uint64_t) io_get_offset(mkv_ctx->writer));
    if (ret < 0) return ret;

    tracks = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_TRACKS, 0);

    for (i = 0; i < mkv_ctx->stream_list_size; i++)
    {
        stream_io_t *stream = get_stream(mkv_ctx->stream_list, i);
        ebml_master_t subinfo, track;

        track = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_TRACKENTRY, 0);
        mkv_put_ebml_uint (mkv_ctx, MATROSKA_ID_TRACKNUMBER     ,(uint64_t)(i + 1));
        mkv_put_ebml_uint (mkv_ctx, MATROSKA_ID_TRACKUID        ,(uint64_t)(i + 1));
        mkv_put_ebml_uint (mkv_ctx, MATROSKA_ID_TRACKFLAGLACING , 0);    // no lacing (yet)
		mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_TRACKFLAGDEFAULT, 1);

        char* mkv_codec_name;
        if(stream->type == STREAM_TYPE_VIDEO)
        {
			int codec_index = get_video_codec_list_index(stream->codec_id);
			if(codec_index < 0)
			{
				fprintf(stderr, "ENCODER: (matroska) bad video codec index for id:0x%x\n",stream->codec_id);
				return -1;
			}
            mkv_codec_name = (char *) encoder_get_video_mkv_codec(codec_index);
		}
		else
		{
			int codec_index = get_audio_codec_list_index(stream->codec_id);
			if(codec_index < 0)
			{
				fprintf(stderr, "ENCODER: (matroska) bad audio codec index for id:0x%x\n",stream->codec_id);
				return -1;
			}
			mkv_codec_name = (char *) encoder_get_audio_mkv_codec(codec_index);
		}

        mkv_put_ebml_string(mkv_ctx, MATROSKA_ID_CODECID, mkv_codec_name);

        if ((mkv_ctx->mode == ENCODER_MUX_WEBM) && !(stream->codec_id == AV_CODEC_ID_VP8 ||
#if LIBAVCODEC_VER_AT_LEAST(54,42)
										stream->codec_id == AV_CODEC_ID_VP9 ||
#endif
                                        stream->codec_id == AV_CODEC_ID_VORBIS))
		{
            fprintf(stderr, "ENCODER: (matroska) Only VP8 or VP9 video and Vorbis audio are supported for WebM.\n");
            return -2;
        }

        switch (stream->type)
        {
            case STREAM_TYPE_VIDEO:
                mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_TRACKTYPE, MATROSKA_TRACK_TYPE_VIDEO);
                subinfo = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_TRACKVIDEO, 0);
                // XXX: interlace flag?
                mkv_put_ebml_uint (mkv_ctx, MATROSKA_ID_VIDEOPIXELWIDTH ,(uint64_t) stream->width);
                mkv_put_ebml_uint (mkv_ctx, MATROSKA_ID_VIDEOPIXELHEIGHT,(uint64_t) stream->height);
                mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_VIDEODISPLAYWIDTH , (uint64_t)stream->width);
                mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_VIDEODISPLAYHEIGHT,(uint64_t) stream->height);
                mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_VIDEODISPLAYUNIT, 3);

                mkv_end_ebml_master(mkv_ctx, subinfo);
                break;

            case STREAM_TYPE_AUDIO:
                mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_TRACKTYPE, MATROSKA_TRACK_TYPE_AUDIO);


                //no mkv-specific ID, use ACM mode
                //put_ebml_string(pb, MATROSKA_ID_CODECID, "A_MS/ACM");

                subinfo = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_TRACKAUDIO, 0);
                mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_AUDIOCHANNELS, (uint64_t)stream->a_chans);
                mkv_put_ebml_float(mkv_ctx, MATROSKA_ID_AUDIOSAMPLINGFREQ, stream->a_rate);
                mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_AUDIOBITDEPTH, (uint64_t)stream->a_bits);
                mkv_end_ebml_master(mkv_ctx, subinfo);
                break;

            default:
               fprintf(stderr, "ENCODER: (matroska) Only audio and video are supported by the Matroska muxer.\n");
               break;
        }

        mkv_write_codecprivate(mkv_ctx, stream);
        mkv_write_trackdefaultduration(mkv_ctx, stream);

        mkv_end_ebml_master(mkv_ctx, track);
    }
    mkv_put_ebml_void(mkv_ctx, 200); // add some extra space
    mkv_end_ebml_master(mkv_ctx, tracks);
    return 0;
}

int mkv_write_header(mkv_context_t *mkv_ctx)
{
    ebml_master_t ebml_header, segment_info;
    int ret;

    ebml_header = mkv_start_ebml_master(mkv_ctx, EBML_ID_HEADER, 0);
    mkv_put_ebml_uint   (mkv_ctx, EBML_ID_EBMLVERSION        ,           1);
    mkv_put_ebml_uint   (mkv_ctx, EBML_ID_EBMLREADVERSION    ,           1);
    mkv_put_ebml_uint   (mkv_ctx, EBML_ID_EBMLMAXIDLENGTH    ,           4);
    mkv_put_ebml_uint   (mkv_ctx, EBML_ID_EBMLMAXSIZELENGTH  ,           8);
    if (mkv_ctx->mode == ENCODER_MUX_WEBM)
        mkv_put_ebml_string (mkv_ctx, EBML_ID_DOCTYPE        , "webm");
	else
		mkv_put_ebml_string (mkv_ctx, EBML_ID_DOCTYPE        , "matroska");
    mkv_put_ebml_uint   (mkv_ctx, EBML_ID_DOCTYPEVERSION     ,           2);
    mkv_put_ebml_uint   (mkv_ctx, EBML_ID_DOCTYPEREADVERSION ,           2);
    mkv_end_ebml_master(mkv_ctx, ebml_header);

    mkv_ctx->segment = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_SEGMENT, 0);
    mkv_ctx->segment_offset = io_get_offset(mkv_ctx->writer);

    /*
     * we write 2 seek heads - one at the end of the file to point to each
     * cluster, and one at the beginning to point to all other level one
     * elements (including the seek head at the end of the file), which
     * isn't more than 10 elements if we only write one of each other
     * currently defined level 1 element
     */
    mkv_ctx->main_seekhead    = mkv_start_seekhead(mkv_ctx, mkv_ctx->segment_offset, 10);

    if (!mkv_ctx->main_seekhead)
    {
		fprintf(stderr,"ENCODER: (matroska) couldn't allocate seekhead\n");
        return -1;
    }

    ret = mkv_add_seekhead_entry(mkv_ctx->main_seekhead, MATROSKA_ID_INFO, (uint64_t)io_get_offset(mkv_ctx->writer));
    if (ret < 0) return ret;

    segment_info = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_INFO, 0);
    mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_TIMECODESCALE, mkv_ctx->timescale);
    mkv_put_ebml_string(mkv_ctx, MATROSKA_ID_MUXINGAPP , "Deepin-camera Muxer-2014.04");
    mkv_put_ebml_string(mkv_ctx, MATROSKA_ID_WRITINGAPP, "Deepin-camera");
	
	int32_t seg_uid[4] = {0,0,0,0};
	/*generate seg uid - 16 byte random int*/
#ifdef HAS_GSL
	/*random generator setup*/
	gsl_rng_env_setup();
	const gsl_rng_type *T = gsl_rng_default;
	gsl_rng *r = gsl_rng_alloc (T);

	seg_uid[0] = INT_MIN + (int32_t) lround( INT_MAX * gsl_rng_uniform (r));
	seg_uid[1] = INT_MIN + (int32_t) lround( INT_MAX * gsl_rng_uniform (r));
	seg_uid[2] = INT_MIN + (int32_t) lround( INT_MAX * gsl_rng_uniform (r));
	seg_uid[3] = INT_MIN + (int32_t) lround( INT_MAX * gsl_rng_uniform (r));

	/*free the random seed generator*/
	gsl_rng_free (r);
#else
	time_t current_time = time(NULL);
	if(current_time != (time_t) -1)
	{
		struct tm *btime = localtime (&current_time);
		seg_uid[0] = btime->tm_year + 1900; //year
		seg_uid[1] = ((btime->tm_mon + 1) << 16) + btime->tm_mday; //month + day
		seg_uid[2] = (btime->tm_hour << 16) + btime->tm_min;       //hour + min
		seg_uid[3] = btime->tm_sec; //sec
	}
#endif

    mkv_put_ebml_binary(mkv_ctx, MATROSKA_ID_SEGMENTUID, seg_uid, 16);

    /* reserve space for the duration*/
    mkv_ctx->duration = 0;
    mkv_ctx->duration_offset = io_get_offset(mkv_ctx->writer);
    mkv_put_ebml_void(mkv_ctx, 11); /* assumes double-precision float to be written*/
    mkv_end_ebml_master(mkv_ctx, segment_info);

    ret = mkv_write_tracks(mkv_ctx);
    if (ret < 0) return ret;


    mkv_ctx->cues = mkv_start_cues(mkv_ctx->segment_offset);
    if (mkv_ctx->cues == NULL)
    {
		fprintf(stderr,"ENCODER: (matroska) couldn't allocate cues\n");
        return -1;
    }

    io_flush_buffer(mkv_ctx->writer);
    return 0;
}

static int mkv_processh264_nalu(uint8_t *data, int size)
{
	/*replace 00 00 00 01 (nalu marker) with nalu size*/
	int last_nalu = 0; /*marks last nalu in buffer*/
	int tot_nal = 0;
	uint8_t *nal_start = data;
	uint8_t *sp = data;
	uint8_t *ep = NULL;
	uint32_t nal_size = 0;

	while (!last_nalu)
	{
		nal_size = 0;

		/*search for NALU marker*/
		for(sp = nal_start; sp < data + size - 4; ++sp)
		{
			if(sp[0] == 0x00 &&
			   sp[1] == 0x00 &&
			   sp[2] == 0x00 &&
			   sp[3] == 0x01)
			{
				nal_start = sp + 4;
				break;
			}
		}

		/*search for end of NALU*/
		for(ep = nal_start; ep < data + size - 4; ++ep)
		{
			if(ep[0] == 0x00 &&
			   ep[1] == 0x00 &&
			   ep[2] == 0x00 &&
			   ep[3] == 0x01)
			{
                nal_size = (uint32_t)(ep - nal_start);
				nal_start = ep;/*reset for next NALU*/
				break;
			}
		}

		if(!nal_size)
		{
			last_nalu = 1;
            nal_size = (uint32_t)(data + size - nal_start);
		}

		sp[0] = (nal_size >> 24) & 0x000000FF;
		sp[1] = (nal_size >> 16) & 0x000000FF;
		sp[2] = (nal_size >> 8) & 0x000000FF;
		sp[3] = (nal_size) & 0x000000FF;

		tot_nal++;

	}

	return tot_nal;
}

static int mkv_blockgroup_size(int pkt_size)
{
    int size = pkt_size + 4;
    size += ebml_num_size((uint64_t)size);
    size += 2;              // EBML ID for block and block duration
    size += 8;              // max size of block duration
    size += ebml_num_size((uint64_t)size);
    size += 1;              // blockgroup EBML ID
    return size;
}

static void mkv_write_block(mkv_context_t* mkv_ctx,
                            unsigned int blockid,
                            int stream_index,
                            uint8_t *data,
                            int size,
                            uint64_t pts,
                            int flags)
{
	stream_io_t *stream = get_stream(mkv_ctx->stream_list, stream_index);
	if(stream->codec_id == AV_CODEC_ID_H264 && stream->h264_process)
		mkv_processh264_nalu(data, size);

	uint8_t block_flags = 0x00;

	if(!!(flags & AV_PKT_FLAG_KEY)) //for simple block
		block_flags |= 0x80;

    mkv_put_ebml_id(mkv_ctx, blockid);
    mkv_put_ebml_num(mkv_ctx,(uint64_t) (size+4), 0);
    io_write_w8(mkv_ctx->writer, (uint8_t)(0x80 | (stream_index + 1)));// this assumes stream_index is less than 126
    io_write_wb16(mkv_ctx->writer,(uint16_t)( pts - (uint64_t)mkv_ctx->cluster_pts)); //pts and cluster_pts are scaled
    io_write_w8(mkv_ctx->writer, block_flags);
    io_write_buf(mkv_ctx->writer, data, size);
}

static int mkv_write_packet_internal(mkv_context_t* mkv_ctx,
							int stream_index,
							uint8_t *data,
                            int size,
                            int duration,
                            uint64_t pts,
                            int flags)
{
    int keyframe = !!(flags & AV_PKT_FLAG_KEY);

	int use_simpleblock = 1;

    uint64_t ts = pts / mkv_ctx->timescale; //scale the time stamp:1000000

	stream_io_t *stream = get_stream(mkv_ctx->stream_list, stream_index);
	stream->packet_count++;

    if (!mkv_ctx->cluster_pos)
    {
        mkv_ctx->cluster_pos = io_get_offset(mkv_ctx->writer);
        mkv_ctx->cluster = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_CLUSTER, 0);
        mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_CLUSTERTIMECODE, MAX(0, ts));
        mkv_ctx->cluster_pts = MAX(0,(int64_t) ts);
    }

	if(use_simpleblock)
		mkv_write_block(mkv_ctx, MATROSKA_ID_SIMPLEBLOCK, stream_index, data, size, ts, flags);
	else
	{
        ebml_master_t blockgroup = mkv_start_ebml_master(mkv_ctx, MATROSKA_ID_BLOCKGROUP,(unsigned int) mkv_blockgroup_size(size));
		mkv_write_block(mkv_ctx, MATROSKA_ID_BLOCK, stream_index, data, size, ts, flags);
		if(duration)
            mkv_put_ebml_uint(mkv_ctx, MATROSKA_ID_BLOCKDURATION, (uint64_t)duration);
		mkv_end_ebml_master(mkv_ctx, blockgroup);
	}

    if (get_stream(mkv_ctx->stream_list, stream_index)->type == STREAM_TYPE_VIDEO && keyframe)
    {
		//fprintf(stderr,"mkv_ctx: add a cue point\n");
        int ret = mkv_add_cuepoint(mkv_ctx->cues, stream_index, (int64_t)ts, mkv_ctx->cluster_pos);
        if (ret < 0) 
			return ret;
    }

    mkv_ctx->duration = MAX(mkv_ctx->duration, (int64_t)ts /*+ duration*/);
    double strsa = (double) mkv_ctx->duration/1000;
    set_video_time_capture(strsa);

    //    update duration
    int64_t currentpos = io_get_offset(mkv_ctx->writer);
    io_seek(mkv_ctx->writer, mkv_ctx->duration_offset);

    mkv_put_ebml_float(mkv_ctx, MATROSKA_ID_DURATION, (double) mkv_ctx->duration);
    io_seek(mkv_ctx->writer, currentpos);

    return 0;
}

static int mkv_cache_packet(mkv_context_t* mkv_ctx,
							int stream_index,
							uint8_t *data,
                            int size,
                            int duration,
                            uint64_t pts,
                            int flags)
{

	if(mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].data_size > 0)
	{
		if(verbosity > 0)
			fprintf(stderr,"ENCODER: (matroska) packet buffer [%i] is in use: flushing cached data\n",
				mkv_ctx->pkt_buffer_write_index);

		int ret = mkv_write_packet_internal(mkv_ctx,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].stream_index,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].data,
                            (int)mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].data_size,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].duration,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].pts,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].flags);

        mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].data_size = 0;
        /*advance read index to next buffer*/
        mkv_ctx->pkt_buffer_read_index = mkv_ctx->pkt_buffer_write_index;
		NEXT_IND(mkv_ctx->pkt_buffer_read_index, mkv_ctx->pkt_buffer_list_size);

        if (ret < 0)
        {
            fprintf(stderr, "ENCODER: (matroska) Could not write cached audio packet\n");
            return ret;
        }

	}

    if(size > (int)mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].max_size)
	{
        mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].max_size = (unsigned int)size;

		if(mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].data == NULL)
            mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].data = calloc((size_t)size, sizeof(size_t));
		else
			mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].data = realloc(
				mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].data,
                (uint8_t)size * sizeof(uint8_t));
	}
	
	if (mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].data == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (mkv_cache_packet): %s\n", strerror(errno));
		exit(-1);
	}


	if(verbosity > 3)
		printf("ENCODER: (matroska) caching packet [%i]\n", mkv_ctx->pkt_buffer_write_index);

    memcpy(mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].data, data,(size_t) size);
    mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].data_size = (unsigned int)size;
    mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].duration = duration;
    mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].pts = pts;
    mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].flags = flags;
    mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_write_index].stream_index = stream_index;

    NEXT_IND(mkv_ctx->pkt_buffer_write_index, mkv_ctx->pkt_buffer_list_size);

    return 0;
}

/** public interface */
int mkv_write_packet(mkv_context_t* mkv_ctx,
					int stream_index,
					uint8_t *data,
                    int size,
                    int duration,
                    uint64_t pts,
                    int flags)
{
    int ret, keyframe = !!(flags & AV_PKT_FLAG_KEY);
    uint64_t ts = pts;

	ts -= mkv_ctx->first_pts;

    int cluster_size = (int)(io_get_offset(mkv_ctx->writer) - mkv_ctx->cluster_pos);

	stream_io_t *stream = get_stream(mkv_ctx->stream_list, stream_index);

    /* check if we have audio packets cached and write them up to video pts*/
    if (stream->type == STREAM_TYPE_VIDEO && 
		mkv_ctx->pkt_buffer_list_size > 0 &&
		mkv_ctx->pkt_buffer_list != NULL)
    {
		while(mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].pts < ts &&
			mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].data_size > 0)
		{
			if(verbosity > 3)
				printf("ENCODER: (matroska) writing cached packet[%i] of %i\n", 
					mkv_ctx->pkt_buffer_read_index, mkv_ctx->pkt_buffer_list_size);
			ret = mkv_write_packet_internal(mkv_ctx,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].stream_index,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].data,
                            (int)mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].data_size,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].duration,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].pts,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].flags);

			mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].data_size = 0;
			/*advance read index*/
			NEXT_IND(mkv_ctx->pkt_buffer_read_index, mkv_ctx->pkt_buffer_list_size);

			if (ret < 0)
			{
				fprintf(stderr, "ENCODER: (matroska) Could not write cached audio packet\n");
				return ret;
			}
		}
    }

    /*
     * start a new cluster every 6 MB and at least 5 sec,
     * or on a keyframe,
     * or every 3 MB if it is a video packet
     */
    if (mkv_ctx->cluster_pos &&
        ((cluster_size > 6*1024*1024 && ts >(uint64_t)( mkv_ctx->cluster_pts + 5000)) ||
         (stream->type == STREAM_TYPE_VIDEO && keyframe) ||
         (stream->type == STREAM_TYPE_VIDEO && cluster_size > 3*1024*1024)))
    {
        mkv_end_ebml_master(mkv_ctx, mkv_ctx->cluster);
        mkv_ctx->cluster_pos = 0;
    }

    /*
     *  buffer audio packets to ensure the packet containing the video
     *  timecode is contained in the same cluster
     */
    if (stream->type == STREAM_TYPE_AUDIO)
        ret = mkv_cache_packet(mkv_ctx, stream_index, data, size, duration, ts, flags);
    else
		ret = mkv_write_packet_internal(mkv_ctx, stream_index, data, size, duration, ts, flags);

    return ret;
}

int mkv_close(mkv_context_t* mkv_ctx)
{
    int64_t currentpos, cuespos;
    int ret;
	printf("ENCODER: (matroska) closing context\n");

    /* check if we have audio packets cached and write them */
    if (mkv_ctx->pkt_buffer_list != NULL && mkv_ctx->pkt_buffer_list_size > 0)
    {
		while(mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].data_size > 0)
		{
			ret = mkv_write_packet_internal(mkv_ctx,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].stream_index,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].data,
                            (int)mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].data_size,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].duration,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].pts,
							mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].flags);

			mkv_ctx->pkt_buffer_list[mkv_ctx->pkt_buffer_read_index].data_size = 0;
			/*advance read index*/
			NEXT_IND(mkv_ctx->pkt_buffer_read_index, mkv_ctx->pkt_buffer_list_size);

			if (ret < 0)
			{
				fprintf(stderr, "ENCODER: (matroska) Could not write cached audio packet\n");
				return ret;
			}
		}
    }

	printf("ENCODER: (matroska) closing cluster\n");
	if(mkv_ctx->cluster_pos)
		mkv_end_ebml_master(mkv_ctx, mkv_ctx->cluster);

	if (mkv_ctx->cues->num_entries)
	{
		printf("ENCODER: (matroska)writing cues\n");
		cuespos = mkv_write_cues(mkv_ctx, mkv_ctx->cues, mkv_ctx->stream_list_size);
		printf("ENCODER: (matroska)add seekhead\n");
        ret = mkv_add_seekhead_entry(mkv_ctx->main_seekhead, MATROSKA_ID_CUES, (uint64_t)cuespos);
        if (ret < 0) return ret;
	}
	printf("ENCODER: (matroska)write seekhead\n");
    mkv_write_seekhead(mkv_ctx, mkv_ctx->main_seekhead);

    // update the duration
    fprintf(stderr,"ENCODER: (matroska) end duration = %" PRIu64 " (%f) \n", mkv_ctx->duration, (double) mkv_ctx->duration);
    currentpos = io_get_offset(mkv_ctx->writer);
    io_seek(mkv_ctx->writer, mkv_ctx->duration_offset);

    double strsa = (double) mkv_ctx->duration/1000;
    set_video_time_capture(strsa);

    mkv_put_ebml_float(mkv_ctx, MATROSKA_ID_DURATION, (double) mkv_ctx->duration);
	io_seek(mkv_ctx->writer, currentpos);

    mkv_end_ebml_master(mkv_ctx, mkv_ctx->segment);
    getAvutil()->m_av_freep(&mkv_ctx->cues->entries);
    getAvutil()->m_av_freep(&mkv_ctx->cues);

    return 0;
}

mkv_context_t *mkv_create_context(const char* filename, int mode)
{
	mkv_context_t *mkv_ctx = calloc(1, sizeof(mkv_context_t));
	if (mkv_ctx == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (mkv_create_context): %s\n", strerror(errno));
		exit(-1);
	}

	mkv_ctx->writer = io_create_writer(filename, 0);
	mkv_ctx->mode = mode;
	mkv_ctx->main_seekhead = NULL;
	mkv_ctx->cues = NULL;
	mkv_ctx->stream_list = NULL;
	mkv_ctx->timescale = 1000000;

	mkv_ctx->pkt_buffer_list = NULL;
	mkv_ctx->pkt_buffer_list_size = 0;
	mkv_ctx->pkt_buffer_read_index = 0;
	mkv_ctx->pkt_buffer_write_index = 0;

	return mkv_ctx;
}

void mkv_destroy_context(mkv_context_t *mkv_ctx)
{
	io_destroy_writer(mkv_ctx->writer);

	destroy_stream_list(mkv_ctx->stream_list, &mkv_ctx->stream_list_size);

	/*free buffer list*/
	if(mkv_ctx->pkt_buffer_list)
	{
		int i = 0;
		for(i=0; i<mkv_ctx->pkt_buffer_list_size; ++i)
		{
			if(mkv_ctx->pkt_buffer_list[i].data)
				free(mkv_ctx->pkt_buffer_list[i].data);
		}
		free(mkv_ctx->pkt_buffer_list);
	}

	mkv_ctx->pkt_buffer_list = NULL;
	mkv_ctx->pkt_buffer_list_size = 0;
    free(mkv_ctx);
}

stream_io_t *mkv_add_video_stream(mkv_context_t *mkv_ctx,
					int32_t width,
					int32_t height,
					int32_t fps,
					int32_t fps_num,
					int32_t codec_id)
{
	stream_io_t *stream = add_new_stream(&mkv_ctx->stream_list, &mkv_ctx->stream_list_size);
	stream->type = STREAM_TYPE_VIDEO;
	stream->width = width;
	stream->height = height;
	stream->codec_id = codec_id;
	
	/*we have delayed video frames so increase the cached audio*/
	if(codec_id == AV_CODEC_ID_H264)
		mkv_ctx->pkt_buffer_list_size = 2 * PKT_BUFFER_DEF_SIZE;

	stream->fps = (double) fps/fps_num;
	stream->indexes = NULL;

	return stream;
}

stream_io_t *mkv_add_audio_stream(mkv_context_t *mkv_ctx,
					int32_t   channels,
					int32_t   rate,
					int32_t   bits,
					int32_t   mpgrate,
					int32_t   codec_id,
					int32_t   format)
{
	stream_io_t *stream = add_new_stream(&mkv_ctx->stream_list, &mkv_ctx->stream_list_size);
	stream->type = STREAM_TYPE_AUDIO;

	stream->a_chans = channels;
	stream->a_rate = rate;
	stream->a_bits = bits;
	stream->mpgrate = mpgrate;
	stream->a_vbr = 0;
	stream->codec_id = codec_id;
	stream->a_fmt = format;
	
	/*aprox. 4 sec cache*/
	if(!mkv_ctx->audio_frame_size)
		mkv_ctx->audio_frame_size = 1152;	
	if(mkv_ctx->pkt_buffer_list_size == 0)
		mkv_ctx->pkt_buffer_list_size = 4 * (rate/mkv_ctx->audio_frame_size);
	else if(mkv_ctx->pkt_buffer_list_size == 2 * PKT_BUFFER_DEF_SIZE) /*H264*/
	{
		if(4 * (rate/mkv_ctx->audio_frame_size) > mkv_ctx->pkt_buffer_list_size)
			mkv_ctx->pkt_buffer_list_size = 4 * (rate/mkv_ctx->audio_frame_size);
	}
	
	if(mkv_ctx->pkt_buffer_list == NULL)
	{
		mkv_ctx->pkt_buffer_write_index = 0;
		mkv_ctx->pkt_buffer_read_index = 0;
        mkv_ctx->pkt_buffer_list = calloc((size_t)(mkv_ctx->pkt_buffer_list_size), sizeof(mkv_packet_buff_t));
		if (mkv_ctx->pkt_buffer_list == NULL)
		{
			fprintf(stderr, "ENCODER: FATAL memory allocation failure (mkv_add_audio_stream): %s\n", strerror(errno));
			exit(-1);
		}

		int i = 0;
		for(i = 0; i < mkv_ctx->pkt_buffer_list_size; ++i)
		{
			mkv_ctx->pkt_buffer_list[i].max_size = 0;
			mkv_ctx->pkt_buffer_list[i].data_size = 0;
			mkv_ctx->pkt_buffer_list[i].data = NULL;
		}
	}

	stream->indexes = NULL;

	return stream;
}
