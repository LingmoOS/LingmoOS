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
#include <linux/videodev2.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
/* support for internationalization - i18n */
#include <locale.h>
#include <libintl.h>

#include "gview.h"
#include "encoder.h"
#include "gviewencoder.h"
#include "load_libs.h"
extern int verbosity;

/*if not defined don't set any bits but prevent build error*/
#ifndef CODEC_FLAG2_INTRA_REFRESH
#define CODEC_FLAG2_INTRA_REFRESH 0
#endif


static bmp_info_header_t mkv_codecPriv =
{
	.biSize = 0x00000028, /*40 bytes*/
	.biWidth = 640, /*default values (must be set before use)*/
	.biHeight = 480,
	.biPlanes = 1,
	.biBitCount = 24,
	.biCompression = V4L2_PIX_FMT_MJPEG,
	.biSizeImage = 640*480*2, /*2 bytes per pixel (max buffer - use x3 for RGB)*/
	.biXPelsPerMeter = 0,
	.biYPelsPerMeter = 0,
	.biClrUsed = 0,
	.biClrImportant = 0
};

/*list of software supported formats*/
static video_codec_t listSupCodecs[] =
{
	/*
	 * Raw camera input (yuvy or mjpg or H264)
	 *  we will set the correct codec data
	 *  depending on the V4L2_PIX_FMT_*
	 *  (this codec entry is always valid - index 0)
	 */
	{
		.valid        = 1,
		.compressor   = "YUY2", /*these will change according to camera input*/
		.mkv_4cc      = v4l2_fourcc('Y','U','Y','2'),
		.mkv_codec    = "V_MS/VFW/FOURCC",
		.mkv_codecPriv= &mkv_codecPriv,
		.description  = N_("Raw camera input"),
		.pix_fmt      = AV_PIX_FMT_NONE,
		.fps          = 0,
		.monotonic_pts= 0,
		.bit_rate     = 0,
		.qmax         = 0,
		.qmin         = 0,
		.max_qdiff    = 0,
		.dia          = 0,
		.pre_dia      = 0,
		.pre_me       = 0,
		.me_pre_cmp   = 0,
		.me_cmp       = 0,
		.me_sub_cmp   = 0,
		.last_pred    = 0,
		.gop_size     = 0,
		.qcompress    = 0,
		.qblur        = 0,
		.subq         = 0,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_NONE,
		.codec_name   = "none",
		.mb_decision  = 0,
		.trellis      = 0,
		.me_method    = 0,
		.mpeg_quant   = 0,
		.max_b_frames = 0,
		.num_threads  = 0,
		.flags        = 0
	},
	{
		.valid        = 1,
		.compressor   = "MJPG",
		.mkv_4cc      = v4l2_fourcc('M','J','P','G'),
		.mkv_codec    = "V_MS/VFW/FOURCC",
		.mkv_codecPriv= &mkv_codecPriv,
		.description  = N_("MJPG - compressed"),
		.pix_fmt      = AV_PIX_FMT_YUVJ420P,
		.fps          = 0,
		.monotonic_pts= 0,
		.bit_rate     = 0,
		.qmax         = 4,
		.qmin         = 1,
		.max_qdiff    = 2,
		.dia          = 2,
		.pre_dia      = 2,
		.pre_me       = 2,
		.me_pre_cmp   = 0,
		.me_cmp       = 3,
		.me_sub_cmp   = 3,
		.last_pred    = 2,
		.gop_size     = 12,
		.qcompress    = 0.5,
		.qblur        = 0.1,
		.subq         = 0,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_MJPEG,
		.codec_name   = "mjpeg",
		.mb_decision  = 0,
		.trellis      = 0,
		.me_method    = 0,
		.mpeg_quant   = 0,
		.max_b_frames = 0,
        .num_threads  = 0,
		.flags        = 0
	},
	{
		.valid        = 1,
		.compressor   = "MPEG",
        .mkv_4cc      = v4l2_fourcc('M','P','E','G'),
		.mkv_codec    = "V_MPEG1",
		.mkv_codecPriv= NULL,
		.description  = N_("MPEG video 1"),
		.pix_fmt      = AV_PIX_FMT_YUV420P,
		.fps          = 30,
		.monotonic_pts= 1,
		.bit_rate     = 3000000,
		.qmax         = 8,
		.qmin         = 2,
		.max_qdiff    = 2,
		.dia          = 2,
		.pre_dia      = 2,
		.pre_me       = 2,
		.me_pre_cmp   = 0,
		.me_cmp       = 3,
		.me_sub_cmp   = 3,
		.last_pred    = 2,
		.gop_size     = 12,
		.qcompress    = 0.5,
		.qblur        = 0.5,
		.subq         = 0,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_MPEG1VIDEO,
		.codec_name   = "mpeg1video",
		.mb_decision  = FF_MB_DECISION_RD,
		.trellis      = 1,
		.me_method    = 0,
		.mpeg_quant   = 0,
		.max_b_frames = 0,
		.num_threads  = 1,
		.flags        = 0
	},
	{
		.valid        = 1,
		.compressor   = "FLV1",
		.mkv_4cc      = v4l2_fourcc('F','L','V','1'),
		.mkv_codec    = "V_MS/VFW/FOURCC",
		.mkv_codecPriv= &mkv_codecPriv,
		.description  = N_("FLV1 - flash video 1"),
		.pix_fmt      = AV_PIX_FMT_YUV420P,
		.fps          = 0,
		.monotonic_pts= 1,
		.bit_rate     = 3000000,
		.qmax         = 31,
		.qmin         = 2,
		.max_qdiff    = 3,
		.dia          = 2,
		.pre_dia      = 2,
		.pre_me       = 2,
		.me_pre_cmp   = 0,
		.me_cmp       = 3,
		.me_sub_cmp   = 3,
		.last_pred    = 2,
		.gop_size     = 100,
		.qcompress    = 0.5,
		.qblur        = 0.5,
		.subq         = 0,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_FLV1,
		.codec_name   = "flv",
		.mb_decision  = FF_MB_DECISION_RD,
		.trellis      = 1,
		.me_method    = 0,
		.mpeg_quant   = 0,
		.max_b_frames = 0,
		.num_threads  = 1,
		.flags        = 0
	},
	{
		.valid        = 1,
		.compressor   = "WMV1",
		.mkv_4cc      = v4l2_fourcc('W','M','V','1'),
		.mkv_codec    = "V_MS/VFW/FOURCC",
		.mkv_codecPriv= &mkv_codecPriv,
		.description  = N_("WMV1 - win. med. video 7"),
		.pix_fmt      = AV_PIX_FMT_YUV420P,
		.fps          = 0,
		.monotonic_pts= 1,
		.bit_rate     = 3000000,
		.qmax         = 8,
		.qmin         = 2,
		.max_qdiff    = 2,
		.dia          = 2,
		.pre_dia      = 2,
		.pre_me       = 2,
		.me_pre_cmp   = 0,
		.me_cmp       = 3,
		.me_sub_cmp   = 3,
		.last_pred    = 2,
		.gop_size     = 100,
		.qcompress    = 0.5,
		.qblur        = 0.5,
		.subq         = 0,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_WMV1,
		.codec_name   = "wmv1",
		.mb_decision  = FF_MB_DECISION_RD,
		.trellis      = 1,
		.me_method    = 0,
		.mpeg_quant   = 0,
		.max_b_frames = 0,
		.num_threads  = 1,
		.flags        = 0
	},
	{
		.valid        = 1,
		.compressor   = "MPG2",
		.mkv_4cc      = v4l2_fourcc('M','P','G','2'),
		.mkv_codec    = "V_MPEG2",
		.mkv_codecPriv= NULL,
		.description  = N_("MPG2 - MPG2 format"),
		.pix_fmt      = AV_PIX_FMT_YUV420P,
		.fps          = 30,
		.monotonic_pts= 1,
		.bit_rate     = 3000000,
		.qmax         = 31,
		.qmin         = 2,
		.max_qdiff    = 3,
		.dia          = 2,
		.pre_dia      = 2,
		.pre_me       = 2,
		.me_pre_cmp   = 0,
		.me_cmp       = 3,
		.me_sub_cmp   = 3,
		.last_pred    = 2,
		.gop_size     = 12,
		.qcompress    = 0.5,
		.qblur        = 0.5,
		.subq         = 0,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_MPEG2VIDEO,
		.codec_name   = "mpeg2video",
		.mb_decision  = FF_MB_DECISION_RD,
		.trellis      = 1,
		.me_method    = 0,
		.mpeg_quant   = 0,
		.max_b_frames = 0,
		.num_threads  = 1,
		.flags        = 0
	},
	{
		.valid        = 1,
		.compressor   = "MP43",
		.mkv_4cc      = v4l2_fourcc('M','P','4','3'),
		.mkv_codec    = "V_MPEG4/MS/V3",
		.mkv_codecPriv= NULL,
		.description  = N_("MS MP4 V3"),
		.pix_fmt      = AV_PIX_FMT_YUV420P,
		.fps          = 0,
		.monotonic_pts= 1,
		.bit_rate     = 3000000,
		.qmax         = 31,
		.qmin         = 2,
		.max_qdiff    = 3,
		.dia          = 2,
		.pre_dia      = 2,
		.pre_me       = 2,
		.me_pre_cmp   = 0,
		.me_cmp       = 3,
		.me_sub_cmp   = 3,
		.last_pred    = 2,
		.gop_size     = 100,
		.qcompress    = 0.5,
		.qblur        = 0.5,
		.subq         = 0,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_MSMPEG4V3,
		.codec_name   = "msmpeg4v3",
		.mb_decision  = FF_MB_DECISION_RD,
		.trellis      = 1,
		.me_method    = 0,
		.mpeg_quant   = 0,
		.max_b_frames = 0,
		.num_threads  = 1,
		.flags        = 0
	},
	{
		.valid        = 1,
		.compressor   = "DX50",
		.mkv_4cc      = v4l2_fourcc('D','X','5','0'),
		.mkv_codec    = "V_MPEG4/ISO/ASP",
		.mkv_codecPriv= NULL,
		.description  = N_("MPEG4-ASP"),
		.pix_fmt      = AV_PIX_FMT_YUV420P,
		.fps          = 0,
		.monotonic_pts= 1,
		.bit_rate     = 1500000,
		.qmax         = 31,
		.qmin         = 2,
		.max_qdiff    = 3,
		.dia          = 2,
		.pre_dia      = 2,
		.pre_me       = 2,
		.me_pre_cmp   = 0,
		.me_cmp       = 3,
		.me_sub_cmp   = 3,
		.last_pred    = 2,
		.gop_size     = 100,
		.qcompress    = 0.5,
		.qblur        = 0.5,
		.subq         = 0,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_MPEG4,
		.codec_name   = "mpeg4",
		.mb_decision  = FF_MB_DECISION_RD,
		.trellis      = 1,
		.me_method    = 0,
		.mpeg_quant   = 1,
		.max_b_frames = 0,
		.num_threads  = 1,
		.flags        = 0
	},
	{
		.valid        = 1,
		.compressor   = "H264",
		.mkv_4cc      = v4l2_fourcc('H','2','6','4'),
		.mkv_codec    = "V_MPEG4/ISO/AVC",
		.mkv_codecPriv= NULL,
		.description  = N_("MPEG4-AVC (H264)"),
		.pix_fmt      = AV_PIX_FMT_YUV420P,
		.fps          = 0,
		.monotonic_pts= 1,
		.bit_rate     = 1500000,
		.qmax         = 51,
		.qmin         = 10,
		.max_qdiff    = 4,
		.dia          = 2,
		.pre_dia      = 2,
		.pre_me       = 2,
		.me_pre_cmp   = 0,
		.me_cmp       = 3,
		.me_sub_cmp   = 3,
		.last_pred    = 2,
		.gop_size     = 25,
		.qcompress    = 0.6,
		.qblur        = 0.5,
		.subq         = 5,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_H264,
		.codec_name   = "libx264",
		.mb_decision  = FF_MB_DECISION_RD,
		.trellis      = 0,
		.me_method    = X264_ME_HEX,
		.mpeg_quant   = 1,
		.max_b_frames = 16,
		.num_threads  = 4,
#if LIBAVCODEC_VER_AT_LEAST(54,01)
		.flags        = CODEC_FLAG2_INTRA_REFRESH
#else
		.flags        = CODEC_FLAG2_BPYRAMID | CODEC_FLAG2_WPRED | CODEC_FLAG2_FASTPSKIP | CODEC_FLAG2_INTRA_REFRESH
#endif
	},
#if LIBAVCODEC_VER_AT_LEAST(55,24)
	{
		.valid        = 1,
		.compressor   = "HEVC", //h265
		.mkv_4cc      = v4l2_fourcc('H','E','V','C'),
		.mkv_codec    = "V_MPEGH/ISO/HEVC",
		.mkv_codecPriv= NULL,
		.description  = N_("HEVC (H265)"),
		.pix_fmt      = AV_PIX_FMT_YUV420P,
		.fps          = 0,
		.monotonic_pts= 1,
		.bit_rate     = 1500000,
		.qmax         = 51,
		.qmin         = 10,
		.max_qdiff    = 4,
		.dia          = 2,
		.pre_dia      = 2,
		.pre_me       = 2,
		.me_pre_cmp   = 0,
		.me_cmp       = 3,
		.me_sub_cmp   = 3,
		.last_pred    = 2,
		.gop_size     = 100,
		.qcompress    = 0.6,
		.qblur        = 0.5,
		.subq         = 5,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_HEVC,
		.codec_name   = "libx265",
		.mb_decision  = FF_MB_DECISION_RD,
		.trellis      = 0,
		.me_method    = 0,
		.mpeg_quant   = 1,
		.max_b_frames = 16,
		.num_threads  = 4,
		.flags        = CODEC_FLAG2_INTRA_REFRESH
	},
#endif
	{
		.valid        = 1,
		.compressor   = "VP80",
		.mkv_4cc      = v4l2_fourcc('V','P','8','0'),
		.mkv_codec    = "V_VP8",
		.mkv_codecPriv= NULL,
		.description  = N_("VP8 (VP8)"),
		.pix_fmt      = AV_PIX_FMT_YUV420P,
		.fps          = 0,
		.monotonic_pts= 1,
		.bit_rate     = 600000,
		.qmax         = 51,
		.qmin         = 11,
		.max_qdiff    = 4,
		.dia          = 2,
		.pre_dia      = 2,
		.pre_me       = 2,
		.me_pre_cmp   = 0,
		.me_cmp       = 3,
		.me_sub_cmp   = 3,
		.last_pred    = 2,
		.gop_size     = 120,
		.qcompress    = 0.8,
		.qblur        = 0.5,
		.subq         = 5,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_VP8,
		.codec_name   = "libvpx_vp8",
		.mb_decision  = FF_MB_DECISION_RD,
		.trellis      = 0,
		.me_method    = 0,
		.mpeg_quant   = 1,
		.max_b_frames = 0,
		.num_threads  = 4,
		.flags        = 0
	},
#if LIBAVCODEC_VER_AT_LEAST(54,42)
	{
		.valid        = 1,
		.compressor   = "VP90",
		.mkv_4cc      = v4l2_fourcc('V','P','9','0'),
		.mkv_codec    = "V_VP9",
		.mkv_codecPriv= NULL,
		.description  = N_("VP9 (VP9)"),
		.pix_fmt      = AV_PIX_FMT_YUV420P,
		.fps          = 0,
		.monotonic_pts= 1,
		.bit_rate     = 600000,
		.qmax         = 51,
		.qmin         = 11,
		.max_qdiff    = 4,
		.dia          = 2,
		.pre_dia      = 2,
		.pre_me       = 2,
		.me_pre_cmp   = 0,
		.me_cmp       = 3,
		.me_sub_cmp   = 3,
		.last_pred    = 2,
		.gop_size     = 120,
		.qcompress    = 0.8,
		.qblur        = 0.5,
		.subq         = 5,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_VP9,
		.codec_name   = "libvpx_vp9",
		.mb_decision  = FF_MB_DECISION_RD,
		.trellis      = 0,
		.me_method    = 0,
		.mpeg_quant   = 1,
		.max_b_frames = 16,
		.num_threads  = 4,
		.flags        = 0
	},
#endif
	{
		.valid        = 1,
		.compressor   = "theo",
		.mkv_4cc      = v4l2_fourcc('t','h','e','o'),
		.mkv_codec    = "V_THEORA",
		.mkv_codecPriv= NULL,
		.description  = N_("Theora (ogg theora)"),
		.pix_fmt      = AV_PIX_FMT_YUV420P,
		.fps          = 0,
		.monotonic_pts= 1,
		.bit_rate     = 1500000,
		.qmax         = 51,
		.qmin         = 11,
		.max_qdiff    = 4,
		.dia          = 2,
		.pre_dia      = 2,
		.pre_me       = 2,
		.me_pre_cmp   = 0,
		.me_cmp       = 3,
		.me_sub_cmp   = 3,
		.last_pred    = 2,
		.gop_size     = 120,
		.qcompress    = 0.8,
		.qblur        = 0.5,
		.subq         = 5,
		.framerefs    = 0,
		.codec_id     = AV_CODEC_ID_THEORA,
		.codec_name   = "libtheora",
		.mb_decision  = FF_MB_DECISION_RD,
		.trellis      = 0,
		.me_method    = 0,
		.mpeg_quant   = 1,
		.max_b_frames = 0,
		.num_threads  = 4,
		.flags        = 0
	}
};

