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

/*******************************************************************************#
#                                                                               #
#  M/Jpeg decoding and frame capture taken from luvcview                        #
#                                                                               #
********************************************************************************/

#ifndef JPEG_DECODER_H
#define JPEG_DECODER_H

#define HEADERFRAME1 0xaf

/*******Error codes *******/
#define ERR_NO_SOI 1
#define ERR_NOT_8BIT 2
#define ERR_HEIGHT_MISMATCH 3
#define ERR_WIDTH_MISMATCH 4
#define ERR_BAD_WIDTH_OR_HEIGHT 5
#define ERR_TOO_MANY_COMPPS 6
#define ERR_ILLEGAL_HV 7
#define ERR_QUANT_TABLE_SELECTOR 8
#define ERR_NOT_YCBCR_221111 9
#define ERR_UNKNOWN_CID_IN_SCAN 10
#define ERR_NOT_SEQUENTIAL_DCT 11
#define ERR_WRONG_MARKER 12
#define ERR_NO_EOI 13
#define ERR_BAD_TABLES 14
#define ERR_DEPTH_MISMATCH 15

/*
 * init (m)jpeg decoder context
 * args:
 *    width - image width
 *    height - image height
 *
 * asserts:
 *    none
 *
 * returns: error code (0 - E_OK)
 */
int jpeg_init_decoder(int width, int height);

/*
 * jpeg decode
 * args:
 *   out_buf -  pointer to picture data ( decoded image - yuyv format)
 *   in_buf -  pointer to input data ( compressed jpeg )
 *   size - picture size
 *
 * asserts:
 *   out_buf not null
 *   in_buf not null
 *
 * returns: error code (0 - OK)
 */
int jpeg_decode(uint8_t *out_buf, uint8_t *in_buf, int size);

/*
 * close (m)jpeg decoder context
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void jpeg_close_decoder();

#endif

