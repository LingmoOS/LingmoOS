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

#ifndef GVIEWV4L2CORE_H
#define GVIEWV4L2CORE_H

#include <features.h>

#include <linux/videodev2.h>
#include <linux/uvcvideo.h>
#include <linux/media.h>
#include <libudev.h>
#include <pthread.h>
#include <inttypes.h>
#include <sys/types.h>


/*make sure we support c++*/
__BEGIN_DECLS

/*
 * LOGITECH Dynamic controls defs
 */

#define V4L2_CID_BASE_EXTCTR                    0x0A046D01
#define V4L2_CID_BASE_LOGITECH                  V4L2_CID_BASE_EXTCTR
//#define V4L2_CID_PAN_RELATIVE_LOGITECH        V4L2_CID_BASE_LOGITECH
//#define V4L2_CID_TILT_RELATIVE_LOGITECH       V4L2_CID_BASE_LOGITECH+1
#define V4L2_CID_PANTILT_RESET_LOGITECH         V4L2_CID_BASE_LOGITECH+2

/*this should realy be replaced by V4L2_CID_FOCUS_ABSOLUTE in libwebcam*/
#define V4L2_CID_FOCUS_LOGITECH                 V4L2_CID_BASE_LOGITECH+3
#define V4L2_CID_LED1_MODE_LOGITECH             V4L2_CID_BASE_LOGITECH+4
#define V4L2_CID_LED1_FREQUENCY_LOGITECH        V4L2_CID_BASE_LOGITECH+5
#define V4L2_CID_DISABLE_PROCESSING_LOGITECH    V4L2_CID_BASE_LOGITECH+0x70
#define V4L2_CID_RAW_BITS_PER_PIXEL_LOGITECH    V4L2_CID_BASE_LOGITECH+0x71
#define V4L2_CID_LAST_EXTCTR                    V4L2_CID_RAW_BITS_PER_PIXEL_LOGITECH

/*
 * Error Codes
 */
#define E_OK                      (0)
#define E_ALLOC_ERR               (-1)
#define E_QUERYCAP_ERR            (-2)
#define E_READ_ERR                (-3)
#define E_MMAP_ERR                (-4)
#define E_QUERYBUF_ERR            (-5)
#define E_QBUF_ERR                (-6)
#define E_DQBUF_ERR               (-7)
#define E_STREAMON_ERR            (-8)
#define E_STREAMOFF_ERR           (-9)
#define E_FORMAT_ERR              (-10)
#define E_REQBUFS_ERR             (-11)
#define E_DEVICE_ERR              (-12)
#define E_SELECT_ERR              (-13)
#define E_SELECT_TIMEOUT_ERR      (-14)
#define E_FBALLOC_ERR             (-15)
#define E_NO_STREAM_ERR           (-16)
#define E_NO_DATA                 (-17)
#define E_NO_CODEC                (-18)
#define E_DECODE_ERR              (-19)
#define E_BAD_TABLES_ERR          (-20)
#define E_NO_SOI_ERR              (-21)
#define E_NOT_8BIT_ERR            (-22)
#define E_BAD_WIDTH_OR_HEIGHT_ERR (-23)
#define E_TOO_MANY_COMPPS_ERR     (-24)
#define E_ILLEGAL_HV_ERR          (-25)
#define E_QUANT_TBL_SEL_ERR       (-26)
#define E_NOT_YCBCR_ERR           (-27)
#define E_UNKNOWN_CID_ERR         (-28)
#define E_WRONG_MARKER_ERR        (-29)
#define E_NO_EOI_ERR              (-30)
#define E_FILE_IO_ERR             (-31)
#define E_NO_DEVICE_ERR         (-32)
#define E_UNKNOWN_ERR             (-40)

/*
 * stream status codes
 */
#define STRM_STOP        (0)
#define STRM_REQ_STOP    (1)
#define STRM_OK          (2)

/*
 * IO methods
 */
#define IO_MMAP 1
#define IO_READ 2

/*
 * Frame status
 */
#define FRAME_READY (0)
#define FRAME_DECODING (1)
#define FRAME_DONE (2)

/*
 * software autofocus sort method
 * quick sort
 * shell sort
 * insert sort
 * bubble sort
 */
#define AUTOF_SORT_QUICK  1
#define AUTOF_SORT_SHELL  2
#define AUTOF_SORT_INSERT 3
#define AUTOF_SORT_BUBBLE 4

