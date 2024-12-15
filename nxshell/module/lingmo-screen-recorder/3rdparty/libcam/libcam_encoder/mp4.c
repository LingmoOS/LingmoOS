/*
* Copyright (C) 2020 ~ %YEAR% Uniontech Software Technology Co.,Ltd.
*
* Author:     shicetu <shicetu@uniontech.com>
*             hujianbo <hujianbo@uniontech.com>
* Maintainer: shicetu <shicetu@uniontech.com>
*             hujianbo <hujianbo@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mp4.h"
#include <libavutil/mathematics.h>
#include "load_libs.h"

static int64_t video_pts = 0;
static int64_t audio_pts = 0;
static int64_t first_pts = 0;

AVFormatContext *mp4_create_context(const char *filename)
{
    AVFormatContext *mp4_ctx;

    getAvformat()->m_avformat_alloc_output_context2(&mp4_ctx, NULL, NULL, filename);

    if (!mp4_ctx) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        getAvformat()->m_avformat_alloc_output_context2(&mp4_ctx, NULL, "mpeg", filename);
    }

    if (!mp4_ctx)
    {
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (mp4_create_context): %s\n", strerror(errno));
		exit(-1);
	}

    return mp4_ctx;
}

void mp4_add_video_stream(
		AVFormatContext *mp4_ctx,
        encoder_codec_data_t *video_codec_data,
        OutputStream *video_stream)
{
    video_stream->st = getAvformat()->m_avformat_new_stream(mp4_ctx, video_codec_data->codec);

    if(!video_stream->st)
    {
        fprintf(stderr,"Could not allocate stream\n");
        exit(1);
    }

    video_stream->st->id = mp4_ctx->nb_streams - 1;
    video_stream->enc = video_codec_data->codec_context;
    video_stream->st->time_base = video_codec_data->codec_context->time_base;

    if(mp4_ctx->oformat->flags & AVFMT_GLOBALHEADER)
    {
        video_stream->enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
}

void mp4_add_audio_stream(
		AVFormatContext *mp4_ctx,
        encoder_codec_data_t *audio_codec_data,
        OutputStream *audio_stream)
{

    audio_stream->st = getAvformat()->m_avformat_new_stream(mp4_ctx,audio_codec_data->codec);
    if(!audio_stream->st)
    {
        fprintf(stderr,"Could not allocate stream\n");
        exit(1);
    }
    audio_stream->st->id = mp4_ctx->nb_streams - 1;
    audio_stream->st->time_base = audio_codec_data->codec_context->time_base;
    audio_stream->enc = audio_codec_data->codec_context;
    if(mp4_ctx->oformat->flags & AVFMT_GLOBALHEADER)
    {
        audio_stream->enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
}

int mp4_write_packet(
        AVFormatContext *mp4_ctx,
        encoder_codec_data_t *codec_data,
        int stream_index,
        uint8_t *outbuf,
        uint32_t outbuf_size,
        uint64_t pts,
        int flags)
{
    AVPacket *outpacket = codec_data->outpkt;
    outpacket->data = (uint8_t*)calloc((unsigned int)outbuf_size, sizeof(uint8_t));

    memcpy(outpacket->data, outbuf, outbuf_size);
    outpacket->size =(int)outbuf_size;

    if(codec_data->codec_context->codec_type == AVMEDIA_TYPE_VIDEO){
        outpacket->pts = video_pts;
        outpacket->dts = video_pts;
        outpacket->duration = 0;
        outpacket->flags = flags;
        outpacket->stream_index = stream_index;
        AVRational video_time = mp4_ctx->streams[stream_index]->time_base;
        AVRational time_base = codec_data->codec_context->time_base;

        getLoadLibsInstance()->m_av_packet_rescale_ts(outpacket, time_base, video_time);
        getAvformat()->m_av_write_frame(mp4_ctx, outpacket);

        set_video_time_capture((double)(pts)/1000/1000000);
        if (first_pts == 0) {
            first_pts = pts;
        } else {
            video_pts = (double)(pts - first_pts) / 1000000 / 33;
        }
    }

    if(codec_data->codec_context->codec_type == AVMEDIA_TYPE_AUDIO) {

        outpacket->pts = audio_pts;
        outpacket->flags = flags;
        outpacket->stream_index = stream_index;
        AVRational audio_time = mp4_ctx->streams[stream_index]->time_base;
        AVRational time_base = codec_data->codec_context->time_base;

        getLoadLibsInstance()->m_av_packet_rescale_ts(outpacket, time_base, audio_time);
        getAvformat()->m_av_write_frame(mp4_ctx, outpacket);

        //AAC音频标准是1024
        //MP3音频标准是1152
        audio_pts+= 1024;
    }

    if(outpacket->data){
        free(outpacket->data);
        outpacket->data = NULL;
        getLoadLibsInstance()->m_av_packet_unref(outpacket);
    }
    return 0;
}


void mp4_destroy_context(AVFormatContext *mp4_ctx)
{
    video_pts = 0;
    audio_pts = 0;
    first_pts = 0;
    if(mp4_ctx != NULL)
    {
        getAvformat()->m_avformat_free_context(mp4_ctx);
        //mp4_ctx = NULL;
    }
}


