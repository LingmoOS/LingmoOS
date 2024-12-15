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
#include <ctype.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "gview.h"
#include "v4l2_formats.h"

extern int verbosity;

/*  Four-character-code (FOURCC) */
#ifndef v4l2_fourcc
#define v4l2_fourcc(a, b, c, d)\
			((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))
#endif

#define v4l2_pixformat2fourcc(pixelformat)\
		{  pixelformat & 0x000000FF,\
		  (pixelformat >> 8) & 0x000000FF,\
		  (pixelformat >> 16) & 0x000000FF,\
		  (pixelformat >> 24) & 0x000000FF,\
		  0\
		}

static uint32_t decoder_supported_formats[] =
{
	V4L2_PIX_FMT_YUYV,
	V4L2_PIX_FMT_MJPEG,
	V4L2_PIX_FMT_JPEG,
	V4L2_PIX_FMT_H264,
	V4L2_PIX_FMT_YVYU,
	V4L2_PIX_FMT_UYVY,
	V4L2_PIX_FMT_VYUY,
	V4L2_PIX_FMT_YYUV,
	V4L2_PIX_FMT_YUV444,
	V4L2_PIX_FMT_YUV555,
	V4L2_PIX_FMT_YUV565,
	V4L2_PIX_FMT_YUV32,
	V4L2_PIX_FMT_Y41P,
	V4L2_PIX_FMT_GREY,
	V4L2_PIX_FMT_Y10BPACK,
	V4L2_PIX_FMT_Y16,
#ifdef V4L2_PIX_FMT_Y16_BE
	V4L2_PIX_FMT_Y16_BE,
#endif
	V4L2_PIX_FMT_YUV420,
	V4L2_PIX_FMT_YUV422P,
	V4L2_PIX_FMT_YVU420,
	V4L2_PIX_FMT_NV12,
	V4L2_PIX_FMT_NV21,
	V4L2_PIX_FMT_NV16,
	V4L2_PIX_FMT_NV61,
	V4L2_PIX_FMT_NV24,
	V4L2_PIX_FMT_NV42,
	V4L2_PIX_FMT_SPCA501,
	V4L2_PIX_FMT_SPCA505,
	V4L2_PIX_FMT_SPCA508,
	V4L2_PIX_FMT_SGBRG8,
	V4L2_PIX_FMT_SGRBG8,
	V4L2_PIX_FMT_SBGGR8,
	V4L2_PIX_FMT_SRGGB8,
	V4L2_PIX_FMT_RGB24,
	V4L2_PIX_FMT_BGR24,
	V4L2_PIX_FMT_RGB332,
	V4L2_PIX_FMT_RGB565,
	V4L2_PIX_FMT_RGB565X,
	V4L2_PIX_FMT_RGB444,
	V4L2_PIX_FMT_RGB555,
	V4L2_PIX_FMT_RGB555X,
	V4L2_PIX_FMT_BGR666,
	V4L2_PIX_FMT_BGR32,
	V4L2_PIX_FMT_RGB32,
#ifdef V4L2_PIX_FMT_ARGB444
	V4L2_PIX_FMT_ARGB444,
	V4L2_PIX_FMT_XRGB444,
#endif
#ifdef V4L2_PIX_FMT_ARGB555
	V4L2_PIX_FMT_ARGB555,
	V4L2_PIX_FMT_XRGB555,
#endif
#ifdef V4L2_PIX_FMT_ARGB555X
	V4L2_PIX_FMT_ARGB555X,
	V4L2_PIX_FMT_XRGB555X,
#endif
#ifdef V4L2_PIX_FMT_ABGR32
	V4L2_PIX_FMT_ABGR32,
	V4L2_PIX_FMT_XBGR32,
#endif
#ifdef V4L2_PIX_FMT_ARGB32
	V4L2_PIX_FMT_ARGB32,
	V4L2_PIX_FMT_XRGB32,
#endif
	/*last one (zero terminated)*/
	0
};

/* FIXME: doesn't support bigendian formats=> fourcc | (1 << 31)
 * get pixelformat from fourcc
 * args:
 *    fourcc - fourcc code for format
 *
 * asserts:
 *    none
 *
 * returns: v4l2 pixel format
 */