/*
 * Image Formats
 */
#define IMG_FMT_RAW     (0)
#define IMG_FMT_JPG     (1)
#define IMG_FMT_PNG     (2)
#define IMG_FMT_BMP     (3)


/*
 * buffer number (for driver mmap ops)
 */
#define NB_BUFFER 4

/*jpeg header def*/
#define HEADERFRAME1 0xaf

/*
 * set ioctl retries to 4
 */
#define IOCTL_RETRY 4

/* A.8. Video Class-Specific Request Codes */
#define UVC_RC_UNDEFINED                                0x00
#define UVC_SET_CUR                                     0x01
#define UVC_GET_CUR                                     0x81
#define UVC_GET_MIN                                     0x82
#define UVC_GET_MAX                                     0x83
#define UVC_GET_RES                                     0x84
#define UVC_GET_LEN                                     0x85
#define UVC_GET_INFO                                    0x86
#define UVC_GET_DEF                                     0x87

/*
 * h264 probe commit struct (uvc 1.1)
 */
typedef struct _uvcx_video_config_probe_commit_t {
    uint32_t  dwFrameInterval;
    uint32_t  dwBitRate;
    uint16_t  bmHints;
    uint16_t  wConfigurationIndex;
    uint16_t  wWidth;
    uint16_t  wHeight;
    uint16_t  wSliceUnits;
    uint16_t  wSliceMode;
    uint16_t  wProfile;
    uint16_t  wIFramePeriod;
    uint16_t  wEstimatedVideoDelay;
    uint16_t  wEstimatedMaxConfigDelay;
    uint8_t   bUsageType;
    uint8_t   bRateControlMode;
    uint8_t   bTemporalScaleMode;
    uint8_t   bSpatialScaleMode;
    uint8_t   bSNRScaleMode;
    uint8_t   bStreamMuxOption;
    uint8_t   bStreamFormat;
    uint8_t   bEntropyCABAC;
    uint8_t   bTimestamp;
    uint8_t   bNumOfReorderFrames;
    uint8_t   bPreviewFlipped;
    uint8_t   bView;
    uint8_t   bReserved1;
    uint8_t   bReserved2;
    uint8_t   bStreamID;
    uint8_t   bSpatialLayerRatio;
    uint16_t  wLeakyBucketSize;
} __attribute__((__packed__)) uvcx_video_config_probe_commit_t;

/*
 * v4l2 stream capability data
 */
typedef struct _v4l2_stream_cap_t {
    int width;            //width
    int height;           //height
    int *framerate_num;   //list of numerator values - should be 1 in almost all cases
    int *framerate_denom; //list of denominator values - gives fps
    int numb_frates;      //number of frame rates (numerator and denominator lists size)
} v4l2_stream_cap_t;

/*
 * v4l2 stream format data
 */
typedef struct _v4l2_stream_format_t {
    uint8_t dec_support; //decoder support (1-supported; 0-not supported)
    int format;          //v4l2 pixel format
    char fourcc[5];      //corresponding fourcc (mode)
    char description[32];//format description
    int numb_res;        //available number of resolutions for format (v4l2_stream_cap_t list size)
    v4l2_stream_cap_t *list_stream_cap;  //list of stream capabilities for format
} v4l2_stream_formats_t;

/*
 * v4l2 control data
 */
typedef struct _v4l2_ctrl_t {
    struct v4l2_queryctrl control;
    struct v4l2_querymenu *menu;
    int32_t cclass; //don't use 'class' to avoid issues with c++
    int32_t value; //also used for string max size
    int64_t value64;
    char *string;

    /*localization*/
    char *name; /*gettext translated name*/
    int menu_entries;
    char **menu_entry; /*gettext translated menu entry name*/

    //next control in the list
    struct _v4l2_ctrl_t *next;
} v4l2_ctrl_t;

/*
 * frame buffer struct
 */
