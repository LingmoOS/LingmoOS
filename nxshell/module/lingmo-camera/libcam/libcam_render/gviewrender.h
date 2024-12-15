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

/*******************************************************************************#
#                                                                               #
#  Render library                                                               #
#                                                                               #
********************************************************************************/

#ifndef GVIEWRENDER_H
#define GVIEWRENDER_H

#include <features.h>

#include <inttypes.h>
#include <sys/types.h>

/*make sure we support c++*/
__BEGIN_DECLS

#define RENDER_NONE     (0)
#define RENDER_SDL      (1)
#define RENDER_SFML     (2)

#define EV_QUIT      (0)
#define EV_KEY_UP    (1)
#define EV_KEY_DOWN  (2)
#define EV_KEY_LEFT  (3)
#define EV_KEY_RIGHT (4)
#define EV_KEY_SPACE (5)
#define EV_KEY_I     (6)
#define EV_KEY_V     (7)

/*FX FILTER FLAGS*/
#define REND_FX_YUV_NOFILT (0)
#define REND_FX_YUV_MIRROR (1<<0)
#define REND_FX_YUV_UPTURN (1<<1)
#define REND_FX_YUV_NEGATE (1<<2)
#define REND_FX_YUV_MONOCR (1<<3)
#define REND_FX_YUV_PIECES (1<<4)
#define REND_FX_YUV_PARTICLES (1<<5)
#define REND_FX_YUV_HALF_MIRROR (1<<6)
#define REND_FX_YUV_HALF_UPTURN (1<<7)
#define REND_FX_YUV_SQRT_DISTORT (1<<8)
#define REND_FX_YUV_POW_DISTORT (1<<9)
#define REND_FX_YUV_POW2_DISTORT (1<<10)
#define REND_FX_YUV_BLUR (1<<11)
#define REND_FX_YUV_BLUR2 (1<<12)

/*OSD FLAGS*/
#define REND_OSD_NONE           (0)
#define REND_OSD_VUMETER_MONO   (1<<0)
#define REND_OSD_VUMETER_STEREO (1<<1)
#define REND_OSD_CROSSHAIR      (1<<2)

typedef int (*render_event_callback)(void *data);

typedef struct _render_events_t
{
	int id;
	render_event_callback callback;
	void *data;

} render_events_t;
/*
 * set verbosity
 * args:
 *   value - verbosity value
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void render_set_verbosity(int value);

/*
 * set the osd mask
 * args:
 *   mask - osd mask (ored)
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void render_set_osd_mask(uint32_t mask);

/*
 * get the osd mask
 * args:
 *   none
 *
 * asserts:
 *    none
 *
 * returns: osd mask
 */
uint32_t render_get_osd_mask();

/*
 * set the osd crosshair color
 * args:
 *   rgb_color - 0x00RRGGBB
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void render_set_crosshair_color(uint32_t rgb_color);

/*
 * get the osd crosshair color
 * args:
 *   none
 *
 * asserts:
 *    none
 *
 * returns: osd rgb color
 */
uint32_t render_get_crosshair_color();

/*
 * get render width
 * args:
 *   none
 *
 * asserts:
 *    none
 *
 * returns: render width
 */
int render_get_width();

/*
 * get render height
 * args:
 *   none
 *
 * asserts:
 *    none
 *
 * returns: render height
 */
int render_get_height();

/*
 * render initialization
 * args:
 *   render - render API to use (RENDER_NONE, RENDER_SDL1, ...)
 *   width - render width
 *   height - render height
 *   flags - window flags:
 *              0- none
 *              1- fullscreen
 *              2- maximized
 *   win_w - window width (0 use render width)
 *   win_h - window height (0 use render height)
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int render_init(int render, int width, int height, int flags, int win_w, int win_h);

/*
 * set caption
 * args:
 *   caption - string with render window caption
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void render_set_caption(const char* caption);

/*
 * render a frame
 * args:
 *   frame - pointer to frame data (yuyv format)
 *
 * asserts:
 *   frame is not null
 *
 * returns: error code
 */
int render_frame(uint8_t *frame);

/*
 * get event index on render_events_list
 * args:
 *    id - event id
 *
 * asserts:
 *    none
 *
 * returns: event index or -1 on error
 */
int render_get_event_index(int id);

/*
 * set event callback
 * args:
 *    id - event id
 *    callback_function - pointer to callback function
 *    data - pointer to user data (passed to callback)
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
int render_set_event_callback(int id, render_event_callback callback_function, void *data);

/*
 * call the event callback for event id
 * args:
 *    id - event id
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
int render_call_event_callback(int id);

/*
 * Apply fx filters
 * args:
 *    frame - pointer to frame buffer (yuyv format)
 *    mask  - or'ed filter mask
 *
 * asserts:
 *    frame is not null
 *
 * returns: void
 */
void render_frame_fx(uint8_t *frame, uint32_t mask);

/*
 * Apply OSD mask
 * args:
 *    frame - pointer to frame buffer (yuyv format)
 *
 * asserts:
 *    frame is not null
 *
 * returns: void
 */
void render_frame_osd(uint8_t *frame);

/*
 * set the vu level for the osd vu meter
 * args:
 *   vu_level - vu level value (2 channel array)
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void render_set_vu_level(float vu_level[2]);

/*
 * get the vu level for the osd vu meter
 * args:
 *   vu_level - two channel array were vu_level is to be copied
 *
 * asserts:
 *   none
 *
 * returns array with vu meter level
 */
void render_get_vu_level(float vu_level[2]);

/*
 * clean fx filters
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: void
 */
void render_clean_fx();

/*
 * clean render data
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void render_close();

__END_DECLS

#endif
