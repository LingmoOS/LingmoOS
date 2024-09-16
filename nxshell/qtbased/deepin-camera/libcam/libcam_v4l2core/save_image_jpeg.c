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
#include "dct.h"
#include "cameraconfig.h"

/*huffman table from jpeg decoder*/
#define JPG_HUFFMAN_TABLE_LENGTH 0x01A0
extern const uint8_t jpeg_huffman_table[JPG_HUFFMAN_TABLE_LENGTH];

typedef struct _jpeg_file_header_t
{
	uint8_t SOI[2];/*SOI Marker 0xFFD8*/
	uint8_t APP0[2];/*APP0 MARKER 0xFF0E*/
	uint8_t length[2];/*length of header without APP0 in bytes*/
	uint8_t JFIF[5];/*set to JFIF0 0x4A46494600*/
	uint8_t VERS[2];/*1-2 0x0102*/
    /*
     * density:
     * 0 - No units, aspect ratio only specified
     * 1 - Pixels per Inch on quickcam5000pro
     * 2 - Pixels per Centimetre
     */
	uint8_t density;
	uint8_t xdensity[2];/*120 on quickcam5000pro*/
	uint8_t ydensity[2];/*120 on quickcam5000pro*/
	uint8_t WTN;/*width Thumbnail 0*/
	uint8_t HTN;/*height Thumbnail 0*/
} __attribute__ ((packed)) jpeg_file_header_t;

#define PUTBITS	\
{	\
	bits_in_next_word = (int16_t) (jpeg_ctx->bitindex + numbits - 32);	\
	if (bits_in_next_word < 0)	\
	{	\
		jpeg_ctx->lcode = (jpeg_ctx->lcode << numbits) | data;	\
		jpeg_ctx->bitindex += numbits;	\
	}	\
	else	\
	{	\
		jpeg_ctx->lcode = (jpeg_ctx->lcode << (32 - jpeg_ctx->bitindex)) | (data >> bits_in_next_word);	\
		if ((*output++ = (uint8_t)(jpeg_ctx->lcode >> 24)) == 0xff)	\
			*output++ = 0;	\
		if ((*output++ = (uint8_t)(jpeg_ctx->lcode >> 16)) == 0xff)	\
			*output++ = 0;	\
		if ((*output++ = (uint8_t)(jpeg_ctx->lcode >> 8)) == 0xff)	\
			*output++ = 0;	\
		if ((*output++ = (uint8_t) jpeg_ctx->lcode) == 0xff)	\
			*output++ = 0;	\
		jpeg_ctx->lcode = data;	\
		jpeg_ctx->bitindex = bits_in_next_word;	\
	}	\
}

/*
 * Encoder Tables
 */

static uint16_t luminance_dc_code_table[12] =
{
	0x0000, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006,
	0x000E, 0x001E, 0x003E, 0x007E, 0x00FE, 0x01FE
};

static uint16_t luminance_dc_size_table [12] =
{
	0x0002, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003,
	0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009
};

static uint16_t chrominance_dc_code_table [12] =
{
	0x0000, 0x0001, 0x0002, 0x0006, 0x000E, 0x001E,
	0x003E, 0x007E, 0x00FE, 0x01FE, 0x03FE, 0x07FE
};

static uint16_t chrominance_dc_size_table [12] =
{
	0x0002, 0x0002, 0x0002, 0x0003, 0x0004, 0x0005,
	0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B
};

