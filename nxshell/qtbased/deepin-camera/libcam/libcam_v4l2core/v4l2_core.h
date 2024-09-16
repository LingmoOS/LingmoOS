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

#ifndef V4L2CORE_H
#define V4L2CORE_H

#include "gviewv4l2core.h"
#include "gview.h"

/*
 * video device data
 */
struct _v4l2_dev_t
{
	int fd;                             // device file descriptor
	char *videodevice;                  // video device string (default "/dev/video0)"
	
	__MUTEX_TYPE mutex;                // device mutex

	int cap_meth;                       // capture method: IO_READ or IO_MMAP
	v4l2_stream_formats_t* list_stream_formats; //list of available stream formats
	int numb_formats;                   //list size
	//int current_format_index;           //index of current stream format
	//int current_resolution_index;       //index of current resolution for current format

	struct v4l2_capability cap;          // v4l2 capability struct
	struct v4l2_format format;           // v4l2 format struct
	struct v4l2_buffer buf;              // v4l2 buffer struct
	struct v4l2_requestbuffers rb;       // v4l2 request buffers struct
	struct v4l2_streamparm streamparm;   // v4l2 stream parameters struct
	struct v4l2_event_subscription evsub;// v4l2 event subscription struct

	int requested_fmt;                  //requested format (may differ from format.fmt.pix.pixelformat)

	int fps_num;                        //fps numerator
	int fps_denom;                      //fps denominator
	
	double real_fps;                    //real fps (calculated from number of captured frames)

	uint8_t streaming;                  // flag device stream : STRM_STOP ; STRM_REQ_STOP; STRM_OK
	uint64_t frame_index;               // captured frame index from 0 to max(uint64_t)
	void *mem[NB_BUFFER];               // memory buffers for mmap driver frames
	uint32_t buff_length[NB_BUFFER];    // memory buffers length as set by VIDIOC_QUERYBUF
	uint32_t buff_offset[NB_BUFFER];    // memory buffers offset as set by VIDIOC_QUERYBUF

	v4l2_frame_buff_t *frame_queue;     //frame queue
	int frame_queue_size;               //size of frame queue (in frames)

	uint8_t h264_unit_id;  				// uvc h264 unit id, if <= 0 then uvc h264 is not supported
	uint8_t h264_no_probe_default;      // flag core to use the preset h264_config_probe_req data (don't reset to default before commit)
	uvcx_video_config_probe_commit_t h264_config_probe_req; //probe commit struct for h264 streams
	uint8_t *h264_last_IDR;             // last IDR frame retrieved from uvc h264 stream
	int h264_last_IDR_size;             // last IDR frame size
	uint8_t *h264_SPS;                  // h264 SPS info
	uint16_t h264_SPS_size;             // SPS size
	uint8_t *h264_PPS;                  // h264 PPS info
	uint16_t h264_PPS_size;             // PPS size

    int this_device;                    // index of this device in device list

    v4l2_ctrl_t* list_device_controls;    //null terminated linked list of available device controls
    int num_controls;                   //number of controls in list

    uint8_t isbayer;                    //flag if we are streaming bayer data in yuyv frame (logitech only)
    uint8_t bayer_pix_order;            //bayer pixel order

    int pan_step;                       //pan step for relative pan tilt controls (logitech sphere/orbit/BCC950)
    int tilt_step;                      //tilt step for relative pan tilt controls (logitech sphere/orbit/BCC950)

	int has_focus_control_id;           //it's set to control id if a focus control is available (enables software autofocus)
	int has_pantilt_control_id;         //it's set to 1 if a pan/tilt control is available
	uint8_t pantilt_unit_id;            //logitech peripheral V3 unit id (if any)
};
void /*__attribute__ ((constructor))*/ v4l2core_init();
#endif
