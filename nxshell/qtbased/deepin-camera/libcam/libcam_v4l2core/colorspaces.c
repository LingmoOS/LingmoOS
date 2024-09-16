/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
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
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "gview.h"
#include "cameraconfig.h"

extern int verbosity;

/*------------------------------- Color space conversions --------------------*/

/*---------------- raw bayer -------------*/

/* raw bayer functions
 *  from libv4l bayer.c, (C) 2008 Hans de Goede <j.w.r.degoede@hhs.nl>
 *  Note: original bayer_to_bgr24 code from :
 *     1394-Based Digital Camera Control Library
 *
 * Bayer pattern decoding functions
 *  Written by Damien Douxchamps and Frederic Devernay
 *
 */
static void convert_border_bayer_line_to_bgr24( uint8_t* bayer, uint8_t* adjacent_bayer,
	uint8_t *bgr, int width, uint8_t start_with_green, uint8_t blue_line)
{
    //LMH0612之前是强制类型转换int到uint8_t,这里初始化就为uint8_t
    uint8_t t0, t1;

	if (start_with_green)
	{
	/* First pixel */
		if (blue_line)
		{
			*bgr++ = bayer[1];
			*bgr++ = bayer[0];
			*bgr++ = adjacent_bayer[0];
		}
		else
		{
			*bgr++ = adjacent_bayer[0];
			*bgr++ = bayer[0];
			*bgr++ = bayer[1];
		}
		/* Second pixel */
		t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
		t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
		if (blue_line)
		{
			*bgr++ = bayer[1];
			*bgr++ = t0;
			*bgr++ = t1;
		}
		else
		{
			*bgr++ = t1;
			*bgr++ = t0;
			*bgr++ = bayer[1];
		}
		bayer++;
		adjacent_bayer++;
		width -= 2;
	}
	else
	{
		/* First pixel */
		t0 = (bayer[1] + adjacent_bayer[0] + 1) >> 1;
		if (blue_line)
		{
			*bgr++ = bayer[0];
			*bgr++ = t0;
			*bgr++ = adjacent_bayer[1];
		}
		else
		{
			*bgr++ = adjacent_bayer[1];
			*bgr++ = t0;
			*bgr++ = bayer[0];
		}
		width--;
	}

	if (blue_line)
	{
		for ( ; width > 2; width -= 2)
		{
			t0 = (bayer[0] + bayer[2] + 1) >> 1;
			*bgr++ = t0;
			*bgr++ = bayer[1];
			*bgr++ = adjacent_bayer[1];
			bayer++;
			adjacent_bayer++;

			t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
			t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
			*bgr++ = bayer[1];
			*bgr++ = t0;
			*bgr++ = t1;
			bayer++;
			adjacent_bayer++;
		}
	}
	else
	{
		for ( ; width > 2; width -= 2)
		{
			t0 = (bayer[0] + bayer[2] + 1) >> 1;
			*bgr++ = adjacent_bayer[1];
			*bgr++ = bayer[1];
			*bgr++ = t0;
			bayer++;
			adjacent_bayer++;

			t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
			t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
			*bgr++ = t1;
			*bgr++ = t0;
			*bgr++ = bayer[1];
			bayer++;
			adjacent_bayer++;
		}
	}

	if (width == 2)
	{
		/* Second to last pixel */
		t0 = (bayer[0] + bayer[2] + 1) >> 1;
		if (blue_line)
		{
			*bgr++ = t0;
			*bgr++ = bayer[1];
			*bgr++ = adjacent_bayer[1];
		}
		else
		{
			*bgr++ = adjacent_bayer[1];
			*bgr++ = bayer[1];
			*bgr++ = t0;
		}
		/* Last pixel */
		t0 = (bayer[1] + adjacent_bayer[2] + 1) >> 1;
		if (blue_line)
		{
			*bgr++ = bayer[2];
			*bgr++ = t0;
			*bgr++ = adjacent_bayer[1];
		}
		else
		{
			*bgr++ = adjacent_bayer[1];
			*bgr++ = t0;
			*bgr++ = bayer[2];
		}
	}
	else
	{
		/* Last pixel */
		if (blue_line)
		{
			*bgr++ = bayer[0];
			*bgr++ = bayer[1];
			*bgr++ = adjacent_bayer[1];
		}
		else
		{
			*bgr++ = adjacent_bayer[1];
			*bgr++ = bayer[1];
			*bgr++ = bayer[0];
		}
	}
}

/*
 * From libdc1394, which on turn was based on OpenCV's Bayer decoding
 */
static void bayer_to_rgbbgr24(uint8_t *bayer,
	uint8_t *bgr, int width, int height,
	uint8_t start_with_green, uint8_t blue_line)
{
	/* render the first line */
	convert_border_bayer_line_to_bgr24(bayer, bayer + width, bgr, width,
		start_with_green, blue_line);
	bgr += width * 3;

	/* reduce height by 2 because of the special case top/bottom line */
	for (height -= 2; height; height--)
	{
        //LMH0612之前是强制类型转换int到uint8_t,这里初始化就为uint8_t
        uint8_t t0, t1;
		/* (width - 2) because of the border */
		uint8_t *bayerEnd = bayer + (width - 2);

		if (start_with_green)
		{
			/* OpenCV has a bug in the next line, which was
			t0 = (bayer[0] + bayer[width * 2] + 1) >> 1; */
			t0 = (bayer[1] + bayer[width * 2 + 1] + 1) >> 1;
			/* Write first pixel */
			t1 = (bayer[0] + bayer[width * 2] + bayer[width + 1] + 1) / 3;
			if (blue_line)
			{
				*bgr++ = t0;
				*bgr++ = t1;
				*bgr++ = bayer[width];
			}
			else
			{
				*bgr++ = bayer[width];
				*bgr++ = t1;
				*bgr++ = t0;
			}

			/* Write second pixel */
			t1 = (bayer[width] + bayer[width + 2] + 1) >> 1;
			if (blue_line)
			{
				*bgr++ = t0;
				*bgr++ = bayer[width + 1];
				*bgr++ = t1;
			}
			else
			{
				*bgr++ = t1;
				*bgr++ = bayer[width + 1];
				*bgr++ = t0;
			}
			bayer++;
		}
		else
		{
			/* Write first pixel */
			t0 = (bayer[0] + bayer[width * 2] + 1) >> 1;
			if (blue_line)
			{
				*bgr++ = t0;
				*bgr++ = bayer[width];
				*bgr++ = bayer[width + 1];
			}
			else
			{
				*bgr++ = bayer[width + 1];
				*bgr++ = bayer[width];
				*bgr++ = t0;
			}
		}

		if (blue_line)
		{
			for (; bayer <= bayerEnd - 2; bayer += 2)
			{
				t0 = (bayer[0] + bayer[2] + bayer[width * 2] +
					bayer[width * 2 + 2] + 2) >> 2;
				t1 = (bayer[1] + bayer[width] +
					bayer[width + 2] + bayer[width * 2 + 1] +
					2) >> 2;
				*bgr++ = t0;
				*bgr++ = t1;
				*bgr++ = bayer[width + 1];

				t0 = (bayer[2] + bayer[width * 2 + 2] + 1) >> 1;
				t1 = (bayer[width + 1] + bayer[width + 3] +
					1) >> 1;
				*bgr++ = t0;
				*bgr++ = bayer[width + 2];
				*bgr++ = t1;
			}
		}
		else
		{
			for (; bayer <= bayerEnd - 2; bayer += 2)
			{
				t0 = (bayer[0] + bayer[2] + bayer[width * 2] +
					bayer[width * 2 + 2] + 2) >> 2;
				t1 = (bayer[1] + bayer[width] +
					bayer[width + 2] + bayer[width * 2 + 1] +
					2) >> 2;
				*bgr++ = bayer[width + 1];
				*bgr++ = t1;
				*bgr++ = t0;

				t0 = (bayer[2] + bayer[width * 2 + 2] + 1) >> 1;
				t1 = (bayer[width + 1] + bayer[width + 3] +
					1) >> 1;
				*bgr++ = t1;
				*bgr++ = bayer[width + 2];
				*bgr++ = t0;
			}
		}

		if (bayer < bayerEnd)
		{
			/* write second to last pixel */
			t0 = (bayer[0] + bayer[2] + bayer[width * 2] +
				bayer[width * 2 + 2] + 2) >> 2;
			t1 = (bayer[1] + bayer[width] +
				bayer[width + 2] + bayer[width * 2 + 1] +
				2) >> 2;
			if (blue_line)
			{
				*bgr++ = t0;
				*bgr++ = t1;
				*bgr++ = bayer[width + 1];
			}
			else
			{
				*bgr++ = bayer[width + 1];
				*bgr++ = t1;
				*bgr++ = t0;
			}
			/* write last pixel */
			t0 = (bayer[2] + bayer[width * 2 + 2] + 1) >> 1;
			if (blue_line)
			{
				*bgr++ = t0;
				*bgr++ = bayer[width + 2];
				*bgr++ = bayer[width + 1];
			}
			else
			{
				*bgr++ = bayer[width + 1];
				*bgr++ = bayer[width + 2];
				*bgr++ = t0;
			}
			bayer++;
		}
		else
		{
			/* write last pixel */
			t0 = (bayer[0] + bayer[width * 2] + 1) >> 1;
			t1 = (bayer[1] + bayer[width * 2 + 1] + bayer[width] + 1) / 3;
			if (blue_line)
			{
				*bgr++ = t0;
				*bgr++ = t1;
				*bgr++ = bayer[width + 1];
			}
			else
			{
				*bgr++ = bayer[width + 1];
				*bgr++ = t1;
				*bgr++ = t0;
			}
		}

		/* skip 2 border pixels */
		bayer += 2;

		blue_line = !blue_line;
		start_with_green = !start_with_green;
	}

	/* render the last line */
	convert_border_bayer_line_to_bgr24(bayer + width, bayer, bgr, width,
		!start_with_green, !blue_line);
}

/*
 * convert bayer raw data to rgb24
 * args:
 *   pBay: pointer to buffer containing Raw bayer data
 *   pRGB24: pointer to buffer containing rgb24 data
 *   width: picture width
 *   height: picture height
 *   pix_order: bayer pixel order (0=gb/rg   1=gr/bg  2=bg/gr  3=rg/bg)
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void bayer_to_rgb24(uint8_t *pBay, uint8_t *pRGB24, int width, int height, int pix_order)
{
	switch (pix_order)
	{
		//conversion functions are build for bgr, by switching b and r lines we get rgb
		case 0: /* gbgbgb... | rgrgrg... (V4L2_PIX_FMT_SGBRG8)*/
			bayer_to_rgbbgr24(pBay, pRGB24, width, height, TRUE, FALSE);
			break;

		case 1: /* grgrgr... | bgbgbg... (V4L2_PIX_FMT_SGRBG8)*/
			bayer_to_rgbbgr24(pBay, pRGB24, width, height, TRUE, TRUE);
			break;

		case 2: /* bgbgbg... | grgrgr... (V4L2_PIX_FMT_SBGGR8)*/
			bayer_to_rgbbgr24(pBay, pRGB24, width, height, FALSE, FALSE);
			break;

		case 3: /* rgrgrg... ! gbgbgb... (V4L2_PIX_FMT_SRGGB8)*/
			bayer_to_rgbbgr24(pBay, pRGB24, width, height, FALSE, TRUE);
			break;

		default: /* default is 0*/
			bayer_to_rgbbgr24(pBay, pRGB24, width, height, TRUE, FALSE);
			break;
	}
}

/*------------------ YU12 ----------------------*/

/*
 *convert from packed 422 yuv (yuyv) to 420 planar (yu12)
 * args:
 *    out - pointer to output yu12 planar data buffer
 *    in - pointer to input yuyv packed data buffer
 *    width - frame width
 *    height - frame height
 *
 * asserts:
 *    in is not null
 *    out is not null
 *
 * returns: none
 */
