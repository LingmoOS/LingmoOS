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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "gviewv4l2core.h"
#include "save_image.h"
#include "colorspaces.h"

extern int verbosity;
/*
 * save data to file
 * args:
 *   filename - string with filename
 *   data - pointer to data
 *   size - data size in bytes = sizeof(uint8_t)
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int v4l2core_save_data_to_file(const char *filename, uint8_t *data, int size)
{
	FILE *fp;
	int ret = 0;

	if ((fp = fopen(filename, "wb")) !=NULL)
	{
		ret = fwrite(data, size, 1, fp);

		if (ret<1) ret=1;/*write error*/
		else ret=0;

		fflush(fp); /*flush data stream to file system*/
		if(fsync(fileno(fp)) || fclose(fp))
			fprintf(stderr, "V4L2_CORE: (save_data_to_file) error - couldn't write buffer to file: %s\n", strerror(errno));
		else if(verbosity > 0)
			printf("V4L2_CORE: saved data to %s\n", filename);
	}
	else ret = 1;

	return (ret);
}

/*
 * save the current frame to file
 * args:
 *    frame - pointer to frame buffer
 *    filename - output file name
 *    format - image type
 *           (IMG_FMT_RAW, IMG_FMT_JPG, IMG_FMT_PNG, IMG_FMT_BMP)
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
int save_frame_image(v4l2_frame_buff_t *frame, const char *filename, int format)
{
	int ret= E_OK;

	switch(format)
	{
		case IMG_FMT_RAW:
			if(verbosity > 0)
				printf("V4L2_CORE: saving raw data to %s\n", filename);
			ret = v4l2core_save_data_to_file(filename, frame->raw_frame, frame->raw_frame_size);
			break;

		case IMG_FMT_JPG:
			if(verbosity > 0)
				printf("V4L2_CORE: saving jpeg frame to %s\n", filename);
		    ret = save_image_jpeg(frame, filename);
		    break;

		case IMG_FMT_BMP:
			if(verbosity > 0)
				printf("V4L2_CORE: saving bmp frame to %s\n", filename);
			ret = save_image_bmp(frame, filename);
			break;

//		case IMG_FMT_PNG:
//			if(verbosity > 0)
//				printf("V4L2_CORE: saving png frame to %s\n", filename);
//			ret = save_image_png(frame, filename);
//			break;

		default:
			fprintf(stderr, "V4L2_CORE: (save_image) Image format %i not supported\n", format);
			ret = E_FORMAT_ERR;
			break;
	}

	return ret;
}