static uint16_t luminance_ac_code_table [162] =
{
	0x000A,
	0x0000, 0x0001, 0x0004, 0x000B, 0x001A, 0x0078, 0x00F8, 0x03F6, 0xFF82, 0xFF83,
	0x000C, 0x001B, 0x0079, 0x01F6, 0x07F6, 0xFF84, 0xFF85, 0xFF86, 0xFF87, 0xFF88,
	0x001C, 0x00F9, 0x03F7, 0x0FF4, 0xFF89, 0xFF8A, 0xFF8b, 0xFF8C, 0xFF8D, 0xFF8E,
	0x003A, 0x01F7, 0x0FF5, 0xFF8F, 0xFF90, 0xFF91, 0xFF92, 0xFF93, 0xFF94, 0xFF95,
	0x003B, 0x03F8, 0xFF96, 0xFF97, 0xFF98, 0xFF99, 0xFF9A, 0xFF9B, 0xFF9C, 0xFF9D,
	0x007A, 0x07F7, 0xFF9E, 0xFF9F, 0xFFA0, 0xFFA1, 0xFFA2, 0xFFA3, 0xFFA4, 0xFFA5,
	0x007B, 0x0FF6, 0xFFA6, 0xFFA7, 0xFFA8, 0xFFA9, 0xFFAA, 0xFFAB, 0xFFAC, 0xFFAD,
	0x00FA, 0x0FF7, 0xFFAE, 0xFFAF, 0xFFB0, 0xFFB1, 0xFFB2, 0xFFB3, 0xFFB4, 0xFFB5,
	0x01F8, 0x7FC0, 0xFFB6, 0xFFB7, 0xFFB8, 0xFFB9, 0xFFBA, 0xFFBB, 0xFFBC, 0xFFBD,
	0x01F9, 0xFFBE, 0xFFBF, 0xFFC0, 0xFFC1, 0xFFC2, 0xFFC3, 0xFFC4, 0xFFC5, 0xFFC6,
	0x01FA, 0xFFC7, 0xFFC8, 0xFFC9, 0xFFCA, 0xFFCB, 0xFFCC, 0xFFCD, 0xFFCE, 0xFFCF,
	0x03F9, 0xFFD0, 0xFFD1, 0xFFD2, 0xFFD3, 0xFFD4, 0xFFD5, 0xFFD6, 0xFFD7, 0xFFD8,
	0x03FA, 0xFFD9, 0xFFDA, 0xFFDB, 0xFFDC, 0xFFDD, 0xFFDE, 0xFFDF, 0xFFE0, 0xFFE1,
	0x07F8, 0xFFE2, 0xFFE3, 0xFFE4, 0xFFE5, 0xFFE6, 0xFFE7, 0xFFE8, 0xFFE9, 0xFFEA,
	0xFFEB, 0xFFEC, 0xFFED, 0xFFEE, 0xFFEF, 0xFFF0, 0xFFF1, 0xFFF2, 0xFFF3, 0xFFF4,
	0xFFF5, 0xFFF6, 0xFFF7, 0xFFF8, 0xFFF9, 0xFFFA, 0xFFFB, 0xFFFC, 0xFFFD, 0xFFFE,
	0x07F9
};

static uint16_t luminance_ac_size_table [162] =
{
	0x0004,
	0x0002, 0x0002, 0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000A, 0x0010, 0x0010,
	0x0004, 0x0005, 0x0007, 0x0009, 0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0005, 0x0008, 0x000A, 0x000C, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0006, 0x0009, 0x000C, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0006, 0x000A, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0007, 0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0007, 0x000C, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0008, 0x000C, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x000F, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000A, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000A, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000B
};

