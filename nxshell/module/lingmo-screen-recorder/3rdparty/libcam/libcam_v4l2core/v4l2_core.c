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

/*******************************************************************************#
#                                                                               #
#  V4L2 core library                                                            #
#                                                                               #
********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <libv4l2.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <errno.h>
#include <assert.h>
/* support for internationalization - i18n */
#include <locale.h>
#include <libintl.h>

#include "gview.h"
#include "gviewv4l2core.h"
#include "v4l2_core.h"
#include "save_image.h"
#include "soft_autofocus.h"
#include "core_time.h"
#include "uvc_h264.h"
#include "frame_decoder.h"
#include "control_profile.h"
#include "v4l2_formats.h"
#include "v4l2_controls.h"
#include "v4l2_devices.h"
#include "cameraconfig.h"
#include "load_libs.h"

#ifndef GETTEXT_PACKAGE_V4L2CORE
#define GETTEXT_PACKAGE_V4L2CORE "gview_v4l2core"
#endif

#define __PMUTEX &(vd->mutex)

/*verbosity (global scope)*/
int verbosity = 0;

/*encode environment 0:ffmpeg 2:gstreamer default to ffmpeg(0) (global scope)*/
int encodeenv = 0;

/*requested format data*/
static int my_pixelformat = 0;
static int my_width = 0;
static int my_height = 0;

static uint64_t fps_ref_ts = 0;
static uint32_t fps_frame_count = 0;

static uint8_t flag_fps_change = 0; /*set to 1 to request a fps change*/

static uint8_t disable_libv4l2 = 0; /*set to 1 to disable libv4l2 calls*/

static int frame_queue_size = 1; /*just one frame in queue (enough for a single thread)*/

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
int xioctl(int fd, int IOCTL_X, void *arg)
{
	int ret = 0;
	int tries= IOCTL_RETRY;
	do
	{
		if(!disable_libv4l2)
            ret = getV4l2()->m_v4l2_ioctl(fd, (unsigned long int)IOCTL_X, arg);
		else
            ret = ioctl(fd, (unsigned long int)IOCTL_X, arg);
	}
	while (ret && tries-- &&
			((errno == EINTR) || (errno == EAGAIN) || (errno == ETIMEDOUT)));

	if (ret && (tries <= 0)) fprintf(stderr, "V4L2_CORE: ioctl (%i) retried %i times - giving up: %s)\n", IOCTL_X, IOCTL_RETRY, strerror(errno));

	return (ret);
}

/*
 * v4l2core constructor (called before dlopen or main)
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void /*__attribute__ ((constructor))*/ v4l2core_init()
{
	//initialize device list (with udev monitoring)
	v4l2core_init_device_list();

	/*set defaults*/
	frame_queue_size = 1;
	disable_libv4l2 = 0;
	
}

/*
 * v4l2core destructor (called before dlclose os exit)
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void /*__attribute__ ((destructor))*/ v4l2core_fini()
{
	//close and free the device list
	if(verbosity > 2)
		printf("V4L2_CORE: closing device list\n");
	v4l2core_close_v4l2_device_list();
}



/*
 * Query video device capabilities and supported formats
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid ( > 0 )
 *
 * returns: error code  (E_OK)
 */
static int check_v4l2_dev(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	assert(vd->fd > 0);

	memset(&vd->cap, 0, sizeof(struct v4l2_capability));

    if ( xioctl(vd->fd, (int)VIDIOC_QUERYCAP, &vd->cap) < 0 )
	{
		fprintf( stderr, "V4L2_CORE: (VIDIOC_QUERYCAP) error: %s\n", strerror(errno));
		return E_QUERYCAP_ERR;
	}

	if ( ( vd->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE ) == 0)
	{
		fprintf(stderr, "V4L2_CORE: Error opening device %s: video capture not supported.\n",
				vd->videodevice);
		return E_QUERYCAP_ERR;
	}
	if (!(vd->cap.capabilities & V4L2_CAP_STREAMING))
	{
		fprintf(stderr, "V4L2_CORE: %s does not support streaming i/o\n",
			vd->videodevice);
		return E_QUERYCAP_ERR;
	}

	if(vd->cap_meth == IO_READ)
	{

		vd->mem[vd->buf.index] = NULL;
		if (!(vd->cap.capabilities & V4L2_CAP_READWRITE))
		{
			fprintf(stderr, "V4L2_CORE: %s does not support read, try with mmap\n",
				vd->videodevice);
			return E_READ_ERR;
		}
	}
	if(verbosity > 0)
		printf("V4L2_CORE: Init. %s (location: %s)\n", vd->cap.card, vd->cap.bus_info);

	/*enumerate frame formats supported by device*/
	int ret = enum_frame_formats(vd);
	if(ret != E_OK)
	{
		fprintf(stderr, "V4L2_CORE: no valid frame formats (with valid sizes) found for device\n");
		return ret;
	}	

	/*add h264 (uvc muxed) to format list if supported by device*/
	add_h264_format(vd);

	/*enumerate device controls*/
	enumerate_v4l2_control(vd);
	/*gets the current control values and sets their flags*/
	get_v4l2_control_values(vd);

	/*if we have a focus control initiate the software autofocus*/
	if(vd->has_focus_control_id)
	{
		if(v4l2core_soft_autofocus_init (vd) != E_OK)
			vd->has_focus_control_id = 0;
	}

	return E_OK;
}

/*
 * unmaps v4l2 buffers
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code  (0- E_OK)
 */
static int unmap_buff(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);

	if(verbosity > 2)
		printf("V4L2_CORE: unmapping v4l2 buffers\n");
	int i=0;
	int ret=E_OK;

	switch(vd->cap_meth)
	{
		case IO_READ:
			break;

		case IO_MMAP:
			for (i = 0; i < NB_BUFFER; i++)
			{
				// unmap old buffer
				if((vd->mem[i] != MAP_FAILED) && vd->buff_length[i])
                    if((ret=getV4l2()->m_v4l2_munmap(vd->mem[i], vd->buff_length[i]))<0)
					{
						fprintf(stderr, "V4L2_CORE: couldn't unmap buff: %s\n", strerror(errno));
					}
			}
	}
	return ret;
}

/*
 * maps v4l2 buffers
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code  (0- E_OK)
 */
static int map_buff(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);

	if(verbosity > 2)
		printf("V4L2_CORE: mapping v4l2 buffers\n");

	int i = 0;
	// map new buffer
	for (i = 0; i < NB_BUFFER; i++)
	{
        vd->mem[i] = getV4l2()->m_v4l2_mmap( NULL, // start anywhere
			vd->buff_length[i],
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			vd->fd,
			vd->buff_offset[i]);
		if (vd->mem[i] == MAP_FAILED)
		{
			fprintf(stderr, "V4L2_CORE: Unable to map buffer: %s\n", strerror(errno));
			return E_MMAP_ERR;
		}
		if(verbosity > 1)
            printf("V4L2_CORE: mapped buffer[%i] with length %u to pos %p\n",
				i,
				vd->buff_length[i],
				vd->mem[i]);
	}

	return (E_OK);
}

/*
 * Query and map buffers
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code  (0- E_OK)
 */
