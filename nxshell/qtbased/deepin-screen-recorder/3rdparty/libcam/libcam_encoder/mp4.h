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

#ifndef MP4_H
#define MP4_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#include "encoder.h"
#include "gviewencoder.h"
#include "gview.h"

#ifdef __cplusplus
} // endof extern "C"
#endif

typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;

AVFormatContext *mp4_create_context(const char *filename);

void mp4_add_video_stream(
    AVFormatContext *mp4_ctx,
    encoder_codec_data_t *video_codec_data,
    OutputStream *video_stream);

void mp4_add_audio_stream(
    AVFormatContext *mp4_ctx,
    encoder_codec_data_t *audio_codec_data,
    OutputStream *audio_stream);


int mp4_write_packet(
    AVFormatContext *mp4_ctx,
    encoder_codec_data_t *codec_data,
    int stream_index,
    uint8_t *outbuf,
    uint32_t outbuf_size,
    uint64_t pts,
    int flags);

int mp4_write_header(AVFormatContext *mp4_ctx);

// int mp4_close(AVFormatContext *mp4_ctx);

void mp4_destroy_context(AVFormatContext *mp4_ctx);

#endif