static uint16_t chrominance_ac_code_table [162] =
{
	0x0000,
	0x0001, 0x0004, 0x000A, 0x0018, 0x0019, 0x0038, 0x0078, 0x01F4, 0x03F6, 0x0FF4,
	0x000B, 0x0039, 0x00F6, 0x01F5, 0x07F6, 0x0FF5, 0xFF88, 0xFF89, 0xFF8A, 0xFF8B,
	0x001A, 0x00F7, 0x03F7, 0x0FF6, 0x7FC2, 0xFF8C, 0xFF8D, 0xFF8E, 0xFF8F, 0xFF90,
	0x001B, 0x00F8, 0x03F8, 0x0FF7, 0xFF91, 0xFF92, 0xFF93, 0xFF94, 0xFF95, 0xFF96,
	0x003A, 0x01F6, 0xFF97, 0xFF98, 0xFF99, 0xFF9A, 0xFF9B, 0xFF9C, 0xFF9D, 0xFF9E,
	0x003B, 0x03F9, 0xFF9F, 0xFFA0, 0xFFA1, 0xFFA2, 0xFFA3, 0xFFA4, 0xFFA5, 0xFFA6,
	0x0079, 0x07F7, 0xFFA7, 0xFFA8, 0xFFA9, 0xFFAA, 0xFFAB, 0xFFAC, 0xFFAD, 0xFFAE,
	0x007A, 0x07F8, 0xFFAF, 0xFFB0, 0xFFB1, 0xFFB2, 0xFFB3, 0xFFB4, 0xFFB5, 0xFFB6,
	0x00F9, 0xFFB7, 0xFFB8, 0xFFB9, 0xFFBA, 0xFFBB, 0xFFBC, 0xFFBD, 0xFFBE, 0xFFBF,
	0x01F7, 0xFFC0, 0xFFC1, 0xFFC2, 0xFFC3, 0xFFC4, 0xFFC5, 0xFFC6, 0xFFC7, 0xFFC8,
	0x01F8, 0xFFC9, 0xFFCA, 0xFFCB, 0xFFCC, 0xFFCD, 0xFFCE, 0xFFCF, 0xFFD0, 0xFFD1,
	0x01F9, 0xFFD2, 0xFFD3, 0xFFD4, 0xFFD5, 0xFFD6, 0xFFD7, 0xFFD8, 0xFFD9, 0xFFDA,
	0x01FA, 0xFFDB, 0xFFDC, 0xFFDD, 0xFFDE, 0xFFDF, 0xFFE0, 0xFFE1, 0xFFE2, 0xFFE3,
	0x07F9, 0xFFE4, 0xFFE5, 0xFFE6, 0xFFE7, 0xFFE8, 0xFFE9, 0xFFEA, 0xFFEb, 0xFFEC,
	0x3FE0, 0xFFED, 0xFFEE, 0xFFEF, 0xFFF0, 0xFFF1, 0xFFF2, 0xFFF3, 0xFFF4, 0xFFF5,
	0x7FC3, 0xFFF6, 0xFFF7, 0xFFF8, 0xFFF9, 0xFFFA, 0xFFFB, 0xFFFC, 0xFFFD, 0xFFFE,
	0x03FA
};

static uint16_t chrominance_ac_size_table [162] =
{
	0x0002,
	0x0002, 0x0003, 0x0004, 0x0005, 0x0005, 0x0006, 0x0007, 0x0009, 0x000A, 0x000C,
	0x0004, 0x0006, 0x0008, 0x0009, 0x000B, 0x000C, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0005, 0x0008, 0x000A, 0x000C, 0x000F, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0005, 0x0008, 0x000A, 0x000C, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0006, 0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0006, 0x000A, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0007, 0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0007, 0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0008, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000E, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000F, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000A
};

static uint8_t bitsize [256] =/* bit size from 0 to 255 */
{
	0, 1, 2, 2, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8
};

static uint8_t zigzag_table [] =
{
	0,  1,   5,  6, 14, 15, 27, 28,
	2,  4,   7, 13, 16, 26, 29, 42,
	3,  8,  12, 17, 25, 30, 41, 43,
	9,  11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63
};

/* quickcam 5000pro tables (very good quality) */
static uint8_t luminance_quant_table [] =
{
	0x04, 0x02, 0x03, 0x03, 0x03, 0x02, 0x04, 0x03,
	0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x06, 0x0a,
	0x06, 0x06, 0x05, 0x05, 0x06, 0x0c, 0x08, 0x09,
	0x07, 0x0a, 0x0e, 0x0c, 0x0f, 0x0f, 0x0e, 0x0c,
	0x0e, 0x0f, 0x10, 0x12, 0x17, 0x13, 0x10, 0x11,
	0x15, 0x11, 0x0d, 0x0e, 0x14, 0x1a, 0x14, 0x15,
	0x17, 0x18, 0x19, 0x1a, 0x19, 0x0f, 0x13, 0x1c,
	0x1e, 0x1c, 0x19, 0x1e, 0x17, 0x19, 0x19, 0x18
};

static uint8_t chrominance_quant_table [] =
{
	0x04, 0x04, 0x04, 0x06, 0x05, 0x06, 0x0b, 0x06,
	0x06, 0x0b, 0x18, 0x10, 0x0e, 0x10, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18
};

/*
 * ####### Encoder functions #######
 */

/*
 * This function implements 16 Step division for Q.15 format data
 * args:
 *    numer - numerator
 *    denom - denominator
 *
 * asserts:
 *   none
 *
 * returns: division result
 */