static int query_buff(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);

	if(verbosity > 2)
		printf("V4L2_CORE: query v4l2 buffers\n");

	int i=0;
	int ret=E_OK;

	switch(vd->cap_meth)
	{
		case IO_READ:
			break;

		case IO_MMAP:
			for (i = 0; i < NB_BUFFER; i++)
			{
				memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
                vd->buf.index = (__u32)i;
				vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				//vd->buf.flags = V4L2_BUF_FLAG_TIMECODE;
				//vd->buf.timecode = vd->timecode;
				//vd->buf.timestamp.tv_sec = 0;
				//vd->buf.timestamp.tv_usec = 0;
				vd->buf.memory = V4L2_MEMORY_MMAP;
                ret = xioctl(vd->fd, (int)VIDIOC_QUERYBUF, &vd->buf);

				if (ret < 0)
				{
					fprintf(stderr, "V4L2_CORE: (VIDIOC_QUERYBUF) Unable to query buffer[%i]: %s\n", i, strerror(errno));
					if(errno == EINVAL)
						fprintf(stderr, "         try with read method instead\n");

					return E_QUERYBUF_ERR;
				}

				if (vd->buf.length <= 0)
					fprintf(stderr, "V4L2_CORE: (VIDIOC_QUERYBUF) - buffer length is %i\n",
						vd->buf.length);

				vd->buff_length[i] = vd->buf.length;
				vd->buff_offset[i] = vd->buf.m.offset;
			}
			// map the new buffers
			if(map_buff(vd) != 0)
				ret = E_MMAP_ERR;
			break;
	}
	for(i = 0; i < vd->frame_queue_size; ++i)
		vd->frame_queue[i].raw_frame_max_size = vd->buf.length;

	return ret;
}

/*
 * Queue Buffers
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code  (0- E_OK)
 */
static int queue_buff(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);

	if(verbosity > 2)
		printf("V4L2_CORE: queue v4l2 buffers\n");

	int i=0;
	int ret=E_OK;

	switch(vd->cap_meth)
	{
		case IO_READ:
			break;

		case IO_MMAP:
		default:
			for (i = 0; i < NB_BUFFER; ++i)
			{
				memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
                vd->buf.index = (__u32)i;
				vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				//vd->buf.flags = V4L2_BUF_FLAG_TIMECODE;
				//vd->buf.timecode = vd->timecode;
				//vd->buf.timestamp.tv_sec = 0;
				//vd->buf.timestamp.tv_usec = 0;
				vd->buf.memory = V4L2_MEMORY_MMAP;
                ret = xioctl(vd->fd, (int)VIDIOC_QBUF, &vd->buf);
				if (ret < 0)
				{
					fprintf(stderr, "V4L2_CORE: (VIDIOC_QBUF) Unable to queue buffer: %s\n", strerror(errno));
					return E_QBUF_ERR;
				}
			}
			vd->buf.index = 0; /*reset index*/
	}
	return ret;
}

/*
 * do a VIDIOC_S_PARM ioctl for setting frame rate
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * returns: error code
 */
static int do_v4l2_framerate_update(v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);

	int ret = 0;

	/*get the current stream parameters*/
	vd->streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = xioctl(vd->fd, (int)VIDIOC_G_PARM, &vd->streamparm);
	if (ret < 0)
	{
		fprintf(stderr, "V4L2_CORE: (VIDIOC_G_PARM) error: %s\n", strerror(errno));
		fprintf(stderr, "V4L2_CORE: Unable to set %d/%d fps\n", vd->fps_num, vd->fps_denom);
		return ret;
	}

	if (!(vd->streamparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME))
	{
		fprintf(stderr, "V4L2_CORE: V4L2_CAP_TIMEPERFRAME not supported\n");
	}

    vd->streamparm.parm.capture.timeperframe.numerator = (__u32)vd->fps_num;
    vd->streamparm.parm.capture.timeperframe.denominator = (__u32)vd->fps_denom;

	/*request the new frame rate*/
    ret = xioctl(vd->fd, (int)VIDIOC_S_PARM, &vd->streamparm);

	if (ret < 0)
	{
		fprintf(stderr, "V4L2_CORE: (VIDIOC_S_PARM) error: %s\n", strerror(errno));
		fprintf(stderr, "V4L2_CORE: Unable to set %d/%d fps\n", vd->fps_num, vd->fps_denom);
	}

	return ret;
}

/*
 * sets video device frame rate
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: VIDIOC_S_PARM ioctl result value
 * (sets vd->fps_denom and vd->fps_num to device value)
 */
static int set_v4l2_framerate (v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);

	if(verbosity > 2)
		printf("V4L2_CORE: trying to change fps to %i/%i\n", vd->fps_num, vd->fps_denom);

	int ret = 0;

	/*lock the mutex*/
	__LOCK_MUTEX( __PMUTEX );

	/*store streaming flag*/
	uint8_t stream_status = vd->streaming;

	/*try to stop the video stream*/
	if(stream_status == STRM_OK)
		v4l2core_stop_stream(vd);

	switch(vd->cap_meth)
	{
		case IO_READ:
			ret = do_v4l2_framerate_update(vd);
			break;

		case IO_MMAP:
			if(stream_status == STRM_OK)
			{
				/*unmap the buffers*/
				unmap_buff(vd);
			}

			ret = do_v4l2_framerate_update(vd);
			/*
			 * For uvc muxed H264 stream
			 * since we are restarting the video stream and codec values will be reset
			 * commit the codec data again
			 */
			if(vd->requested_fmt == V4L2_PIX_FMT_H264 && h264_get_support() == H264_MUXED)
			{
				if(verbosity > 0)
					printf("V4L2_CORE: setting muxed H264 stream in MJPG container\n");
				set_h264_muxed_format(vd);
			}
			break;
	}
	
	if(stream_status == STRM_OK)
	{
		query_buff(vd); /*also mmaps the buffers*/
		queue_buff(vd);
	}

	/*try to start the video stream*/
	if(stream_status == STRM_OK)
		v4l2core_start_stream(vd);

	/*unlock the mutex*/
	__UNLOCK_MUTEX( __PMUTEX );

	return ret;
}

/*
 * checks if frame data is available
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code  (0- E_OK)
 */
static int check_frame_available(v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);

	int ret = E_OK;
	fd_set rdset;
	struct timeval timeout;

	/*lock the mutex*/
	__LOCK_MUTEX( __PMUTEX );
	int stream_state = vd->streaming;
	/*unlock the mutex*/
	__UNLOCK_MUTEX( __PMUTEX );

	/*make sure streaming is on*/
	if(stream_state != STRM_OK)
	{
		if(stream_state == STRM_REQ_STOP)
			v4l2core_stop_stream(vd);

        if(verbosity > 0) fprintf(stderr, "V4L2_CORE: (get_v4l2_frame) video stream must be started first\n");
		return E_NO_STREAM_ERR;
	}

	/*a fps change was requested while streaming*/
	if(flag_fps_change > 0)
	{
		if(verbosity > 2)
			printf("V4L2_CORE: fps change request detected\n");
		set_v4l2_framerate(vd);
		flag_fps_change = 0;
	}

	FD_ZERO(&rdset);
	FD_SET(vd->fd, &rdset);
	timeout.tv_sec = 1; /* 1 sec timeout*/
	timeout.tv_usec = 0;
	/* select - wait for data or timeout*/
	ret = select(vd->fd + 1, &rdset, NULL, NULL, &timeout);
	if (ret < 0)
	{
		fprintf(stderr, "V4L2_CORE: Could not grab image (select error): %s\n", strerror(errno));
		return E_SELECT_ERR;
	}

	if (ret == 0)
	{
		fprintf(stderr, "V4L2_CORE: Could not grab image (select timeout): %s\n", strerror(errno));
		return E_SELECT_TIMEOUT_ERR;
	}

	if ((ret > 0) && (FD_ISSET(vd->fd, &rdset)))
		return E_OK;

	return E_UNKNOWN_ERR;
}