typedef struct _v4l2_frame_buff_t {
    int index; //buffer index
    int status; //frame status {FRAME_DECODING; FRAME_DONE; FRAME_READY}

    int width; //frame width (in pixels)
    int height;//frame height (in pixels)

    int isKeyframe; // current buffer contains a keyframe (h264 IDR)

    size_t raw_frame_size; // raw frame size (bytes)
    size_t raw_frame_max_size; //maximum size for raw frame (bytes)
    size_t h264_frame_size; // h264 frame size (bytes)
    size_t h264_frame_max_size; //size limit for h264 frame (bytes)
    size_t tmp_buffer_max_size; //maximum size for temp buffer (bytes)

    uint64_t timestamp; // captured frame timestamp

    uint8_t *raw_frame; // pointer to raw frame
    uint8_t *yuv_frame; // pointer to decoded yuv frame
    uint8_t *h264_frame; // pointer to regular or demultiplexed h264 frame
    uint8_t *tmp_buffer; //temporary buffer used in decoding

} v4l2_frame_buff_t;

/*
 * v4l2 device system data
 */
typedef struct _v4l2_dev_sys_data_t {
    char *device;
    char *name;
    char *driver;
    char *location;
    uint32_t vendor;
    uint32_t product;
    int valid;
    int current;
    uint64_t busnum;
    uint64_t devnum;
} v4l2_dev_sys_data_t;

/* v4l2 device handler - opaque data structure*/
typedef struct _v4l2_dev_t v4l2_dev_t;

/*
 * ioctl with a number of retries in the case of I/O failure
 * args:
 *   fd - device descriptor
 *   IOCTL_X - ioctl reference
 *   arg - pointer to ioctl data
 *
 * asserts:
 *   none
 *
 * returns - ioctl result
 */
int xioctl(int fd, int IOCTL_X, void *arg);

/*
 * set verbosity level
 * args:
 *   level - verbosity level (def = 0)
 *
 * asserts:
 *   none
 *
 * returns - void
 */
void v4l2core_set_verbosity(int level);

/*
 * define fps values
 * args:
 *   vd - pointer to v4l2 device handler
 *   num - fps numerator
 *   denom - fps denominator
 *
 * asserts:
 *   vd is not null
 *
 * returns - void
 */
void v4l2core_define_fps(v4l2_dev_t *vd, int num, int denom);

/*
 * get requested fps numerator
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns - requested fps numerator
 */
int v4l2core_get_fps_num(v4l2_dev_t *vd);

/*
 * get requested fps denominator
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns - requested fps denominator
 */
int v4l2core_get_fps_denom(v4l2_dev_t *vd);

/*
 * get device available number of formats
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   none
 *
 * returns - number of formats for device
 */
int v4l2core_get_number_formats(v4l2_dev_t *vd);

/*
 * sets bayer pixel order
 * args:
 *   vd - pointer to v4l2 device handler
 *   order - pixel order
 *
 * asserts:
 *   none
 *
 * returns - void
 */
void v4l2core_set_bayer_pix_order(v4l2_dev_t *vd, uint8_t order);

/*
 * gets bayer pixel order
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   none
 *
 * returns - bayer pixel order
 */
uint8_t v4l2core_get_bayer_pix_order(v4l2_dev_t *vd);

/*
 * flags bayer mode
 * args:
 *   vd - pointer to v4l2 device handler
 *   flag - 1 if we are streaming bayer data (0 otherwise)
 *
 * asserts:
 *   none
 *
 * returns - void
 */
void v4l2core_set_isbayer(v4l2_dev_t *vd, uint8_t flag);

/*
 * gets bayer pixel order
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   none
 *
 * returns - isbayer flag
 */
uint8_t v4l2core_get_isbayer(v4l2_dev_t *vd);

/*
 * gets current device index
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   none
 *
 * returns - device index
 */
int v4l2core_get_this_device_index(v4l2_dev_t *vd);

/*
 * disable libv4l2 calls
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns void
 */
void v4l2core_disable_libv4l2();

int get_my_width(void);

int get_my_height(void);

/*
 * enable libv4l2 calls (default)
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns void
 */
void v4l2core_enable_libv4l2();

/*
 * get pixelformat from fourcc
 * args:
 *    fourcc - fourcc code for format
 *
 * asserts:
 *    none
 *
 * returns: v4l2 pixel format
 */
uint32_t v4l2core_fourcc_2_v4l2_pixelformat(const char *fourcc);

/*
 * get real fps
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: double with real fps value
 */
double v4l2core_get_realfps(v4l2_dev_t *vd);

