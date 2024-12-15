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

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdio.h>
#include "options.h"

#define PACKAGE_LOCALE_DIR "./"
#define GETTEXT_PACKAGE "cheese"
//#define VERSION "cheese-1.0"
#define HAS_QT5 1

#define ENABLE_SDL2 1

#ifndef GETTEXT_PACKAGE_V4L2CORE
#define GETTEXT_PACKAGE_V4L2CORE "gview_v4l2core"
#endif

typedef struct _config_t {
    int  width;      /*width*/
    int  height;     /*height*/
    char *device_name; /*device name*/
    char *device_location; /*device location*/
    unsigned int format;  /*pixelformat - v4l2 fourcc*/
    char render[5];  /*render api*/
    char gui[5];     /*gui api*/
    char audio[6];   /*audio api - none; port; pulse*/
    char capture[5]; /*capture method: read or mmap*/
    char video_codec[5]; /*video codec*/
    char audio_codec[5]; /*video codec*/
    char *profile_path;
    char *profile_name;
    char *video_path;
    char *video_name;
    char *photo_path;
    char *photo_name;
    int video_sufix; /*flag if video file has auto suffix enable*/
    int photo_sufix; /*flag if photo file has auto suffix enable*/
    int fps_num;
    int fps_denom;
    int audio_device;/*audio device index*/
    uint32_t video_fx;
    uint32_t audio_fx;
    uint32_t osd_mask; /*OSD bit mask*/
    uint32_t crosshair_color; /*osd crosshair rgb color (0x00RRGGBB)*/
} config_t;

/*video sufix flag*/
__attribute__((unused))static int video_sufix_flag = 1;

/*photo sufix flag*/
__attribute__((unused))static int photo_sufix_flag = 1;

/*control profile file name*/
__attribute__((unused))static char *profile_name = NULL;

/*divece name*/
__attribute__((unused))static char *device_name = NULL;

/*device location*/
__attribute__((unused))static char *device_location = NULL;

/*control profile path to dir*/
__attribute__((unused))static char *profile_path = NULL;

__attribute__((unused))static int debug_level;
/*
 * get the internal config data
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: pointer to internal config_t struct
 */
config_t *config_get();

/*
 * sets the photo sufix flag
 * args:
 *   flag: photo sufix flag
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_photo_sufix_flag(int flag);

/*
 * sets the device location
 * args:
 *   name: device location
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_device_location(const char *name);

/*
 * sets the device name
 * args:
 *   name: device name
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_device_name(const char *name);

/*
 * sets the control profile file name
 * args:
 *   name: control profile file name
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_profile_name(const char *name);

/*
 * sets the control profile path (to dir)
 * args:
 *   path: control profile path
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_profile_path(const char *path);

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


/*
 * save options to config file
 * args:
 *    filename - config file
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
int config_save(const char *filename);

/*
 * load options from config file
 * args:
 *    filename - config file
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
int config_load(const char *filename);

/*
 * update config data with options data
 * args:
 *    my_options - pointer to options data
 *
 * asserts:
 *    none
 *
 * returns: none
 */

void config_update(options_t *my_options);

/*
 * sets the video sufix flag
 * args:
 *   flag: video sufix flag
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_video_sufix_flag(int flag);

/*
 * cleans internal config allocations
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void config_clean();

#endif
