/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
#                             Add UYVY color support(Macbook iSight)            #
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

#ifndef FRAME_DECODER_H
#define FRAME_DECODER_H

#include "gviewv4l2core.h"
#include "v4l2_core.h"

/*h264 and jpeg decoder (libavcodec)*/
#ifdef HAVE_FFMPEG_AVCODEC_H
#include <ffmpeg/avcodec.h>
#else
#include <libavcodec/avcodec.h>
	#ifdef HAVE_LIBAVUTIL_VERSION_H
#include <libavutil/version.h>
#include <libavutil/imgutils.h>
	#endif
#include <libavutil/avutil.h>
#endif

#define LIBAVCODEC_VER_AT_LEAST(major,minor)  (LIBAVCODEC_VERSION_MAJOR > major || \
                                              (LIBAVCODEC_VERSION_MAJOR == major && \
                                               LIBAVCODEC_VERSION_MINOR >= minor))

#ifdef LIBAVUTIL_VERSION_MAJOR
#define LIBAVUTIL_VER_AT_LEAST(major,minor)  (LIBAVUTIL_VERSION_MAJOR > major || \
                                              (LIBAVUTIL_VERSION_MAJOR == major && \
                                               LIBAVUTIL_VERSION_MINOR >= minor))
#else
#define LIBAVUTIL_VER_AT_LEAST(major,minor) 0
#endif

int libav_decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt);

/*
 * Alloc image buffers for decoding video stream
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code  (0- E_OK)
 */
int alloc_v4l2_frames(v4l2_dev_t *vd);

/*
 * decode video stream ( from raw_frame to frame buffer (yuyv format))
 * args:
 *    vd - pointer to device data
 *
 * asserts:
 *    vd is not null
 *
 * returns: error code (E_OK)
 */
int decode_v4l2_frame(v4l2_dev_t *vd, v4l2_frame_buff_t *frame);

/*
 * free image buffers for decoding video stream
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: none
 */
void clean_v4l2_frames(v4l2_dev_t *vd);

#endif
