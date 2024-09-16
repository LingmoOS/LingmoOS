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

#ifndef GUI_H
#define GUI_H

#include "gviewv4l2core.h"
#define DEF_ACTION_IMAGE  (0)
#define DEF_ACTION_VIDEO  (1)



/*
 * gets the current video codec index
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: current codec index
 */
int get_video_codec_ind();

/*
 * sets the current video codec index
 * args:
 *   index - codec index
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_video_codec_ind(int index);

/*
 * gets the current audio codec index
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: current codec index
 */
int get_audio_codec_ind();

/*
 * sets the current audio codec index
 * args:
 *   index - codec index
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_audio_codec_ind(int index);

/*
 * gets the current fps numerator
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: current fps numerator
 */
//int gui_get_fps_num();

/*
 * gets the current fps denominator
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: current fps denominator
 */
//int gui_get_fps_denom();

/*
 * stores the fps
 * args:
 *   fps - array with fps numerator and denominator
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void gui_set_fps(int fps[2]);

/*
 * gets the control profile file name
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: control profile file name
 */
char *get_profile_name();

/*
 * gets the control profile path (to dir)
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: control profile file name
 */
char *get_profile_path();



/*
 * gets video sufix flag
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: video sufix flag
 */
int get_video_sufix_flag();
/*
 * gets video muxer
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: video muxer
 */
int get_video_muxer();

/*
 * sets video muxer
 * args:
 *   muxer - video muxer (ENCODER_MUX_[MKV|WEBM|AVI])
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_video_muxer(int muxer);

/*
 * gets the video file basename
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: video file basename
 */
char *get_video_name();

/*
 * sets the video file basename
 * args:
 *   name: video file basename
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_video_name(const char *name);

/*
 * gets the video file path (to dir)
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: video file path
 */
char *get_video_path();

/*
 * sets video path (to dir)
 * args:
 *   path: video file path
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_video_path(const char *path);

/*
 * gets photo sufix flag
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: photo sufix flag
 */
int get_photo_sufix_flag();


/*
 * gets photo format
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: photo format
 */
int get_photo_format();

/*
 * sets photo format
 * args:
 *   format - photo format (IMG_FMT_[JPG|BMP|PNG|RAW])
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_photo_format(int format);

/*
 * gets the photo file basename
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: photo file basename
 */
char *get_photo_name();

/*
 * sets the photo file basename and image format
 * args:
 *   name: photo file basename
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_photo_name(const char *name);

/*
 * gets the photo file path (to dir)
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: photo file path
 */
char *get_photo_path();

/*
 * sets photo path (to dir)
 * args:
 *   path: photo file path
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_photo_path(const char *path);
#endif
