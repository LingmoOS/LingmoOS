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
#include <getopt.h>

#include "gviewv4l2core.h"
#include "gview.h"
#include "core_io.h"
#include "gui.h"
#include "options.h"
#include "cameraconfig.h"

typedef struct _opt_values_t
{
	char opt_short;
	char opt_long[20];
	int  req_arg;
	char opt_help_arg[20];
	char opt_help[80];
} opt_values_t;

static opt_values_t opt_values[] =
{
	{
		.opt_short = 'h',
		.opt_long = "help",
		.req_arg = 0,
		.opt_help_arg = "",
		.opt_help = N_("Print help")
	},
	{
		.opt_short = 'v',
		.opt_long = "version",
		.req_arg = 0,
		.opt_help_arg = "",
		.opt_help = N_("Print version"),
	},
	{
		.opt_short = 'w',
		.opt_long = "verbosity",
		.req_arg = 1,
		.opt_help_arg = N_("LEVEL"),
		.opt_help = N_("Set Verbosity level (def: 0)")
	},
	{
		.opt_short = 'd',
		.opt_long = "device",
		.req_arg = 1,
		.opt_help_arg = N_("DEVICE"),
		.opt_help = N_("Set device name (def: /dev/video0)"),
	},
	{
		.opt_short = 'c',
		.opt_long = "capture",
		.req_arg = 1,
		.opt_help_arg = N_("METHOD"),
		.opt_help = N_("Set capture method [read | mmap (def)]"),
	},
	{
		.opt_short = 'b',
		.opt_long = "disable_libv4l2",
		.req_arg = 0,
		.opt_help_arg = "",
		.opt_help = N_("disable calls to libv4l2"),
	},
	{
		.opt_short = 'x',
		.opt_long = "resolution",
		.req_arg = 1,
		.opt_help_arg = N_("WIDTHxHEIGHT"),
		.opt_help = N_("Request resolution (e.g 640x480)")
	},
	{
		.opt_short = 'f',
		.opt_long = "format",
		.req_arg = 1,
		.opt_help_arg = N_("FOURCC"),
		.opt_help = N_("Request format (e.g MJPG)")
	},
	{
		.opt_short = 'F',
		.opt_long = "fps",
		.req_arg = 1,
		.opt_help_arg = N_("FPS_NUM[/FPS_DENOM]"),
		.opt_help = N_("Request fps (e.g 25 ; 7/3; ...)")
	},
	{
		.opt_short = 'm',
		.opt_long = "render_window",
		.req_arg = 1,
		.opt_help_arg = N_("RENDER_WINDOW_FLAGS"),
		.opt_help = N_("Set render window flags (e.g none; full; max; WIDTHxHEIGHT)")
	},
	{
		.opt_short = 'a',
		.opt_long = "audio",
		.req_arg = 1,
		.opt_help_arg = N_("AUDIO_API"),
		.opt_help = N_("Select audio API (e.g none; port; pulse)")
	},
	{
		.opt_short = 'k',
		.opt_long = "audio_device",
		.req_arg = 1,
		.opt_help_arg = N_("AUDIO_DEVICE"),
		.opt_help = N_("Select audio device index for selected api (0..N)")
	},
	{
		.opt_short = 'o',
		.opt_long = "audio_codec",
		.req_arg = 1,
		.opt_help_arg = N_("CODEC"),
		.opt_help = N_("Audio codec [pcm mp2 mp3 aac ac3 vorb]")
	},
	{
		.opt_short = 'u',
		.opt_long = "video_codec",
		.req_arg = 1,
		.opt_help_arg = N_("CODEC"),
		.opt_help = N_("Video codec [raw mjpg mpeg flv1 wmv1 mpg2 mp43 dx50 h264 vp80 theo]")
	},
	{
		.opt_short = 'p',
		.opt_long = "profile",
		.req_arg = 1,
		.opt_help_arg = N_("FILENAME"),
		.opt_help = N_("load control profile")
	},
	{
		.opt_short = 'j',
		.opt_long = "video",
		.req_arg = 1,
		.opt_help_arg = N_("FILENAME"),
		.opt_help = N_("filename for captured video)")
	},
	{
		.opt_short = 'i',
		.opt_long = "image",
		.req_arg = 1,
		.opt_help_arg = N_("FILENAME"),
		.opt_help = N_("filename for captured image)")
	},
	{
		.opt_short = 'y',
		.opt_long = "video_timer",
		.req_arg = 1,
		.opt_help_arg = N_("TIME_IN_SEC"),
		.opt_help = N_("time (double) in sec. for video capture)")
	},
	{
		.opt_short = 't',
		.opt_long = "photo_timer",
		.req_arg = 1,
		.opt_help_arg = N_("TIME_IN_SEC"),
		.opt_help = N_("time (double) in sec. between captured photos)")
	},
	{
		.opt_short = 'n',
		.opt_long = "photo_total",
		.req_arg = 1,
		.opt_help_arg = N_("TOTAL"),
		.opt_help = N_("total number of captured photos)")
	},
	{
		.opt_short = 'e',
		.opt_long = "exit_on_term",
		.req_arg = 0,
		.opt_help_arg = "",
		.opt_help = N_("exit app after video or image capture ends")
	},
	{
		.opt_short = 'z',
		.opt_long = "control_panel",
		.req_arg = 0,
		.opt_help_arg = "",
		.opt_help = N_("Start in control panel mode")
	},
	{
		.opt_short = 0,
		.opt_long = "",
		.req_arg = 0,
		.opt_help_arg = "",
		.opt_help = ""
	},
};