/*
 * get default mkv_codecPriv
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: pointer to bmp_info_header_t
 */
bmp_info_header_t *get_default_mkv_codecPriv()
{
	return &mkv_codecPriv;
}

/*
 * get video codec list size
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: listSupCodecs size (number of elements)
 */
int encoder_get_video_codec_list_size()
{
	int size = sizeof(listSupCodecs)/sizeof(video_codec_t);

	if(verbosity > 3)
		printf("ENCODER: video codec list size:%i\n", size);

	return size;
}

/*
 * get video codec valid list size
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: listSupCodecs valid number of elements
 */
//int encoder_get_video_codec_valid_list_size()
//{
//	int valid_size = 0;
//
//	int i = 0;
//	for(i = 0;  i < encoder_get_video_codec_list_size(); ++i)
//		if(listSupCodecs[i].valid)
//			valid_size++;
//
//	if(verbosity > 3)
//		printf("ENCODER: video codec valid list size:%i\n", valid_size);
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
	for (i = 0; i < encoder_get_video_codec_list_size(); ++i)
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
		real_index >= encoder_get_video_codec_list_size() ||
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
 * get the mkv codec private data
 * args:
 *    codec_ind - codec list index
 *
 * asserts:
 *    none
 *
 * returns: pointer to mkvCodecPriv data
 */