/*
 * set verbosity
 * args:
 *   level - verbosity level
 *
 * asserts:
 *   none
 *
 * returns void
 */
void v4l2core_set_verbosity(int level)
{
    verbosity = level;
}

/*
 * set frame queue size (set before v4l2core_init_dev)
 * args:
 *   size - size in frames of frame queue
 *
 * asserts:
 *   none
 *
 * returns void
 */
void v4l2core_set_frame_queue_size(int size)
{
	frame_queue_size = size;
}

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
void v4l2core_disable_libv4l2()
{
	disable_libv4l2 = 1;
}

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
void v4l2core_enable_libv4l2()
{
	disable_libv4l2 = 0;
}

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
void v4l2core_set_capture_method(v4l2_dev_t *vd, int method)
{
	/*asserts*/
	assert(vd != NULL);

	vd->cap_meth = method;
}

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
void v4l2core_define_fps(v4l2_dev_t *vd, int num, int denom)
{
	/*assertions*/
	assert(vd != NULL);
	
	if(num > 0)
		vd->fps_num = num;
	if(denom > 0)
		vd->fps_denom = denom;
	
	if(verbosity > 2)
		printf("V4L2_CORE: fps configured to %i/%i\n", vd->fps_num, vd->fps_denom);
}

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
int v4l2core_get_fps_num(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->fps_num;	
}

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
int v4l2core_get_fps_denom(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->fps_denom;	
}

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
double v4l2core_get_realfps(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return(vd->real_fps);
}

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
const char *v4l2core_get_videodevice(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return (const char *) vd->videodevice;	
}

/*
 * get device available number of formats
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns - number of formats for device
 */
int v4l2core_get_number_formats(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->numb_formats;	
}

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
int v4l2core_has_pantilt_id(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->has_pantilt_control_id;
}

/*
 * get has_focus_control_id flag
 * args:
 *   vd -pointer to device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: has_focus_control_id flag
 */
int v4l2core_has_focus_control_id(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->has_focus_control_id;
}

/*
 * sets bayer pixel order
 * args:
 *   vd - pointer to v4l2 device handler
 *   order - pixel order
 *
 * asserts:
 *   vd is not null
 *
 * returns - void
 */
void v4l2core_set_bayer_pix_order(v4l2_dev_t *vd, uint8_t order)
{
	/*assertions*/
	assert(vd != NULL);
	
	vd->bayer_pix_order = order;
}

/*
 * gets bayer pixel order
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns - bayer pixel order
 */
uint8_t v4l2core_get_bayer_pix_order(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->bayer_pix_order;
}

/*
 * flags bayer mode
 * args:
 *   vd -pointer to device data
 *   flag - 1 if we are streaming bayer data (0 otherwise)
 *
 * asserts:
 *   vd is not null
 *
 * returns - void
 */
void v4l2core_set_isbayer(v4l2_dev_t *vd, uint8_t flag)
{
	/*assertions*/
	assert(vd != NULL);
	
	vd->isbayer = flag;
}

/*
 * gets bayer pixel order
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns - isbayer flag
 */
uint8_t v4l2core_get_isbayer(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->isbayer;
}

/*
 * gets current device index
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns - device index
 */
int v4l2core_get_this_device_index(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->this_device;
}

/*
 * Start video stream
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: VIDIOC_STREAMON ioctl result (E_OK or E_STREAMON_ERR)
*/
int v4l2core_start_stream(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);

	if(vd->streaming == STRM_OK)
	{
		fprintf(stderr, "V4L2_CORE: (stream already started) stream_status = STRM_OK\n");
		return E_OK;
	}

	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret=E_OK;
	switch(vd->cap_meth)
	{
		case IO_READ:
			//do nothing
			break;

		case IO_MMAP:
		default:
			ret = xioctl(vd->fd, VIDIOC_STREAMON, &type);
			if (ret < 0)
			{
				fprintf(stderr, "V4L2_CORE: (VIDIOC_STREAMON) Unable to start stream: %s \n", strerror(errno));
				return E_STREAMON_ERR;
			}
			break;
	}

	vd->streaming = STRM_OK;
	
	if(verbosity > 2)
		printf("V4L2_CORE: (VIDIOC_STREAMON) stream_status = STRM_OK\n");

	return ret;
}

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
int v4l2core_request_stop_stream(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);

	if(vd->streaming != STRM_OK)
		return -1;

	vd->streaming = STRM_REQ_STOP;
	
	if(verbosity > 2)
		printf("V4L2_CORE: (request stream stop) stream_status = STRM_REQ_STOP\n");

	return 0;
}

/*
 * Stops the video stream
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: VIDIOC_STREAMON ioctl result (E_OK)
*/
int v4l2core_stop_stream(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);

	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret=E_OK;
	switch(vd->cap_meth)
	{
		case IO_READ:
		case IO_MMAP:
		default:
			ret = xioctl(vd->fd, VIDIOC_STREAMOFF, &type);
			if (ret < 0)
			{
				if(errno == 9) /* stream allready stoped*/
					vd->streaming = STRM_STOP;
				fprintf(stderr, "V4L2_CORE: (VIDIOC_STREAMOFF) Unable to stop stream: %s\n", strerror(errno));
				return E_STREAMOFF_ERR;
			}
			break;
	}

	vd->streaming = STRM_STOP;
	
	if(verbosity > 2)
		printf("V4L2_CORE: (VIDIOC_STREAMOFF) stream_status = STRM_STOP\n");
		
	return ret;
}

/*
 * get next ready flaged frame from queue
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * returns: index of frame queue or -1 if none
 */
static int get_next_ready_frame(v4l2_dev_t *vd)
{
	int i = 0;
	for(i=0; i<vd->frame_queue_size; ++i)
	{
		if(vd->frame_queue[i].status == FRAME_READY)
			return (i);
	}
	
	return -1;
}

int get_my_width()
{
    return my_width;
}

int get_my_height()
{
    return my_height;
}
/*
 * process input buffer
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * returns: frame_queue index
 */