static options_t my_options =
{
    .verbosity = 3,
    .device = "",
	.width = 0,
	.height = 0,
	.control_panel = 0,
	.disable_libv4l2 = 0,
    .format = "MJPG",
	.render = "",
	.gui = "",
	.audio = "",
	.audio_device = -1, /*use default*/
	.capture = "",
	.video_codec = "",
	.audio_codec = "",
	.prof_filename = NULL,
	.profile_name = NULL,
	.profile_path = NULL,
    .video_name = "",
    .video_path = "",
    .photo_name = "",
    .photo_path = "",
	.video_timer = 0,
	.photo_timer = 0,
	.photo_npics = 0,
	.exit_on_term = 0,
	.render_flag = "none",
	.render_width = 0,
	.render_height = 0
};

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
options_t *options_get()
{
	return &my_options;
}

/*
 * prints the number of command line options
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: last valid index of opt_values
 */
int opt_get_number()
{
	int i = 0;

	/*long option must always be set*/
	do
	{
		i++;
	}
	while(strlen(opt_values[i].opt_long) > 0);

	return i;
}

/*
 * gets the max length of help string (up to end of opt_help_arg)
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: max lenght
 */
static int opt_get_help_max_len()
{
	int i = 0;

	int max_len = 0;

	/*long option must always be set*/
	do
	{
		int len = 5 + /*-c, and --*/
			  strlen(opt_values[i].opt_long);
		if(strlen(opt_values[i].opt_help_arg) > 0)
			len += strlen(opt_values[i].opt_help_arg) + 1; /*add =*/
		if(len > max_len)
				max_len = len;
		i++;
	}
	while(strlen(opt_values[i].opt_long) > 0);


	return max_len;
}

/*
 * prints the command line help
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void opt_print_help()
{
//	printf(_("Deepin-camera version %s\n\n"), VERSION);
	printf(_("Usage:\n   Deepin-camera [OPTIONS]\n\n"));
	printf(_("OPTIONS:\n"));

	int max_len = opt_get_help_max_len();
	int len = 0;

	int i = 0;

	/*long option must always be set*/
	do
	{
		if(opt_values[i].opt_short > 0)
		{
			len = 3;
			printf("-%c,", opt_values[i].opt_short);
		}

		printf("--%s", opt_values[i].opt_long);
		len += strlen(opt_values[i].opt_long) + 2;

		if(strlen(opt_values[i].opt_help_arg) > 0)
		{
			len += strlen(opt_values[i].opt_help_arg) + 1;
			printf("=%s", _(opt_values[i].opt_help_arg));
		}

		int spaces = max_len - len;
		int j = 0;
		for(j=0; j < spaces; j++)
			printf(" ");

		if(strlen(opt_values[i].opt_help) > 0)
			printf("\t:%s\n", _(opt_values[i].opt_help));

		i++;
	}
	while(strlen(opt_values[i].opt_long) > 0);
}

