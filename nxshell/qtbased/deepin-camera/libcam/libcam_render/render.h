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


#ifndef RENDER_H
#define RENDER_H
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
void render_osd_vu_meter(uint8_t *frame, int width, int height, float vu_level[2]);

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
void render_osd_crosshair(uint8_t *frame, int width, int height);

/*
 * Apply fx filters
 * args:
 *    frame - pointer to frame buffer (yuyv format)
 *    width - frame width
 *    height - frame height
 *    mask  - or'ed filter mask
 *
 * asserts:
 *    frame is not null
 *
 * returns: void
 */
void render_fx_apply(uint8_t *frame, int width, int height, uint32_t mask);

#endif