uint32_t v4l2core_fourcc_2_v4l2_pixelformat(const char *fourcc)
{
	int fmt = 0;
	if(!fourcc || strlen(fourcc) !=  4)
		return fmt;
	else
		fmt = v4l2_fourcc(toupper(fourcc[0]), toupper(fourcc[1]), toupper(fourcc[2]), toupper(fourcc[3]));

	return fmt;
}

/*
 * check pixelformat against decoder support formats
 * args:
 *    pixelformat - v4l2 pixelformat
 *
 * asserts:
 *    none
 *
 * returns: TRUE(1) if format is supported
 *          FALSE(0) if not
 */
uint8_t can_decode_format(uint32_t pixelformat)
{
	int i = 0;
	uint32_t sup_fmt = 0;

	do
	{
		sup_fmt = decoder_supported_formats[i];

		if(pixelformat == sup_fmt)
			return TRUE;

		i++;
	}
	while(sup_fmt); /*last format is always 0*/

	return FALSE;
}

/*
 * enumerate frame intervals (fps)
 * args:
 *   vd - pointer to video device data
 *   pixfmt - v4l2 pixel format that we want to list frame intervals for
 *   width - video width that we want to list frame intervals for
 *   height - video height that we want to list frame intervals for
 *   fmtind - current index of format list
 *   fsizeind - current index of frame size list
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid ( > 0 )
 *   vd->list_stream_formats is not null
 *   fmtind is valid
 *   vd->list_stream_formats->list_stream_cap is not null
 *   fsizeind is valid
 *
 * returns 0 if enumeration succeded or errno otherwise
 */
static int enum_frame_intervals(v4l2_dev_t *vd,
		uint32_t pixfmt, uint32_t width, uint32_t height,
		int fmtind, int fsizeind)
{
	/*assertions*/
	assert(vd != NULL);
	assert(vd->fd > 0);
	assert(vd->list_stream_formats != NULL);
	assert(vd->numb_formats >= fmtind);
	assert(vd->list_stream_formats->list_stream_cap != NULL);
	assert(vd->list_stream_formats[fmtind-1].numb_res >= fsizeind);

	int ret=0;
	struct v4l2_frmivalenum fival;
	int list_fps=0;
	memset(&fival, 0, sizeof(fival));
	fival.index = 0;
	fival.pixel_format = pixfmt;
	fival.width = width;
	fival.height = height;

	vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_num = NULL;
	vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_denom = NULL;

	if(verbosity > 0)
		printf("\tTime interval between frame: ");
	while ((ret = xioctl(vd->fd, VIDIOC_ENUM_FRAMEINTERVALS, &fival)) == 0)
	{
		fival.index++;
		if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
		{
			if(verbosity > 0)
				printf("%u/%u, ", fival.discrete.numerator, fival.discrete.denominator);

			list_fps++;
			vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_num = realloc(
				vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_num,
				sizeof(int) * list_fps);
			if(vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_num == NULL)
			{
				fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (enum_frame_intervals): %s\n", strerror(errno));
				exit(-1);
			}
			vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_denom = realloc(
				vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_denom,
				sizeof(int) * list_fps);
			if(vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_denom == NULL)
			{
				fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (enum_frame_intervals): %s\n", strerror(errno));
				exit(-1);
			}

			vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_num[list_fps-1] = fival.discrete.numerator;
			vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_denom[list_fps-1] = fival.discrete.denominator;
		}
		else if (fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS)
		{
			if(verbosity > 0)
				printf("{min { %u/%u } .. max { %u/%u } }, ",
					fival.stepwise.min.numerator, fival.stepwise.min.numerator,
					fival.stepwise.max.denominator, fival.stepwise.max.denominator);
			break;
		}
		else if (fival.type == V4L2_FRMIVAL_TYPE_STEPWISE)
		{
			if(verbosity > 0)
				printf("{min { %u/%u } .. max { %u/%u } / "
					"stepsize { %u/%u } }, ",
					fival.stepwise.min.numerator, fival.stepwise.min.denominator,
					fival.stepwise.max.numerator, fival.stepwise.max.denominator,
					fival.stepwise.step.numerator, fival.stepwise.step.denominator);
			break;
		}
	}

	if (list_fps==0)
	{
		vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].numb_frates = 1;
		vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_num = realloc(
				vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_num,
				sizeof(int));
		if(vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_num == NULL)
		{
			fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (enum_frame_intervals): %s\n", strerror(errno));
			exit(-1);
		}
		vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_denom = realloc(
				vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_denom,
				sizeof(int));
		if(vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_denom == NULL)
		{
			fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (enum_frame_intervals): %s\n", strerror(errno));
			exit(-1);
		}

		vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_num[0] = 1;
		vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].framerate_denom[0] = 1;
	}
	else
		vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].numb_frates = list_fps;

	if(verbosity > 0)
		printf("\n");
	if (ret != 0 && errno != EINVAL)
	{
		fprintf(stderr, "V4L2_CORE: (VIDIOC_ENUM_FRAMEINTERVALS) Error enumerating frame intervals\n");
		return errno;
	}
	return 0;
}

