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

#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct _options_t
{
	int  verbosity;  /*verbosity level*/
	char device[30]; /*device name*/
	int  width;      /*width*/
	int  height;     /*height*/
	int fps_num;     /*fps numerator*/
	int fps_denom;   /*fps denominator*/
	int  control_panel; /*flag control panel mode*/
	int  disable_libv4l2; /*set to 1 to disable libv4l2 calls*/
	char format[5];  /*pixelformat fourcc*/
	char render[5];  /*render api*/
	char gui[5];     /*gui api*/
	char audio[6];   /*audio api - none; port; pulse*/
	int audio_device; /*audio device index 0..N (-1 = default)*/
	char capture[5]; /*capture method: read or mmap*/
	char audio_codec[5]; /*audio codec*/
	char video_codec[5]; /*video codec*/
	char *prof_filename; /*profile_filename (if set load it on start)*/
	char *profile_name;
	char *profile_path;
	char *video_name;
	char *video_path;
	char *photo_name;
	char *photo_path;
	double video_timer; /*video capture time in seconds (double)*/
	double photo_timer; /*photo capture timer interval in seconds (double)*/
	int photo_npics; /*number of photo captures*/
	int exit_on_term; /*flag if we should exit after video or image capture ends*/
	char render_flag[5]; /*render window flag => default (none) | FULLSCREEN (full) | MAXIMIZED (max)*/
	int render_width; //render window width (default 0), if set, render window flag is none
	int render_height; //render window height (default 0), if set, render window flag is none
} options_t;

/*
 * get the internal options data
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: pointer to internal options_t struct
 */
options_t *options_get();

/*
 * parses the command line options
 * args:
 *   argc - number of comman line args
 *   argv - pointer to list of command line args
 *
 * asserts:
 *   none
 *
 * returns: int (if > 0 app should terminate (help, version)
 */
int options_parse(int argc, char *argv[]);

/*
 * cleans internal options allocations
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void options_clean();

#endif