void *encoder_get_video_mkvCodecPriv(int codec_ind)
{
	int real_index = get_real_index (codec_ind);
	if(real_index >= 0 && real_index < encoder_get_video_codec_list_size())
		return ((void *) listSupCodecs[real_index].mkv_codecPriv);
	else
	{
		fprintf(stderr, "ENCODER: (mkvCodecPriv) bad codec index\n");
		return NULL;
	}
}

/*
 * set the video codec mkv private data
 * args:
 *    encoder_ctx - pointer to encoder context
 *
 * asserts:
 *    encoder_ctx is not null
 *
 * returns: mkvCodecPriv size
 */
int encoder_set_video_mkvCodecPriv(encoder_context_t *encoder_ctx)
{
	/*assertions*/
	assert(encoder_ctx != NULL);

	int size = 0;

	/*raw encoder*/
	if(encoder_ctx->video_codec_ind == 0)
	{
		switch(encoder_ctx->input_format)
		{
			case V4L2_PIX_FMT_H264:
			{
				/*do we have SPS and PPS data ?*/
				if(encoder_ctx->h264_sps_size <= 0 || encoder_ctx->h264_sps == NULL)
				{
					fprintf(stderr,"ENCODER: can't store H264 codec private data: No SPS data\n");
					return 0;
				}
				if(encoder_ctx->h264_pps_size <= 0 || encoder_ctx->h264_pps == NULL)
				{
					fprintf(stderr,"ENCODER: can't store H264 codec private data: No PPS data\n");
					return 0;
				}

				/*alloc the private data*/
				size = 6 + 2 + encoder_ctx->h264_sps_size + 1 + 2 + encoder_ctx->h264_pps_size;
				encoder_ctx->enc_video_ctx->priv_data = calloc(size, sizeof(uint8_t));
				if (encoder_ctx->enc_video_ctx->priv_data == NULL)
				{
					fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_set_video_mkvCodecPriv): %s\n", strerror(errno));
					exit(-1);
				}

				if(verbosity > 1)
					printf("ENCODER: (video priv_data) processing %i bytes\n", size);

				/*write the codec private data*/
				uint8_t *tp = encoder_ctx->enc_video_ctx->priv_data;
				/*header (6 bytes)*/
				tp[0] = 1; //version
				tp[1] = encoder_ctx->h264_sps[1]; /* profile */
				tp[2] = encoder_ctx->h264_sps[2]; /* profile compat */
				tp[3] = encoder_ctx->h264_sps[3]; /* level */
				tp[4] = 0xff; /* 6 bits reserved (111111) + 2 bits nal size length - 1 (11) */
				tp[5] = 0xe1; /* 3 bits reserved (111) + 5 bits number of sps (00001) */
				tp += 6;
				/*SPS: size (2 bytes) + SPS data*/
				tp[0] = (uint8_t) (encoder_ctx->h264_sps_size >> 8);
				tp[1] = (uint8_t) encoder_ctx->h264_sps_size; //38 for logitech uvc 1.1
				tp += 2; //SPS size (16 bit)
				memcpy(tp, encoder_ctx->h264_sps , encoder_ctx->h264_sps_size);
				tp += encoder_ctx->h264_sps_size;
				/*PPS number of pps (1 byte) + size (2 bytes) + PPS data*/
				tp[0] = 0x01; //number of pps
				tp[1] = (uint8_t) (encoder_ctx->h264_pps_size >> 8);
				tp[2] = (uint8_t) encoder_ctx->h264_pps_size; //4 for logitech uvc 1.1
				tp += 3; //PPS size (16 bit)
				memcpy(tp, encoder_ctx->h264_pps , encoder_ctx->h264_pps_size);

				listSupCodecs[0].mkv_codecPriv = encoder_ctx->enc_video_ctx->priv_data;
				break;
			}

			default:
			{
				bmp_info_header_t *mkv_codecPriv = get_default_mkv_codecPriv();
				size = 40;
				mkv_codecPriv->biWidth = encoder_ctx->video_width;
				mkv_codecPriv->biHeight = encoder_ctx->video_height;
				mkv_codecPriv->biCompression = encoder_ctx->input_format;
				mkv_codecPriv->biSizeImage = encoder_ctx->video_width*encoder_ctx->video_height*3; /*3 bytes per pixel (max buffer - use x3 for RGB)*/
				listSupCodecs[0].mkv_codecPriv = (void *) mkv_codecPriv;
				break;
			}
		}

		return(size);
	}

	/*assert video encoder context is not null*/
	assert( encoder_ctx->enc_video_ctx);
	encoder_codec_data_t *video_codec_data = (encoder_codec_data_t *) encoder_ctx->enc_video_ctx->codec_data;
	/*assert video codec data is not null*/	
	assert(video_codec_data);
	
	int codec_id = video_codec_data->codec_context->codec_id;
	int real_index = get_video_codec_index(codec_id);

	if(codec_id == AV_CODEC_ID_THEORA)
	{
		/*get the 3 first header packets*/
		uint8_t *header_start[3];
		int header_len[3];
		int first_header_size;

		first_header_size = 42; /*vorbis = 30*/
    	if (avpriv_split_xiph_headers(
			video_codec_data->codec_context->extradata,
			video_codec_data->codec_context->extradata_size,
			first_header_size, header_start, header_len) < 0)
        {
			fprintf(stderr, "ENCODER: (theora codec) - Extradata corrupt.\n");
			return -1;
		}

		/*get the allocation needed for headers size*/
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

		size = 1 + /*number of packets -1*/
				header_lace_size[0] +  //first packet size
				header_lace_size[1] +  //second packet size
				header_len[0] + //first packet header
				header_len[1] + //second packet header
				header_len[2];  //third packet header

		/*should check and clean before allocating ??*/
		encoder_ctx->enc_video_ctx->priv_data = calloc(size, sizeof(uint8_t));
		if (encoder_ctx->enc_video_ctx->priv_data == NULL)
		{
			fprintf(stderr, "ENCODER: FATAL memory allocation failure (encoder_set_video_mkvCodecPriv): %s\n", strerror(errno));
			exit(-1);
		}
		/*write header*/
		uint8_t *tmp = encoder_ctx->enc_video_ctx->priv_data;
		*tmp++ = 0x02; /*number of packets -1*/
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

		listSupCodecs[real_index].mkv_codecPriv = encoder_ctx->enc_video_ctx->priv_data;
	}
	else if(listSupCodecs[real_index].mkv_codecPriv != NULL)
	{
		bmp_info_header_t *mkv_codecPriv = get_default_mkv_codecPriv();

		mkv_codecPriv->biWidth = encoder_ctx->video_width;
		mkv_codecPriv->biHeight = encoder_ctx->video_height;
		mkv_codecPriv->biCompression = listSupCodecs[real_index].mkv_4cc;;
		mkv_codecPriv->biSizeImage = mkv_codecPriv->biWidth * mkv_codecPriv->biHeight * 2; /*2 bytes per pixel (max buffer - use x3 for RGB)*/
		listSupCodecs[real_index].mkv_codecPriv = (void *) mkv_codecPriv;

		size = 40; //40 bytes
	}

	return (size);
}

