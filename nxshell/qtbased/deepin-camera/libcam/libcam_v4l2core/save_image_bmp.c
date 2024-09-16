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
#include "cameraconfig.h"

typedef struct _bmp_file_header_t
{
	uint16_t    bfType; //Specifies the file type, must be BM
	uint32_t    bfSize; //Specifies the size, in bytes, of the bitmap file
	uint16_t    bfReserved1; //Reserved; must be zero
	uint16_t    bfReserved2; //Reserved; must be zero
	uint32_t    bfOffBits; /*Specifies the offset, in bytes,
			    from the beginning of the BITMAPFILEHEADER structure
			    to the bitmap bits= FileHeader+InfoHeader+RGBQUAD(0 for 24bit BMP)=64*/
}   __attribute__ ((packed)) bmp_file_header_t;


typedef struct _bmp_info_header_t
{
	uint32_t   biSize;  /*size of this header 40 bytes*/
	int32_t    biWidth;
	int32_t    biHeight;
	uint16_t   biPlanes; /*color planes - set to 1*/
	uint16_t   biBitCount; /*bits per pixel - color depth (use 24)*/
	uint32_t   biCompression; /*BI_RGB = 0*/
	uint32_t   biSizeImage;
	uint32_t   biXPelsPerMeter;
	uint32_t   biYPelsPerMeter;
	uint32_t   biClrUsed;
	uint32_t   biClrImportant;
}  __attribute__ ((packed)) bmp_info_header_t;


/*
 * save bmp data to file
 * args:
 *   filename - bmp file name
 *   data - pixel data (rgb form)
 *   width - image width
 *   height - image height
 *   BitCount - bits per pixel
 *
 * asserts:
 *     data is not null
 *
 * returns: error code
 */
static int save_bmp(const char *filename, uint8_t *data, int width, int height, int BitCount)
{
	/*assertions*/
	assert(data != NULL);

	int ret = E_OK;
	bmp_file_header_t BmpFileh;
	bmp_info_header_t BmpInfoh;
	FILE *fp;

	int imgsize = width * height * BitCount / 8;

	BmpFileh.bfType=0x4d42;//must be BM (x4d42)
	/*Specifies the size, in bytes, of the bitmap file*/
	BmpFileh.bfSize=sizeof(bmp_file_header_t)+sizeof(bmp_info_header_t)+imgsize;
	BmpFileh.bfReserved1=0; //Reserved; must be zero
	BmpFileh.bfReserved2=0; //Reserved; must be zero
	/*Specifies the offset, in bytes,                      */
	/*from the beginning of the BITMAPFILEHEADER structure */
	/* to the bitmap bits                                  */
	BmpFileh.bfOffBits=sizeof(bmp_file_header_t)+sizeof(bmp_info_header_t);

	BmpInfoh.biSize=40;
	BmpInfoh.biWidth=width;
	BmpInfoh.biHeight=height;
	BmpInfoh.biPlanes=1;
	BmpInfoh.biBitCount=BitCount;
	BmpInfoh.biCompression=0; // 0
	BmpInfoh.biSizeImage=imgsize;
	BmpInfoh.biXPelsPerMeter=0;
	BmpInfoh.biYPelsPerMeter=0;
	BmpInfoh.biClrUsed=0;
	BmpInfoh.biClrImportant=0;

	if ((fp = fopen(filename, "wb")) != NULL)
	{	// (wb) write in binary mode
		ret=fwrite(&BmpFileh, sizeof(bmp_file_header_t), 1, fp);
		ret+=fwrite(&BmpInfoh, sizeof(bmp_info_header_t),1,fp);
		ret+=fwrite(data, imgsize, 1, fp);
		if (ret < 3)
			ret = E_FILE_IO_ERR;//write error
		else
			ret = E_OK;

		fflush(fp); //flush data stream to file system
		if(fsync(fileno(fp)) || fclose(fp))
		{
			fprintf(stderr, "V4L2_CORE: (save bmp) couldn't write to file %s: %s\n",
				filename, strerror(errno));
			ret = E_FILE_IO_ERR;
		}
	}
	else
	{
		ret=1;
		fprintf(stderr, "V4L2_CORE: (save bmp) could not open file %s for write \n",
			filename);
	}
	return ret;
}

/*
 * save frame data to a bmp file
 * args:
 *    frame - pointer to frame buffer
 *    filename - filename string
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
int save_image_bmp(v4l2_frame_buff_t *frame, const char *filename)
{
	int ret = E_OK;
	int width = frame->width;
	int height = frame->height;

	uint8_t *bmp = calloc(width * height * 3, sizeof(uint8_t));
	if(bmp == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (save_img_bmp): %s\n", strerror(errno));
		exit(-1);
	}
	yu12_to_dib24(bmp, frame->yuv_frame, width, height);

	ret = save_bmp(filename, bmp, width, height, 24);
	free(bmp);
	
	return ret;
}