static int process_input_buffer(v4l2_dev_t *vd)
{
	/*get next available frame in queue*/
	int qind = get_next_ready_frame(vd);
	
	if(verbosity > 2)
		printf("V4L2_CORE: process frame queue index %i\n", qind);
	
	if(qind < 0 || qind >= vd->frame_queue_size)
	{
		if(verbosity > 2)
		fprintf(stderr,"V4L2_CORE: frame queue index %i is invalid (no free frames in queue?)\n", qind);
		return -1; 
	}
	
	vd->frame_queue[qind].status = FRAME_DECODING;
	
	/*
     * driver timestamp is unreliable
	 * use monotonic system time
	 */
	vd->frame_queue[qind].timestamp = ns_time_monotonic();
	
    vd->frame_queue[qind].index = (int)vd->buf.index;
	 
	vd->frame_index++;
	
	vd->frame_queue[qind].raw_frame_size = vd->buf.bytesused;
	if(vd->frame_queue[qind].raw_frame_size == 0)
	{
		if(verbosity > 1)
			fprintf(stderr, "V4L2_CORE: VIDIOC_QBUF returned buf.bytesused = 0 \n");
	}
	
	/*point vd->raw_frame to current frame buffer*/
	vd->frame_queue[qind].raw_frame = vd->mem[vd->buf.index];
	
	/*determine real fps every 3 sec aprox.*/
	fps_frame_count++;

	if(vd->frame_queue[qind].timestamp - fps_ref_ts >= (3 * NSEC_PER_SEC))
	{
		if(verbosity > 2)
			printf("V4L2CORE: (fps) ref:%"PRId64" ts:%"PRId64" frames:%i\n",
				fps_ref_ts, vd->frame_queue[qind].timestamp, fps_frame_count);
		vd->real_fps = (double) (fps_frame_count * NSEC_PER_SEC) / (double) (vd->frame_queue[qind].timestamp - fps_ref_ts);
		fps_frame_count = 0;
		fps_ref_ts = vd->frame_queue[qind].timestamp;
	}
	
	return qind;
} 
 
/*
 * gets the next video frame (must be released after processing)
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: pointer frame buffer (NULL on error)
 */
v4l2_frame_buff_t *v4l2core_get_frame(v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);

	/*for H264 streams request a IDR frame with SPS and PPS data if it's the first frame*/
	if(vd->requested_fmt == V4L2_PIX_FMT_H264 && vd->frame_index < 1)
		request_h264_frame_type(vd, PICTURE_TYPE_IDR_FULL);

	int res = 0;
	int ret = check_frame_available(vd);

	int qind = -1;
	
	if (ret < 0)
		return NULL;

	int bytes_used = 0;

	switch(vd->cap_meth)
	{
		case IO_READ:

			/*lock the mutex*/
			__LOCK_MUTEX( __PMUTEX );
			if(vd->streaming == STRM_OK)
			{
                vd->buf.bytesused = (__u32)getV4l2()->m_v4l2_read (vd->fd, vd->mem[vd->buf.index], vd->buf.length);
                bytes_used = (int)vd->buf.bytesused;

				if(bytes_used > 0)
					qind = process_input_buffer(vd);
			}
			else res = -1;
			/*unlock the mutex*/
			__UNLOCK_MUTEX( __PMUTEX );

			if(res < 0)
				return NULL;

			if (-1 == bytes_used )
			{
				switch (errno)
				{
					case EAGAIN:
						fprintf(stderr, "V4L2_CORE: No data available for read: %s\n", strerror(errno));
						break;
					case EINVAL:
						fprintf(stderr, "V4L2_CORE: Read method error, try mmap instead: %s\n", strerror(errno));
						break;
					case EIO:
						fprintf(stderr, "V4L2_CORE: read I/O Error: %s\n", strerror(errno));
						break;
					default:
						fprintf(stderr, "V4L2_CORE: read: %s\n", strerror(errno));
						break;
				}
				return NULL;
			}
			break;

		case IO_MMAP:
		default:
			//if((vd->setH264ConfigProbe > 0))
			//{

				//if(vd->setH264ConfigProbe)
				//{
					//video_disable(vd);
					//unmap_buff();

					//h264_commit(vd, global);

					//vd->setH264ConfigProbe = 0;
					//query_buff(vd);
					//queue_buff(vd);
					//video_enable(vd);
				//}

				//ret = check_frame_available(vd);

				//if (ret < 0)
					//return ret;
			//}

			/* dequeue the buffers */

			/*lock the mutex*/
			__LOCK_MUTEX( __PMUTEX );

			if(vd->streaming == STRM_OK)
			{
				memset(&vd->buf, 0, sizeof(struct v4l2_buffer));

				vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				vd->buf.memory = V4L2_MEMORY_MMAP;

                ret = xioctl(vd->fd, (int)VIDIOC_DQBUF, &vd->buf);

				if(!ret)
					qind = process_input_buffer(vd);
				else
					fprintf(stderr, "V4L2_CORE: (VIDIOC_DQBUF) Unable to dequeue buffer: %s\n", strerror(errno));
			}
			else res = -1;

			/*unlock the mutex*/
			__UNLOCK_MUTEX( __PMUTEX );

			if(res < 0 || ret < 0)
				return NULL;
	}

	if(qind < 0 || qind >= vd->frame_queue_size)
		return NULL;

    vd->frame_queue[qind].width = (int)vd->format.fmt.pix.width;
    vd->frame_queue[qind].height = (int)vd->format.fmt.pix.height;
	
	return &vd->frame_queue[qind];
}

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
int v4l2core_release_frame(v4l2_dev_t *vd, v4l2_frame_buff_t *frame)
{
	int ret = 0;
	
	//match the v4l2_buffer with the correspondig frame
    vd->buf.index = (__u32)frame->index;
	
	switch(vd->cap_meth)
	{
		case IO_READ:
			break;
		
		case IO_MMAP:
		default:
			/* queue the buffer */
            ret = xioctl(vd->fd, (int)VIDIOC_QBUF, &vd->buf);

			if(ret)
				fprintf(stderr, "V4L2_CORE: (VIDIOC_QBUF) Unable to queue buffer %i: %s\n", frame->index, strerror(errno));
			break;	
	}
	
	/*lock the mutex*/
	__LOCK_MUTEX( __PMUTEX );
	frame->raw_frame = NULL;
	frame->raw_frame_size = 0;
	frame->status = FRAME_READY;
	/*unlock the mutex*/
	__UNLOCK_MUTEX( __PMUTEX );
	
	if (ret < 0)
		return E_QBUF_ERR;
	
	return E_OK;
}

/*
 * gets the next video frame and decodes it
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * returns: pointer to decoded frame buffer ( NULL on error)
 */
v4l2_frame_buff_t *v4l2core_get_decoded_frame(v4l2_dev_t *vd)
{
	v4l2_frame_buff_t *frame = v4l2core_get_frame(vd);
    if(frame != NULL)
    {
        /*decode the raw frame*/
        if(decode_v4l2_frame(vd, frame) != E_OK)
        {
            fprintf(stderr, "V4L2_CORE: Error - Couldn't decode frame\n");
        }
    }
	
	return frame;
}

/*
 * Try/Set device video stream format
 * args:
 *   vd -pointer to device data
 *   width - requested video frame width
 *   height - requested video frame height
 *   pixelformat - requested v4l2 pixelformat
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code ( E_OK)
 */
