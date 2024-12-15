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

#ifndef UVC_H264_H
#define UVC_H264_H

#include "gviewv4l2core.h"
#include "v4l2_core.h"
#include "v4l2_xu_ctrls.h"

/*H264 support type*/
#define H264_NONE    (0)
#define H264_FRAME   (1)
#define H264_MUXED   (2)

/* UVC H.264 control selectors */
#define UVCX_VIDEO_CONFIG_PROBE			0x01
#define	UVCX_VIDEO_CONFIG_COMMIT		0x02
#define	UVCX_RATE_CONTROL_MODE			0x03
#define	UVCX_TEMPORAL_SCALE_MODE		0x04
#define UVCX_SPATIAL_SCALE_MODE			0x05
#define	UVCX_SNR_SCALE_MODE				0x06
#define	UVCX_LTR_BUFFER_SIZE_CONTROL	0x07
#define	UVCX_LTR_PICTURE_CONTROL		0x08
#define	UVCX_PICTURE_TYPE_CONTROL		0x09
#define	UVCX_VERSION					0x0A
#define	UVCX_ENCODER_RESET				0x0B
#define	UVCX_FRAMERATE_CONFIG			0x0C
#define	UVCX_VIDEO_ADVANCE_CONFIG		0x0D
#define	UVCX_BITRATE_LAYERS				0x0E
#define	UVCX_QP_STEPS_LAYERS			0x0F

/* bmHints defines */
#define BMHINTS_RESOLUTION        0x0001
#define BMHINTS_PROFILE           0x0002
#define BMHINTS_RATECONTROL       0x0004
#define BMHINTS_USAGE             0x0008
#define BMHINTS_SLICEMODE         0x0010
#define BMHINTS_SLICEUNITS        0x0020
#define BMHINTS_MVCVIEW           0x0040
#define BMHINTS_TEMPORAL          0x0080
#define BMHINTS_SNR               0x0100
#define BMHINTS_SPATIAL           0x0200
#define BMHINTS_SPATIAL_RATIO     0x0400
#define BMHINTS_FRAME_INTERVAL    0x0800
#define BMHINTS_LEAKY_BKT_SIZE    0x1000
#define BMHINTS_BITRATE           0x2000
#define BMHINTS_ENTROPY           0x4000
#define BMHINTS_IFRAMEPERIOD      0x8000

/* wSliceMode defines */
#define SLICEMODE_BITSPERSLICE    0x0001
#define SLICEMODE_MBSPERSLICE     0x0002
#define SLICEMODE_SLICEPERFRAME   0x0003

/***********************************************************************************************************************
* bUsageType defines
* The bUsageType used in Probe/Commit structure. The UCCONFIG parameters are based on "UCConfig Modes v1.1".
* bUsageType  UCConfig   Description
*   4           0        Non-scalable single layer AVC bitstream with simulcast(number of simulcast streams>=1)
*   5           1        SVC temporal scalability with hierarchical P with simulcast(number of simulcast streams>=1)
*   6           2q       SVC temporal scalability + Quality/SNR scalability with simulcast(number of simulcast streams>=1)
*   7           2s       SVC temporal scalability + spatial scalability with simulcast(number of simulcast streams>=1)
*   8           3        Full SVC scalability (temporal scalability + SNR scalability + spatial scalability)
*                        with simulcast(number of simulcast streams>=1)
************************************************************************************************************************/

#define USAGETYPE_REALTIME        0x01
#define USAGETYPE_BROADCAST       0x02
#define USAGETYPE_STORAGE         0x03
#define USAGETYPE_UCCONFIG_0      0x04
#define USAGETYPE_UCCONFIG_1      0x05
#define USAGETYPE_UCCONFIG_2Q     0x06
#define USAGETYPE_UCCONFIG_2S     0x07
#define USAGETYPE_UCCONFIG_3      0x08

/* bRateControlMode defines */
#define RATECONTROL_CBR           0x01
#define RATECONTROL_VBR           0x02
#define RATECONTROL_CONST_QP      0x03
#define RATECONTROL_FIXED_FRM_FLG 0x10

/* bStreamFormat defines */
#define STREAMFORMAT_ANNEXB       0x00
#define STREAMFORMAT_NAL          0x01

/* bEntropyCABAC defines */
#define ENTROPY_CAVLC             0x00
#define ENTROPY_CABAC             0x01

/* bTimingstamp defines */
#define TIMESTAMP_SEI_DISABLE     0x00
#define TIMESTAMP_SEI_ENABLE      0x01

/* bPreviewFlipped defines */
#define PREFLIPPED_DISABLE        0x00
#define PREFLIPPED_HORIZONTAL     0x01

/* wPictureType defines */
#define PICTURE_TYPE_IFRAME 	0x0000 //Generate an IFRAME
#define PICTURE_TYPE_IDR		0x0001 //Generate an IDR
#define PICTURE_TYPE_IDR_FULL	0x0002 //Generate an IDR frame with new SPS and PPS