void yuyv_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	int w = 0, h = 0;

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in1 + (width * 2); //second line in yuyv buffer

	uint8_t *py1 = out; // first line
	uint8_t *py2 = py1 + width; //second line
	uint8_t *pu = py1 + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	for(h = 0; h < height; h+=2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 + width;
		for(w = 0; w < width; w+=2) //yuyv 2 bytes per sample
		{
			//printf("decoding: h:%i w:%i\n", h, w);
			*py1++ = *in1++;
			*py2++ = *in2++;
			*pu++ = ((*in1++) + (*in2++)) /2; //average u samples
			*py1++ = *in1++;
			*py2++ = *in2++;
			*pv++ = ((*in1++) + (*in2++)) /2; //average v samples
		}
		in1 = in2;
		py1 = py2;
	}

}

/*
 *convert from packed 422 yuv (yvyu) to 420 planar (yu12)
 * args:
 *    out - pointer to output yu12 planar data buffer
 *    in - pointer to input yvyu packed data buffer
 *    width - frame width
 *    height - frame height
 *
 * asserts:
 *    in is not null
 *    out is not null
 *
 * returns: none
 */
void yvyu_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	int w = 0, h = 0;

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in1 + (width * 2); //second line in yvyu buffer

	uint8_t *py1 = out; // first line
	uint8_t *py2 = py1 + width; //second line
	uint8_t *pu = py1 + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	for(h = 0; h < height; h+=2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 + width;
		for(w = 0; w < width; w+=2) //yuyv 2 bytes per sample
		{
			//printf("decoding: h:%i w:%i\n", h, w);
			*py1++ = *in1++;
			*py2++ = *in2++;
			*pv++ = ((*in1++) + (*in2++)) /2; //average v samples
			*py1++ = *in1++;
			*py2++ = *in2++;
			*pu++ = ((*in1++) + (*in2++)) /2; //average u samples
		}
		in1 = in2;
		py1 = py2;
	}
}

/*
 *convert from packed 422 yuv (uyvy) to 420 planar (yu12)
 * args:
 *    out - pointer to output yu12 planar data buffer
 *    in - pointer to input uyvy packed data buffer
 *    width - frame width
 *    height - frame height
 *
 * asserts:
 *    in is not null
 *    out is not null
 *
 * returns: none
 */
void uyvy_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	int w = 0, h = 0;

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in1 + (width * 2); //second line in yuyv buffer

	uint8_t *py1 = out; // first line
	uint8_t *py2 = py1 + width; //second line
	uint8_t *pu = py1 + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	for(h = 0; h < height; h+=2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 + width;
		for(w = 0; w < width; w+=2) //yuyv 2 bytes per sample
		{
			*pu++ = ((*in1++) + (*in2++)) /2; //average u samples
			*py1++ = *in1++;
			*py2++ = *in2++;
			*pv++ = ((*in1++) + (*in2++)) /2; //average v samples
			*py1++ = *in1++;
			*py2++ = *in2++;
		}
		in1 = in2;
		py1 = py2;
	}
}

/*
 *convert from packed 422 yuv (vyuy) to 420 planar (yu12)
 * args:
 *    out - pointer to output yu12 planar data buffer
 *    in - pointer to input vyuy packed data buffer
 *    width - frame width
 *    height - frame height
 *
 * asserts:
 *    in is not null
 *    out is not null
 *
 * returns: none
 */
void vyuy_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	int w = 0, h = 0;
	int y_sizeline = width;
    //LMH0612消除警告,该地方c_sizeline并未使用到
    //int c_sizeline = width/2;
	
	uint8_t *in1 = in; //first line
	uint8_t *in2 = in1 + (width * 2); //second line in yuyv buffer

	uint8_t *py1 = out; // first line
	uint8_t *py2 = py1 + y_sizeline; //second line
	uint8_t *pu = py1 + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	for(h = 0; h < height; h+=2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 +  width;
		for(w = 0; w < width; w+=2) //yuyv 2 bytes per sample
		{
			*pv++ = ((*in1++) + (*in2++)) /2; //average v samples
			*py1++ = *in1++;
			*py2++ = *in2++;
			*pu++ = ((*in1++) + (*in2++)) /2; //average u samples
			*py1++ = *in1++;
			*py2++ = *in2++;
		}
		in1 = in2;
		py1 = py2;
	}
}


/*
 *convert from 422 planar yuv to 420 planar (yu12)
 * args:
 *    out - pointer to output yu12 planar data buffer
 *    in - pointer to input 422 planar data buffer
 *    width - frame width
 *    height - frame height
 *
 * asserts:
 *    in is not null
 *    out is not null
 *
 * returns: none
 */
void yuv422p_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

    /*copy y data*/

    //LMH0612,强制类型转换警告消除
    memcpy(out, in, (unsigned long)width*(unsigned long)height);

	int w = 0, h = 0;
    //LMH0612后面并未使用到c_sizeline
    //int c_sizeline = width/2;
	
	uint8_t *pu = out + (width * height);
    uint8_t *inu1 = in + (width * height);
    uint8_t *inu2 = inu1 + (width/2);

	uint8_t *pv = pu + ((width * height) / 4);
    uint8_t *inv1 = inu1 + ((width * height) / 2);
    uint8_t *inv2 = inv1 + (width / 2);

	for(h = 0; h < height; h+=2)
	{
		inu2 = inu1 + (width / 2);
		inv2 = inv1 + (width / 2);
		for(w = 0; w < width/2; w++) 
		{
			*pu++ = ((*inu1++) + (*inu2++)) /2; //average u sample
			*pv++ = ((*inv1++) + (*inv2++)) /2; //average v samples
		}
        inu1 = inu2;
		inv1 = inv2;
	}

}

/*
 * convert yyuv (packed) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing yyuv packed data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void yyuv_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	int w = 0, h = 0;
	int y_sizeline = width;

    //LMH0612后面并未使用c_sizeline
   // int c_sizeline = width/2;
	
	uint8_t *in1 = in; //first line
	uint8_t *in2 = in1 + (width * 2); //second line in yyuv buffer

	uint8_t *py1 = out; // first line
	uint8_t *py2 = py1 + y_sizeline; //second line
	uint8_t *pu = py1 + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	for(h = 0; h < height; h+=2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 +  width;
		for(w = 0; w < width; w+=2) //yyuv 2 bytes per sample
		{
			*py1++ = *in1++;
			*py1++ = *in1++;
			*py2++ = *in2++;
			*py2++ = *in2++;
			*pu++ = ((*in1++) + (*in2++)) /2; //average v samples
			*pv++ = ((*in1++) + (*in2++)) /2; //average u samples
		}
		in1 = in2;
		py1 = py2;
	}
}

/*
 * convert y444 (packed) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing y444 (yuv-4-4-4) packed data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void y444_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	int w = 0, h = 0;
	
	uint8_t *in1 = in; //first line
	uint8_t *in2 = in1 + (width * 2); //second line

	uint8_t *py1 = out; // first line
	uint8_t *py2 = py1 + width; //second line
	uint8_t *pu = py1 + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	for(h = 0; h < height; h+=2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 +  width;
		for(w = 0; w < (width*2); w+=4)
		{
			uint8_t yuv10 = *in1++;
			uint8_t yuv11 = *in1++;
			uint8_t yuv12 = *in1++;
			uint8_t yuv13 = *in1++;

			uint8_t yuv20 = *in2++;
			uint8_t yuv21 = *in2++;
			uint8_t yuv22 = *in2++;
			uint8_t yuv23 = *in2++;

			*py1++ = (uint8_t) (yuv11 << 4) & 0xF0;
			*py1++ = (uint8_t) (yuv13 << 4) & 0xF0;
			*py2++ = (uint8_t) (yuv21 << 4) & 0xF0;
			*py2++ = (uint8_t) (yuv23 << 4) & 0xF0;

			uint8_t u10 = yuv10 & 0xF0;
			uint8_t u11 = yuv12 & 0xF0;
			uint8_t u1 = (u10 + u11) /2;
			uint8_t v10 = (yuv10 << 4) & 0xF0;
			uint8_t v11 = (yuv12 << 4) & 0xF0;
			uint8_t v1 = (v10 + v11) /2;

			uint8_t u20 = yuv20 & 0xF0;
			uint8_t u21 = yuv22 & 0xF0;
			uint8_t u2 = (u20 + u21) /2;
			uint8_t v20 = (yuv20 << 4) & 0xF0;
			uint8_t v21 = (yuv22 << 4) & 0xF0;
			uint8_t v2 = (v20 +v21) /2;

			*pu++ = (u1 + u2) /2;
			*pv++ = (v1 + v2) /2;
		}
		in1 = in2;
		py1 = py2;
	}
}

/*
 * convert yuvo (packed) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing yuvo (yuv-5-5-5) packed data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void yuvo_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	int w = 0, h = 0;
	
	uint8_t *in1 = in; //first line
	uint8_t *in2 = in1 + (width * 2); //second line

	uint8_t *py1 = out; // first line
	uint8_t *py2 = py1 + width; //second line
	uint8_t *pu = py1 + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	for(h = 0; h < height; h+=2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 +  width;
		for(w = 0; w < (width*2); w+=4)
		{
			uint8_t yuv10 = *in1++;
			uint8_t yuv11 = *in1++;
			uint8_t yuv12 = *in1++;
			uint8_t yuv13 = *in1++;

			uint8_t yuv20 = *in2++;
			uint8_t yuv21 = *in2++;
			uint8_t yuv22 = *in2++;
			uint8_t yuv23 = *in2++;

			*py1++ = (uint8_t) (yuv11 << 1) & 0xF8;
			*py1++ = (uint8_t) (yuv13 << 1) & 0xF8;
			*py2++ = (uint8_t) (yuv21 << 1) & 0xF8;
			*py2++ = (uint8_t) (yuv23 << 1) & 0xF8;

			uint8_t u10 = ((yuv10 >> 2) & 0x38) | ((yuv11 << 6) & 0xC0);
			uint8_t u11 = ((yuv12 >> 2) & 0x38) | ((yuv13 << 6) & 0xC0);
			uint8_t u1 = (u10 + u11) /2;
			uint8_t v10 = (yuv10 << 3) & 0xF8;
			uint8_t v11 = (yuv12 << 3) & 0xF8;
			uint8_t v1 = (v10 + v11) /2;

			uint8_t u20 = ((yuv20 >> 2) & 0x38) | ((yuv21 << 6) & 0xC0);
			uint8_t u21 = ((yuv22 >> 2) & 0x38) | ((yuv23 << 6) & 0xC0);
			uint8_t u2 = (u20 + u21) /2;
			uint8_t v20 = (yuv20 << 3) & 0xF8;
			uint8_t v21 = (yuv22 << 3) & 0xF8;
			uint8_t v2 = (v20 +v21) /2;

			*pu++ = (u1 + u2) /2;
			*pv++ = (v1 + v2) /2;
		}
		in1 = in2;
		py1 = py2;
	}
}

/*
 * convert yuvp (packed) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing yuvp (yuv-5-6-5) packed data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void yuvp_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	int w = 0, h = 0;
	
	uint8_t *in1 = in; //first line
	uint8_t *in2 = in1 + (width * 2); //second line

	uint8_t *py1 = out; // first line
	uint8_t *py2 = py1 + width; //second line
	uint8_t *pu = py1 + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	for(h = 0; h < height; h+=2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 +  width;
		for(w = 0; w < (width*2); w+=4)
		{
			uint8_t yuv10 = *in1++;
			uint8_t yuv11 = *in1++;
			uint8_t yuv12 = *in1++;
			uint8_t yuv13 = *in1++;

			uint8_t yuv20 = *in2++;
			uint8_t yuv21 = *in2++;
			uint8_t yuv22 = *in2++;
			uint8_t yuv23 = *in2++;

			*py1++ = (uint8_t) yuv11 & 0xF8;
			*py1++ = (uint8_t) yuv13 & 0xF8;
			*py2++ = (uint8_t) yuv21 & 0xF8;
			*py2++ = (uint8_t) yuv23 & 0xF8;

			uint8_t u10 = ((yuv10 >> 3) & 0x1C) | ((yuv11 << 5) & 0xE0);
			uint8_t u11 = ((yuv12 >> 3) & 0x1C) | ((yuv13 << 5) & 0xE0);
			uint8_t u1 = (u10 + u11) /2;
			uint8_t v10 = (yuv10 << 3) & 0xF8;
			uint8_t v11 = (yuv12 << 3) & 0xF8;
			uint8_t v1 = (v10 + v11) /2;

			uint8_t u20 = ((yuv20 >> 3) & 0x1C) | ((yuv21 << 5) & 0xE0);
			uint8_t u21 = ((yuv22 >> 3) & 0x1C) | ((yuv23 << 5) & 0xE0);
			uint8_t u2 = (u20 + u21) /2;
			uint8_t v20 = (yuv20 << 3) & 0xF8;
			uint8_t v21 = (yuv22 << 3) & 0xF8;
			uint8_t v2 = (v20 +v21) /2;

			*pu++ = (u1 + u2) /2;
			*pv++ = (v1 + v2) /2;
		}
		in1 = in2;
		py1 = py2;
	}
}

/*
 * convert yuv4 (packed) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing yuv4 (yuv32) packed data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void yuv4_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	int w = 0, h = 0;
	
	uint8_t *in1 = in; //first line
	uint8_t *in2 = in1 + (width * 4); //second line

	uint8_t *py1 = out; // first line
	uint8_t *py2 = py1 + width; //second line
	uint8_t *pu = py1 + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	for(h = 0; h < height; h+=2)
	{
		in2 = in1 + (width * 4);
		py2 = py1 +  width;
		for(w = 0; w < (width*4); w+=8)
		{
			in1++; //alpha 10
			*py1++ = *in1++; //y10
			uint8_t u10 = *in1++; //u10
			uint8_t v10 = *in1++; //v10
			in1++; //alpha 11
			*py1++ = *in1++; //y11
			uint8_t u11 = *in1++; //u11
			uint8_t v11 = *in1++; //v11

			in2++; //alpha 20
			*py2++ = *in2++; //y20
			uint8_t u20 = *in2++; //u20
			uint8_t v20 = *in2++; //v20
			in2++; //alpha 21
			*py2++ = *in2++; //y21
			uint8_t u21 = *in2++; //u21
			uint8_t v21 = *in2++; //v21

			uint8_t u1 = (u10 + u11) /2;
			uint8_t v1 = (v10 + v11) /2;
			uint8_t u2 = (u20 + u21) /2;
			uint8_t v2 = (v20 + v21) /2;

			*pu++ = (u1 + u2) /2;
			*pv++ = (v1 + v2) /2;
		}
		in1 = in2;
		py1 = py2;
	}
}

/*
 *convert from 420 planar (yv12) to 420 planar (yu12)
 * args:
 *    out - pointer to output yu12 planar data buffer
 *    in - pointer to input yv12 planar data buffer
 *    width - frame width
 *    height - frame height
 *
 * asserts:
 *    in is not null
 *    out is not null
 *
 * returns: none
 */