/*
 * set v4l2 capture method to use
 * args:
 *   vd - pointer to v4l2 device handler
 *   method - capture method (IO_READ or IO_MMAP)
 *
 * asserts:
 *   vd is not null
 *
 * returns: none
*/
void v4l2core_set_capture_method(v4l2_dev_t *vd, int method);

/*
 * Initiate video device handler with default values
 * args:
 *   device - device name (e.g: "/dev/video0")
 *
 * asserts:
 *   device is not null
 *
 * returns: pointer to v4l2 device handler (or NULL on error)
 */
v4l2_dev_t *v4l2core_init_dev(const char *device);

/*
 * get device control list
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * return: pointer to first control in the list
 */
v4l2_ctrl_t *v4l2core_get_control_list(v4l2_dev_t *vd);

/*
 * get stream frame format list for device
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * return: pointer to first format in the list
 */
v4l2_stream_formats_t *v4l2core_get_formats_list(v4l2_dev_t *vd);

/*
 * get videodevice name
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * return: string with videodevice name
 */
const char *v4l2core_get_videodevice(v4l2_dev_t *vd);

/*
 * get device pan step value
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * return: pan step value
 */
int v4l2core_get_pan_step(v4l2_dev_t *vd);

/*
 * get device tilt step value
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * return: tilt step value
 */
int v4l2core_get_tilt_step(v4l2_dev_t *vd);

/*
 * set device pan step value
 * args:
 *    vd - pointer to v4l2 device handler
 *    step - pan step value
 *
 * asserts:
 *    vd is not null
 *
 * return: none
 */
void v4l2core_set_pan_step(v4l2_dev_t *vd, int step);

/*
 * set device tilt step value
 * args:
 *    vd - pointer to v4l2 device handler
 *    step -tilt step value
 *
 * asserts:
 *    vd is not null
 *
 * return: none
 */
void v4l2core_set_tilt_step(v4l2_dev_t *vd, int step);

/*
 * get the number of available v4l2 devices
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: number of available v4l2 devices
 */
int v4l2core_get_num_devices();

/*
 * get the device sys data for index
 * args:
 *   index - device index
 *
 * asserts:
 *   none
 *
 * returns: pointer to device sys data
 */
v4l2_dev_sys_data_t *v4l2core_get_device_sys_data(int index);

/*
 * get the device index in device list
 * args:
 *   videodevice - string with videodevice node (e.g: /dev/video0)
 *
 * asserts:
 *   none
 *
 * returns:
 *   videodevice index in device list [0 - num_devices[ or -1 on error
 */
int v4l2core_get_device_index(const char *videodevice);

/*
 * check for new devices
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: true(1) if device list was updated, false(0) otherwise
 */
int v4l2core_check_device_list_events();

/*
 * check for control events
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: number of processed control events
 */
int v4l2core_check_control_events(v4l2_dev_t *vd);

/*
 * get requested frame format
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: requested frame format
 */
int v4l2core_get_requested_frame_format(v4l2_dev_t *vd);

/*
 * get has_pantilt_id flag
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: has_pantilt_id flag
 */
int v4l2core_has_pantilt_id(v4l2_dev_t *vd);

/*
 * get has_focus_control_id flag
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: has_focus_control_id flag
 */
int v4l2core_has_focus_control_id(v4l2_dev_t *vd);

/*
 * get frame width
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: frame width
 */
int v4l2core_get_frame_width(v4l2_dev_t *vd);

/*
 * get frame height
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: frame height
 */
int v4l2core_get_frame_height(v4l2_dev_t *vd);

/* get frame format index from format list
 * args:
 *   vd - pointer to v4l2 device handler
 *   format - v4l2 pixel format
 *
 * asserts:
 *   vd is not null
 *
 * returns: format list index or -1 if not available
 */
int v4l2core_get_frame_format_index(v4l2_dev_t *vd, int format);

/* get resolution index for format index from format list
 * args:
 *   vd - pointer to v4l2 device handler
 *   format - format index from format list
 *   width - requested width
 *   height - requested height
 *
 * asserts:
 *   vd is not null
 *
 * returns: resolution list index for format index or -1 if not available
 */
int v4l2core_get_format_resolution_index(v4l2_dev_t *vd,
                                         int format, int width, int height);

/*
 * prepare a valid format (first in the format list)
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: none
 */
void v4l2core_prepare_valid_format(v4l2_dev_t *vd);