static int try_video_stream_format(v4l2_dev_t *vd, 
	int width, int height, int pixelformat)
{
	/*assertions*/
	assert(vd != NULL);

	int ret = E_OK;

	/*lock the mutex*/
	__LOCK_MUTEX( __PMUTEX );

        int old_format = vd->requested_fmt;

	vd->requested_fmt = pixelformat;

	uint8_t stream_status = vd->streaming;

	if(stream_status == STRM_OK)
		v4l2core_stop_stream(vd);

	if(vd->requested_fmt == V4L2_PIX_FMT_H264 && h264_get_support() == H264_MUXED)
	{
		if(verbosity > 0)
			printf("V4L2_CORE: requested H264 stream is supported through muxed MJPG\n");
		pixelformat = V4L2_PIX_FMT_MJPEG;
	}

    vd->format.fmt.pix.pixelformat = (__u32)pixelformat;
    vd->format.fmt.pix.width = (__u32)width;
    vd->format.fmt.pix.height = (__u32)height;

	/* make sure we set a valid format*/
	if(verbosity > 0)
		printf("V4L2_CORE: checking format: %c%c%c%c\n",
			(vd->format.fmt.pix.pixelformat) & 0xFF, ((vd->format.fmt.pix.pixelformat) >> 8) & 0xFF,
			((vd->format.fmt.pix.pixelformat) >> 16) & 0xFF, ((vd->format.fmt.pix.pixelformat) >> 24) & 0xFF);

	/*override field and type entries*/
	vd->format.fmt.pix.field = V4L2_FIELD_ANY;
	vd->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = xioctl(vd->fd, (int)VIDIOC_S_FMT, &vd->format);

	if(!ret && (vd->requested_fmt == V4L2_PIX_FMT_H264) && (h264_get_support() == H264_MUXED))
	{
		if(verbosity > 0)
			printf("V4L2_CORE: setting muxed H264 stream in MJPG container\n");
		set_h264_muxed_format(vd);
	}

	/*unlock the mutex*/
	__UNLOCK_MUTEX( __PMUTEX );

	if (ret != 0)
	{
		fprintf(stderr, "V4L2_CORE: (VIDIOC_S_FORMAT) Unable to set format: %s\n", strerror(errno));
                //reset to old format
                vd->requested_fmt = old_format;
                my_pixelformat = vd->requested_fmt;

		return E_FORMAT_ERR;
	}

	my_pixelformat = vd->requested_fmt;

    if (((int)vd->format.fmt.pix.width != width) ||
        ((int)vd->format.fmt.pix.height != height))
	{
		fprintf(stderr, "V4L2_CORE: Requested resolution unavailable: got width %d height %d\n",
		vd->format.fmt.pix.width, vd->format.fmt.pix.height);
	}

	/*
	 * try to alloc frame buffers based on requested format
	 */
	ret = alloc_v4l2_frames(vd);
	if( ret != E_OK)
	{
		fprintf(stderr, "V4L2_CORE: Frame allocation returned error (%i)\n", ret);
		return E_ALLOC_ERR;
	}

	switch (vd->cap_meth)
	{
		case IO_READ: /*allocate buffer for read*/
			/*lock the mutex*/
			__LOCK_MUTEX( __PMUTEX );

			memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
			vd->buf.length = (vd->format.fmt.pix.width) * (vd->format.fmt.pix.height) * 3; //worst case (rgb)
			vd->mem[vd->buf.index] = calloc(vd->buf.length, sizeof(uint8_t));
			if(vd->mem[vd->buf.index] == NULL)
			{
				fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (try_video_stream_format): %s\n", strerror(errno));
				exit(-1);
			}

			/*unlock the mutex*/
			__UNLOCK_MUTEX( __PMUTEX );
			break;

		case IO_MMAP:
		default:
			/* request buffers */
			memset(&vd->rb, 0, sizeof(struct v4l2_requestbuffers));
			vd->rb.count = NB_BUFFER;
			vd->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			vd->rb.memory = V4L2_MEMORY_MMAP;

            ret = xioctl(vd->fd, (int)VIDIOC_REQBUFS, &vd->rb);

			if (ret < 0)
			{
				fprintf(stderr, "V4L2_CORE: (VIDIOC_REQBUFS) Unable to allocate buffers: %s\n", strerror(errno));
				return E_REQBUFS_ERR;
			}
			/* map the buffers */
			if (query_buff(vd))
			{
				fprintf(stderr, "V4L2_CORE: (VIDIOC_QBUFS) Unable to query buffers: %s\n", strerror(errno));
				/*
				 * delete requested buffers
				 * no need to unmap as mmap failed for sure
				 */
				if(verbosity > 0)
					printf("V4L2_CORE: cleaning requestbuffers\n");
				memset(&vd->rb, 0, sizeof(struct v4l2_requestbuffers));
				vd->rb.count = 0;
				vd->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				vd->rb.memory = V4L2_MEMORY_MMAP;
                if(xioctl(vd->fd, (int)VIDIOC_REQBUFS, &vd->rb)<0)
					fprintf(stderr, "V4L2_CORE: (VIDIOC_REQBUFS) Unable to delete buffers: %s\n", strerror(errno));

				return E_QUERYBUF_ERR;
			}

			/* Queue the buffers */
			if (queue_buff(vd))
			{
				fprintf(stderr, "V4L2_CORE: (VIDIOC_QBUFS) Unable to queue buffers: %s\n", strerror(errno));
				/*delete requested buffers */
				if(verbosity > 0)
					printf("V4L2_CORE: cleaning requestbuffers\n");
				unmap_buff(vd);
				memset(&vd->rb, 0, sizeof(struct v4l2_requestbuffers));
				vd->rb.count = 0;
				vd->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				vd->rb.memory = V4L2_MEMORY_MMAP;
                if(xioctl(vd->fd, (int)VIDIOC_REQBUFS, &vd->rb)<0)
					fprintf(stderr, "V4L2_CORE: (VIDIOC_REQBUFS) Unable to delete buffers: %s\n", strerror(errno));
				return E_QBUF_ERR;
			}
	}

	/*this locks the mutex (can't be called while the mutex is being locked)*/
	v4l2core_request_framerate_update(vd);

	if(stream_status == STRM_OK)
		v4l2core_start_stream(vd);

	/*update the current framerate for the device*/
	v4l2core_get_framerate(vd);

	return E_OK;
}

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
int v4l2core_get_frame_width(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
    return (int)vd->format.fmt.pix.width;
}

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
int v4l2core_get_frame_height(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
    return (int)vd->format.fmt.pix.height;
}

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
int v4l2core_get_requested_frame_format(v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);

        return my_pixelformat;

	//return vd->requested_fmt;
}

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
void v4l2core_prepare_new_format(v4l2_dev_t *vd, int new_format)
{
	/*asserts*/
	assert(vd != NULL);

	int format_index = v4l2core_get_frame_format_index(vd, new_format);

	if(format_index < 0)
		format_index = 0;

	my_pixelformat = vd->list_stream_formats[format_index].format;
}

/*
 * prepare a valid format (first in the format list)
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * returns: none
 */
void v4l2core_prepare_valid_format(v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);

	int format_index = 0;

	my_pixelformat = vd->list_stream_formats[format_index].format;
}

/*
 * prepare new resolution
 * args:
 *   vd - pointer to v4l2 device handler
 *   new_width - new width
 *   new_height - new height
 *
 * asserts:
 *    vd is not null
 *
 * returns: none
 */