void yv12_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

    /*copy y data*/
    memcpy(out, in, width*height);
	/*copy u data*/
	memcpy(out+(width*height), in+((width * height * 5) / 4), width * height / 4);
	/*copy v data*/
	memcpy(out+((width * height * 5) / 4), in+(width * height), width * height / 4);
}

/*
 * convert nv12 planar (uv interleaved) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing nv12 planar data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void nv12_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	/*copy y data*/
    memcpy(out, in, width*height);
	
	uint8_t *puv = in + (width * height);
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	/*uv plane*/
	int i = 0;
	for(i=0; i< width * height /2; i+=2)
	{
		*pu++ = *puv++;
		*pv++ = *puv++;
	}
}

/*
 * convert nv21 planar (vu interleaved) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing nv21 planar data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void nv21_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	/*copy y data*/
    memcpy(out, in, width*height);
	
	uint8_t *puv = in + (width * height);
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	/*uv plane*/
	int i = 0;
	for(i=0; i< width * height /2; i+=2)
	{
		*pv++ = *puv++;
		*pu++ = *puv++;
	}
}

/*
 * convert yuv 422 planar (uv interleaved) (nv16) to yuv420 planar (yu12)
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing yuv422 (nv16) planar data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void nv16_to_yu12 (uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	/*copy y data*/
    memcpy(out, in, width*height);

	//uv plane
	uint8_t *puv1 = in + (width * height); //first line
	uint8_t *puv2 = puv1 + width; //second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	int h = 0;
	int w = 0;
	for(h=0; h < height; h+=2)
	{
		puv2 = puv1 + width;
		for(w=0; w < width; w+=2)
		{
			*pu++ = ((*puv1++) + (*puv2++)) / 2; //average
			*pv++ = ((*puv1++) + (*puv2++)) / 2; //average
		}
		puv1 = puv2;
	}
}

/*
 * convert yuv 422 planar (vu interleaved) (nv61) to yuv420 planar (yu12)
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing yuv422 (nv61) planar data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void nv61_to_yu12 (uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	/*copy y data*/
    memcpy(out, in, width*height);
	
	/*uv plane*/
	uint8_t *puv1 = in + (width * height); //first line
	uint8_t *puv2 = puv1 + width; //second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	int h = 0;
	int w = 0;
	for(h=0; h < height; h+=2)
	{
		puv2 = puv1 + width;
		for(w=0; w < width; w+=2)
		{
			*pv++ = ((*puv1++) + (*puv2++)) / 2; //average
			*pu++ = ((*puv1++) + (*puv2++)) / 2; //average
		}
		puv1 = puv2;
	}
}

/*
 * convert yuv444 planar (uv interleaved) (nv24) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing nv24 planar data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void nv24_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	/*copy y data*/
    memcpy(out, in, width*height);

	//uv plane
	uint8_t *puv1 = in + (width * height); //first line
	uint8_t *puv2 = puv1 + (width * 2); //second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	int h = 0;
	int w = 0;
	for(h=0; h < height; h+=2)
	{
		puv2 = puv1 + (width * 2);
		for(w=0; w < (width * 2); w+=4)
		{
			uint8_t u1 = ((*puv1++) + (*puv2++)) / 2;
			uint8_t v1 = ((*puv1++) + (*puv2++)) / 2;
			uint8_t u2 = ((*puv1++) + (*puv2++)) / 2;
			uint8_t v2 = ((*puv1++) + (*puv2++)) / 2;

			*pu++ = (u1 + u2)/2; //average
			*pv++ = (v1 + v2)/2; //average
		}
		puv1 = puv2;
	}
}

/*
 * convert yuv444 planar (uv interleaved) (nv42) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing nv42 planar data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void nv42_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);

	/*copy y data*/
    memcpy(out, in, width*height);

	//uv plane
	uint8_t *puv1 = in + (width * height); //first line
	uint8_t *puv2 = puv1 + (width * 2); //second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	int h = 0;
	int w = 0;
	for(h=0; h < height; h+=2)
	{
		puv2 = puv1 + (width * 2);
		for(w=0; w < (width * 2); w+=4)
		{
			uint8_t v1 = ((*puv1++) + (*puv2++)) / 2;
			uint8_t u1 = ((*puv1++) + (*puv2++)) / 2;
			uint8_t v2 = ((*puv1++) + (*puv2++)) / 2;
			uint8_t u2 = ((*puv1++) + (*puv2++)) / 2;

			*pu++ = (u1 + u2)/2; //average
			*pv++ = (v1 + v2)/2; //average
		}
		puv1 = puv2;
	}
}

/*
 * Unpack buffer of (vw bit) data into padded 16bit buffer.
 * args:
 *    raw - pointer to input raw packed data buffer
 *    unpacked - pointer to unpacked output data buffer
 *    vw - vw bit
 *    unpacked_len - length
 *
 * asserts:
 *    none
 *
 * returns: none
 */
static inline void convert_packed_to_16bit(uint8_t *raw, uint16_t *unpacked, int vw, int unpacked_len)
{
	int mask = (1 << vw) - 1;
	uint32_t buffer = 0;
	int bitsIn = 0;
	while (unpacked_len--) {
		while (bitsIn < vw) {
			buffer = (buffer << 8) | *(raw++);
			bitsIn += 8;
		}
		bitsIn -= vw;
		*(unpacked++) = (buffer >> bitsIn) & mask;
	}
}

/*
 * convert y10b (bit-packed array greyscale format) to yu12
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing y10b (bit-packed array) data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void y10b_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);
	
	uint16_t *unpacked_buffer = NULL;
	uint16_t *ptmp;
	uint8_t *py = out;
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);
	
	int h = 0;

	unpacked_buffer = malloc(width * height * sizeof(uint16_t));
	
	if (unpacked_buffer == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (y10b_to_yu12): %s\n", strerror(errno));
		exit(-1);
	}
	
	convert_packed_to_16bit(in, unpacked_buffer, 10, width * height);

	ptmp = unpacked_buffer;

	for (h = 0; h < height; h++)
	{
		int w=0;
		for (w = 0; w < width; w++)
		{
			/* Y */
			*py++ = (uint8_t) ((*ptmp++ & 0x3FF) >> 2);
		}
	}
	
	for(h=0; h < (width * height / 4); h++)
	{
		/* U */
		*pu++ = 0x80;
		/* V */
		*pv++ = 0x80;
	}


	free(unpacked_buffer);
}

/*
 * convert yuv 411 packed (y41p) to planar yuv 420 (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing y41p data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void y41p_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);
	
	uint8_t *py1 = out;
	uint8_t *py2 = out + width;
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);
	
	int h=0;
	int linesize = width * 3 /2;

	
	for(h = 0; h < height; h += 2)
	{
		py1 = out + (h * width); // first line
		py2 = out + ((h + 1) * width); // second line
		int offset1 = linesize * h; //line 1
		int offset2 = linesize * (h + 1); //line 2
		int w = 0;
		for(w = 0; w < linesize; w += 12)
		{
			/* y first line */
			*py1++ = in[w + 1 + offset1]; //Y00
			*py1++ = in[w + 3 + offset1]; //Y01
			*py1++ = in[w + 5 + offset1]; //Y02
			*py1++ = in[w + 7 + offset1]; //Y03
			*py1++ = in[w + 8 + offset1]; //Y04
			*py1++ = in[w + 9 + offset1]; //Y05
			*py1++ = in[w + 10 + offset1]; //Y06
			*py1++ = in[w + 11 + offset1]; //Y07
			/* y second line */
			*py2++ = in[w + 1 + offset2]; //Y10
			*py2++ = in[w + 3 + offset2]; //Y11
			*py2++ = in[w + 5 + offset2]; //Y12
			*py2++ = in[w + 7 + offset2]; //Y13
			*py2++ = in[w + 8 + offset2]; //Y14
			*py2++ = in[w + 9 + offset2]; //Y15
			*py2++ = in[w + 10 + offset2]; //Y16
			*py2++ = in[w + 11 + offset2]; //Y17 
			
			/*U0 and U1 average first and second lines*/
			*pu++ = (in[w + offset1] + in[w + offset2]) / 2;         //U00 + U10 /2
			*pu++ = (in[w + offset1] + in[w + offset2]) / 2;         //U00 + U10 /2
			/*U2 and U3 average first and second lines*/
			*pu++ = (in[w + 4 + offset1] + in[w + 4 + offset2]) / 2; //U01 + U11 /2
			*pu++ = (in[w + 4 + offset1] + in[w + 4 + offset2]) / 2; //U01 + U11 /2
			/*V0 and V1 average first and second lines*/
			*pv++ = (in[w + 2 + offset1] + in[w + 2 + offset2]) / 2; //V00 + V10 /2
			*pv++ = (in[w + 2 + offset1] + in[w + 2 + offset2]) / 2; //V00 + V10 /2
			/*V2 and V3 average first and second lines*/
			*pv++ = (in[w + 6 + offset1] + in[w + 6 + offset2]) / 2; //V01 + V11 /2
			*pv++ = (in[w + 6 + offset1] + in[w + 6 + offset2]) / 2; //V01 + V11 /2	
		}
	}
}

/*
 * convert yuv mono (grey) to yuv 420 planar (yu12)
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing grey (y only) data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void grey_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);
	
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);
	
	int h=0;

	/* Y */
	memcpy(out, in, width * height);
	
	/* U and V */
	for (h=0; h < (width * height / 4); h++)
	{
		*pu++ = 0x80;
		*pv++ = 0x80;
	}
}