/* bStreamMuxOption defines */
#define STREAMMUX_H264          (1 << 0) | (1 << 1)
#define STREAMMUX_YUY2          (1 << 0) | (1 << 2)
#define STREAMMUX_YUYV          (1 << 0) | (1 << 2)
#define STREAMMUX_NV12          (1 << 0) | (1 << 3)

/* wLayerID Macro */

/*                              wLayerID
  |------------+------------+------------+----------------+------------|
  |  Reserved  |  StreamID  | QualityID  |  DependencyID  | TemporalID |
  |  (3 bits)  |  (3 bits)  | (3 bits)   |  (4 bits)      | (3 bits)   |
  |------------+------------+------------+----------------+------------|
  |15        13|12        10|9          7|6              3|2          0|
  |------------+------------+------------+----------------+------------|
*/

#define xLayerID(stream_id, quality_id, dependency_id, temporal_id) ((((stream_id)&7)<<10)|(((quality_id)&7)<<7)|(((dependency_id)&15)<<3)|((temporal_id)&7))

/* id extraction from wLayerID */
#define xStream_id(layer_id)      (((layer_id)>>10)&7)
#define xQuality_id(layer_id)     (((layer_id)>>7)&7)
#define xDependency_id(layer_id)  (((layer_id)>>3)&15)
#define xTemporal_id(layer_id)    ((layer_id)&7)

/* h264 probe commit struct (defined in gviewv4l2core.h) */

/* rate control */
typedef struct _uvcx_rate_control_mode_t
{
	uint16_t	wLayerID;
	uint8_t		bRateControlMode;
} __attribute__((__packed__)) uvcx_rate_control_mode_t;

/* temporal scale */
typedef struct _uvcx_temporal_scale_mode_t
{
	uint16_t	wLayerID;
	uint8_t		bTemporalScaleMode;
} __attribute__((__packed__)) uvcx_temporal_scale_mode_t;

/* spatial scale mode */
typedef struct _uvcx_spatial_scale_mode_t
{
	uint16_t	wLayerID;
	uint8_t		bSpatialScaleMode;
} __attribute__((__packed__)) uvcx_spatial_scale_mode_t;

/* snr scale mode */
typedef struct _uvcx_snr_scale_mode_t
{
	uint16_t	wLayerID;
	uint8_t		bSNRScaleMode;
	uint8_t		bMGSSublayerMode;
} __attribute__((__packed__)) uvcx_snr_scale_mode_t;

/* buffer size control*/
typedef struct _uvcx_ltr_buffer_size_control_t
{
	uint16_t	wLayerID;
	uint8_t		bLTRBufferSize;
	uint8_t		bLTREncoderControl;
} __attribute__((__packed__)) uvcx_ltr_buffer_size_control_t;

/* ltr picture control */
typedef struct _uvcx_ltr_picture_control
{
	uint16_t	wLayerID;
	uint8_t		bPutAtPositionInLTRBuffer;
	uint8_t		bEncodeUsingLTR;
} __attribute__((__packed__)) uvcx_ltr_picture_control;

/* picture type control */
typedef struct _uvcx_picture_type_control_t
{
	uint16_t	wLayerID;
	uint16_t	wPicType;
} __attribute__((__packed__)) uvcx_picture_type_control_t;

/* version */
typedef struct _uvcx_version_t
{
	uint16_t	wVersion;
} __attribute__((__packed__)) uvcx_version_t;

/* encoder reset */
typedef struct _uvcx_encoder_reset
{
	uint16_t	wLayerID;
} __attribute__((__packed__)) uvcx_encoder_reset;

/* frame rate */
typedef struct _uvcx_framerate_config_t
{
	uint16_t	wLayerID;
	uint32_t	dwFrameInterval;
} __attribute__((__packed__)) uvcx_framerate_config_t;

/* advance config */
typedef struct _uvcx_video_advance_config_t
{
	uint16_t	wLayerID;
	uint32_t	dwMb_max;
	uint8_t		blevel_idc;
	uint8_t		bReserved;
} __attribute__((__packed__)) uvcx_video_advance_config_t;

/* bit rate */
typedef struct _uvcx_bitrate_layers_t
{
	uint16_t	wLayerID;
	uint32_t	dwPeakBitrate;
	uint32_t	dwAverageBitrate;
} __attribute__((__packed__)) uvcx_bitrate_layers_t;

/* qp steps */
typedef struct _uvcx_qp_steps_layers_t
{
	uint16_t	wLayerID;
	uint8_t		bFrameType;
	uint8_t		bMinQp;
	uint8_t		bMaxQp;
} __attribute__((__packed__)) uvcx_qp_steps_layers_t;

/*
 * get h264 support type
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: support type (H264_NONE; H264_MUXED; H264_FRAME)
 */
int h264_get_support();

/*
 * gets the uvc h264 xu control unit id, if any
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *   vd->list_devices is not null
 *
 * returns: unit id or 0 if none
 *  (also sets vd->h264_unit_id)
 */
uint8_t get_uvc_h624_unit_id (v4l2_dev_t *vd);