static uint16_t DSP_Division (uint32_t numer, uint32_t denom)
{
	uint16_t i;

	denom <<= 15;

	for (i=16; i>0; i--)
	{
		if (numer > denom)
		{
			numer -= denom;
			numer <<= 1;
			numer++;
		}
		else numer <<= 1;
	}

	return (uint16_t) numer;
}

/*
 * split yuyv data into YCBr planes
 *   and fill matching encoder context fields
 * args:
 *    jpeg_ctx - pointer to jpeg encoder context
 *    input - pointer to input data
 *
 * asserts:
 *    jpeg_ctx is not null
 *    input is not null
 *
 * returns: none
 */
static void read_422_format (jpeg_encoder_ctx_t *jpeg_ctx, uint8_t *input)
{
	/*assertions*/
	assert(jpeg_ctx != NULL);
	assert(input != NULL);

	int32_t i, j;

	int16_t *Y1 = jpeg_ctx->Y1; /*64 int16 block*/
	int16_t *Y2 = jpeg_ctx->Y2;
	int16_t *CB = jpeg_ctx->CB;
	int16_t *CR = jpeg_ctx->CR;

	uint16_t incr = jpeg_ctx->incr;

	uint8_t *tmp = NULL;
	tmp = input;

	for (i=8; i>0; i--) /*8 rows*/
	{
		for (j=4; j>0; j--) /* 8 cols*/
		{
			*Y1++ = *tmp++;
			*CB++ = *tmp++;
			*Y1++ = *tmp++;
			*CR++ = *tmp++;
		}

		for (j=4; j>0; j--) /* 8 cols*/
		{
			*Y2++ = *tmp++;
			*CB++ = *tmp++;
			*Y2++ = *tmp++;
			*CR++ = *tmp++;
		}

		tmp += incr; /* next row (width - mcu_width)*/
	}
}

/* Multiply Quantization table with quality factor to get LQT and CQT
 *  (Will use constant Quantization tables to make it faster)
 * args:
 *    jpeg_ctx - pointer to jpeg encoder context
 *
 * asserts:
 *    jpeg_ctx is not null
 *
 * returns: none
 */
void initialize_quantization_tables (jpeg_encoder_ctx_t *jpeg_ctx)
{
	/*assertions*/
	assert(jpeg_ctx != NULL);

	uint16_t i, index;
	uint32_t value;

	for (i = 0; i < 64; i++)
	{
		index = zigzag_table [i];

		value= luminance_quant_table [i];

		jpeg_ctx->Lqt [index] = (uint8_t) value;
		jpeg_ctx->ILqt [i] = DSP_Division (0x8000, value);


		value = chrominance_quant_table [i];

		jpeg_ctx->Cqt[index] = (uint8_t) value;
		jpeg_ctx->ICqt [i] = DSP_Division (0x8000, value);
	}
}

/*
 * multiply DCT Coefficients with Quantization table
 *  and store in ZigZag location
 * args:
 *    jpeg_ctx - pointer to jpeg encoder context
 *    data - pointer to data for quantization (from DCT)
 *    quant_table_ptr - pointer to quantization table
 *
 * asserts:
 *    jpeg_ctx is not null
 *    data is not null
 *    quant_table_ptr is not null
 *
 * returns: none
 */
static void quantization (jpeg_encoder_ctx_t *jpeg_ctx, int16_t * const data, uint16_t * const quant_table_ptr)
{
	/*assertions*/
	assert(jpeg_ctx != NULL);
	assert(data != NULL);
	assert(quant_table_ptr != NULL);

	int16_t i;
	int32_t value;

	for (i=63; i>=0; i--)
	{
		value = data [i] * quant_table_ptr [i];
		value = (value + 0x4000) >> 15;

		jpeg_ctx->Temp [zigzag_table [i]] = (int16_t) value;
	}
}

/*
 * add/code huffman table
 * args:
 *    jpeg_ctx - pointer to jpeg encoder context
 *    component - image component
 *    output - pointer to output buffer
 *
 * asserts:
 *    jpeg_ctx is not null
 *    output is not null
 *
 * returns: pointer to output buffer
 */