/*
 * enumerate frame sizes (width and height)
 * args:
 *   vd - pointer to video device data
 *   pixfmt - v4l2 pixel format that we want to list frame sizes for
 *   fmtind - current index of format list
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid ( > 0 )
 *   vd->list_stream_formats is not null
 *   fmtind is valid
 *
 * returns 0 if enumeration succeded or errno otherwise
 */
static int enum_frame_sizes(v4l2_dev_t *vd, uint32_t pixfmt, int fmtind)
{
	/*assertions*/
	assert(vd != NULL);
	assert(vd->fd > 0);
	assert(vd->list_stream_formats != NULL);
	assert(vd->numb_formats >= fmtind);

	int ret=0;
	int fsizeind=0; /*index for supported sizes*/
	vd->list_stream_formats[fmtind-1].list_stream_cap = NULL;
	struct v4l2_frmsizeenum fsize;

	memset(&fsize, 0, sizeof(fsize));
	fsize.index = 0;
	fsize.pixel_format = pixfmt;

	while ((ret = xioctl(vd->fd, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0)
	{
		fsize.index++;
		if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
		{
			if(verbosity > 0)
				printf("{ discrete: width = %u, height = %u }\n",
					fsize.discrete.width, fsize.discrete.height);

			fsizeind++;
			vd->list_stream_formats[fmtind-1].list_stream_cap = realloc(
				vd->list_stream_formats[fmtind-1].list_stream_cap,
				fsizeind * sizeof(v4l2_stream_cap_t));

			assert(vd->list_stream_formats[fmtind-1].list_stream_cap != NULL);

			vd->list_stream_formats[fmtind-1].numb_res = fsizeind;

			vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].width = fsize.discrete.width;
			vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].height = fsize.discrete.height;

			ret = enum_frame_intervals(vd,
				pixfmt,
				fsize.discrete.width,
				fsize.discrete.height,
				fmtind,
				fsizeind);

			if (ret != 0)
				fprintf(stderr, "V4L2_CORE:  Unable to enumerate frame sizes %s\n", strerror(ret));
		}
		else if (fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS || fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE)
		{
			if(verbosity > 0)
			{
				if(fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS)
					printf("{ continuous: min { width = %u, height = %u } .. "
						"max { width = %u, height = %u } }\n",
						fsize.stepwise.min_width, fsize.stepwise.min_height,
						fsize.stepwise.max_width, fsize.stepwise.max_height);
				else
					printf("{ stepwise: min { width = %u, height = %u } .. "
						"max { width = %u, height = %u } / "
						"stepsize { width = %u, height = %u } }\n",
						fsize.stepwise.min_width, fsize.stepwise.min_height,
						fsize.stepwise.max_width, fsize.stepwise.max_height,
						fsize.stepwise.step_width, fsize.stepwise.step_height);
			}
				
			/*add at least min and max values*/
			fsizeind++; /*min*/
				
			vd->list_stream_formats[fmtind-1].list_stream_cap = realloc(
				vd->list_stream_formats[fmtind-1].list_stream_cap,
				fsizeind * sizeof(v4l2_stream_cap_t));

			assert(vd->list_stream_formats[fmtind-1].list_stream_cap != NULL);
				
			vd->list_stream_formats[fmtind-1].numb_res = fsizeind;

			vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].width = fsize.stepwise.min_width;
			vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].height = fsize.stepwise.min_height;

			ret = enum_frame_intervals(vd,
				pixfmt,
				fsize.stepwise.min_width,
				fsize.stepwise.min_height,
				fmtind,
				fsizeind);

			if (ret != 0)
				fprintf(stderr, "V4L2_CORE:  Unable to enumerate frame sizes %s\n", strerror(ret));
					
				
			fsizeind++; /*max*/
				
			vd->list_stream_formats[fmtind-1].list_stream_cap = realloc(
				vd->list_stream_formats[fmtind-1].list_stream_cap,
				fsizeind * sizeof(v4l2_stream_cap_t));

			assert(vd->list_stream_formats[fmtind-1].list_stream_cap != NULL);
				
			vd->list_stream_formats[fmtind-1].numb_res = fsizeind;

			vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].width = fsize.stepwise.max_width;
			vd->list_stream_formats[fmtind-1].list_stream_cap[fsizeind-1].height = fsize.stepwise.max_height;

			ret = enum_frame_intervals(vd,
				pixfmt,
				fsize.stepwise.max_width,
				fsize.stepwise.max_height,
				fmtind,
				fsizeind);

			if (ret != 0)
				fprintf(stderr, "V4L2_CORE:  Unable to enumerate frame sizes %s\n", strerror(ret));	
				
		}
		else
		{
			fprintf(stderr, "V4L2_CORE: fsize.type not supported: %d\n", fsize.type);
			fprintf(stderr, "    (Discrete: %d   Continuous: %d  Stepwise: %d)\n",
				V4L2_FRMSIZE_TYPE_DISCRETE,
				V4L2_FRMSIZE_TYPE_CONTINUOUS,
				V4L2_FRMSIZE_TYPE_STEPWISE);
		}
	}

	if (ret != 0 && errno != EINVAL)
	{
		fprintf(stderr, "V4L2_CORE: (VIDIOC_ENUM_FRAMESIZES) - Error enumerating frame sizes\n");
		return errno;
	}
	else if ((ret != 0) && (fsizeind == 0))
	{
		/* ------ some drivers don't enumerate frame sizes ------ */
		/*         negotiate with VIDIOC_TRY_FMT instead          */

		/*if fsizeind = 0 list_stream_cap shouldn't have been allocated*/
		assert(vd->list_stream_formats[fmtind-1].list_stream_cap == NULL);

		fsizeind++;
		struct v4l2_format fmt;
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width = vd->format.fmt.pix.width; /* check defined size*/
		fmt.fmt.pix.height = vd->format.fmt.pix.height;
		fmt.fmt.pix.pixelformat = pixfmt;
		fmt.fmt.pix.field = V4L2_FIELD_ANY;

		xioctl(vd->fd, VIDIOC_TRY_FMT, &fmt);

		/*use the returned values*/
		int width = fmt.fmt.pix.width;
		int height = fmt.fmt.pix.height;

		if(width <= 0 || height <= 0)
		{
			printf("{ VIDIOC_TRY_FMT (invalid values): width = %u, height = %u }\n",
				vd->format.fmt.pix.width,
				vd->format.fmt.pix.height);
			return EINVAL;
		}
		
		if(verbosity > 0)
		{
			printf("{ VIDIOC_TRY_FMT : width = %u, height = %u }\n",
				vd->format.fmt.pix.width,
				vd->format.fmt.pix.height);
			printf("fmtind:%i fsizeind: %i\n", fmtind, fsizeind);
		}

		vd->list_stream_formats[fmtind-1].list_stream_cap = realloc(
			vd->list_stream_formats[fmtind-1].list_stream_cap,
			sizeof(v4l2_stream_cap_t) * fsizeind);

		assert(vd->list_stream_formats[fmtind-1].list_stream_cap != NULL);

		vd->list_stream_formats[fmtind-1].numb_res=fsizeind;

		/*don't enumerateintervals, use a default of 1/25 fps instead*/
		vd->list_stream_formats[fmtind-1].list_stream_cap[0].framerate_num = NULL;
		vd->list_stream_formats[fmtind-1].list_stream_cap[0].framerate_num = realloc(
			vd->list_stream_formats[fmtind-1].list_stream_cap[0].framerate_num,
			sizeof(int));
		if(vd->list_stream_formats[fmtind-1].list_stream_cap[0].framerate_num == NULL)
		{
			fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (enum_frame_intervals): %s\n", strerror(errno));
			exit(-1);
		}

		vd->list_stream_formats[fmtind-1].list_stream_cap[0].framerate_denom = NULL;
		vd->list_stream_formats[fmtind-1].list_stream_cap[0].framerate_denom = realloc(
			vd->list_stream_formats[fmtind-1].list_stream_cap[0].framerate_denom,
			sizeof(int));
		if(vd->list_stream_formats[fmtind-1].list_stream_cap[0].framerate_denom == NULL)
		{
			fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (enum_frame_intervals): %s\n", strerror(errno));
			exit(-1);
		}

		vd->list_stream_formats[fmtind-1].list_stream_cap[0].width = width;
		vd->list_stream_formats[fmtind-1].list_stream_cap[0].height = height;
		vd->list_stream_formats[fmtind-1].list_stream_cap[0].framerate_num[0] = 1;
		vd->list_stream_formats[fmtind-1].list_stream_cap[0].framerate_denom[0] = 25;
		vd->list_stream_formats[fmtind-1].list_stream_cap[0].numb_frates = 1;
	}

	return 0;
}