/*
 * checks if the video codec index corresponds to VP8 or VP9 (webm) codec
 * args:
 *    codec_ind - video codec list index
 *
 * asserts:
 *    none
 *
 * returns: 1 true; 0 false
 */
int encoder_check_webm_video_codec(int codec_ind)
{
	int real_index = get_real_index (codec_ind);

	int ret = 0;
	if(real_index >= 0 && real_index < encoder_get_video_codec_list_size())
		ret = ((listSupCodecs[real_index].codec_id == AV_CODEC_ID_VP8)
#if LIBAVCODEC_VER_AT_LEAST(54,42)
				|| (listSupCodecs[real_index].codec_id == AV_CODEC_ID_VP9)
#endif
			 ) ? 1: 0;

	return ret;
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
int get_video_codec_index(int codec_id)
{
	int i = 0;
	for(i = 0; i < encoder_get_video_codec_list_size(); ++i )
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
int get_video_codec_list_index(int codec_id)
{
	return get_list_index(get_video_codec_index(codec_id));
}

/*
 * get the video codec index for VP8 (webm) codec
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: index for VP8 codec or -1 if error
 */
int encoder_get_webm_video_codec_index()
{
    return get_video_codec_list_index(AV_CODEC_ID_VP8);
}

/*
 * get video list codec entry for codec index
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: list codec entry or NULL if none
 */
video_codec_t *encoder_get_video_codec_defaults(int codec_ind)
{
	int real_index = get_real_index (codec_ind);

	if(real_index >= 0 && real_index < encoder_get_video_codec_list_size())
		return (&(listSupCodecs[real_index]));
	else
	{
		fprintf(stderr, "ENCODER: (video codec defaults) bad codec index (%i)\n", codec_ind);
		return NULL;
	}
}

/*
 * sets the valid flag in the video codecs list
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: number of valid video codecs in list
 */
int encoder_set_valid_video_codec_list ()
{
	int ind = 1;
	int num_codecs = 1; /*raw codec (no encoding) is always valid*/
	for ( ind = 1; ind < encoder_get_video_codec_list_size(); ++ind)
	{
        AVCodec *codec = getLoadLibsInstance()->m_avcodec_find_encoder(listSupCodecs[ind].codec_id);
		if (!codec)
		{
			printf("ENCODER: no video codec detected for %s\n", listSupCodecs[ind].description);
			listSupCodecs[ind].valid = 0;
		}
		else num_codecs++;
	}

	return num_codecs;
}

/*
 * get video list codec description
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: list codec entry or NULL if none
 */
const char *encoder_get_video_codec_description(int codec_ind)
{
	int real_index = get_real_index (codec_ind);
	if(real_index >= 0 && real_index < encoder_get_video_codec_list_size())
		return (listSupCodecs[real_index].description);
	else
	{
		fprintf(stderr, "ENCODER: (video codec description) bad codec index (%i)\n", codec_ind);
		return NULL;
	}
}

/*
 * get video mkv codec
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: mkv codec entry or NULL if none
 */
const char *encoder_get_video_mkv_codec(int codec_ind)
{
	int real_index = get_real_index (codec_ind);
	if(real_index >= 0 && real_index < encoder_get_video_codec_list_size())
		return (listSupCodecs[real_index].mkv_codec);
	else
	{
		fprintf(stderr, "ENCODER: (video mkv codec) bad codec index (%i)\n", codec_ind);;
		return NULL;
	}
}

/*
 * get video compressor (avi 4cc code)
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: compressor codec entry or NULL if none
 */
const char *encoder_get_video_codec_4cc(int codec_ind)
{
	int real_index = get_real_index (codec_ind);
	if(real_index >= 0 && real_index < encoder_get_video_codec_list_size())
		return (listSupCodecs[real_index].compressor);
	else
	{
		fprintf(stderr, "ENCODER: (video mkv codec) bad codec index (%i)\n", codec_ind);;
		return NULL;
	}
}

/*
 * get video codec list index for avi 4cc
 * args:
 *   codec_4cc - codec 4cc
 *
 * asserts:
 *   none
 *
 * returns: codec index or -1 if error
 */
int encoder_get_video_codec_ind_4cc(const char *codec_4cc)
{
	if(strcasecmp(codec_4cc, "raw") == 0)
		return 0; /*raw is always 0*/

	int real_index = 1;
	int index = 0; /*skip raw codec*/
	for(real_index = 1; real_index < encoder_get_video_codec_list_size(); ++real_index)
	{
		if(listSupCodecs[real_index].valid)
			index++;
		if(strcasecmp(codec_4cc, listSupCodecs[real_index].compressor) == 0)
			return index;
	}

	return -1;
}