/*
 * prepare new format
 * args:
 *   vd - pointer to v4l2 device handler
 *   new_format - new format
 *
 * asserts:
 *    vd is not null
 *
 * returns: none
 */
void v4l2core_prepare_new_format(v4l2_dev_t *vd, int new_format);

/*
 * prepare valid resolution (first in the resolution list for the format)
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: none
 */
void v4l2core_prepare_valid_resolution(v4l2_dev_t *vd);

/*
 * prepare new resolution
 * args:
 *   vd - pointer to v4l2 device handler
 *   new_width - new width
 *   new_height - new height
 *
 * asserts:
 *   vd is not null
 *
 * returns: none
 */
void v4l2core_prepare_new_resolution(v4l2_dev_t *vd,
                                     int new_width, int new_height);

/*
 * update the old format (pixelformat, width and height)
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * returns:
 *    error code
 */
int v4l2core_update_old_format(v4l2_dev_t *vd, int width, int height, int pixelformat);

/*
 * update the current format (pixelformat, width and height)
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns:
 *    error code
 */
int v4l2core_update_current_format(v4l2_dev_t *vd);

/*
 * gets the next video frame (must be released after processing)
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   none
 *
 * returns: pointer frame buffer (NULL on error)
 */
v4l2_frame_buff_t *v4l2core_get_frame(v4l2_dev_t *vd);

/*
 * releases the video frame (so that it can be reused by the driver)
 * args:
 *   vd - pointer to v4l2 device handler
 *   frame - pointer to decoded frame buffer
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code (E_OK)
 */
int v4l2core_release_frame(v4l2_dev_t *vd, v4l2_frame_buff_t *frame);

/*
 * gets the next video frame and decodes it
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: pointer to decoded frame buffer ( NULL on error)
 */
v4l2_frame_buff_t *v4l2core_get_decoded_frame(v4l2_dev_t *vd);

/*
 * clean v4l2 buffers
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * return: none
 */
void v4l2core_clean_buffers(v4l2_dev_t *vd);

/*
 * closes v4l2 device handler
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: void
 */
void v4l2core_close_dev(v4l2_dev_t *vd);

/*
 * request a fps update
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * returns: none
 */
void v4l2core_request_framerate_update(v4l2_dev_t *vd);

/*
 * gets video device defined frame rate (not real - consider it a maximum value)
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: VIDIOC_G_PARM ioctl result value
 * (sets vd->fps_denom and vd->fps_num to device value)
 */
int v4l2core_get_framerate (v4l2_dev_t *vd);

/*
 * Starts the video stream
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: VIDIOC_STREAMON ioctl result (0- E_OK)
 */
int v4l2core_start_stream(v4l2_dev_t *vd);

/*
 * request video stream to stop
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code (0 -OK)
*/
int v4l2core_request_stop_stream(v4l2_dev_t *vd);

/*
 * Stops the video stream
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: VIDIOC_STREAMOFF ioctl result (0- E_OK)
 */
int v4l2core_stop_stream(v4l2_dev_t *vd);

/*
 *  ######### CONTROLS ##########
 */

/*
 * return the control associated to id from device list
 * args:
 *   vd - pointer to v4l2 device handler
 *   id - control id
 *
 * asserts:
 *   none
 *
 * returns: pointer to v4l2_control if succeded or null otherwise
 */
v4l2_ctrl_t *v4l2core_get_control_by_id(v4l2_dev_t *vd, int id);

/*
 * sets the value of control id in device
 * args:
 *   vd - pointer to v4l2 device handler
 *   id - control id
 *
 * asserts:
 *   vd is not null
 *
 * returns: ioctl result
 */
int v4l2core_set_control_value_by_id(v4l2_dev_t *vd, int id);

/*
 * updates the value for control id from the device
 * also updates control flags
 * args:
 *   vd - pointer to v4l2 device handler
 *   id - control id
 *
 * asserts:
 *   vd is not null
 *
 * returns: ioctl result
 */
int v4l2core_get_control_value_by_id(v4l2_dev_t *vd, int id);

/*
 * goes trough the control list and sets values in device to default
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: void
 */
void v4l2core_set_control_defaults(v4l2_dev_t *vd);

/*
 * set autofocus sort method
 * args:
 *    method - sort method
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void v4l2core_soft_autofocus_set_sort(int method);

/*
 * initiate software autofocus
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * returns: error code (0 - E_OK)
 */