/*
 * check for uvc h264 support by querying UVCX_VERSION
 * although geting a unit id > 0 from xu_get_unit_id
 * should be enought
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid ( > 0 )
 *
 * returns: 1 if support available or 0 otherwise
 */
int check_h264_support(v4l2_dev_t *vd);

/*
 * adds h264 to the format list, if supported by device
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *   vd->list_stream_formats is not null
 *
 * returns: void
 */
void add_h264_format(v4l2_dev_t *vd);

/*
 * sets h264 muxed format (must not be called while streaming)
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: void
 */
void set_h264_muxed_format(v4l2_dev_t *vd);

/*
 * check if h264 decoder is available from libavcodec
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: TRUE (1)
 *          FALSE(0)
 */
//uint8_t h264_has_decoder();

/*
 * request a frame of type wPictureType to the h264 encoder
 * args:
 *   vd - pointer to video device data
 *   type - frame type
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code
 */
int request_h264_frame_type(v4l2_dev_t *vd, uint16_t type);

/*
 * request a IDR frame from the H264 encoder
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: none
 */
void h264_request_idr(v4l2_dev_t *vd);

/*
 * resets the h264 encoder
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: 0 on success or error code on fail
 */
int h264_reset_encoder(v4l2_dev_t *vd);

/*
 * get the video rate control mode
 * args:
 *   vd - pointer to video device data
 *   query - query type
 *
 * asserts:
 *   vd is not null
 *
 * returns: video rate control mode (FIXME: 0xff on error)
 */
uint8_t  h264_get_video_rate_control_mode(v4l2_dev_t *vd, uint8_t query);

/*
 * set the video rate control mode
 * args:
 *   vd - pointer to video device data
 *   mode - rate mode
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code ( 0 -OK)
 */
int h264_set_video_rate_control_mode(v4l2_dev_t *vd, uint8_t mode);

/*
 * get the temporal scale mode
 * args:
 *   vd - pointer to video device data
 *   query - query type
 *
 * asserts:
 *   vd is not null
 *
 * returns: temporal scale mode (FIXME: 0xff on error)
 */
uint8_t h264_get_temporal_scale_mode(v4l2_dev_t *vd, uint8_t query);

/*
 * set the temporal scale mode
 * args:
 *   vd - pointer to video device data
 *   mode - temporal scale mode
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code ( 0 -OK)
 */
int h264_set_temporal_scale_mode(v4l2_dev_t *vd, uint8_t mode);

/*
 * get the spatial scale mode
 * args:
 *   vd - pointer to video device data
 *   query - query type
 *
 * asserts:
 *   vd is not null
 *
 * returns: temporal scale mode (FIXME: 0xff on error)
 */
uint8_t h264_get_spatial_scale_mode(v4l2_dev_t *vd, uint8_t query);

/*
 * set the spatial scale mode
 * args:
 *   vd - pointer to video device data
 *   mode - spatial scale mode
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code ( 0 -OK)
 */
int h264_set_spatial_scale_mode(v4l2_dev_t *vd, uint8_t mode);

/*
 * query the frame rate config
 * args:
 *   vd - pointer to video device data
 *   query - query type
 *
 * asserts:
 *   vd is not null
 *
 * returns: frame rate config (FIXME: 0xffffffff on error)
 */
uint32_t h264_query_frame_rate_config(v4l2_dev_t *vd, uint8_t query);

/*
 * get the frame rate config
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: frame rate config (FIXME: 0xffffffff on error)
 */
uint32_t h264_get_frame_rate_config(v4l2_dev_t *vd);

/*
 * set the frame rate config
 * args:
 *   vd - pointer to video device data
 *   framerate - framerate
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code ( 0 -OK)
 */
int h264_set_frame_rate_config(v4l2_dev_t *vd, uint32_t framerate);

/*
 * updates the h264_probe_commit_req field
 * args:
 *   vd - pointer to video device data
 *   query - (UVC_GET_CUR; UVC_GET_MAX; UVC_GET_MIN)
 *   config_probe_cur - pointer to uvcx_video_config_probe_commit_t:
 *     if null vd->h264_config_probe_req will be used
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code ( 0 -OK)
 */
int h264_probe_config_probe_req(
			v4l2_dev_t *vd,
			uint8_t query,
			uvcx_video_config_probe_commit_t *config_probe_req);

/*
 * init h264 decoder context
 * args:
 *    width - image width
 *    height - image height
 *
 * asserts:
 *    none
 *
 * returns: error code (0 - E_OK)
 */
int h264_init_decoder(int width, int height);

/*
 * decode h264 frame
 * args:
 *    out_buf - pointer to decoded data
 *    in_buf - pointer to h264 data
 *    size - in_buf size
 *
 * asserts:
 *    h264_ctx is not null
 *    in_buf is not null
 *    out_buf is not null
 *
 * returns: decoded data size
 */
int h264_decode(uint8_t *out_buf, uint8_t *in_buf, int size);

/*
 * close h264 decoder context
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void h264_close_decoder();

#endif /*UVC_H264_H*/