void v4l2core_prepare_new_resolution(v4l2_dev_t *vd, 
	int new_width, int new_height)
{
	/*asserts*/
	assert(vd != NULL);

	int format_index = v4l2core_get_frame_format_index(vd, my_pixelformat);

    if(format_index < 0){
        if (vd->list_stream_formats){
            format_index = 0;
        } else {
            return;
        }
    }

	int resolution_index = v4l2core_get_format_resolution_index(vd, 
		format_index, new_width, new_height);

    if(resolution_index < 0 ){
        if (vd->list_stream_formats[format_index].list_stream_cap){
            resolution_index = 0;
        } else {
            return;
        }
    }
	my_width  = vd->list_stream_formats[format_index].list_stream_cap[resolution_index].width;
	my_height = vd->list_stream_formats[format_index].list_stream_cap[resolution_index].height;
}

/*
 * prepare valid resolution (first in the resolution list for the format)
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * returns: none
 */
void v4l2core_prepare_valid_resolution(v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);

	int format_index = v4l2core_get_frame_format_index(vd, my_pixelformat);
    my_width = 0;
    my_height = 0;

    if(format_index < 0){
        if (vd->list_stream_formats){
            format_index = 0;
        } else {
            return;
        }
    }

    for(int i=0; i < vd->list_stream_formats[format_index].numb_res; i++)
    {
        if( my_width <= vd->list_stream_formats[format_index].list_stream_cap[i].width &&
            my_height <= vd->list_stream_formats[format_index].list_stream_cap[i].height)
        {
            my_width = vd->list_stream_formats[format_index].list_stream_cap[i].width;
            my_height = vd->list_stream_formats[format_index].list_stream_cap[i].height;
        }
    }
}

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
int v4l2core_update_old_format(v4l2_dev_t *vd,int width,int height ,int pixelformat)
{
    /*asserts*/
    assert(vd != NULL);
    my_pixelformat = pixelformat;
    my_width = width;
    my_height = height;

    return(try_video_stream_format(vd, my_width, my_height, my_pixelformat));
}


/*
 * update the current format (pixelformat, width and height)
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * returns:
 *    error code
 */
int v4l2core_update_current_format(v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);

	return(try_video_stream_format(vd, my_width, my_height, my_pixelformat));
}

/*
 * clean video device data allocation
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   vd is not null
 *
 * returns: void
 */
static void clean_v4l2_dev(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);

	if(vd->videodevice)
		free(vd->videodevice);
	vd->videodevice = NULL;

	if(vd->has_focus_control_id)
		v4l2core_soft_autofocus_close();

	if(vd->list_device_controls)
		free_v4l2_control_list(vd);

	if(vd->list_stream_formats)
		free_frame_formats(vd);

	if(vd->frame_queue)
		free(vd->frame_queue);

	/*close descriptor*/
	if(vd->fd > 0)
        getV4l2()->m_v4l2_close(vd->fd);

	vd->fd = 0;

	free(vd);
}

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
v4l2_dev_t* v4l2core_init_dev(const char *device)
{
	/*assertions*/
    //assert(device != NULL);
	
	///*make sure to close and clean any existing device data*/
	//if(vd != NULL)
	//	v4l2core_close_dev();

	/*localization*/
	char* lc_all = setlocale (LC_ALL, "");
	char* lc_dir = bindtextdomain (GETTEXT_PACKAGE_V4L2CORE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE_V4L2CORE, "UTF-8");
	if (verbosity > 1) printf("V4L2_CORE: language catalog=> dir:%s type:%s cat:%s.mo\n",
		lc_dir, lc_all, GETTEXT_PACKAGE_V4L2CORE);

	/*alloc the device data*/
	v4l2_dev_t* vd = calloc(1, sizeof(v4l2_dev_t));

	assert(vd != NULL);
	
	/*init the device mutex*/
	__INIT_MUTEX(__PMUTEX);

	/*MMAP by default*/
	vd->cap_meth = IO_MMAP;

	vd->videodevice = strdup(device);

	if(verbosity > 0)
	{
		printf("V4L2_CORE: capture method mmap (%i)\n",vd->cap_meth);
		printf("V4L2_CORE: video device: %s \n", vd->videodevice);
	}

	vd->frame_queue_size = frame_queue_size;
	/*alloc frame buffer queue*/
    vd->frame_queue = calloc((size_t)vd->frame_queue_size, sizeof(v4l2_frame_buff_t));
	
	vd->h264_no_probe_default = 0;
	vd->h264_SPS = NULL;
	vd->h264_SPS_size = 0;
	vd->h264_PPS = NULL;
	vd->h264_PPS_size = 0;
	vd->h264_last_IDR = NULL;
	vd->h264_last_IDR_size = 0;

	/*set some defaults*/
	vd->fps_num = 1;
	vd->fps_denom = 25;

	vd->pan_step = 128;
	vd->tilt_step = 128;

	/*open device*/
    if ((vd->fd = getV4l2()->m_v4l2_open(vd->videodevice, O_RDWR | O_NONBLOCK, 0)) < 0)
	{
		fprintf(stderr, "V4L2_CORE: ERROR opening V4L interface: %s\n", strerror(errno));
		clean_v4l2_dev(vd);
		return (NULL);
	}

    vd->this_device = v4l2core_get_device_index(vd->videodevice);
	if(vd->this_device < 0)
		vd->this_device = 0;

	v4l2_device_list_t *device_list = get_device_list();
	
	if(device_list && device_list->list_devices)
		device_list->list_devices[vd->this_device].current = 1;

	/*try to map known xu controls (we could/should leave this for libwebcam)*/
	init_xu_ctrls(vd);

	/*zero structs*/
	memset(&vd->cap, 0, sizeof(struct v4l2_capability));
	memset(&vd->format, 0, sizeof(struct v4l2_format));
	memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
	memset(&vd->rb, 0, sizeof(struct v4l2_requestbuffers));
	memset(&vd->streamparm, 0, sizeof(struct v4l2_streamparm));
	memset(&vd->evsub, 0, sizeof(struct v4l2_event_subscription));

	if(check_v4l2_dev(vd) != E_OK)
	{
		clean_v4l2_dev(vd);
		return (NULL);
	}

	int i = 0;
	for (i = 0; i < NB_BUFFER; i++)
	{
		vd->mem[i] = MAP_FAILED; /*not mmaped yet*/
	}

	return (vd);
}

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
v4l2_stream_formats_t *v4l2core_get_formats_list(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->list_stream_formats;
}

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
v4l2_ctrl_t *v4l2core_get_control_list(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->list_device_controls;
}

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
int v4l2core_check_control_events(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	int ret = 0;
	struct v4l2_event ev;

    while (xioctl(vd->fd, (int)VIDIOC_DQEVENT, &ev) == 0)
	{
		if (ev.type != V4L2_EVENT_CTRL)
			continue;

		ret++;
		//update control
        v4l2_ctrl_t *control = v4l2core_get_control_by_id(vd, (int)ev.id);
		if(control != NULL)
		{
			control->control.flags = ev.u.ctrl.flags;
			if(control->control.flags & V4L2_CTRL_FLAG_DISABLED)
				continue;

			control->control.minimum = ev.u.ctrl.minimum;
			control->control.maximum = ev.u.ctrl.maximum;
			control->control.step = ev.u.ctrl.step;
			control->control.default_value = ev.u.ctrl.default_value;

			switch (control->control.type)
			{
#ifdef V4L2_CTRL_TYPE_INTEGER64
				case V4L2_CTRL_TYPE_INTEGER64:
					control->value64 = ev.u.ctrl.value64;
					break;
#endif
#ifdef V4L2_CTRL_TYPE_STRING
				case V4L2_CTRL_TYPE_STRING:
					break;
#endif
				default:
					control->value = ev.u.ctrl.value;
			}
		}
	}

	return ret;
}

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
int v4l2core_get_pan_step(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->pan_step;
}