int v4l2core_soft_autofocus_init (v4l2_dev_t *vd);

/*
 * run the software autofocus
 * args:
 *    vd - pointer to v4l2 device handler
 *    frame - pointer to frame buffer
 *
 * asserts:
 *    vd is not null
 *
 * returns: 1 - running  0- focused
 *  (only matters for non-continue focus)
 */
int v4l2core_soft_autofocus_run(v4l2_dev_t *vd, v4l2_frame_buff_t *frame);

/*
 * sets a focus loop while autofocus is on
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void v4l2core_soft_autofocus_set_focus();

/*
 * close and clean software autofocus
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void v4l2core_soft_autofocus_close();

/*
 * save the device control values into a profile file
 * args:
 *   vd - pointer to v4l2 device handler
 *   filename - profile filename
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code (0 -E_OK)
 */
int v4l2core_save_control_profile(v4l2_dev_t *vd, const char *filename);

/*
 * load the device control values from a profile file
 * args:
 *   vd - pointer to v4l2 device handler
 *   filename - profile filename
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code (0 -E_OK)
 */
int v4l2core_load_control_profile(v4l2_dev_t *vd, const char *filename);

/*
 * ########### H264 controls ###########
 */

/*
 * resets the h264 encoder
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: 0 on success or error code on fail
 */
int v4l2core_reset_h264_encoder(v4l2_dev_t *vd);

/*
 * get h264 unit id
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: unit id on success or error code ( < 0 ) on fail
 */
int v4l2core_get_h264_unit_id(v4l2_dev_t *vd);

/*
 * get PPS NALU size
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: PPS size
 */
int v4l2core_get_h264_pps_size(v4l2_dev_t *vd);

/*
 * get PPS data
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: pointer to PPS data
 */
uint8_t *v4l2core_get_h264_pps(v4l2_dev_t *vd);

/*
 * get SPS NALU size
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: SPS size
 */
int v4l2core_get_h264_sps_size(v4l2_dev_t *vd);

/*
 * get SPS data
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: pointer to SPS data
 */
uint8_t *v4l2core_get_h264_sps(v4l2_dev_t *vd);

/*
 * request a IDR frame from the H264 encoder
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: none
 */
void v4l2core_h264_request_idr(v4l2_dev_t *vd);

/*
 * query the frame rate config
 * args:
 *   vd - pointer to v4l2 device handler
 *   query - query type
 *
 * asserts:
 *   vd is not null
 *
 * returns: frame rate config (FIXME: 0xffffffff on error)
 */
uint32_t v4l2core_query_h264_frame_rate_config(v4l2_dev_t *vd, uint8_t query);

/*
 * get the frame rate config
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: frame rate config (FIXME: 0xffffffff on error)
 */
uint32_t v4l2core_get_h264_frame_rate_config(v4l2_dev_t *vd);

/*
 * set the frame rate config
 * args:
 *   vd - pointer to v4l2 device handler
 *   framerate - framerate
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code ( 0 -OK)
 */
int v4l2core_set_h264_frame_rate_config(v4l2_dev_t *vd, uint32_t framerate);

/*
 * updates the h264_probe_commit_req field
 * args:
 *   vd - pointer to v4l2 device handler
 *   query - (UVC_GET_CUR; UVC_GET_MAX; UVC_GET_MIN)
 *   config_probe_req - pointer to uvcx_video_config_probe_commit_t:
 *     if null vd->h264_config_probe_req will be used
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code ( 0 -OK)
 */
int v4l2core_probe_h264_config_probe_req(
    v4l2_dev_t *vd,
    uint8_t query,
    uvcx_video_config_probe_commit_t *config_probe_req);


/*
 * gets the current h264_config_probe_req data struct
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: pointer to current h264_config_probe_req data struct
 */
uvcx_video_config_probe_commit_t *v4l2core_get_h264_config_probe_req(v4l2_dev_t *vd);

/*
 * flag core to use the preset h264_config_probe_req data (don't reset to default before commit)
 * args:
 *   vd - pointer to v4l2 device handler
 *   flag - value to set
 *
 * asserts:
 *   vd is not null
 *
 * returns: none
 */
void v4l2core_set_h264_no_probe_default(v4l2_dev_t *vd, uint8_t flag);