static uint8_t *huffman (jpeg_encoder_ctx_t *jpeg_ctx, uint16_t component, uint8_t *output)
{
	/*assertions*/
	assert(jpeg_ctx != NULL);
	assert(output != NULL);

	uint16_t i;
	uint16_t *DcCodeTable, *DcSizeTable, *AcCodeTable, *AcSizeTable;

	int16_t *Temp_Ptr, Coeff, LastDc;
	uint16_t AbsCoeff, HuffCode, HuffSize, RunLength=0, DataSize=0, index;

	int16_t bits_in_next_word;
	uint16_t numbits;
	uint32_t data;

	Temp_Ptr = jpeg_ctx->Temp;
	Coeff = *Temp_Ptr++;/* Coeff = DC */

	/* code DC - Temp[0] */
	if (component == 1)/* luminance - Y */
	{
		DcCodeTable = luminance_dc_code_table;
		DcSizeTable = luminance_dc_size_table;
		AcCodeTable = luminance_ac_code_table;
		AcSizeTable = luminance_ac_size_table;

		LastDc = jpeg_ctx->ldc1;
		jpeg_ctx->ldc1 = Coeff;
	}
	else /* Chrominance - U V */
	{
		DcCodeTable = chrominance_dc_code_table;
		DcSizeTable = chrominance_dc_size_table;
		AcCodeTable = chrominance_ac_code_table;
		AcSizeTable = chrominance_ac_size_table;

		if (component == 2) /* Chrominance - U */
		{
			LastDc = jpeg_ctx->ldc2;
			jpeg_ctx->ldc2 = Coeff;
		}
		else/* Chrominance - V */
		{
			LastDc = jpeg_ctx->ldc3;
			jpeg_ctx->ldc3 = Coeff;
		}
	}

	Coeff = Coeff - LastDc; /* DC - LastDC */

	AbsCoeff = (Coeff < 0) ? -(Coeff--) : Coeff;

	/*calculate data size*/
	while (AbsCoeff != 0)
	{
		AbsCoeff >>= 1;
		DataSize++;
	}

	HuffCode = DcCodeTable [DataSize];
	HuffSize = DcSizeTable [DataSize];

	Coeff &= (1 << DataSize) - 1;
	data = (HuffCode << DataSize) | Coeff;
	numbits = HuffSize + DataSize;

	PUTBITS

    /* code AC */
	for (i=63; i>0; i--)
	{

		if ((Coeff = *Temp_Ptr++) != 0)
		{
			while (RunLength > 15)
			{
				RunLength -= 16;
				data = AcCodeTable [161];   /* ZRL 0xF0 ( 16 - 0) */
				numbits = AcSizeTable [161];/* ZRL                */
				PUTBITS
			}

			AbsCoeff = (Coeff < 0) ? -(Coeff--) : Coeff;

			if (AbsCoeff >> 8 == 0) /* Size <= 8 bits */
				DataSize = bitsize [AbsCoeff];
			else /* 16 => Size => 8 */
				DataSize = bitsize [AbsCoeff >> 8] + 8;

			index = RunLength * 10 + DataSize;


			HuffCode = AcCodeTable [index];
			HuffSize = AcSizeTable [index];

			Coeff &= (1 << DataSize) - 1;
			data = (HuffCode << DataSize) | Coeff;
			numbits = HuffSize + DataSize;

			PUTBITS
			RunLength = 0;
		}
		else
			RunLength++;/* Add while Zero */
	}

	if (RunLength != 0)
	{
		data = AcCodeTable [0];   /* EOB - 0x00 end of block */
		numbits = AcSizeTable [0];/* EOB                     */
		PUTBITS
	}
	return output;
}

/*
 * For bit Stuffing and EOI marker
 * args:
 *     jpeg_ctx - pointer to jpeg encoder context
 *     output - pointer to output buffer
 *
 * asserts:
 *     jpeg_ctx is not null
 *     output is not null
 *
 * returns: pointer to output buffer
 */