/*
 * convert y16 (16 bit greyscale format) to yu12
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing y16 (16 bit greyscale) data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void y16_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);
	
	uint16_t *ptmp = (uint16_t *) in;
	uint8_t *py = out;
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);
	
	int h = 0;

	for (h = 0; h < height; h++)
	{
		int w=0;
		for (w = 0; w < width; w++)
		{
			/* Y */
			*py++ = (uint8_t) ((*ptmp++ & 0xFF00) >> 8);
		}
	}
	
	for(h=0; h < (width * height / 4); h++)
	{
		/* U */
		*pu++ = 0x80;
		/* V */
		*pv++ = 0x80;
	}
}

/*
 * convert y16x (16 bit greyscale format - be) to yu12
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing y16x (16 bit greyscale bigendian) data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void y16x_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);
	
	uint16_t *ptmp = (uint16_t *) in;
	uint8_t *py = out;
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);
	
	int h = 0;

	for (h = 0; h < height; h++)
	{
		int w=0;
		for (w = 0; w < width; w++)
		{
			/* Y */
			*py++ = (uint8_t) (*ptmp++ & 0x00FF);
		}
	}
	
	for(h=0; h < (width * height / 4); h++)
	{
		/* U */
		*pu++ = 0x80;
		/* V */
		*pv++ = 0x80;
	}
}

/*
 * convert SPCA501 (s501) to yuv 420 planar (yu12)
 *   s501  |Y0..width..Y0|U..width/2..U|Y1..width..Y1|V..width/2..V|
 *   signed values (-128;+127) must be converted to unsigned (0; 255)
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing s501 data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void s501_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);
	
	/*assertions*/
	assert(in);
	assert(out);
	
	int h = 0;

	int8_t *pin = (int8_t *) in;
	
	uint8_t *py = out;
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height ) / 4);

	for (h = 0; h < height; h += 2 )
	{
		int w = 0;
		/* Y */
		for (w = 0; w < width; w++)
		{
			*py++ = (uint8_t) 0x80 + *pin++;
		}
		
		/* U */
		for (w = 0; w < width /2; w++)
		{
			*pu++ = (uint8_t) 0x80 + *pin++;
		}
		
		/* Y */
		for (w = 0; w < width; w++)
		{
			*py++ = (uint8_t) 0x80 + *pin++;
		}
		
		/* V */
		for (w = 0; w < width /2; w++)
		{
			*pv++ = (uint8_t) 0x80 + *pin++;
		}
	}
}

/*
 * convert SPCA505 (s505) to yuv 420 planar (yu12)
 *   s505  |Y0..width..Y0|Y1..width..Y1|U..width/2..U|V..width/2..V|
 *   signed values (-128;+127) must be converted to unsigned (0; 255)
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing s501 data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void s505_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);
	
	int h = 0;

	int8_t *pin = (int8_t *) in;
	
	uint8_t *py = out;
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height ) / 4);

	for (h = 0; h < height; h += 2 )
	{
		int w = 0;
		/* Y */
		for (w = 0; w < width * 2; w++) // 2 lines
		{
			*py++ = (uint8_t) 0x80 + *pin++;
		}
		
		/* U */
		for (w = 0; w < width /2; w++)
		{
			*pu++ = (uint8_t) 0x80 + *pin++;
		}
		
		/* V */
		for (w = 0; w < width /2; w++)
		{
			*pv++ = (uint8_t) 0x80 + *pin++;
		}
	}
}

/*
 * convert SPCA508 (s508) to yuv 420 planar (yu12)
 *   s508  |Y0..width..Y0|U..width/2..U|V..width/2..V|Y1..width..Y1|
 *   signed values (-128;+127) must be converted to unsigned (0; 255)
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing s501 data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void s508_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(in);
	assert(out);
	
	int h = 0;

	int8_t *pin = (int8_t *) in;
	
	uint8_t *py = out;
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height ) / 4);

	for (h = 0; h < height; h += 2 )
	{
		int w = 0;
		/* Y */
		for (w = 0; w < width; w++)
		{
			*py++ = (uint8_t) 0x80 + *pin++;
		}
		
		/* U */
		for (w = 0; w < width /2; w++)
		{
			*pu++ = (uint8_t) 0x80 + *pin++;
		}
		
		/* V */
		for (w = 0; w < width /2; w++)
		{
			*pv++ = (uint8_t) 0x80 + *pin++;
		}
		
		/* Y */
		for (w = 0; w < width; w++)
		{
			*py++ = (uint8_t) 0x80 + *pin++;
		}
	}
}

/*
 * convert rgb24 to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing rgb24 data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void rgb24_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(out);
	assert(in);

	uint8_t *py = out;
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in + (width * 3); //second line

	int i=0;
	for(i = 0; i < (width * height * 3); i += 3)
	{
		/* y */
		*py++ =CLIP(0.299 * (in1[i] - 128) + 0.587 * (in1[i+1] - 128) + 0.114 * (in1[i+2] - 128) + 128);
	}

	int h = 0;
	for(h = 0; h < height; h += 2)
	{
		in1 = in + (h * width * 3);
		in2 = in1 + (width * 3);

		for(i = 0; i < (width * 3); i += 6)
		{
			/* u v */
			uint8_t u1 = CLIP(((- 0.147 * (in1[i] - 128) - 0.289 * (in1[i+1] - 128) + 0.436 * (in1[i+2] - 128) + 128) +
				(- 0.147 * (in1[i+3] - 128) - 0.289 * (in1[i+4] - 128) + 0.436 * (in1[i+5] - 128) + 128))/2);
			uint8_t v1 =CLIP(((0.615 * (in1[i] - 128) - 0.515 * (in1[i+1] - 128) - 0.100 * (in1[i+2] - 128) + 128) +
				(0.615 * (in1[i+3] - 128) - 0.515 * (in1[i+4] - 128) - 0.100 * (in1[i+5] - 128) + 128))/2);

			uint8_t u2 = CLIP(((- 0.147 * (in2[i] - 128) - 0.289 * (in2[i+1] - 128) + 0.436 * (in2[i+2] - 128) + 128) +
				(- 0.147 * (in2[i+3] - 128) - 0.289 * (in2[i+4] - 128) + 0.436 * (in2[i+5] - 128) + 128))/2);
			uint8_t v2 =CLIP(((0.615 * (in2[i] - 128) - 0.515 * (in2[i+1] - 128) - 0.100 * (in2[i+2] - 128) + 128) +
				(0.615 * (in2[i+3] - 128) - 0.515 * (in2[i+4] - 128) - 0.100 * (in2[i+5] - 128) + 128))/2);	

			*pu++ = (u1 + u2) / 2;
			*pv++ = (v1 + v2) / 2;
		}
	}
}

/*
 * convert bgr24 to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing bgr24 data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void bgr24_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{	
	/*assertions*/
	assert(out);
	assert(in);

	uint8_t *py = out;
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in + (width * 3); //second line

	int i = 0;
	for(i = 0; i < (width * height * 3); i += 3)
	{
		/* y */
		*py++ =CLIP(0.299 * (in1[i+2] - 128) + 0.587 * (in1[i+1] - 128) + 0.114 * (in1[i] - 128) + 128);
	}

	int h = 0;
	for(h = 0; h < height; h += 2)
	{
		in1 = in + (h * width * 3);
		in2 = in1 + (width * 3);

		for(i = 0; i < (width * 3); i += 6)
		{
			/* u */
			uint8_t u1 = CLIP(((- 0.147 * (in1[i+2] - 128) - 0.289 * (in1[i+1] - 128) + 0.436 * (in1[i] - 128) + 128) +
				(- 0.147 * (in1[i+5] - 128) - 0.289 * (in1[i+4] - 128) + 0.436 * (in1[i+3] - 128) + 128))/2);
			uint8_t u2 = CLIP(((- 0.147 * (in2[i+2] - 128) - 0.289 * (in2[i+1] - 128) + 0.436 * (in2[i] - 128) + 128) +
				(- 0.147 * (in2[i+5] - 128) - 0.289 * (in2[i+4] - 128) + 0.436 * (in2[i+3] - 128) + 128))/2);

			/* v*/
			uint8_t v1 =CLIP(((0.615 * (in1[i+2] - 128) - 0.515 * (in1[i+1] - 128) - 0.100 * (in1[i] - 128) + 128) +
				(0.615 * (in1[i+5] - 128) - 0.515 * (in1[i+4] - 128) - 0.100 * (in1[i+3] - 128) + 128))/2);
			uint8_t v2 =CLIP(((0.615 * (in2[i+2] - 128) - 0.515 * (in2[i+1] - 128) - 0.100 * (in2[i] - 128) + 128) +
				(0.615 * (in2[i+5] - 128) - 0.515 * (in2[i+4] - 128) - 0.100 * (in2[i+3] - 128) + 128))/2);

			*pu++ = (u1 + u2) / 2;
			*pv++ = (v1 + v2) / 2;
		}
	}
}

/*
 * convert rgb1 (rgb332) to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing rgb332 data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void rgb1_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(out);
	assert(in);

	uint8_t *py1 = out;//first line
	uint8_t *py2 = py1 + width;//second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in + width; //second line (1 byte per pixel)

	int h = 0;
	int w = 0;

	for(h = 0; h < height; h += 2)
	{
		in2 = in1 + width;
		py2 = py1 + width;

		for(w = 0; w < width; w +=2)
		{
			uint8_t px00 = *in1++;
			uint8_t r00 = px00 & 0xE0;
			uint8_t g00 = (px00 << 3) & 0xE0;
			uint8_t b00 = (px00 << 6) & 0xC0;

			/* y */
			*py1++ = CLIP(0.299 * (r00 - 128) + 0.587 * (g00 - 128) + 0.114 * (b00 - 128) + 128);

			uint8_t px01 = *in1++;
			uint8_t r01 = px01 & 0xE0;
			uint8_t g01 = (px01 << 3) & 0xE0;
			uint8_t b01 = (px01 << 6) & 0xC0;

			/* y */
			*py1++ = CLIP(0.299 * (r01 - 128) + 0.587 * (g01 - 128) + 0.114 * (b01 - 128) + 128);

			uint8_t px10 = *in2++;
			uint8_t r10 = px10 & 0xE0;
			uint8_t g10 = (px10 << 3) & 0xE0;
			uint8_t b10 = (px10 << 6) & 0xC0;

			/* y */
			*py2++ = CLIP(0.299 * (r10 - 128) + 0.587 * (g10 - 128) + 0.114 * (b10 - 128) + 128);

			uint8_t px11 = *in2++;
			uint8_t r11 = px11 & 0xE0;
			uint8_t g11 = (px11 << 3) & 0xE0;
			uint8_t b11 = (px11 << 6) & 0xC0;

			/* y */
			*py2++ = CLIP(0.299 * (r11 - 128) + 0.587 * (g11 - 128) + 0.114 * (b11 - 128) + 128);

			/* u v */
			uint8_t u1 = CLIP(((- 0.147 * (r00 - 128) - 0.289 * (g00 - 128) + 0.436 * (b00 - 128) + 128) +
				(- 0.147 * (r01 - 128) - 0.289 * (g01 - 128) + 0.436 * (b01 - 128) + 128))/2);
			uint8_t v1 =CLIP(((0.615 * (r00 - 128) - 0.515 * (g00 - 128) - 0.100 * (b00 - 128) + 128) +
				(0.615 * (r01 - 128) - 0.515 * (g01 - 128) - 0.100 * (b01 - 128) + 128))/2);

			uint8_t u2 = CLIP(((- 0.147 * (r10 - 128) - 0.289 * (g10 - 128) + 0.436 * (b10 - 128) + 128) +
				(- 0.147 * (r11 - 128) - 0.289 * (g11 - 128) + 0.436 * (b11 - 128) + 128))/2);
			uint8_t v2 =CLIP(((0.615 * (r10 - 128) - 0.515 * (g10 - 128) - 0.100 * (b10 - 128) + 128) +
				(0.615 * (r11 - 128) - 0.515 * (g11 - 128) - 0.100 * (b11 - 128) + 128))/2);	

			*pu++ = (u1 + u2) / 2;
			*pv++ = (v1 + v2) / 2;
		}

		in1 = in2;
		py1 = py2;
	}
}

