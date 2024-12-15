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
#include <stdio.h>

#include "gview.h"
#include "gviewrender.h"

extern int verbosity;

typedef struct _yuv_color_t
{
	uint8_t y;
	uint8_t u;
	uint8_t v;
} yuv_color_t;

/*
 * plot a crosshair in a yu12 frame (planar)
 * args:
 *   frame - pointer to yu12 frame data
 *   size  - frame line size in pixels (width)
 *   width - width
 *   height - height 
 *   color - line color
 *
 * asserts:
 *   none
 *
 * returns: none
 */
static void plot_crosshair_yu12(uint8_t *frame, int size, int width, int height, yuv_color_t *color)
{
	uint8_t *py = frame;
	uint8_t *pu = frame + (width * height);
	uint8_t *pv = pu + ((width * height) / 4);

	/*y - 1st vertical line*/
	int h = (height-size)/2;
	for(h = (height-size)/2; h < height/2 - 2; h++)
	{
		py = frame + (h * width) + width/2;
		*py = color->y;
	}
	/*y - 1st horizontal line*/
	int w = (width-size)/2;
	for(w = (width-size)/2; w < width/2 - 2; w++)
	{
		py = frame + ((height/2) * width) + w;
		*py = color->y;
	}
	/*y - 2nd horizontal line*/
	for(w = width/2 + 2; w < (width+size)/2; w++)
	{
		py = frame + ((height/2) * width) + w;
		*py = color->y;
	}
	/*y - 2nd vertical line*/
	for(h = height/2 + 2; h < (height+size)/2; h++)
	{
		py = frame + (h * width) + width/2;
		*py = color->y;
	}

				
	/*u v - 1st vertical line*/
	for(h = (height-size)/4; h < height/4 - 1; h++) /*every two rows*/
	{
		pu = frame + (width * height) + (h * width/2) + width/4;
		*pu = color->u;
		pv = pu + (width * height)/4;
		*pv = color->v;
	}
	/*u v - 1st horizontal line*/
	for(w = (width-size)/4; w < width/4 - 1; w++) /*every two rows*/
	{
		pu = frame + (width * height) + ((height/4) * width/2) + w;
		*pu = color->u;
		pv = pu + (width * height)/4;
		*pv = color->v;
	}
	/*u v - 2nd horizontal line*/
	for(w = width/4 + 1; w < (width+size)/4; w++) /*every two rows*/
	{
		pu = frame + (width * height) + ((height/4) * width/2) + w;
		*pu = color->u;
		pv = pu + (width * height)/4;
		*pv = color->v;
	}
	/*u v - 2nd vertical line*/
	for(h = height/4 + 1; h < (height+size)/4; h++) /*every two rows*/
	{
		pu = frame + (width * height) + (h * width/2) + width/4;
		*pu = color->u;
		pv = pu + (width * height)/4;
		*pv = color->v;
	}
}

/*
 * render a crosshair
 * args:
 *   frame - pointer to yuyv frame data
 *   width - frame width
 *   height - frame height
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void render_osd_crosshair(uint8_t *frame, int width, int height)
{
	yuv_color_t color;
	color.y = 0;
	color.u = 0;
	color.v = 0;

	uint32_t rgb_color = render_get_crosshair_color();

	uint8_t r = (uint8_t) ((rgb_color & 0x00FF0000) >> 16);
	uint8_t g = (uint8_t) ((rgb_color & 0x0000FF00) >> 8);
	uint8_t b = (uint8_t) (rgb_color & 0x000000FF);

	color.y = CLIP(0.299*(r-128) + 0.587*(g-128) + 0.114*(b-128) + 128) ;
	color.u = CLIP(-0.147*(r-128) - 0.289*(g-128) + 0.436*(b-128) + 128);
	color.v = CLIP(0.615*(r-128) - 0.515*(g-128) - 0.100*(b-128) + 128);

	plot_crosshair_yu12(frame, 24, width, height, &color);
}