/*
 * get device tilt step value
* args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * return: pan step value
 */
int v4l2core_get_tilt_step(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->tilt_step;
}

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
void v4l2core_set_pan_step(v4l2_dev_t *vd, int step)
{
	/*assertions*/
	assert(vd != NULL);
	
	vd->pan_step = step;
}

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
void v4l2core_set_tilt_step(v4l2_dev_t *vd, int step)
{
	/*assertions*/
	assert(vd != NULL);
	
	vd->tilt_step = step;
}

/*
 * initiate software autofocus
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    none
 *
 * returns: error code (0 - E_OK)
 */
int v4l2core_soft_autofocus_init (v4l2_dev_t *vd)
{
	return soft_autofocus_init(vd);
}

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
 * 	(only matters for non-continue focus)
 */
int v4l2core_soft_autofocus_run(v4l2_dev_t *vd, v4l2_frame_buff_t *frame)
{
	return soft_autofocus_run(vd, frame);
}

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
void v4l2core_clean_buffers(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);

	if(verbosity > 1)
		printf("V4L2_CORE: cleaning v4l2 buffers\n");

	if(vd->streaming == STRM_OK)
		v4l2core_stop_stream(vd);

	clean_v4l2_frames(vd);

	// unmap queue buffers
	switch(vd->cap_meth)
	{
		case IO_READ:
			if(vd->mem[vd->buf.index]!= NULL)
	    	{
				free(vd->mem[vd->buf.index]);
				vd->mem[vd->buf.index] = NULL;
			}
			break;

		case IO_MMAP:
		default:
			//delete requested buffers
			unmap_buff(vd);
			memset(&vd->rb, 0, sizeof(struct v4l2_requestbuffers));
			vd->rb.count = 0;
			vd->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			vd->rb.memory = V4L2_MEMORY_MMAP;
            if(xioctl(vd->fd, (int)VIDIOC_REQBUFS, &vd->rb)<0)
			{
				fprintf(stderr, "V4L2_CORE: (VIDIOC_REQBUFS) Failed to delete buffers: %s (errno %d)\n", strerror(errno), errno);
			}
			break;
	}
}
/*
 * cleans video device data and allocations
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   none
 *
 * returns: void
 */
void v4l2core_close_dev(v4l2_dev_t *vd)
{
	if(vd == NULL)
		return;

	/* thread must be joined before destroying the mutex
         * so no need to unlock before destroying it
         */

	/*destroy the device mutex*/
	__CLOSE_MUTEX(__PMUTEX);

	v4l2core_clean_buffers(vd);
	clean_v4l2_dev(vd);
}

/*
 * request a fps update - this locks the mutex
 *   (can't be called while the mutex is being locked)
 * args:
 *    vd - pointer to v4l2 device handler
 *
 * asserts:
 *    vd is not null
 *
 * returns: none
 */
void v4l2core_request_framerate_update(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);

	/*
	 * if we are streaming flag a fps change when retrieving frame
	 * else change fps immediatly
	 */
	if(vd->streaming == STRM_OK)
		flag_fps_change = 1;
	else
		set_v4l2_framerate(vd);
}

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
int v4l2core_get_framerate (v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);

	int ret=0;

	vd->streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = xioctl(vd->fd, (int)VIDIOC_G_PARM, &vd->streamparm);
	if (ret < 0)
	{
		fprintf(stderr, "V4L2_CORE: (VIDIOC_G_PARM) error: %s\n", strerror(errno));
		return ret;
	}
	else
	{
		if (vd->streamparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
		{
            vd->fps_denom = (int)vd->streamparm.parm.capture.timeperframe.denominator;
            vd->fps_num = (int)vd->streamparm.parm.capture.timeperframe.numerator;
		}
	}

	if(vd->fps_denom == 0 )
		vd->fps_denom = 1;
	if(vd->fps_num == 0)
		vd->fps_num = 1;

	return ret;
}

/*
 * return the control associated to id from device list
 * args:
 *   vd - pointer to v4l2 device handler
 *   id - control id
 *
 * asserts:
 *   vd is not null
 *   vd->list_device_controls is not null
 *
 * returns: pointer to v4l2_control if succeded or null otherwise
 */
v4l2_ctrl_t *v4l2core_get_control_by_id(v4l2_dev_t *vd, int id)
{
	return get_control_by_id(vd, id);
}

/*
 * updates the value for control id from the device
 * also updates control flags
 * args:
 *   vd - pointer to v4l2 device handler
 *   id -control id
 *
 * asserts:
 *   none
 *
 * returns: ioctl result
 */
int v4l2core_get_control_value_by_id (v4l2_dev_t *vd, int id)
{
	return get_control_value_by_id (vd, id);
}

/*
 * goes trough the control list and sets values in device to default
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   none
 *
 * returns: void
 */
void v4l2core_set_control_defaults(v4l2_dev_t *vd)
{
	set_control_defaults(vd);
}

/*
 * sets the value of control id in device
 * args:
 *   vd - pointer to v4l2 device handler
 *   id - control id
 *
 * asserts:
 *   none
 *
 * returns: ioctl result
 */
int v4l2core_set_control_value_by_id(v4l2_dev_t *vd, int id)
{
	return set_control_value_by_id(vd, id);
}

/*
 * save the current frame to file
 * args:
 *    frame - pointer to frame buffer
 *    filename - output file name
 *    format - image type
 *           (IMG_FMT_RAW, IMG_FMT_JPG, IMG_FMT_PNG, IMG_FMT_BMP)
 *
 * asserts:
 *    vd is not null
 *
 * returns: error code
 */
int v4l2core_save_image(
	v4l2_frame_buff_t *frame, 
	const char *filename, 
	int format)
{
	return save_frame_image(frame, filename, format);
}

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
int v4l2core_get_h264_unit_id(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->h264_unit_id;
}

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
uvcx_video_config_probe_commit_t *v4l2core_get_h264_config_probe_req(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return  &(vd->h264_config_probe_req);
}

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
void v4l2core_set_h264_no_probe_default(v4l2_dev_t *vd, uint8_t flag)
{
	/*assertions*/
	assert(vd != NULL);
	
	vd->h264_no_probe_default = flag;
}

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
uint8_t v4l2core_get_h264_no_probe_default(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->h264_no_probe_default;
}

/*
 * get PPS NALU size
 * args:
 *   none
 *
 * asserts:
 *   vd is not null
 *
 * returns: PPS size
 */
int v4l2core_get_h264_pps_size(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->h264_PPS_size;
}

/*
 * get PPS data
 * args:
 *   none
 *
 * asserts:
 *   vd is not null
 *
 * returns: pointer to PPS data
 */
uint8_t *v4l2core_get_h264_pps(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->h264_PPS;
}

/*
 * get SPS NALU size
 * args:
 *   none
 *
 * asserts:
 *   vd is not null
 *
 * returns: SPS size
 */