/*
 * prints the version info
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void opt_print_version()
{
//	printf("Deepin-camera version %s\n", VERSION);
}

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
int options_parse(int argc, char *argv[])
{
	int ret = 0;
	int long_index =0;
	char *stopstring;

	int n_options = opt_get_number();

	struct option long_options[n_options + 1];

	char opt_string[128] = "";
	char *opt_str_ptr = opt_string;

	int i =0;
	for(i=0; i < n_options; i++)
	{
		long_options[i].name = opt_values[i].opt_long;
		long_options[i].has_arg = opt_values[i].req_arg > 0 ? required_argument: no_argument;
		long_options[i].flag = NULL;
		long_options[i].val = opt_values[i].opt_short;

		/*set opt string (be carefull we don't exceed size)*/
		if(opt_str_ptr - opt_string < 128 - 3)
		{
			*opt_str_ptr++ = opt_values[i].opt_short;
			if(opt_values[i].req_arg > 0)
				*opt_str_ptr++ = ':';
		}
	}

	long_options[n_options].name = 0;
	long_options[n_options].has_arg = 0;
	long_options[n_options].flag = NULL;
	long_options[n_options].val= 0;

	*opt_str_ptr++='\0'; /*null terminated string*/

	int opt = 0;

	while ((opt = getopt_long(argc, argv, opt_string,
		long_options, &long_index )) != -1)
	{
		switch (opt)
		{
			case 'v' :
				opt_print_version();
				ret = 1;
				break;

			case 'w':
				my_options.verbosity = atoi(optarg);
				break;

			case 'd':
			{
				int str_size = strlen(optarg);
				if(str_size > 1) /*device needs at least 2 chars*/
					strncpy(my_options.device, optarg, 29);
				else
					fprintf(stderr, "V4L2_CORE: (options) Error in device usage: -d[--device] DEVICENAME \n");
				break;
			}
			case 'z':
			{
				my_options.control_panel = 1;
				break;
			}
			case 'c':
			{
				int str_size = strlen(optarg);
				if(str_size == 4) /*capture method*/
					strncpy(my_options.capture, optarg, 4);
				break;
			}
			case 'b':
			{
				my_options.disable_libv4l2 = 1;
				break;
			}
			case 'x':
				my_options.width = (int) strtoul(optarg, &stopstring, 10);
				if( *stopstring != 'x')
				{
					fprintf(stderr, "V4L2_CORE: (options) Error in resolution usage: -x[--resolution] WIDTHxHEIGHT \n");
				}
				else
				{
					++stopstring;
					my_options.height = (int) strtoul(stopstring, &stopstring, 10);
				}
				if(my_options.width <= 0)
					my_options.width = 640;
				if(my_options.height <= 0)
					my_options.height = 480;
				break;

			case 'f':
			{
				int str_size = strlen(optarg);
				if(str_size == 4) /*fourcc is 4 chars*/
					strncpy(my_options.format, optarg, 4);
				break;
			}

			case 'F':
				/* numerator and denominator are reversed
				 * since fps here is actually time between frame
				 */
				my_options.fps_num = 1;
				my_options.fps_denom = (int) strtoul(optarg, &stopstring, 10);
				if( *stopstring == '/')
				{
					++stopstring;
					my_options.fps_num = (int) strtoul(stopstring, &stopstring, 10);
				}
				if(my_options.fps_denom <= 0)
					my_options.fps_denom = 25;
				if(my_options.fps_num <= 0)
					my_options.fps_num = 1;
				break;

			case 'r':
			{
				int str_size = strlen(optarg);
				if(str_size <= 4) /*render is at most 4 chars*/
					strncpy(my_options.render, optarg, 4);
				break;
			}
			case 'm':
			{
				int str_size = strlen(optarg);
				if(str_size <= 4) /*[none, full, max] is at most 4 chars*/
					strncpy(my_options.render_flag, optarg, 4);
				else
				{
					my_options.render_width = (int) strtoul(optarg, &stopstring, 10);
					if(my_options.render_width <= 0 || *stopstring != 'x')
						fprintf(stderr, "V4L2_CORE: (options) Error in render_window usage: -m[--render_window=] none|full|max|WIDTHxHEIGHT \n");
					else
					{
						++stopstring;
						my_options.render_height = (int) strtoul(stopstring, &stopstring, 10);
					}
				}

				if(my_options.render_width < 0)
					my_options.render_width = 0;
				if(my_options.render_height < 0)
					my_options.render_height = 0;

				break;
			}
			case 'g':
			{
				int str_size = strlen(optarg);
				if(str_size <= 4) /*gui is at maximum 4 chars*/
					strncpy(my_options.gui, optarg, 4);
				break;
			}
			case 'a':
			{
				/*audio api is at most 5 chars (p u l s e)*/
				strncpy(my_options.audio, optarg, 5);
				break;
			}
			case 'k':
				my_options.audio_device = atoi(optarg);
				break;
			case 'o':
			{
				int str_size = strlen(optarg);
				if(str_size > 2) /*audio codec*/
					strncpy(my_options.audio_codec, optarg, 4);
				break;
			}
			case 'u':
			{
				int str_size = strlen(optarg);
				if(str_size > 2) /*video codec*/
					strncpy(my_options.video_codec, optarg, 4);
				break;
			}
			case 'p':
			{
				if(my_options.prof_filename != NULL)
					free(my_options.prof_filename);
				my_options.prof_filename = strdup(optarg);
				/*get profile path and basename*/
				char *basename = get_file_basename(optarg);
				if(basename)
				{
                    set_profile_name(basename);
					if(my_options.profile_name != NULL)
						free(my_options.profile_name);
					my_options.profile_name = basename;
				}
				char *pathname = get_file_pathname(optarg);
				if(pathname)
				{
                    set_profile_path(pathname);
					if(my_options.profile_path != NULL)
						free(my_options.profile_path);
					my_options.profile_path = pathname;
				}

				break;
			}
			case 'j':
			{
				/*get video path and basename*/
				char *basename = get_file_basename(optarg);
				if(basename)
				{
					//set_video_name(basename);
					//free(basename);
					if(my_options.video_name != NULL)
						free(my_options.video_name);
					my_options.video_name = basename;
				}
				char *pathname = get_file_pathname(optarg);
				if(pathname)
				{
					//set_video_path(pathname);
					//free(pathname);
					if(my_options.video_path != NULL)
						free(my_options.video_path);
					my_options.video_path = pathname;
				}

				break;
			}
			case 'i':
			{
				/*get photo path and basename*/
				char *basename = get_file_basename(optarg);
				if(basename)
				{
					//set_photo_name(basename);
					//free(basename);
					if(my_options.photo_name != NULL)
						free(my_options.photo_name);
					my_options.photo_name = basename;
				}
				char *pathname = get_file_pathname(optarg);
				if(pathname)
				{
					//set_photo_path(pathname);
					//free(pathname);
					if(my_options.photo_path != NULL)
						free(my_options.photo_path);
					my_options.photo_path = pathname;
				}

				break;
			}
			case 'y':
				my_options.video_timer = strtod(optarg, (char **)NULL);
				break;
			case 't':
				my_options.photo_timer = strtod(optarg, (char **)NULL);
				break;
			case 'n':
				my_options.photo_npics = atoi(optarg);
				break;
			case 'e' :
				my_options.exit_on_term = 1;
				break;
			default:
			case 'h':
				opt_print_help();
				ret = 1;
				break;
        }
    }

    return ret;
}

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
void options_clean()
{
	if(my_options.prof_filename != NULL)
		free(my_options.prof_filename);
	my_options.prof_filename = NULL;

	if(my_options.profile_name != NULL)
		free(my_options.profile_name);
	my_options.profile_name = NULL;

	if(my_options.profile_path != NULL)
		free(my_options.profile_path);
	my_options.profile_path = NULL;

	if(my_options.video_name != NULL)
		free(my_options.video_name);
	my_options.video_name = NULL;

	if(my_options.video_path != NULL)
		free(my_options.video_path);
	my_options.video_path = NULL;

	if(my_options.photo_name != NULL)
		free(my_options.photo_name);
	my_options.photo_name = NULL;

	if(my_options.photo_path != NULL)
		free(my_options.photo_path);
	my_options.photo_path = NULL;
}
