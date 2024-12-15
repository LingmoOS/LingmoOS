/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
#                             Add UYVY color support(Macbook iSight)            #
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

#ifndef SAVE_IMAGE_H
#define SAVE_IMAGE_H

#include "gviewv4l2core.h"
#include "v4l2_core.h"

typedef struct _jpeg_encoder_ctx_t {
    uint16_t    image_width;
    uint16_t    image_height;
    uint16_t    mcu_width;
    uint16_t    mcu_height;
    uint16_t    horizontal_mcus;
    uint16_t    vertical_mcus;

    uint16_t    rows;
    uint16_t    cols;

    uint16_t    length_minus_mcu_width;
    uint16_t    length_minus_width;
    uint16_t    incr;
    uint16_t    mcu_width_size;
    uint16_t    offset;

    int16_t     ldc1;
    int16_t     ldc2;
    int16_t     ldc3;

    uint32_t    lcode;
    uint16_t    bitindex;

    /* MCUs */
    int16_t     Y1 [64];
    int16_t     Y2 [64];
    int16_t     Temp [64];
    int16_t     CB [64];
    int16_t     CR [64];

    /* Quantization Tables */
    uint8_t     Lqt [64];
    uint8_t     Cqt [64];
    uint16_t    ILqt [64];
    uint16_t    ICqt [64];

} jpeg_encoder_ctx_t;

/*
 * save the current frame to file
 * args:
 *    frame - pointer to frame buffer
 *    filename - output file name
 *    format - image type
 *           (IMG_FMT_RAW, IMG_FMT_JPG, IMG_FMT_PNG, IMG_FMT_BMP)
 *
 * asserts:
 *    vd is not null
 *
 * returns: error code
 */
int save_frame_image(v4l2_frame_buff_t *frame, const char *filename, int format);

/*
 * save frame data to a jpeg file
 * args:
 *    frame - pointer to frame buffer
 *    filename - filename string
 *
 * asserts:
 *    vd is not null
 *
 * returns: error code
 */
int save_image_jpeg(v4l2_frame_buff_t *frame, const char *filename);

/*
 * save frame data to a bmp file
 * args:
 *    frame - pointer to frame buffer
 *    filename - filename string
 *
 * asserts:
 *    vd is not null
 *
 * returns: error code
 */
int save_image_bmp(v4l2_frame_buff_t *frame, const char *filename);

/*
 * save frame data into a png file
 * args:
 *    frame - pointer to frame buffer
 *    filename - string with png filename name
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code
 */
//int save_image_png(v4l2_frame_buff_t *frame, const char *filename);

/*
 * encode jpeg
 * args:
 *    input - pointer to input buffer (yuyv format)
 *    output - pointer to output buffer (jpeg format)
 *    jpeg_ctx - pointer to jpeg encoder context
 *    huff - huffman flag
 *
 *
 * asserts:
 *    input is not null
 *    ouput is not null
 *    jpeg_ctx is not null
 *
 * returns: ouput size
 */
int encode_jpeg (uint8_t *input, uint8_t *output,
                 jpeg_encoder_ctx_t *jpeg_ctx, int huff);

/*
 * init jpeg encoder context
 * args:
 *    jpeg_ctx - pointer to jpeg encoder context
 *    image_width - image width (in pixels)
 *    image_height - image height (in pixels)
 *
 * asserts:
 *    jpeg_ctx is not null
 *
 * returns: none
 */
void initialization (jpeg_encoder_ctx_t *jpeg_ctx, int image_width, int image_height);

void initialize_quantization_tables (jpeg_encoder_ctx_t *jpeg_ctx);
#endif
