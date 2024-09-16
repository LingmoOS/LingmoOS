/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
#                             Add UYVY color support(Macbook iSight)            #
#           Flemming Frandsen <dren.dk@gmail.com>                               #
#                             Add VU meter OSD                                  #
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

#include <assert.h>
#include <math.h>

#include "gview.h"
#include "gviewrender.h"

extern int verbosity;

typedef struct _yuv_color_t
{
	uint8_t y;
	uint8_t u;
	uint8_t v;
} yuv_color_t;

#define REFERENCE_LEVEL 0.8
#define VU_BARS         20

static float vu_peak[2] = {0.0, 0.0};
static float vu_peak_freeze[2]= {0.0 ,0.0};

/*
 * plot a rectangular box in a yu12 frame (planar)
 * args:
 *   frame - pointer to yu12 frame data
 *   lines - number of lines in frame (height)
 *   linesize - frame line size in pixels (width)
 *   x - box top left x coordinate
 *   y - box top left y coordinate
 *   width - box width
 *   height - box height
 *   color - box color
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void plot_box_yu12(uint8_t *frame, int lines, int linesize, int x, int y, int width, int height, yuv_color_t *color)
{
	uint8_t *py = frame;
	uint8_t *pu = frame + (linesize * lines);
	uint8_t *pv = pu + ((linesize * lines) / 4);

	/*y*/
	int h = 0;
	for(h = 0; h < height; ++h)
	{
		py = frame + x + ((y + h) * linesize);
		int w = 0;
		for(w = 0; w < width; ++w)
		{
			*py++ = color->y;
		}
	}

	/*u v*/
	for(h = 0; h < height/2; h++) /*every two lines*/
	{
		pu = frame + (linesize * lines) + (int) floor(x/2) + (( (int) floor(y/2) + h) * (int) floor(linesize/2));
		pv = pu + (int) floor((linesize * lines) / 4);

		int w = 0;
		for(w = 0; w < width/2; w++) /*every two rows*/
		{
			*pu++ = color->u;
			*pv++ = color->v;
		}
	}
}

/*
 * plot a line in a yu12 frame (planar)
 * args:
 *   frame - pointer to yu12 frame data
 *   lines - number of lines in frame (height)
 *   linesize - frame line size in pixels (width)
 *   x - box top left x coordinate
 *   y - box top left y coordinate
 *   width - line width
 *   color - line color
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void plot_line_yu12(uint8_t *frame, int lines, int linesize, int x, int y, int width, yuv_color_t *color)
{
	uint8_t *py = frame;
	uint8_t *pu = frame + (linesize * lines);
	uint8_t *pv = pu + ((linesize * lines) / 4);

	int w = 0;

	/*y*/
	py = frame + x + (y * linesize);
	for(w = 0; w < width; ++w)
	{
		*py++ = color->y;
	}

	/*u v*/
	pu = frame + (linesize * lines) + (int) floor(x/2) + ((int) floor(y/2) * (int) floor(linesize/2));
	pv = pu + (int) floor((linesize * lines) / 4);
	for(w = 0; w < width/2; w ++) /*every two rows*/
	{
		*pu++ = color->u;
		*pv++ = color->v;
	}
}

/*
 * render a vu meter
 * args:
 *   frame - pointer to yuyv frame data
 *   width - frame width
 *   height - frame height
 *   vu_level - vu level values (array with 2 channels)
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void render_osd_vu_meter(uint8_t *frame, int width, int height, float vu_level[2])
{
	int bw = 2 * (width  / (VU_BARS * 8)); /*make it at least two pixels*/
	int bh = height / 24;

	int channel;
	for (channel = 0; channel < 2; ++channel)
	{
		if((render_get_osd_mask() & REND_OSD_VUMETER_MONO) != 0 && channel > 0)
			continue; /*if mono mode only render first channel*/

		/*make sure we have a positive value (required by log10)*/
		if(vu_level[channel] < 0)
			vu_level[channel] = -vu_level[channel];

		/* Handle peak calculation and falloff */
		if (vu_peak[channel] < vu_level[channel])
		{
			vu_peak[channel] = vu_level[channel];
			vu_peak_freeze[channel] = 30;
		}
		else if (vu_peak_freeze[channel] > 0)
		{
			vu_peak_freeze[channel]--;
  		}
  		else if (vu_peak[channel] > vu_level[channel])
  		{
			vu_peak[channel] -= (vu_peak[channel] - vu_level[channel]) / 10;
  		}

		/*by default no bar is light */
		float dBuLevel = - 4 * (VU_BARS - 1);
		float dBuPeak = - 4 * (VU_BARS - 1);

		if(vu_level[channel] > 0)
			dBuLevel = 10 * log10(vu_level[channel] / REFERENCE_LEVEL);

		if(vu_peak[channel] > 0)
			dBuPeak  = 10 * log10(vu_peak[channel]  / REFERENCE_LEVEL);

  		/* draw the bars */
  		int peaked = 0;
  		int box = 0;
  		for (box = 0; box <= (VU_BARS - 1); ++box)
  		{
			/*
			 * The dB it takes to light the current box
			 * step of 2 db between boxes
			 */
			float db = 2 * (box - (VU_BARS - 1));

			/* start x coordinate for box */
			int bx = box * (bw + 4) + (16);
			/* Start y coordinate for box (box top)*/
			int by = channel * (bh + 4) + bh;

			yuv_color_t color;
			color.y = 127;
			color.u = 127;
			color.v = 127;

			/*green bar*/
			if (db < -10)
			{
					color.y = 154;
  				color.u = 72;
  				color.v = 57;
			}
			else if (db < -2) /*yellow bar*/
			{
					color.y = 203;
  				color.u = 44;
  				color.v = 142;
			}
			else /*red bar*/
			{
				color.y = 107;
				color.u = 100;
				color.v = 212;
			}

			int light = dBuLevel > db;
			if (dBuPeak < db+1 && !peaked)
			{
  				peaked = 1;
  				light = 1;
			}

			if (light)
				plot_box_yu12(frame, height, width, bx, by, bw, bh, &color);
			else if (bw > 0) /*draw single line*/
				plot_line_yu12(frame, height, width, bx, by + (bh /2), bw, &color);
		}
  	}
}