/*
 * convert ar12 (argb444) to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing argb444 data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void ar12_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(out);
	assert(in);

	uint8_t *py1 = out;//first line
	uint8_t *py2 = py1 + width;//second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in + (width * 2); //second line (2 byte per pixel)

	int h = 0;
	int w = 0;

	for(h = 0; h < height; h += 2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 + width;

		for(w = 0; w < (width * 2); w +=4)
		{
			uint8_t px000 = *in1++;
			uint8_t px001 = *in1++;
			uint8_t r00 = (px001 << 4) & 0xF0;
			uint8_t g00 = px000 & 0xF0;
			uint8_t b00 = (px000 << 4) & 0xF0;

			/* y */
			*py1++ = CLIP(0.299 * (r00 - 128) + 0.587 * (g00 - 128) + 0.114 * (b00 - 128) + 128);

			uint8_t px010 = *in1++;
			uint8_t px011 = *in1++;
			uint8_t r01 = (px011 << 4) & 0xF0;
			uint8_t g01 = px010 & 0xF0;
			uint8_t b01 = (px010 << 4) & 0xF0;

			/* y */
			*py1++ = CLIP(0.299 * (r01 - 128) + 0.587 * (g01 - 128) + 0.114 * (b01 - 128) + 128);

			uint8_t px100 = *in2++;
			uint8_t px101 = *in2++;
			uint8_t r10 = (px101 << 4) & 0xF0;
			uint8_t g10 = px100 & 0xF0;
			uint8_t b10 = (px100 << 4) & 0xF0;

			/* y */
			*py2++ = CLIP(0.299 * (r10 - 128) + 0.587 * (g10 - 128) + 0.114 * (b10 - 128) + 128);

			uint8_t px110 = *in2++;
			uint8_t px111 = *in2++;
			uint8_t r11 = (px111 << 4) & 0xF0;
			uint8_t g11 = px110 & 0xF0;
			uint8_t b11 = (px110 << 4) & 0xF0;

			/* y */
			*py2++ = CLIP(0.299 * (r11 - 128) + 0.587 * (g11 - 128) + 0.114 * (b11 - 128) + 128);

			/* u v */
			uint8_t u1 = CLIP(((- 0.147 * (r00 - 128) - 0.289 * (g00 - 128) + 0.436 * (b00 - 128) + 128) +
				(- 0.147 * (r01 - 128) - 0.289 * (g01 - 128) + 0.436 * (b01 - 128) + 128))/2);
			uint8_t v1 =CLIP(((0.615 * (r00 - 128) - 0.515 * (g00 - 128) - 0.100 * (b00 - 128) + 128) +
				(0.615 * (r01 - 128) - 0.515 * (g01 - 128) - 0.100 * (b01 - 128) + 128))/2);

			uint8_t u2 = CLIP(((- 0.147 * (r10 - 128) - 0.289 * (g10 - 128) + 0.436 * (b10 - 128) + 128) +
				(- 0.147 * (r11 - 128) - 0.289 * (g11 - 128) + 0.436 * (b11 - 128) + 128))/2);
			uint8_t v2 =CLIP(((0.615 * (r10 - 128) - 0.515 * (g10 - 128) - 0.100 * (b10 - 128) + 128) +
				(0.615 * (r11 - 128) - 0.515 * (g11 - 128) - 0.100 * (b11 - 128) + 128))/2);	

			*pu++ = (u1 + u2) / 2;
			*pv++ = (v1 + v2) / 2;
		}

		in1 = in2;
		py1 = py2;
	}
}

/*
 * convert ar15 (argb555) to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing argb555 data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void ar15_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(out);
	assert(in);

	uint8_t *py1 = out;//first line
	uint8_t *py2 = py1 + width;//second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in + (width * 2); //second line (2 byte per pixel)

	int h = 0;
	int w = 0;

	for(h = 0; h < height; h += 2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 + width;

		for(w = 0; w < (width * 2); w +=4)
		{
			uint8_t px000 = *in1++;
			uint8_t px001 = *in1++;
			uint8_t r00 = (px001 << 1) & 0xF8;
			uint8_t g00 = ((px001 << 6) & 0xC0) | ((px000 >> 2) & 0x38);
			uint8_t b00 = (px000 << 3) & 0xF8;

			/* y */
			*py1++ = CLIP(0.299 * (r00 - 128) + 0.587 * (g00 - 128) + 0.114 * (b00 - 128) + 128);

			uint8_t px010 = *in1++;
			uint8_t px011 = *in1++;
			uint8_t r01 = (px011 << 1) & 0xF8;
			uint8_t g01 = ((px011 << 6) & 0xC0) | ((px010 >> 2) & 0x38);
			uint8_t b01 = (px010 << 3) & 0xF8;

			/* y */
			*py1++ = CLIP(0.299 * (r01 - 128) + 0.587 * (g01 - 128) + 0.114 * (b01 - 128) + 128);

			uint8_t px100 = *in2++;
			uint8_t px101 = *in2++;
			uint8_t r10 = (px101 << 1) & 0xF8;
			uint8_t g10 = ((px101 << 6) & 0xC0) | ((px100 >> 2) & 0x38);
			uint8_t b10 = (px100 << 3) & 0xF8;

			/* y */
			*py2++ = CLIP(0.299 * (r10 - 128) + 0.587 * (g10 - 128) + 0.114 * (b10 - 128) + 128);

			uint8_t px110 = *in2++;
			uint8_t px111 = *in2++;
			uint8_t r11 = (px111 << 1) & 0xF8;
			uint8_t g11 = ((px111 << 6) & 0xC0) | ((px110 >> 2) & 0x38);
			uint8_t b11 = (px110 << 3) & 0xF8;

			/* y */
			*py2++ = CLIP(0.299 * (r11 - 128) + 0.587 * (g11 - 128) + 0.114 * (b11 - 128) + 128);

			/* u v */
			uint8_t u1 = CLIP(((- 0.147 * (r00 - 128) - 0.289 * (g00 - 128) + 0.436 * (b00 - 128) + 128) +
				(- 0.147 * (r01 - 128) - 0.289 * (g01 - 128) + 0.436 * (b01 - 128) + 128))/2);
			uint8_t v1 =CLIP(((0.615 * (r00 - 128) - 0.515 * (g00 - 128) - 0.100 * (b00 - 128) + 128) +
				(0.615 * (r01 - 128) - 0.515 * (g01 - 128) - 0.100 * (b01 - 128) + 128))/2);

			uint8_t u2 = CLIP(((- 0.147 * (r10 - 128) - 0.289 * (g10 - 128) + 0.436 * (b10 - 128) + 128) +
				(- 0.147 * (r11 - 128) - 0.289 * (g11 - 128) + 0.436 * (b11 - 128) + 128))/2);
			uint8_t v2 =CLIP(((0.615 * (r10 - 128) - 0.515 * (g10 - 128) - 0.100 * (b10 - 128) + 128) +
				(0.615 * (r11 - 128) - 0.515 * (g11 - 128) - 0.100 * (b11 - 128) + 128))/2);	

			*pu++ = (u1 + u2) / 2;
			*pv++ = (v1 + v2) / 2;
		}

		in1 = in2;
		py1 = py2;
	}
}

/*
 * convert ar15_be (argb555X) to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing argb555X (be) data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void ar15x_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(out);
	assert(in);

	uint8_t *py1 = out;//first line
	uint8_t *py2 = py1 + width;//second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in + (width * 2); //second line (2 byte per pixel)

	int h = 0;
	int w = 0;

	for(h = 0; h < height; h += 2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 + width;

		for(w = 0; w < (width * 2); w +=4)
		{
			uint8_t px000 = *in1++;
			uint8_t px001 = *in1++;
			uint8_t r00 = (px000 << 1) & 0xF8;
			uint8_t g00 = ((px000 << 6) & 0xC0) | ((px001 >> 2) & 0x38);
			uint8_t b00 = (px001 << 3) & 0xF8;

			/* y */
			*py1++ = CLIP(0.299 * (r00 - 128) + 0.587 * (g00 - 128) + 0.114 * (b00 - 128) + 128);

			uint8_t px010 = *in1++;
			uint8_t px011 = *in1++;
			uint8_t r01 = (px010 << 1) & 0xF8;
			uint8_t g01 = ((px010 << 6) & 0xC0) | ((px011 >> 2) & 0x38);
			uint8_t b01 = (px011 << 3) & 0xF8;

			/* y */
			*py1++ = CLIP(0.299 * (r01 - 128) + 0.587 * (g01 - 128) + 0.114 * (b01 - 128) + 128);

			uint8_t px100 = *in2++;
			uint8_t px101 = *in2++;
			uint8_t r10 = (px100 << 1) & 0xF8;
			uint8_t g10 = ((px100 << 6) & 0xC0) | ((px101 >> 2) & 0x38);
			uint8_t b10 = (px101 << 3) & 0xF8;

			/* y */
			*py2++ = CLIP(0.299 * (r10 - 128) + 0.587 * (g10 - 128) + 0.114 * (b10 - 128) + 128);

			uint8_t px110 = *in2++;
			uint8_t px111 = *in2++;
			uint8_t r11 = (px110 << 1) & 0xF8;
			uint8_t g11 = ((px110 << 6) & 0xC0) | ((px111 >> 2) & 0x38);
			uint8_t b11 = (px111 << 3) & 0xF8;

			/* y */
			*py2++ = CLIP(0.299 * (r11 - 128) + 0.587 * (g11 - 128) + 0.114 * (b11 - 128) + 128);

			/* u v */
			uint8_t u1 = CLIP(((- 0.147 * (r00 - 128) - 0.289 * (g00 - 128) + 0.436 * (b00 - 128) + 128) +
				(- 0.147 * (r01 - 128) - 0.289 * (g01 - 128) + 0.436 * (b01 - 128) + 128))/2);
			uint8_t v1 =CLIP(((0.615 * (r00 - 128) - 0.515 * (g00 - 128) - 0.100 * (b00 - 128) + 128) +
				(0.615 * (r01 - 128) - 0.515 * (g01 - 128) - 0.100 * (b01 - 128) + 128))/2);

			uint8_t u2 = CLIP(((- 0.147 * (r10 - 128) - 0.289 * (g10 - 128) + 0.436 * (b10 - 128) + 128) +
				(- 0.147 * (r11 - 128) - 0.289 * (g11 - 128) + 0.436 * (b11 - 128) + 128))/2);
			uint8_t v2 =CLIP(((0.615 * (r10 - 128) - 0.515 * (g10 - 128) - 0.100 * (b10 - 128) + 128) +
				(0.615 * (r11 - 128) - 0.515 * (g11 - 128) - 0.100 * (b11 - 128) + 128))/2);	

			*pu++ = (u1 + u2) / 2;
			*pv++ = (v1 + v2) / 2;
		}

		in1 = in2;
		py1 = py2;
	}
}