int v4l2core_get_h264_sps_size(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->h264_SPS_size;
}

/*
 * get SPS data
 * args:
 *   none
 *
 * asserts:
 *   vd is not null
 *
 * returns: pointer to SPS data
 */
uint8_t *v4l2core_get_h264_sps(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	
	return vd->h264_SPS;
}

/*
 * request a IDR frame from the H264 encoder
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void v4l2core_h264_request_idr(v4l2_dev_t *vd)
{
	h264_request_idr(vd);
}

/*
 * resets the h264 encoder
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: 0 on success or error code on fail
 */
int v4l2core_reset_h264_encoder(v4l2_dev_t *vd)
{
	return h264_reset_encoder(vd);
}

/*
 * get the video rate control mode
 * args:
 *   query - query type
 *
 * asserts:
 *   none
 *
 * returns: video rate control mode (FIXME: 0xff on error)
 */
uint8_t v4l2core_get_h264_video_rate_control_mode(v4l2_dev_t *vd, uint8_t query)
{
	return h264_get_video_rate_control_mode(vd, query);
}

/*
 * set the video rate control mode
 * args:
 *   mode - rate mode
 *
 * asserts:
 *   none
 *
 * returns: error code ( 0 -OK)
 */
int v4l2core_set_h264_video_rate_control_mode(v4l2_dev_t *vd, uint8_t mode)
{
	return h264_set_video_rate_control_mode(vd, mode);
}

/*
 * get the temporal scale mode
 * args:
 *   query - query type
 *
 * asserts:
 *   none
 *
 * returns: temporal scale mode (FIXME: 0xff on error)
 */
uint8_t v4l2core_get_h264_temporal_scale_mode(v4l2_dev_t *vd, uint8_t query)
{
	return h264_get_temporal_scale_mode(vd, query);
}

/*
 * set the temporal scale mode
 * args:
 *   mode - temporal scale mode
 *
 * asserts:
 *   none
 *
 * returns: error code ( 0 -OK)
 */
int v4l2core_set_h264_temporal_scale_mode(v4l2_dev_t *vd, uint8_t mode)
{
	return h264_set_temporal_scale_mode(vd, mode);
}

/*
 * get the spatial scale mode
 * args:
 *   query - query type
 *
 * asserts:
 *   none
 *
 * returns: temporal scale mode (FIXME: 0xff on error)
 */
uint8_t v4l2core_get_h264_spatial_scale_mode(v4l2_dev_t *vd, uint8_t query)
{
	return h264_get_spatial_scale_mode(vd, query);
}

/*
 * set the spatial scale mode
 * args:
 *   vd - pointer to v4l2 device handler
 *   mode - spatial scale mode
 *
 * asserts:
 *   none
 *
 * returns: error code ( 0 -OK)
 */
int v4l2core_set_h264_spatial_scale_mode(v4l2_dev_t *vd, uint8_t mode)
{
	return h264_set_spatial_scale_mode(vd, mode);
}

/*
 * query the frame rate config
 * args:
 *   vd - pointer to v4l2 device handler
 *   query - query type
 *
 * asserts:
 *   none
 *
 * returns: frame rate config (FIXME: 0xffffffff on error)
 */
uint32_t v4l2core_query_h264_frame_rate_config(v4l2_dev_t *vd, uint8_t query)
{
	return h264_query_frame_rate_config(vd, query);
}

/*
 * get the frame rate config
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   none
 *
 * returns: frame rate config (FIXME: 0xffffffff on error)
 */
uint32_t v4l2core_get_h264_frame_rate_config(v4l2_dev_t *vd)
{
	return h264_get_frame_rate_config(vd);
}

/*
 * set the frame rate config
 * args:
 *   vd - pointer to v4l2 device handler
 *   framerate - framerate
 *
 * asserts:
 *   none
 *
 * returns: error code ( 0 -OK)
 */
int v4l2core_set_h264_frame_rate_config(v4l2_dev_t *vd, uint32_t framerate)
{
	return h264_set_frame_rate_config(vd, framerate);
}

/*
 * updates the h264_probe_commit_req field
 * args:
 *   vd - pointer to v4l2 device handler
 *   query - (UVC_GET_CUR; UVC_GET_MAX; UVC_GET_MIN)
 *   config_probe_cur - pointer to uvcx_video_config_probe_commit_t:
 *     if null vd->h264_config_probe_req will be used
 *
 * asserts:
 *   none
 *
 * returns: error code ( 0 -OK)
 */
int v4l2core_probe_h264_config_probe_req(
			v4l2_dev_t *vd,
			uint8_t query,
			uvcx_video_config_probe_commit_t *config_probe_req)
{
	return h264_probe_config_probe_req(vd, query, config_probe_req);
}

/*
 * check for new devices
 * args:
 *   vd - pointer to v4l2 device handler
 *
 * asserts:
 *   my_device_list.udev is not null
 *   my_device_list.udev_fd is valid (> 0)
 *   my_device_list.udev_mon is not null
 *
 * returns: true(1) if device list was updated, false(0) otherwise
 */
int v4l2core_check_device_list_events(v4l2_dev_t *vd)
{
	return check_device_list_events(vd);
}

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
int v4l2core_get_frame_format_index(v4l2_dev_t *vd, int format)
{
	return get_frame_format_index(vd, format);
}

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
int v4l2core_get_format_resolution_index(
	v4l2_dev_t *vd,
	int format,
	int width,
	int height)
{
	return get_format_resolution_index(vd, format, width, height);
}

/*
 * save the device control values into a profile file
 * args:
 *   vd - pointer to v4l2 device handler
 *   filename - profile filename
 *
 * asserts:
 *   none
 *
 * returns: error code (0 -E_OK)
 */
int v4l2core_save_control_profile(v4l2_dev_t *vd, const char *filename)
{
	return save_control_profile(vd, filename);
}

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
int v4l2core_load_control_profile(v4l2_dev_t *vd, const char *filename)
{
	return load_control_profile(vd, filename);
}

/*
 * get lenght of xu control defined by unit id and selector
 * args:
 *   vd - pointer to v4l2 device handler
 *   unit - unit id of xu control
 *   selector - selector for control
 *
 * asserts:
 *   none
 *
 * returns: length of xu control
 */
uint16_t v4l2core_get_length_xu_control(
	v4l2_dev_t *vd,
	uint8_t unit,
	uint8_t selector)
{
	return get_length_xu_control(vd, unit, selector);
}

/*
 * get uvc info for xu control defined by unit id and selector
 * args:
 *   vd - pointer to v4l2 device handler
 *   unit - unit id of xu control
 *   selector - selector for control
 *
 * asserts:
 *   none
 *
 * returns: info of xu control
 */
uint8_t v4l2core_get_info_xu_control(
	v4l2_dev_t *vd,
	uint8_t unit,
	uint8_t selector)
{
	return get_info_xu_control(vd, unit, selector);
}

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
 *   none
 *
 * returns: 0 if query succeded or errno otherwise
 */
int v4l2core_query_xu_control(
	v4l2_dev_t *vd,
	uint8_t unit,
	uint8_t selector,
	uint8_t query,
	void *data)
{
	return query_xu_control(vd, unit, selector, query, data);
}