static uint8_t *close_bitstream (jpeg_encoder_ctx_t *jpeg_ctx, uint8_t *output)
{
	/*assertions*/
	assert(jpeg_ctx != NULL);
	assert(output != NULL);

	if (jpeg_ctx->bitindex > 0)
	{
		jpeg_ctx->lcode <<= (32 - jpeg_ctx->bitindex);
		uint16_t count = (jpeg_ctx->bitindex + 7) >> 3;
		uint16_t i = 0;

		uint8_t *ptr = (uint8_t *) &jpeg_ctx->lcode + 3;

		for (i=count; i>0; i--)
		{
			if ((*output++ = *ptr--) == 0xff)
				*output++ = 0;
		}
	}

	/* End of image marker (EOI) */
	*output++ = 0xFF;
	*output++ = 0xD9;
	return output;
}

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
void initialization (jpeg_encoder_ctx_t *jpeg_ctx, int image_width, int image_height)
{
	/*assertions*/
	assert(jpeg_ctx != NULL);

	uint16_t mcu_width, mcu_height, bytes_per_pixel;

	jpeg_ctx->image_width = image_width;
	jpeg_ctx->image_height = image_height;

	jpeg_ctx->mcu_width = mcu_width = 16;
	jpeg_ctx->horizontal_mcus = (uint16_t) (image_width >> 4);/* width/16 */

	jpeg_ctx->mcu_height = mcu_height = 8;
	jpeg_ctx->vertical_mcus = (uint16_t) (image_height >> 3); /* height/8 */

	bytes_per_pixel = 2;

	jpeg_ctx->length_minus_mcu_width = (uint16_t) ((image_width - mcu_width) * bytes_per_pixel);
	jpeg_ctx->length_minus_width = (uint16_t) (image_width * bytes_per_pixel);

	jpeg_ctx->mcu_width_size = (uint16_t) (mcu_width * bytes_per_pixel);

	jpeg_ctx->rows = jpeg_ctx->mcu_height;
	jpeg_ctx->cols = jpeg_ctx->mcu_width;
	jpeg_ctx->incr = jpeg_ctx->length_minus_mcu_width;
	jpeg_ctx->offset = (uint16_t) ((image_width * mcu_height) * bytes_per_pixel);

	jpeg_ctx->ldc1 = 0;
	jpeg_ctx->ldc2 = 0;
	jpeg_ctx->ldc3 = 0;

	jpeg_ctx->lcode = 0;
	jpeg_ctx->bitindex = 0;
}

/*
 * restart jpeg encoder context
 * args:
 *    jpeg_ctx - pointer to jpeg encoder context
 *
 * asserts:
 *    jpeg_ctx is not null
 *
 * returns: none
 */
static void jpeg_restart (jpeg_encoder_ctx_t *jpeg_ctx)
{
	/*assertions*/
	assert(jpeg_ctx != NULL);

	jpeg_ctx->ldc1 = 0;
	jpeg_ctx->ldc2 = 0;
	jpeg_ctx->ldc3 = 0;

	jpeg_ctx->lcode = 0;
	jpeg_ctx->bitindex = 0;
}

/*
 * encode single MCU
 * args:
 *    jpeg_ctx - pointer to jpeg encoder context
 *    output - pointer to output buffer
 *
 * asserts:
 *    jpeg_ctx is not null
 *    output is not null
 *
 * returns: pointer to ouptut buffer
 */
static uint8_t* encode_MCU (jpeg_encoder_ctx_t *jpeg_ctx, uint8_t *output)
{
	/*assertions*/
	assert(jpeg_ctx != NULL);
	assert(output != NULL);

	levelshift (jpeg_ctx->Y1);
	DCT (jpeg_ctx->Y1);

	quantization (jpeg_ctx, jpeg_ctx->Y1, jpeg_ctx->ILqt);

	output = huffman (jpeg_ctx, 1, output);

	levelshift (jpeg_ctx->Y2);
	DCT (jpeg_ctx->Y2);

	quantization (jpeg_ctx, jpeg_ctx->Y2,
		jpeg_ctx->ILqt);

	output = huffman (jpeg_ctx, 1, output);

	levelshift (jpeg_ctx->CB);
	DCT (jpeg_ctx->CB);

	quantization (jpeg_ctx, jpeg_ctx->CB,
		jpeg_ctx->ICqt);

	output = huffman (jpeg_ctx, 2, output);

	levelshift (jpeg_ctx->CR);
	DCT (jpeg_ctx->CR);

	quantization (jpeg_ctx, jpeg_ctx->CR,
		jpeg_ctx->ICqt);

	output = huffman (jpeg_ctx, 3, output);

	return output;
}