/*
 * convert rgbp (rgb565) to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing argb555 data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void rgbp_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(out);
	assert(in);

	uint8_t *py1 = out;//first line
	uint8_t *py2 = py1 + width;//second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in + (width * 2); //second line (2 byte per pixel)

	int h = 0;
	int w = 0;

	for(h = 0; h < height; h += 2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 + width;

		for(w = 0; w < (width * 2); w +=4)
		{
			uint8_t px000 = *in1++;
			uint8_t px001 = *in1++;
			uint8_t r00 = px001 & 0xF8;
			uint8_t g00 = ((px001 << 5) & 0xE0) | ((px000 >> 3) & 0x1C);
			uint8_t b00 = (px000 << 3) & 0xF8;

			/* y */
			*py1++ = CLIP(0.299 * (r00 - 128) + 0.587 * (g00 - 128) + 0.114 * (b00 - 128) + 128);

			uint8_t px010 = *in1++;
			uint8_t px011 = *in1++;
			uint8_t r01 = px011 & 0xF8;
			uint8_t g01 = ((px011 << 5) & 0xE0) | ((px010 >> 3) & 0x1C);
			uint8_t b01 = (px010 << 3) & 0xF8;

			/* y */
			*py1++ = CLIP(0.299 * (r01 - 128) + 0.587 * (g01 - 128) + 0.114 * (b01 - 128) + 128);

			uint8_t px100 = *in2++;
			uint8_t px101 = *in2++;
			uint8_t r10 = px101 & 0xF8;
			uint8_t g10 = ((px101 << 5) & 0xE0) | ((px100 >> 3) & 0x1C);
			uint8_t b10 = (px100 << 3) & 0xF8;

			/* y */
			*py2++ = CLIP(0.299 * (r10 - 128) + 0.587 * (g10 - 128) + 0.114 * (b10 - 128) + 128);

			uint8_t px110 = *in2++;
			uint8_t px111 = *in2++;
			uint8_t r11 = px111 & 0xF8;
			uint8_t g11 = ((px111 << 5) & 0xE0) | ((px110 >> 3) & 0x1C);
			uint8_t b11 = (px110 << 3) & 0xF8;

			/* y */
			*py2++ = CLIP(0.299 * (r11 - 128) + 0.587 * (g11 - 128) + 0.114 * (b11 - 128) + 128);

			/* u v */
			uint8_t u1 = CLIP(((- 0.147 * (r00 - 128) - 0.289 * (g00 - 128) + 0.436 * (b00 - 128) + 128) +
				(- 0.147 * (r01 - 128) - 0.289 * (g01 - 128) + 0.436 * (b01 - 128) + 128))/2);
			uint8_t v1 =CLIP(((0.615 * (r00 - 128) - 0.515 * (g00 - 128) - 0.100 * (b00 - 128) + 128) +
				(0.615 * (r01 - 128) - 0.515 * (g01 - 128) - 0.100 * (b01 - 128) + 128))/2);

			uint8_t u2 = CLIP(((- 0.147 * (r10 - 128) - 0.289 * (g10 - 128) + 0.436 * (b10 - 128) + 128) +
				(- 0.147 * (r11 - 128) - 0.289 * (g11 - 128) + 0.436 * (b11 - 128) + 128))/2);
			uint8_t v2 =CLIP(((0.615 * (r10 - 128) - 0.515 * (g10 - 128) - 0.100 * (b10 - 128) + 128) +
				(0.615 * (r11 - 128) - 0.515 * (g11 - 128) - 0.100 * (b11 - 128) + 128))/2);	

			*pu++ = (u1 + u2) / 2;
			*pv++ = (v1 + v2) / 2;
		}

		in1 = in2;
		py1 = py2;
	}
}

/*
 * convert rgbr (rgb565X) to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing rgb565 bigendian data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void rgbr_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(out);
	assert(in);

	uint8_t *py1 = out;//first line
	uint8_t *py2 = py1 + width;//second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in + (width * 2); //second line (2 byte per pixel)

	int h = 0;
	int w = 0;

	for(h = 0; h < height; h += 2)
	{
		in2 = in1 + (width * 2);
		py2 = py1 + width;

		for(w = 0; w < (width * 2); w +=4)
		{
			uint8_t px000 = *in1++;
			uint8_t px001 = *in1++;
			uint8_t r00 = px000 & 0xF8;
			uint8_t g00 = ((px000 << 5) & 0xE0) | ((px001 >> 3) & 0x1C);
			uint8_t b00 = (px001 << 3) & 0xF8;

			/* y */
			*py1++ = CLIP(0.299 * (r00 - 128) + 0.587 * (g00 - 128) + 0.114 * (b00 - 128) + 128);

			uint8_t px010 = *in1++;
			uint8_t px011 = *in1++;
			uint8_t r01 = px010 & 0xF8;
			uint8_t g01 = ((px010 << 5) & 0xE0) | ((px011 >> 3) & 0x1C);
			uint8_t b01 = (px011 << 3) & 0xF8;

			/* y */
			*py1++ = CLIP(0.299 * (r01 - 128) + 0.587 * (g01 - 128) + 0.114 * (b01 - 128) + 128);

			uint8_t px100 = *in2++;
			uint8_t px101 = *in2++;
			uint8_t r10 = px100 & 0xF8;
			uint8_t g10 = ((px100 << 5) & 0xE0) | ((px101 >> 3) & 0x1C);
			uint8_t b10 = (px101 << 3) & 0xF8;

			/* y */
			*py2++ = CLIP(0.299 * (r10 - 128) + 0.587 * (g10 - 128) + 0.114 * (b10 - 128) + 128);

			uint8_t px110 = *in2++;
			uint8_t px111 = *in2++;
			uint8_t r11 = px110 & 0xF8;
			uint8_t g11 = ((px110 << 5) & 0xE0) | ((px111 >> 3) & 0x1C);
			uint8_t b11 = (px111 << 3) & 0xF8;

			/* y */
			*py2++ = CLIP(0.299 * (r11 - 128) + 0.587 * (g11 - 128) + 0.114 * (b11 - 128) + 128);

			/* u v */
			uint8_t u1 = CLIP(((- 0.147 * (r00 - 128) - 0.289 * (g00 - 128) + 0.436 * (b00 - 128) + 128) +
				(- 0.147 * (r01 - 128) - 0.289 * (g01 - 128) + 0.436 * (b01 - 128) + 128))/2);
			uint8_t v1 =CLIP(((0.615 * (r00 - 128) - 0.515 * (g00 - 128) - 0.100 * (b00 - 128) + 128) +
				(0.615 * (r01 - 128) - 0.515 * (g01 - 128) - 0.100 * (b01 - 128) + 128))/2);

			uint8_t u2 = CLIP(((- 0.147 * (r10 - 128) - 0.289 * (g10 - 128) + 0.436 * (b10 - 128) + 128) +
				(- 0.147 * (r11 - 128) - 0.289 * (g11 - 128) + 0.436 * (b11 - 128) + 128))/2);
			uint8_t v2 =CLIP(((0.615 * (r10 - 128) - 0.515 * (g10 - 128) - 0.100 * (b10 - 128) + 128) +
				(0.615 * (r11 - 128) - 0.515 * (g11 - 128) - 0.100 * (b11 - 128) + 128))/2);	

			*pu++ = (u1 + u2) / 2;
			*pv++ = (v1 + v2) / 2;
		}

		in1 = in2;
		py1 = py2;
	}
}

/*
 * convert bgrh to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing bgrh (bgr666) data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void bgrh_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(out);
	assert(in);

	uint8_t *py1 = out;//first line
	uint8_t *py2 = py1 + width;//second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in + (width * 4); //second line (4 byte per pixel)

	int h = 0;
	int w = 0;

	for(h = 0; h < height; h += 2)
	{
		in2 = in1 + (width * 4);
		py2 = py1 + width;

		for(w = 0; w < (width * 4); w +=8)
		{
			uint8_t px000 = *in1++;
			uint8_t px001 = *in1++;
			uint8_t px002 = *in1++;
			in1++; //last byte has empty data
			uint8_t r00 = ((px002 >> 4) & 0x0C) | ((px001 << 4)  & 0xF0);
			uint8_t g00 = ((px001 >> 2) & 0x3C) | ((px000 << 6) & 0xC0);
			uint8_t b00 = px000  & 0xFC;

			/* y */
			*py1++ = CLIP(0.299 * (r00 - 128) + 0.587 * (g00 - 128) + 0.114 * (b00 - 128) + 128);

			uint8_t px010 = *in1++;
			uint8_t px011 = *in1++;
			uint8_t px012 = *in1++;
			in1++; //last byte has empty data
			uint8_t r01 = ((px012 >> 4) & 0x0C) | ((px011 << 4)  & 0xF0);
			uint8_t g01 = ((px011 >> 2) & 0x3C) | ((px010 << 6) & 0xC0);
			uint8_t b01 = px010  & 0xFC;

			/* y */
			*py1++ = CLIP(0.299 * (r01 - 128) + 0.587 * (g01 - 128) + 0.114 * (b01 - 128) + 128);

			uint8_t px100 = *in2++;
			uint8_t px101 = *in2++;
			uint8_t px102 = *in2++;
			in2++; //last byte has empty data
			uint8_t r10 = ((px102 >> 4) & 0x0C) | ((px101 << 4)  & 0xF0);
			uint8_t g10 = ((px101 >> 2) & 0x3C) | ((px100 << 6) & 0xC0);
			uint8_t b10 = px100  & 0xFC;

			/* y */
			*py2++ = CLIP(0.299 * (r10 - 128) + 0.587 * (g10 - 128) + 0.114 * (b10 - 128) + 128);

			uint8_t px110 = *in2++;
			uint8_t px111 = *in2++;
			uint8_t px112 = *in2++;
			in2++; //last byte has empty data
			uint8_t r11 = ((px112 >> 4) & 0x0C) | ((px111 << 4)  & 0xF0);
			uint8_t g11 = ((px111 >> 2) & 0x3C) | ((px110 << 6) & 0xC0);
			uint8_t b11 = px110  & 0xFC;

			/* y */
			*py2++ = CLIP(0.299 * (r11 - 128) + 0.587 * (g11 - 128) + 0.114 * (b11 - 128) + 128);

			/* u v */
			uint8_t u1 = CLIP(((- 0.147 * (r00 - 128) - 0.289 * (g00 - 128) + 0.436 * (b00 - 128) + 128) +
				(- 0.147 * (r01 - 128) - 0.289 * (g01 - 128) + 0.436 * (b01 - 128) + 128))/2);
			uint8_t v1 =CLIP(((0.615 * (r00 - 128) - 0.515 * (g00 - 128) - 0.100 * (b00 - 128) + 128) +
				(0.615 * (r01 - 128) - 0.515 * (g01 - 128) - 0.100 * (b01 - 128) + 128))/2);

			uint8_t u2 = CLIP(((- 0.147 * (r10 - 128) - 0.289 * (g10 - 128) + 0.436 * (b10 - 128) + 128) +
				(- 0.147 * (r11 - 128) - 0.289 * (g11 - 128) + 0.436 * (b11 - 128) + 128))/2);
			uint8_t v2 =CLIP(((0.615 * (r10 - 128) - 0.515 * (g10 - 128) - 0.100 * (b10 - 128) + 128) +
				(0.615 * (r11 - 128) - 0.515 * (g11 - 128) - 0.100 * (b11 - 128) + 128))/2);	

			*pu++ = (u1 + u2) / 2;
			*pv++ = (v1 + v2) / 2;
		}

		in1 = in2;
		py1 = py2;
	}
}

/*
 * convert ar24 to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing ar24 (bgr32) data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void ar24_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(out);
	assert(in);

	uint8_t *py1 = out;//first line
	uint8_t *py2 = py1 + width;//second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in + (width * 4); //second line (4 byte per pixel)

	int h = 0;
	int w = 0;

	for(h = 0; h < height; h += 2)
	{
		in2 = in1 + (width * 4);
		py2 = py1 + width;

		for(w = 0; w < (width * 4); w +=8)
		{
			uint8_t b00 = *in1++;
			uint8_t g00 = *in1++;
			uint8_t r00 = *in1++;
			in1++; //last byte has alpha data

			/* y */
			*py1++ = CLIP(0.299 * (r00 - 128) + 0.587 * (g00 - 128) + 0.114 * (b00 - 128) + 128);

			uint8_t b01 = *in1++;
			uint8_t g01 = *in1++;
			uint8_t r01 = *in1++;
			in1++; //last byte has alpha data

			/* y */
			*py1++ = CLIP(0.299 * (r01 - 128) + 0.587 * (g01 - 128) + 0.114 * (b01 - 128) + 128);

			uint8_t b10 = *in2++;
			uint8_t g10 = *in2++;
			uint8_t r10 = *in2++;
			in2++; //last byte has alpha data

			/* y */
			*py2++ = CLIP(0.299 * (r10 - 128) + 0.587 * (g10 - 128) + 0.114 * (b10 - 128) + 128);

			uint8_t b11 = *in2++;
			uint8_t g11 = *in2++;
			uint8_t r11 = *in2++;
			in2++; //last byte has alpha data

			/* y */
			*py2++ = CLIP(0.299 * (r11 - 128) + 0.587 * (g11 - 128) + 0.114 * (b11 - 128) + 128);

			/* u v */
			uint8_t u1 = CLIP(((- 0.147 * (r00 - 128) - 0.289 * (g00 - 128) + 0.436 * (b00 - 128) + 128) +
				(- 0.147 * (r01 - 128) - 0.289 * (g01 - 128) + 0.436 * (b01 - 128) + 128))/2);
			uint8_t v1 =CLIP(((0.615 * (r00 - 128) - 0.515 * (g00 - 128) - 0.100 * (b00 - 128) + 128) +
				(0.615 * (r01 - 128) - 0.515 * (g01 - 128) - 0.100 * (b01 - 128) + 128))/2);

			uint8_t u2 = CLIP(((- 0.147 * (r10 - 128) - 0.289 * (g10 - 128) + 0.436 * (b10 - 128) + 128) +
				(- 0.147 * (r11 - 128) - 0.289 * (g11 - 128) + 0.436 * (b11 - 128) + 128))/2);
			uint8_t v2 =CLIP(((0.615 * (r10 - 128) - 0.515 * (g10 - 128) - 0.100 * (b10 - 128) + 128) +
				(0.615 * (r11 - 128) - 0.515 * (g11 - 128) - 0.100 * (b11 - 128) + 128))/2);	

			*pu++ = (u1 + u2) / 2;
			*pv++ = (v1 + v2) / 2;
		}

		in1 = in2;
		py1 = py2;
	}
}