/*
 * enumerate frame formats (pixelformats, resolutions and fps)
 * and creates list in vd->list_stream_formats
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid ( > 0 )
 *   vd->list_stream_formats is null
 *
 * returns: 0 (E_OK) if enumeration succeded or error otherwise
 */
int enum_frame_formats(v4l2_dev_t *vd)
{
	/*assertions*/
	assert(vd != NULL);
	assert(vd->fd > 0);
	assert(vd->list_stream_formats == NULL);

	int ret=E_OK;

	int fmtind=0;
	int valid_formats=0; /*number of valid formats found (with valid frame sizes)*/
	struct v4l2_fmtdesc fmt;
	memset(&fmt, 0, sizeof(fmt));
	fmt.index = 0;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	vd->list_stream_formats = calloc ( 1, sizeof(v4l2_stream_formats_t));
	if(vd->list_stream_formats == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (enum_frame_formats): %s\n", strerror(errno));
		exit(-1);
	}
	vd->list_stream_formats[0].list_stream_cap = NULL;

	while ((ret = xioctl(vd->fd, VIDIOC_ENUM_FMT, &fmt)) == 0)
	{
		uint8_t dec_support = can_decode_format(fmt.pixelformat);

		uint32_t pix_format = fmt.pixelformat;

		fmt.index++;
		if(verbosity > 0)
		{
            if((fmt.pixelformat & (1U<<31)) != 0)
			{
                pix_format &= ~(1U<<31);//need to fix fourcc string
				printf("{ pixelformat = '%c%c%c%c'(BE), description = '%s' }\n",
					pix_format & 0xFF, (pix_format >> 8) & 0xFF,
					(pix_format >> 16) & 0xFF, (pix_format >> 24) & 0xFF,
					fmt.description);
			}
			else
				printf("{ pixelformat = '%c%c%c%c', description = '%s' }\n",
					fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,
					(fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
					fmt.description);

		}

		if(!dec_support)
			printf("    - FORMAT NOT SUPPORTED BY DECODER -\n");

		fmtind++;

		vd->list_stream_formats = realloc(
			vd->list_stream_formats,
			fmtind * sizeof(v4l2_stream_formats_t));

		assert(vd->list_stream_formats != NULL);

		vd->numb_formats = fmtind;

		vd->list_stream_formats[fmtind-1].dec_support = dec_support;
		vd->list_stream_formats[fmtind-1].format = fmt.pixelformat;
        if((fmt.pixelformat & (1U<<31)) != 0) //be format flag
            pix_format &= ~(1U<<31); //need to fix fourcc string
		snprintf(vd->list_stream_formats[fmtind-1].fourcc, 5, "%c%c%c%c",
				pix_format & 0xFF, (pix_format >> 8) & 0xFF,
				(pix_format >> 16) & 0xFF, (pix_format >> 24) & 0xFF);
		strncpy(vd->list_stream_formats[fmtind-1].description, (char *) fmt.description, 31);
		//enumerate frame sizes
		ret = enum_frame_sizes(vd, fmt.pixelformat, fmtind);
		if (ret != 0)
			fprintf( stderr, "v4L2_CORE: Unable to enumerate frame sizes :%s\n", strerror(ret));
		
		if(dec_support && !ret)
			valid_formats++; /*the format can be decoded and it has valid frame sizes*/
	}

	if (errno != EINVAL)
		fprintf( stderr, "v4L2_CORE: (VIDIOC_ENUM_FMT) - Error enumerating frame formats: %s\n", strerror(errno));

	if(valid_formats > 0)
		return E_OK;
	else
		return E_DEVICE_ERR;
}

/* get frame format index from format list
 * args:
 *   vd - pointer to video device data
 *   format - v4l2 pixel format
 *
 * asserts:
 *   vd is not null
 *   vd->list_stream_formats is not null
 *
 * returns: format list index or -1 if not available
 */
int get_frame_format_index(v4l2_dev_t *vd, int format)
{
	/*asserts*/
	assert(vd != NULL);
	assert(vd->list_stream_formats != NULL);

	int i=0;
	for(i=0; i<vd->numb_formats; i++)
	{
        if(verbosity > 0)
            printf("V4L2_CORE: requested format(%x)  [%i] -> %x\n",
            format, i, vd->list_stream_formats[i].format);
		if(format == vd->list_stream_formats[i].format)
			return (i);
	}
	return (-1);
}

/* get resolution index for format index from format list
 * args:
 *   vd - pointer to video device data
 *   format - format index from format list
 *   width - requested width
 *   height - requested height
 *
 * asserts:
 *   vd is not null
 *   vd->list_stream_formats is not null
 *
 * returns: resolution list index for format index or -1 if not available
 */
int get_format_resolution_index(v4l2_dev_t *vd, int format, int width, int height)
{
	/*asserts*/
	assert(vd != NULL);
	assert(vd->list_stream_formats != NULL);

	if(format >= vd->numb_formats || format < 0)
	{
		fprintf(stderr, "V4L2_CORE: [get resolution index] format index (%i) is not valid [0 - %i]\n",
			format, vd->numb_formats - 1);
		return (-1);
	}

	int i=0;
	for(i=0; i < vd->list_stream_formats[format].numb_res; i++)
	{
		if( width == vd->list_stream_formats[format].list_stream_cap[i].width &&
		    height == vd->list_stream_formats[format].list_stream_cap[i].height)
			return (i);
	}
    int tempwidth = 0;
    int tempheight = 0;
    int index = -1;
    for(i=0; i < vd->list_stream_formats[format].numb_res; i++)
    {
        if( tempwidth <= vd->list_stream_formats[format].list_stream_cap[i].width &&
            tempheight <= vd->list_stream_formats[format].list_stream_cap[i].height)
        {
            tempwidth = vd->list_stream_formats[format].list_stream_cap[i].width;
            tempheight = vd->list_stream_formats[format].list_stream_cap[i].height;
            index = i;
        }
    }
    return (index);
}

/*
 * free frame formats list
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *   vd->list_stream_formats is not null
 *
 * returns: void
 */
void free_frame_formats(v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);
	assert(vd->list_stream_formats != NULL);

	int i=0;
	int j=0;
	for(i=0; i < vd->numb_formats; i++)
	{
		if(vd->list_stream_formats[i].list_stream_cap != NULL)
		{
			for(j=0; j < vd->list_stream_formats[i].numb_res;j++)
			{
				if(vd->list_stream_formats[i].list_stream_cap[j].framerate_num != NULL)
					free(vd->list_stream_formats[i].list_stream_cap[j].framerate_num);

				if(vd->list_stream_formats[i].list_stream_cap[j].framerate_denom != NULL)
					free(vd->list_stream_formats[i].list_stream_cap[j].framerate_denom);
			}
			free(vd->list_stream_formats[i].list_stream_cap);
		}
	}
	free(vd->list_stream_formats);
	vd->list_stream_formats = NULL;
}
