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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <libintl.h>


#include "gviewv4l2core.h"
#include "gview.h"
#include "core_io.h"
#include "gui.h"
#include "cameraconfig.h"

#define MAXLINE 100 /*100 char lines max*/

extern int debug_level;

static config_t my_config =
{
    .width = 640,
    .height = 480,
    .device_name = NULL,
    .device_location = NULL,
    .format = V4L2_PIX_FMT_MJPEG,
	.render = "sdl",
	.gui = "qt5",
	.audio = "port",
	.capture = "mmap",
    .video_codec = "mjpg",/*yuy2,mjpg,mpeg,flv1,wmv1,mpg2,mp43,dx50,h264,hevc,vp80,vp90,theo*/
    .audio_codec = "aac",
    .profile_name = "default.gpfl",
	.profile_path = NULL,
    .video_name = NULL,
	.video_path = NULL,
	.photo_name = NULL,
	.photo_path = NULL,
	.video_sufix = 1,
	.photo_sufix = 1,
	.fps_num = 1,
    .fps_denom = 30,
    .audio_device = -1,/*will use API default in this case*/
	.video_fx = 0, /*no video fx*/
	.audio_fx = 0, /*no audio fx*/
	.osd_mask = 0, /*REND_OSD_NONE*/
	.crosshair_color=0x0000FF00, /*osd crosshair rgb color (0x00RRGGBB)*/
};

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
int config_save(const char *filename)
{
	FILE *fp;

	/*open file for write*/
	if((fp = fopen(filename,"w")) == NULL)
	{
        fprintf(stderr, "deepin-camera: couldn't open %s for write: %s\n", filename, strerror(errno));
		return -1;
	}

	/* use c locale - make sure floats are writen with a "." and not a "," */
    setlocale(LC_NUMERIC, "C");

	/*write config data*/
//	fprintf(fp, "#Deepin-camera %s config file\n", VERSION);
	fprintf(fp, "\n");
	fprintf(fp, "#video input width\n");
	fprintf(fp, "width=%i\n", my_config.width);
	fprintf(fp, "#video input height\n");
	fprintf(fp, "height=%i\n", my_config.height);
    fprintf(fp, "#device name\n");
    fprintf(fp, "device_name=%s\n",my_config.device_name);
    fprintf(fp, "#device location\n");
    fprintf(fp, "device_location=%s\n",my_config.device_location);
	fprintf(fp, "#video input format\n");
	fprintf(fp, "v4l2_format=%u\n", my_config.format);
	fprintf(fp, "#video input capture method\n");
	fprintf(fp, "capture=%s\n", my_config.capture);
	fprintf(fp, "#audio api\n");
	fprintf(fp, "audio=%s\n", my_config.audio);
	fprintf(fp, "#gui api\n");
	fprintf(fp, "gui=%s\n", my_config.gui);
	fprintf(fp, "#render api\n");
	fprintf(fp, "render=%s\n", my_config.render);
	fprintf(fp, "#video codec [raw mjpg mpeg flv1 wmv1 mpg2 mp43 dx50 h264 vp80 theo]\n");
	fprintf(fp, "video_codec=%s\n", my_config.video_codec);
	fprintf(fp, "#audio codec [pcm mp2 mp3 aac ac3 vorb]\n");
	fprintf(fp, "audio_codec=%s\n", my_config.audio_codec);
	fprintf(fp, "#profile name\n");
	fprintf(fp, "profile_name=%s\n", my_config.profile_name);
	fprintf(fp, "#profile path\n");
	fprintf(fp, "profile_path=%s\n", my_config.profile_path);
	fprintf(fp, "#video name\n");
	fprintf(fp, "video_name=%s\n", my_config.video_name);
	fprintf(fp, "#video path\n");
	fprintf(fp, "video_path=%s\n", my_config.video_path);
	fprintf(fp, "#video sufix flag\n");
	fprintf(fp, "video_sufix=%i\n", my_config.video_sufix);
	fprintf(fp, "#photo name\n");
	fprintf(fp, "photo_name=%s\n", my_config.photo_name);
	fprintf(fp, "#photo path\n");
	fprintf(fp, "photo_path=%s\n", my_config.photo_path);
	fprintf(fp, "#photo sufix flag\n");
	fprintf(fp, "photo_sufix=%i\n", my_config.photo_sufix);
	fprintf(fp, "#fps numerator (def. 1)\n");
	fprintf(fp, "fps_num=%i\n", my_config.fps_num);
	fprintf(fp, "#fps denominator (def. 25)\n");
	fprintf(fp, "fps_denom=%i\n", my_config.fps_denom);
	fprintf(fp, "#audio device index (-1 - api default)\n");
	fprintf(fp, "audio_device=%i\n", my_config.audio_device);
	fprintf(fp, "#video fx mask \n");
	fprintf(fp, "video_fx=0x%x\n", my_config.video_fx);
	fprintf(fp, "#audio fx mask \n");
	fprintf(fp, "audio_fx=0x%x\n", my_config.audio_fx);
	fprintf(fp, "#OSD mask \n");
	fprintf(fp, "osd_mask=0x%x\n", my_config.osd_mask);
	fprintf(fp, "crosshair_color=0x%x\n", my_config.crosshair_color);

	/* return to system locale */
    setlocale(LC_NUMERIC, "");

	/* flush stream buffers to filesystem */
	fflush(fp);

	/* close file after fsync (sync file data to disk) */
	if (fsync(fileno(fp)) || fclose(fp))
	{
        fprintf(stderr, "deeepin_camera: error writing configuration data to file: %s\n", strerror(errno));
		return -1;
	}

	if(debug_level > 1)
        printf("deeepin_camera: saving config to %s\n", filename);

	return 0;
}

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
int config_load(const char *filename)
{
	FILE *fp;
	char bufr[MAXLINE];
	int line = 0;

	/*open file for read*/
	if((fp = fopen(filename,"r")) == NULL)
	{
        fprintf(stderr, "deepin-camera: couldn't open %s for read: %s\n", filename, strerror(errno));
		return -1;
	}

	while(fgets(bufr, MAXLINE, fp) != NULL)
	{
		line++;
		char *bufp = bufr;
		/*parse config line*/

		/*trim leading and trailing spaces and newline*/
		trim_leading_wspaces(bufp);
		trim_trailing_wspaces(bufp);

		/*skip empty or commented lines */
		int size = strlen(bufp);
		if(size < 1 || *bufp == '#')
		{
			if(debug_level > 1)
                printf("deepin-camera: (config) empty or commented line (%i)\n", line);
			continue;
		}

		char *token = NULL;
		char *value = NULL;

		char *sp = strrchr(bufp, '=');

		if(sp)
		{
            long size = sp - bufp;
            token = strndup(bufp, (ulong)size);
			trim_leading_wspaces(token);
			trim_trailing_wspaces(token);

			value = strdup(sp + 1);
			trim_leading_wspaces(value);
			trim_trailing_wspaces(value);
		}

		/*skip invalid lines */
		if(!token || !value || strlen(token) < 1 || strlen(value) < 1)
		{
            fprintf(stderr, "deepin-camera: (config) skiping invalid config entry at line %i\n", line);
			if(token)
				free(token);
			if(value)
				free(value);
			continue;
		}

		/*check tokens*/
		if(strcmp(token, "width") == 0)
			my_config.width = (int) strtoul(value, NULL, 10);
		else if(strcmp(token, "height") == 0)
			my_config.height = (int) strtoul(value, NULL, 10);
        else if(strcmp(token, "device_name") == 0 && strlen(value) > 0)
        {
            if(my_config.device_name)
                free(my_config.device_name);
            my_config.device_name = strdup(value);
            set_device_name(value);
        }
        else if(strcmp(token, "device_location") == 0 && strlen(value) > 0)
        {
            if(my_config.device_location)
                free(my_config.device_location);
            my_config.device_location = strdup(value);
            set_device_location(value);
        }
        else if(strcmp(token, "v4l2_format") == 0)
            my_config.format = (uint32_t) strtoul("V4L2_PIX_FMT_MJPEG", NULL, 10);
//		else if(strcmp(token, "capture") == 0)
//			strncpy(my_config.capture, value, 4);
//		else if(strcmp(token, "audio") == 0)
//			strncpy(my_config.audio, value, 5);
//		else if(strcmp(token, "gui") == 0)
//			strncpy(my_config.gui, value, 4);
//		else if(strcmp(token, "render") == 0)
//			strncpy(my_config.render, value, 4);
//		else if(strcmp(token, "video_codec") == 0)
//			strncpy(my_config.video_codec, value, 4);
//		else if(strcmp(token, "audio_codec") == 0)
//			strncpy(my_config.audio_codec, value, 4);
//		else if(strcmp(token, "profile_name") == 0 && strlen(value) > 2)
//		{
//			if(my_config.profile_name)
//				free(my_config.profile_name);
//			my_config.profile_name = strdup(value);
//            set_profile_name(value);
//		}
//		else if(strcmp(token, "profile_path") == 0)
//		{
//			if(my_config.profile_path)
//				free(my_config.profile_path);
//			my_config.profile_path = strdup(value);
//            set_profile_path(value);
//		}
        else if(strcmp(token, "video_name") == 0  && strlen(value) > 2)
        {
            if(my_config.video_name)
                free(my_config.video_name);
            my_config.video_name = strdup(value);
        }
        else if(strcmp(token, "video_path") == 0)
        {
            if(my_config.video_path)
                free(my_config.video_path);
            my_config.video_path = strdup(value);
        }
        else if(strcmp(token, "photo_name") == 0  && strlen(value) > 2)
        {
            if(my_config.photo_name)
                free(my_config.photo_name);
            my_config.photo_name = strdup(value);
        }
        else if(strcmp(token, "photo_path") == 0)
        {
            if(my_config.photo_path)
                free(my_config.photo_path);
            my_config.photo_path = strdup(value);
        }
//		else if(strcmp(token, "video_sufix") == 0)
//		{
//			my_config.video_sufix = (int) strtoul(value, NULL, 10);
//            set_video_sufix_flag(my_config.video_sufix);
//		}
//		else if(strcmp(token, "photo_sufix") == 0)
//		{
//			my_config.photo_sufix = (int) strtoul(value, NULL, 10);
//            set_photo_sufix_flag(my_config.photo_sufix);
//		}
//		else if(strcmp(token, "fps_num") == 0)
//			my_config.fps_num = (int) strtoul(value, NULL, 10);
//		else if(strcmp(token, "fps_denom") == 0)
//			my_config.fps_denom = (int) strtoul(value, NULL, 10);
//		else if(strcmp(token, "audio_device") == 0)
//			my_config.audio_device = (int) strtoul(value, NULL, 10);
//		else if(strcmp(token, "video_fx") == 0)
//			my_config.video_fx = (uint32_t) strtoul(value, NULL, 16);
//		else if(strcmp(token, "audio_fx") == 0)
//			my_config.audio_fx = (uint32_t) strtoul(value, NULL, 16);
//		else if(strcmp(token, "osd_mask") == 0)
//			my_config.osd_mask = (uint32_t) strtoul(value, NULL, 16);
//		else if(strcmp(token, "crosshair_color") == 0)
//			my_config.crosshair_color = (uint32_t) strtoul(value, NULL, 16);
//		else
//            fprintf(stderr, "deepin-camera: (config) skiping invalid entry at line %i ('%s', '%s')\n", line, token, value);

		if(token)
			free(token);
		if(value)
			free(value);
	}

	//if(errno)
	//{
    //	fprintf(stderr, "deepin-camera: couldn't read line %i of config file: %s\n", line, strerror(errno));
	//	fclose(fp);
	//	return -1;
	//}

	fclose(fp);
	return 0;
}

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
void config_update(options_t *my_options)
{
	/*check for resolution options*/
	if(my_options->width > 0)
		my_config.width = my_options->width;
	if(my_options->height > 0)
		my_config.height = my_options->height;

	/*check for resolution options*/
	if(my_options->fps_num > 0)
		my_config.fps_num = my_options->fps_num;
	if(my_options->fps_denom > 0)
		my_config.fps_denom = my_options->fps_denom;

	/*capture method*/
	if(strlen(my_options->capture) > 3)
		strncpy(my_config.capture, my_options->capture, 4);

	/*render API*/
	if(strlen(my_options->render) > 2)
		strncpy(my_config.render, my_options->render, 4);

	/*gui API*/
	if(strlen(my_options->gui) > 2)
		strncpy(my_config.gui, my_options->gui, 4);

	/*audio API*/
	if(strlen(my_options->audio) > 3)
		strncpy(my_config.audio, my_options->audio, 5);
	
	/*audio device*/
	if(my_options->audio_device >= 0)
		my_config.audio_device = my_options->audio_device;

	/*input format*/
	if(strlen(my_options->format) > 2)
	{
		//convert to v4l2_format
		my_config.format = v4l2core_fourcc_2_v4l2_pixelformat(my_options->format);
	}

	/*video codec*/
	if(strlen(my_options->video_codec) > 2)
		strncpy(my_config.video_codec, my_options->video_codec, 4);

	/*audio codec*/
	if(strlen(my_options->audio_codec) > 2)
		strncpy(my_config.audio_codec, my_options->audio_codec, 4);

	/*profile*/
	if(my_options->profile_name)
	{
		if(my_config.profile_name)
			free(my_config.profile_name);
		my_config.profile_name = strdup(my_options->profile_name);
	}
	if(my_options->profile_path)
	{
		if(my_config.profile_path)
			free(my_config.profile_path);
		my_config.profile_path = strdup(my_options->profile_path);
	}

	/*video file*/
	if(my_options->video_name)
	{
		if(my_config.video_name)
			free(my_config.video_name);
		my_config.video_name = strdup(my_options->video_name);
	}
	if(my_options->video_path)
	{
		if(my_config.video_path)
			free(my_config.video_path);
		my_config.video_path = strdup(my_options->video_path);
	}

	/*photo*/
	if(my_options->photo_name)
	{
		if(my_config.photo_name)
			free(my_config.photo_name);
		my_config.photo_name = strdup(my_options->photo_name);
	}
	if(my_options->photo_path)
	{
		if(my_config.photo_path)
			free(my_config.photo_path);
		my_config.photo_path = strdup(my_options->photo_path);
	}

}

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
void config_clean()
{
	if(my_config.profile_name != NULL)
		free(my_config.profile_name);

	if(my_config.profile_path != NULL)
		free(my_config.profile_path);

	if(my_config.video_name != NULL)
		free(my_config.video_name);

	if(my_config.video_path != NULL)
		free(my_config.video_path);

	if(my_config.photo_name != NULL)
		free(my_config.photo_name);

	if(my_config.photo_path != NULL)
		free(my_config.photo_path);
    if(my_config.device_name != NULL)
        {
            free(my_config.device_name);
        }
    if(my_config.device_location != NULL)
        {
            free(my_config.device_location);
        }
}

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
config_t *config_get()
{
    return &my_config;
}