/*
 * convert ba24 to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing ba24 (rgb32) data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void ba24_to_yu12(uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(out);
	assert(in);

	uint8_t *py1 = out;//first line
	uint8_t *py2 = py1 + width;//second line
	uint8_t *pu = out + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	uint8_t *in1 = in; //first line
	uint8_t *in2 = in + (width * 4); //second line (4 byte per pixel)

	int h = 0;
	int w = 0;

	for(h = 0; h < height; h += 2)
	{
		in2 = in1 + (width * 4);
		py2 = py1 + width;

		for(w = 0; w < (width * 4); w +=8)
		{
			in1++; //first byte has alpha data
			uint8_t r00 = *in1++;
			uint8_t g00 = *in1++;
			uint8_t b00 = *in1++;

			/* y */
			*py1++ = CLIP(0.299 * (r00 - 128) + 0.587 * (g00 - 128) + 0.114 * (b00 - 128) + 128);

			in1++; //first byte has alpha data
			uint8_t r01 = *in1++;
			uint8_t g01 = *in1++;
			uint8_t b01 = *in1++;

			/* y */
			*py1++ = CLIP(0.299 * (r01 - 128) + 0.587 * (g01 - 128) + 0.114 * (b01 - 128) + 128);

			in2++; //first byte has alpha data
			uint8_t r10 = *in2++;
			uint8_t g10 = *in2++;
			uint8_t b10 = *in2++;

			/* y */
			*py2++ = CLIP(0.299 * (r10 - 128) + 0.587 * (g10 - 128) + 0.114 * (b10 - 128) + 128);

			in2++; //first byte has alpha data
			uint8_t r11 = *in2++;
			uint8_t g11 = *in2++;
			uint8_t b11 = *in2++;

			/* y */
			*py2++ = CLIP(0.299 * (r11 - 128) + 0.587 * (g11 - 128) + 0.114 * (b11 - 128) + 128);

			/* u v */
			uint8_t u1 = CLIP(((- 0.147 * (r00 - 128) - 0.289 * (g00 - 128) + 0.436 * (b00 - 128) + 128) +
				(- 0.147 * (r01 - 128) - 0.289 * (g01 - 128) + 0.436 * (b01 - 128) + 128))/2);
			uint8_t v1 =CLIP(((0.615 * (r00 - 128) - 0.515 * (g00 - 128) - 0.100 * (b00 - 128) + 128) +
				(0.615 * (r01 - 128) - 0.515 * (g01 - 128) - 0.100 * (b01 - 128) + 128))/2);

			uint8_t u2 = CLIP(((- 0.147 * (r10 - 128) - 0.289 * (g10 - 128) + 0.436 * (b10 - 128) + 128) +
				(- 0.147 * (r11 - 128) - 0.289 * (g11 - 128) + 0.436 * (b11 - 128) + 128))/2);
			uint8_t v2 =CLIP(((0.615 * (r10 - 128) - 0.515 * (g10 - 128) - 0.100 * (b10 - 128) + 128) +
				(0.615 * (r11 - 128) - 0.515 * (g11 - 128) - 0.100 * (b11 - 128) + 128))/2);	

			*pu++ = (u1 + u2) / 2;
			*pv++ = (v1 + v2) / 2;
		}

		in1 = in2;
		py1 = py2;
	}
}

/*
 * yu12 to rgb24
 * args:
 *    out - pointer to output rgb data buffer
 *    in - pointer to input yu12 data buffer
 *    width - buffer width (in pixels)
 *    height - buffer height (in pixels)
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void yu12_to_rgb24 (uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(out);
	assert(in);
	
	uint8_t *py1 = in; //line 1
	uint8_t *py2 = py1 + width; //line 2
	uint8_t *pu = in + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);
	
	uint8_t *pout1 = out; //first line
	uint8_t *pout2 = out + (width * 3); //second line
	
	int h=0, w=0;
	
	for(h=0; h < height; h+=2) //every two lines
	{
		py1 = in + (h * width);
		py2 = py1 + width;
		
		pout1 = out + (h * width * 3);
		pout2 = pout1 + (width * 3);
		
		for(w=0; w<width; w+=2) //every 2 pixels
		{
			/* standart: r = y0 + 1.402 (v-128) */
			/* logitech: r = y0 + 1.370705 (v-128) */
			*pout1++=CLIP(*py1 + 1.402 * (*pv-128));
			*pout2++=CLIP(*py2 + 1.402 * (*pv-128));	
			/* standart: g = y0 - 0.34414 (u-128) - 0.71414 (v-128)*/
			/* logitech: g = y0 - 0.337633 (u-128)- 0.698001 (v-128)*/
			*pout1++=CLIP(*py1 - 0.34414 * (*pu-128) -0.71414*(*pv-128));
			*pout2++=CLIP(*py2 - 0.34414 * (*pu-128) -0.71414*(*pv-128));
			/* standart: b = y0 + 1.772 (u-128) */
			/* logitech: b = y0 + 1.732446 (u-128) */
			*pout1++=CLIP(*py1 + 1.772 *( *pu-128));
			*pout2++=CLIP(*py2 + 1.772 *( *pu-128));
			
			py1++;
			py2++;
			
			/* standart: r1 =y1 + 1.402 (v-128) */
			/* logitech: r1 = y1 + 1.370705 (v-128) */
			*pout1++=CLIP(*py1 + 1.402 * (*pv-128));
			*pout2++=CLIP(*py2 + 1.402 * (*pv-128));
			/* standart: g1 = y1 - 0.34414 (u-128) - 0.71414 (v-128)*/
			/* logitech: g1 = y1 - 0.337633 (u-128)- 0.698001 (v-128)*/
			*pout1++=CLIP(*py1 - 0.34414 * (*pu-128) -0.71414 * (*pv-128));
			*pout2++=CLIP(*py2 - 0.34414 * (*pu-128) -0.71414 * (*pv-128));
			/* standart: b1 = y1 + 1.772 (u-128) */
			/* logitech: b1 = y1 + 1.732446 (u-128) */
			*pout1++=CLIP(*py1 + 1.772 * (*pu-128));
			*pout2++=CLIP(*py2 + 1.772 * (*pu-128));
			
			py1++;
			py2++;
			pu++;
			pv++;
		}
	}
}

static __int64_t T1_402[256], T0_34414[256], T0_71414[256], T1_772[256];
void init_yuv2rgb_num_table()
{
    for (int i = 0; i < 256; i++) {
        T1_402[i] = i * 5743;
        T1_402[i] = T1_402[i] >> 12;
        T0_34414[i] = i * 721714;
        T0_34414[i] = T0_34414[i] >> 21;
        T0_71414[i] = i * 5990641;
        T0_71414[i] = T0_71414[i] >> 23;
        T1_772[i] = i * 1858077;
        T1_772[i] = T1_772[i] >> 20;
    }
}

/*
 * yu12 to rgb24 high efficiency, use table inquer improve excution efficency
 * args:
 *    out - pointer to output rgb data buffer
 *    in - pointer to input yu12 data buffer
 *    width - buffer width (in pixels)
 *    height - buffer height (in pixels)
 *
 * asserts:
 *    none
 *
 * returns: none
 */
#define F_H
void yu12_to_rgb24_higheffic (uint8_t *out, uint8_t *in, int width, int height)
{
    /*assertions*/
    assert(out);
    assert(in);

    uint8_t *py1 = in; //line 1
    uint8_t *py2 = py1 + width; //line 2
    uint8_t *pu = in + (width * height);
    uint8_t *pv = pu + ((width * height) / 4);

    uint8_t *pout1 = out; //first line
    uint8_t *pout2 = out + (width * 3); //second line

    int h=0, w=0;

    int groupSize = width * 3;
    int64_t v1_402, u0_34414_v0_71414, u1_772;
    for(h=0; h < height; h+=2) //every two lines
    {
        py1 = in + (h * width);
        py2 = py1 + width;

        pout1 = out + (h * groupSize);
        pout2 = pout1 + (groupSize);

        for(w=0; w<width; w+=2) //every 2 pixels
        {
            v1_402 = T1_402[*pv] - T1_402[128];
            u0_34414_v0_71414 = T0_34414[*pu] - T0_34414[128] + T0_71414[*pv] - T0_71414[128];
            u1_772 = T1_772[*pu] - T1_772[128];
//            printf("v1_402i:%ld, uvi:%ld, u1_772i:%ld\n", v1_402, u0_34414_v0_71414, u1_772);

//            v1_402f = 1.402 * (*pv - 128);
//            u0_34414_v0_71414f = 0.34414 * (*pu - 128) + 0.71414 * (*pv - 128);
//            u1_772f = 1.772 * (*pu - 128);
//            printf("v1_402f:%f, uvf:%f, u1_772f:%f\n", v1_402f, u0_34414_v0_71414f, u1_772f);
#ifdef F_H
            *pout1++=CLIP(*py1 + v1_402);
            *pout2++=CLIP(*py2 + v1_402);
            *pout1++=CLIP(*py1 - u0_34414_v0_71414);
            *pout2++=CLIP(*py2 - u0_34414_v0_71414);
            *pout1++=CLIP(*py1 + u1_772);
            *pout2++=CLIP(*py2 + u1_772);

            py1++;
            py2++;

            *pout1++=CLIP(*py1 + v1_402);
            *pout2++=CLIP(*py2 + v1_402);
            *pout1++=CLIP(*py1 - u0_34414_v0_71414);
            *pout2++=CLIP(*py2 - u0_34414_v0_71414);
            *pout1++=CLIP(*py1 + u1_772);
            *pout2++=CLIP(*py2 + u1_772);
#elif defined F_L
//            uint8_t t1 = CLIP(v1_402f);
//            uint8_t t2 = CLIP(u0_34414_v0_71414f);
//            uint8_t t3 = CLIP(u1_772f);
//            printf("v1_402_ui:%d, uv_ui:%d, u1_772_ui:%d\n", t1, t2, t3);

            v1_402f = 1.402 * (*pv - 128);
            u0_34414_v0_71414f = 0.34414 * (*pu - 128) + 0.71414 * (*pv - 128);
            u1_772f = 1.722 * (*pu - 128);
            *pout1++=CLIP(*py1 + v1_402f);
            *pout2++=CLIP(*py2 + v1_402f);
            *pout1++=CLIP(*py1 - u0_34414_v0_71414f);
            *pout2++=CLIP(*py2 - u0_34414_v0_71414f);
            *pout1++=CLIP(*py1 + u1_772f);
            *pout2++=CLIP(*py2 + u1_772f);

            py1++;
            py2++;

            *pout1++=CLIP(*py1 + v1_402f);
            *pout2++=CLIP(*py2 + v1_402f);
            *pout1++=CLIP(*py1 - u0_34414_v0_71414f);
            *pout2++=CLIP(*py2 - u0_34414_v0_71414f);
            *pout1++=CLIP(*py1 + u1_772f);
            *pout2++=CLIP(*py2 + u1_772f);
#else
            *pout1++=CLIP(*py1 + 1.402 * (*pv-128));
            *pout2++=CLIP(*py2 + 1.402 * (*pv-128));
            *pout1++=CLIP(*py1 - 0.34414 * (*pu-128) -0.71414*(*pv-128));
            *pout2++=CLIP(*py2 - 0.34414 * (*pu-128) -0.71414*(*pv-128));
            *pout1++=CLIP(*py1 + 1.772 *( *pu-128));
            *pout2++=CLIP(*py2 + 1.772 *( *pu-128));

            py1++;
            py2++;

            *pout1++=CLIP(*py1 + 1.402 * (*pv-128));
            *pout2++=CLIP(*py2 + 1.402 * (*pv-128));
            *pout1++=CLIP(*py1 - 0.34414 * (*pu-128) -0.71414 * (*pv-128));
            *pout2++=CLIP(*py2 - 0.34414 * (*pu-128) -0.71414 * (*pv-128));
            *pout1++=CLIP(*py1 + 1.772 * (*pu-128));
            *pout2++=CLIP(*py2 + 1.772 * (*pu-128));
#endif
            py1++;
            py2++;
            pu++;
            pv++;
        }
    }
}