/*
 * get h264_no_probe_default flag
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: h264_no_probe_default flag
 */
uint8_t v4l2core_get_h264_no_probe_default(v4l2_dev_t *vd);

/*
 * get the video rate control mode
 * args:
 *   vd - pointer to v4l2 device handler
 *   query - query type
 *
 * asserts:
 *   vd is not null
 *
 * returns: video rate control mode (FIXME: 0xff on error)
 */
uint8_t v4l2core_get_h264_video_rate_control_mode(v4l2_dev_t *vd, uint8_t query);

/*
 * set the video rate control mode
 * args:
 *   vd - pointer to v4l2 device handler
 *   mode - rate mode
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code ( 0 -OK)
 */
int v4l2core_set_h264_video_rate_control_mode(v4l2_dev_t *vd, uint8_t mode);

/*
 * get the temporal scale mode
 * args:
 *   vd - pointer to v4l2 device handler
 *   query - query type
 *
 * asserts:
 *   vd is not null
 *
 * returns: temporal scale mode (FIXME: 0xff on error)
 */
uint8_t v4l2core_get_h264_temporal_scale_mode(v4l2_dev_t *vd, uint8_t query);

/*
 * set the temporal scale mode
 * args:
 *   vd - pointer to v4l2 device handler
 *   mode - temporal scale mode
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code ( 0 -OK)
 */
int v4l2core_set_h264_temporal_scale_mode(v4l2_dev_t *vd, uint8_t mode);

/*
 * get the spatial scale mode
 * args:
 *   vd - pointer to v4l2 device handler
 *   query - query type
 *
 * asserts:
 *   vd is not null
 *
 * returns: temporal scale mode (FIXME: 0xff on error)
 */
uint8_t v4l2core_get_h264_spatial_scale_mode(v4l2_dev_t *vd, uint8_t query);

/*
 * set the spatial scale mode
 * args:
 *   vd - pointer to v4l2 device handler
 *   mode - spatial scale mode
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code ( 0 -OK)
 */
int v4l2core_set_h264_spatial_scale_mode(v4l2_dev_t *vd, uint8_t mode);

/*
 *  ######### XU CONTROLS ##########
 */

/*
 * get lenght of xu control defined by unit id and selector
 * args:
 *   vd - pointer to v4l2 device handler
 *   unit - unit id of xu control
 *   selector - selector for control
 *
 * asserts:
 *   vd is not null
 *
 * returns: length of xu control
 */
uint16_t v4l2core_get_length_xu_control(v4l2_dev_t *vd,
                                        uint8_t unit, uint8_t selector);

/*
 * get uvc info for xu control defined by unit id and selector
 * args:
 *   vd - pointer to v4l2 device handler
 *   unit - unit id of xu control
 *   selector - selector for control
 *
 * asserts:
 *   vd is not null
 *
 * returns: info of xu control
 */
uint8_t v4l2core_get_info_xu_control(v4l2_dev_t *vd,
                                     uint8_t unit, uint8_t selector);

/*
 * runs a query on xu control defined by unit id and selector
 * args:
 *   vd - pointer to v4l2 device handler
 *   unit - unit id of xu control
 *   selector - selector for control
 *   query - query type
 *   data - pointer to query data
 *
 * asserts:
 *   vd is not null
 *
 * returns: 0 if query succeded or errno otherwise
 */
int v4l2core_query_xu_control(v4l2_dev_t *vd,
                              uint8_t unit, uint8_t selector, uint8_t query, void *data);

/*
 *  ########### FILE IO ###############
 */
/*
 * save data to file
 * args:
 *   filename - string with filename
 *   data - pointer to data
 *   size - data size in bytes = sizeof(uint8_t)
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int v4l2core_save_data_to_file(const char *filename, uint8_t *data, int size);

/*
 * save the current frame to file
 * args:
 *    frame - pointer to frame buffer
 *    filename - output file name
 *    format - image type
 *           (IMG_FMT_RAW, IMG_FMT_JPG, IMG_FMT_PNG, IMG_FMT_BMP)
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
int v4l2core_save_image(
    v4l2_frame_buff_t *frame,
    const char *filename,
    int format);

/*
 * ############### TIME DATA ##############
 */

/*
 * get current timestamp
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: monotonic time in nanoseconds
 */
uint64_t v4l2core_time_get_timestamp();

__END_DECLS

#endif