/*
 * Header markers for JPEG Encoder
 * args:
 *    jpeg_ctx - pointer to jpeg encoder context
 *    output - pointer to output buffer
 *
 * asserts
 *   jpeg_ctx is not null
 *   ouput is not null
 *
 * returns: pointer to output buffer
 */
static uint8_t *write_markers(jpeg_encoder_ctx_t *jpeg_ctx, uint8_t *output, int huff)
{
	/*assertions*/
	assert(jpeg_ctx != NULL);
	assert(output != NULL);

	uint16_t i, header_length;
	uint8_t number_of_components;

	// Start of image marker
	*output++ = 0xFF;
	*output++ = 0xD8;
	//added from here
	// Start of APP0 marker
	*output++ = 0xFF;
	*output++ = 0xE0;
	//header length
	*output++= 0x00;
	*output++= 0x10;//16 bytes

	//type
	if(huff)
	{	//JFIF0 0x4A46494600
		*output++= 0x4A;
		*output++= 0x46;
		*output++= 0x49;
		*output++= 0x46;
		*output++= 0x00;
	}
	else
	{	// AVI10 0x4156493100
		*output++= 0x41;
		*output++= 0x56;
		*output++= 0x49;
		*output++= 0x31;
		*output++= 0x00;
	}
	// version
	*output++= 0x01;
	*output++= 0x02;
	// density 0- no units 1- pix per inch 2- pix per mm
	*output++= 0x01;
	// xdensity - 120
	*output++= 0x00;
	*output++= 0x78;
	// ydensity - 120
	*output++= 0x00;
	*output++= 0x78;

	//thumb x y
	*output++= 0x00;
	*output++= 0x00;
	//to here

	// Quantization table marker
	*output++ = 0xFF;
	*output++ = 0xDB;

	// Quantization table length
	*output++ = 0x00;
	*output++ = 0x43;

	// Pq, Tq
	*output++ = 0x00;

	// Lqt table
	for (i=0; i<64; i++)
		*output++ = jpeg_ctx->Lqt [i];

	// Quantization table marker
	*output++ = 0xFF;
	*output++ = 0xDB;

	// Quantization table length
	*output++ = 0x00;
	*output++ = 0x43;

	// Pq, Tq
	*output++ = 0x01;

	// Cqt table
	for (i=0; i<64; i++)
		*output++ = jpeg_ctx->Cqt [i];

	if (huff)
	{
		// huffman table(DHT)

		*output++=0xff;
		*output++=0xc4;
		*output++=0x01;
		*output++=0xa2;
		memmove(output, &jpeg_huffman_table, JPG_HUFFMAN_TABLE_LENGTH);/*0x01a0*/
		output+=JPG_HUFFMAN_TABLE_LENGTH;

	}

	number_of_components = 3;

	// Frame header(SOF)

	// Start of frame marker
	*output++ = 0xFF;
	*output++ = 0xC0;

	header_length = (uint16_t) (8 + 3 * number_of_components);

	// Frame header length
	*output++ = (uint8_t) (header_length >> 8);
	*output++ = (uint8_t) header_length;

	// Precision (P)
	*output++ = 0x08;/*8 bits*/

	// image height
	*output++ = (uint8_t) (jpeg_ctx->image_height >> 8);
	*output++ = (uint8_t) jpeg_ctx->image_height;

	// image width
	*output++ = (uint8_t) (jpeg_ctx->image_width >> 8);
	*output++ = (uint8_t) jpeg_ctx->image_width;

	// Nf
	*output++ = number_of_components;

	/* type 422 */
	*output++ = 0x01; /*id (y)*/
	*output++ = 0x21; /*horiz|vertical */
	*output++ = 0x00; /*quantization table used*/

	*output++ = 0x02; /*id (u)*/
	*output++ = 0x11; /*horiz|vertical*/
	*output++ = 0x01; /*quantization table used*/

	*output++ = 0x03; /*id (v)*/
	*output++ = 0x11; /*horiz|vertical*/
	*output++ = 0x01; /*quantization table used*/


	// Scan header(SOF)

	// Start of scan marker
	*output++ = 0xFF;
	*output++ = 0xDA;

	header_length = (uint16_t) (6 + (number_of_components << 1));

	// Scan header length
	*output++ = (uint8_t) (header_length >> 8);
	*output++ = (uint8_t) header_length;

	// Ns = number of scans
	*output++ = number_of_components;

	/* type 422*/
	*output++ = 0x01; /*component id (y)*/
	*output++ = 0x00; /*dc|ac tables*/

	*output++ = 0x02; /*component id (u)*/
	*output++ = 0x11; /*dc|ac tables*/

	*output++ = 0x03; /*component id (v)*/
	*output++ = 0x11; /*dc|ac tables*/

	*output++ = 0x00; /*0 */
	*output++ = 0x3F; /*63*/
	*output++ = 0x00; /*0 */

	return output;
}

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
	jpeg_encoder_ctx_t *jpeg_ctx, int huff)
{
	/*assertions*/
	assert(input != NULL);
	assert(output != NULL);
	assert(jpeg_ctx != NULL);

	int size;
	uint16_t i, j;
	uint8_t *tmp_ptr=NULL;
	uint8_t *tmp_iptr = input;
	uint8_t *tmp_optr = output;

	/* clean jpeg parameters*/
	jpeg_restart(jpeg_ctx);

	/* Writing Marker Data */
	tmp_optr = write_markers (jpeg_ctx, tmp_optr, huff);

    uint8_t *yuv422 = (uint8_t*)calloc(jpeg_ctx->image_width * jpeg_ctx->image_height * 2, sizeof(uint8_t));
	if(yuv422 == NULL)
	{
		fprintf(stderr, "V4L2_CORE: couldn't allocate memory for jpeg encoder (fatal)\n");
		exit(-1);
	}
	yu12_to_yuyv(yuv422, input, jpeg_ctx->image_width, jpeg_ctx->image_height);
	tmp_iptr = yuv422;

	for (i=0; i < jpeg_ctx->vertical_mcus; i++) /* height /8 */
	{
		tmp_ptr = tmp_iptr;
		for (j=0; j< jpeg_ctx->horizontal_mcus; j++) /* width /16 */
		{
			/*reads a block*/
			read_422_format (jpeg_ctx, tmp_iptr); /*YUYV*/

			/* Encode the data in MCU */
			tmp_optr = encode_MCU (jpeg_ctx, tmp_optr);

			if(j < (jpeg_ctx->horizontal_mcus -1))
			{
				tmp_iptr += jpeg_ctx->mcu_width_size;
			}
			else
			{
				tmp_iptr=tmp_ptr;
			}
		}
		tmp_iptr += jpeg_ctx->offset;

	}

	/* Close Routine */
	free(yuv422);

	tmp_optr = close_bitstream (jpeg_ctx, tmp_optr);
	size = tmp_optr - output;
	tmp_iptr = NULL;
	tmp_optr = NULL;

	return (size);
}