/*
 * FIXME:  yu12 to bgr24 with lines upsidedown
 *   used for bitmap files (DIB24)
 * args:
 *    out - pointer to output bgr data buffer
 *    in - pointer to input yu12 data buffer
 *    width - buffer width (in pixels)
 *    height - buffer height (in pixels)
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void yu12_to_dib24 (uint8_t *out, uint8_t *in, int width, int height)
{
	/*assertions*/
	assert(out);
	assert(in);
	
	uint8_t *py1 = in + (height * width) - width; //begin of last line
	uint8_t *py2 = py1 - width; //last line -1
	
	uint8_t *pu = in + ((width * height * 5) / 4) - (width/2); //begin of last line
	uint8_t *pv = pu + ((width * height) / 4); //begin of last line
	
	uint8_t *pout1 = out; //first line
	uint8_t *pout2 = pout1 + (width * 3); //second line
	
	int h=0, w=0;
	int uvline = height/2;
	
	for(h=height; h >0 ; h-=2) //every two lines
	{
		uvline--; //begin of uv line
		
		py1 = in + ((h-1) * width);
		py2 = py1 - width;
		
		pu = in + (width * height) + ((uvline * width)/2);
		pv = pu + ((width * height) / 4);
		
		pout1 = out + ((height-h) * width * 3);
		pout2 = pout1 + (width * 3);
		
		for(w=0; w<width; w+=2) //every 2 pixels
		{
			/* standart: b = y0 + 1.772 (u-128) */
			/* logitech: b = y0 + 1.732446 (u-128) */
			*pout1++=CLIP(*py1 + 1.772 *( *pu-128));
			*pout2++=CLIP(*py2 + 1.772 *( *pu-128));	
			/* standart: g = y0 - 0.34414 (u-128) - 0.71414 (v-128)*/
			/* logitech: g = y0 - 0.337633 (u-128)- 0.698001 (v-128)*/
			*pout1++=CLIP(*py1 - 0.34414 * (*pu-128) -0.71414*(*pv-128));
			*pout2++=CLIP(*py2 - 0.34414 * (*pu-128) -0.71414*(*pv-128));
			/* standart: r = y0 + 1.402 (v-128) */
			/* logitech: r = y0 + 1.370705 (v-128) */
			*pout1++=CLIP(*py1 + 1.402 * (*pv-128));
			*pout2++=CLIP(*py2 + 1.402 * (*pv-128));
			
			py1++;
			py2++;
			
			/* standart: b = y0 + 1.772 (u-128) */
			/* logitech: b = y0 + 1.732446 (u-128) */
			*pout1++=CLIP(*py1 + 1.772 *( *pu-128));
			*pout2++=CLIP(*py2 + 1.772 *( *pu-128));	
			/* standart: g = y0 - 0.34414 (u-128) - 0.71414 (v-128)*/
			/* logitech: g = y0 - 0.337633 (u-128)- 0.698001 (v-128)*/
			*pout1++=CLIP(*py1 - 0.34414 * (*pu-128) -0.71414*(*pv-128));
			*pout2++=CLIP(*py2 - 0.34414 * (*pu-128) -0.71414*(*pv-128));
			/* standart: r = y0 + 1.402 (v-128) */
			/* logitech: r = y0 + 1.370705 (v-128) */
			*pout1++=CLIP(*py1 + 1.402 * (*pv-128));
			*pout2++=CLIP(*py2 + 1.402 * (*pv-128));

			py1++;
			py2++;
			pu++;
			pv++;
		}
	}
}

/*
 * convert yuv 420 planar (yu12) to yuv 422 (save_image_jpeg)
 * args:
 *    out- pointer to output buffer (yuyv)
 *    in- pointer to input buffer (yuv420 planar data frame (yu12))
 *    width- picture width
 *    height- picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void yu12_to_yuyv (uint8_t *out, uint8_t *in, int width, int height)
{
	uint8_t *py;
	uint8_t *pu;
	uint8_t *pv;

	int linesize = width * 2;
	int uvlinesize = width / 2;

	py=in;
	pu=py+(width*height);
	pv=pu+(width*height/4);

	int h=0;
	int huv=0;

	for(h=0;h<height;h+=2)
	{
		int wy = 0;
		int wuv = 0;
		int offset = h * linesize;
		int offset1 = (h + 1) * linesize;
		int offsety = h * width;
		int offsety1 = (h + 1) * width;
		int offsetuv = huv * uvlinesize;
		int w = 0;

		for(w=0;w<linesize;w+=4)
		{
			/*y00*/
			out[w + offset] = py[wy + offsety];
			/*u0*/
			out[(w + 1) + offset] = pu[wuv + offsetuv];
			/*y01*/
			out[(w + 2) + offset] = py[(wy + 1) + offsety];
			/*v0*/
			out[(w + 3) + offset] = pv[wuv + offsetuv];

			/*y10*/
			out[w + offset1] = py[wy + offsety1];
			/*u0*/
			out[(w + 1) + offset1] = pu[wuv + offsetuv];
			/*y11*/
			out[(w + 2) + offset1] = py[(wy + 1) + offsety1];
			/*v0*/
			out[(w + 3) + offset1] = pv[wuv + offsetuv];

			wuv++;
			wy+=2;
		}
		huv++;
	}
}

#if MJPG_BUILTIN //use internal jpeg decoder
/*
 * used for internal jpeg decoding  420 planar to 422
 * args:
 *   out: pointer to data output of idct (macroblocks yyyy u v)
 *   pic: pointer to picture buffer (yuyv)
 *   width: picture width
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void yuv420pto422(int *out, uint8_t *pic, int width)
{
	int j = 0;
	int outy1 = 0;
	int outy2 = 8;
	//yyyyuv
	uint8_t *pic0 = pic;
	uint8_t *pic1 = pic + width;
	int *outy = out;
	int *outu = out + 64 * 4;
	int *outv = out + 64 * 5;

	for (j = 0; j < 8; j++)
	{
		int k = 0;
		for (k = 0; k < 8; k++)
		{
			if( k == 4)
			{
				outy1 += 56;
				outy2 += 56;
			}
			*pic0++ = CLIP(outy[outy1]);   //y1 line 1
			*pic0++ = CLIP(128 + *outu);   //u  line 1-2
			*pic0++ = CLIP(outy[outy1+1]); //y2 line 1
			*pic0++ = CLIP(128 + *outv);   //v  line 1-2
			*pic1++ = CLIP(outy[outy2]);   //y1 line 2
			*pic1++ = CLIP(128 + *outu);   //u  line 1-2
			*pic1++ = CLIP(outy[outy2+1]); //y2 line 2
			*pic1++ = CLIP(128 + *outv);   //v  line 1-2
			outy1 +=2; outy2 += 2; outu++; outv++;
		}
		if(j==3)
		{
			outy = out + 128;
		}
		else
		{
			outy += 16;
		}
		outy1 = 0;
		outy2 = 8;
		pic0 += 2 * (width -16);
		pic1 += 2 * (width -16);
	}
}

/*
 * used for internal jpeg decoding 422 planar to 422
 * args:
 *   out: pointer to data output of idct (macroblocks yyyy u v)
 *   pic: pointer to picture buffer (yuyv)
 *   width: picture width
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void yuv422pto422(int *out, uint8_t *pic, int width)
{
	int j = 0;
	int outy1 = 0;
	int outy2 = 8;
	int outu1 = 0;
	int outv1 = 0;
	//yyyyuv
	uint8_t *pic0 = pic;
	uint8_t *pic1 = pic + width;
	int *outy = out;
	int *outu = out + 64 * 4;
	int *outv = out + 64 * 5;

	for (j = 0; j < 4; j++)
	{
		int k = 0;
		for (k = 0; k < 8; k++)
		{
			if( k == 4)
			{
				outy1 += 56;
				outy2 += 56;
			}
			*pic0++ = CLIP(outy[outy1]);        //y1 line 1
			*pic0++ = CLIP(128 + outu[outu1]);  //u  line 1
			*pic0++ = CLIP(outy[outy1+1]);      //y2 line 1
			*pic0++ = CLIP(128 + outv[outv1]);  //v  line 1
			*pic1++ = CLIP(outy[outy2]);        //y1 line 2
			*pic1++ = CLIP(128 + outu[outu1+8]);//u  line 2
			*pic1++ = CLIP(outy[outy2+1]);      //y2 line 2
			*pic1++ = CLIP(128 + outv[outv1+8]);//v  line 2
			outv1 += 1; outu1 += 1;
			outy1 +=2; outy2 +=2;
		}
		outy += 16;outu +=8; outv +=8;
		outv1 = 0; outu1=0;
		outy1 = 0;
		outy2 = 8;
		pic0 += 2 * (width -16);
		pic1 += 2 * (width -16);
	}
}

/*
 * used for internal jpeg decoding 444 planar to 422
 * args:
 *   out: pointer to data output of idct (macroblocks yyyy u v)
 *   pic: pointer to picture buffer (yuyv)
 *   width: picture width
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void yuv444pto422(int *out, uint8_t *pic, int width)
{
	int j = 0;
	int outy1 = 0;
	int outy2 = 8;
	int outu1 = 0;
	int outv1 = 0;
	//yyyyuv
	uint8_t *pic0 = pic;
	uint8_t *pic1 = pic + width;
	int *outy = out;
	int *outu = out + 64 * 4; // Ooops where did i invert ??
	int *outv = out + 64 * 5;

	for (j = 0; j < 4; j++)
	{
		int k = 0;
		for (k = 0; k < 4; k++)
		{
			*pic0++ =CLIP( outy[outy1]);        //y1 line 1
			*pic0++ =CLIP( 128 + outu[outu1]);  //u  line 1
			*pic0++ =CLIP( outy[outy1+1]);      //y2 line 1
			*pic0++ =CLIP( 128 + outv[outv1]);  //v  line 1
			*pic1++ =CLIP( outy[outy2]);        //y1 line 2
			*pic1++ =CLIP( 128 + outu[outu1+8]);//u  line 2
			*pic1++ =CLIP( outy[outy2+1]);      //y2 line 2
			*pic1++ =CLIP( 128 + outv[outv1+8]);//v  line 2
			outv1 += 2; outu1 += 2;
			outy1 +=2; outy2 +=2;
		}
		outy += 16;outu +=16; outv +=16;
		outv1 = 0; outu1=0;
		outy1 = 0;
		outy2 = 8;
		pic0 += 2 * (width -8);
		pic1 += 2 * (width -8);
	}
}

/*
 * used for internal jpeg decoding 400 planar to 422
 * args:
 *   out: pointer to data output of idct (macroblocks yyyy )
 *   pic: pointer to picture buffer (yuyv)
 *   width: picture width
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void yuv400pto422(int *out, uint8_t *pic, int width)
{
	int j = 0;
	int outy1 = 0;
	int outy2 = 8;
	uint8_t *pic0 = pic;
	uint8_t *pic1 = pic + width;
	int *outy = out;

	//yyyy
	for (j = 0; j < 4; j++)
	{
		int k = 0;
		for (k = 0; k < 4; k++)
		{
			*pic0++ = CLIP(outy[outy1]);  //y1 line 1
			*pic0++ = 128 ;               //u
			*pic0++ = CLIP(outy[outy1+1]);//y2 line 1
			*pic0++ = 128 ;               //v
			*pic1++ = CLIP(outy[outy2]);  //y1 line 2
			*pic1++ = 128 ;               //u
			*pic1++ = CLIP(outy[outy2+1]);//y2 line 2
			*pic1++ = 128 ;               //v
			outy1 +=2; outy2 +=2;
		}
		outy += 16;
		outy1 = 0;
		outy2 = 8;
		pic0 += 2 * (width -8);
		pic1 += 2 * (width -8);
	}
}

#endif