/*
 * save frame data to a jpeg file
 * args:
 *    frame - pointer to frame buffer
 *    filename - filename string
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
int save_image_jpeg(v4l2_frame_buff_t *frame, const char *filename)
{
	int ret = E_OK;

	jpeg_encoder_ctx_t *jpeg_ctx = calloc(1, sizeof(jpeg_encoder_ctx_t));
	if(jpeg_ctx == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (save_image_jpeg): %s\n", strerror(errno));
		exit(-1);
	}

	uint8_t *jpeg = calloc((frame->width * frame->height) >> 1, sizeof(uint8_t));
	if(jpeg == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (save_image_jpeg): %s\n", strerror(errno));
		exit(-1);
	}

	/* Initialization of JPEG control structure */
	initialization (jpeg_ctx, frame->width, frame->height);

	/* Initialization of Quantization Tables  */
	initialize_quantization_tables (jpeg_ctx);

	int jpeg_size = encode_jpeg(frame->yuv_frame, jpeg, jpeg_ctx, 1);

	if(v4l2core_save_data_to_file(filename, jpeg, jpeg_size))
	{
		fprintf (stderr, "V4L2_CORE: (save_image_jpeg) couldn't capture Image to %s \n",
					filename);
		ret = E_FILE_IO_ERR;
	}

	/*clean up*/
	free(jpeg);
	free(jpeg_ctx);

	return ret;
}
