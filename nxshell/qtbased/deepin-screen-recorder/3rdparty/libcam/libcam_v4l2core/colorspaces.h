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

#ifndef COLORSPACES_H
#define COLORSPACES_H

#include "gview.h"
#include "cameraconfig.h"

/*
 *convert from packed 422 yuv (yuyv) to 420 planar (yu12)
 * args:
 *    out - pointer to output yu12 planar data buffer
 *    in - pointer to input yuyv packed data buffer
 *    width - frame width
 *    height - frame height
 *
 * asserts:
 *    in is not null
 *    out is not null
 *
 * returns: none
 */
void yuyv_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 *convert from packed 422 yuv (yvyu) to 420 planar (yu12)
 * args:
 *    out - pointer to output yu12 planar data buffer
 *    in - pointer to input yvyu packed data buffer
 *    width - frame width
 *    height - frame height
 *
 * asserts:
 *    in is not null
 *    out is not null
 *
 * returns: none
 */
void yvyu_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 *convert from packed 422 yuv (uyvy) to 420 planar (yu12)
 * args:
 *    out - pointer to output yu12 planar data buffer
 *    in - pointer to input uyvy packed data buffer
 *    width - frame width
 *    height - frame height
 *
 * asserts:
 *    in is not null
 *    out is not null
 *
 * returns: none
 */
void uyvy_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 *convert from packed 422 yuv (vyuy) to 420 planar (yu12)
 * args:
 *    out - pointer to output yu12 planar data buffer
 *    in - pointer to input vyuy packed data buffer
 *    width - frame width
 *    height - frame height
 *
 * asserts:
 *    in is not null
 *    out is not null
 *
 * returns: none
 */
void vyuy_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 *convert from 422 planar yuv to 420 planar (yu12)
 * args:
 *    out - pointer to output yu12 planar data buffer
 *    in - pointer to input 422 planar data buffer
 *    width - frame width
 *    height - frame height
 *
 * asserts:
 *    in is not null
 *    out is not null
 *
 * returns: none
 */
void yuv422p_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert yyuv (packed) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing yyuv packed data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void yyuv_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert y444 (packed) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing y444 (yuv-4-4-4) packed data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void y444_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert yuvo (packed) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing yuvo (yuv-5-5-5) packed data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void yuvo_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert yuvp (packed) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing yuvp (yuv-5-6-5) packed data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void yuvp_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert yuv4 (packed) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing yuv4 (yuv32) packed data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void yuv4_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 *convert from 420 planar (yv12) to 420 planar (yu12)
 * args:
 *    out - pointer to output yu12 planar data buffer
 *    in - pointer to input yv12 planar data buffer
 *    width - frame width
 *    height - frame height
 *
 * asserts:
 *    in is not null
 *    out is not null
 *
 * returns: none
 */
void yv12_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert nv12 planar (uv interleaved) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing nv12 planar data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void nv12_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert nv21 planar (vu interleaved) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing nv21 planar data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void nv21_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert yuv 422 planar (uv interleaved) (nv16) to yuv420 planar (yu12)
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing yuv422 (nv16) planar data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void nv16_to_yu12 (uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert yuv444 planar (uv interleaved) (nv24) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing nv24 planar data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void nv24_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert yuv444 planar (uv interleaved) (nv42) to yuv420 planar (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing nv42 planar data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void nv42_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert yuv 422 planar (vu interleaved) (nv61) to yuv420 planar (yu12)
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing yuv422 (nv61) planar data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void nv61_to_yu12 (uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert y10b (bit-packed array greyscale format) to yu12
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing y10b (bit-packed array) data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void y10b_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert yuv 411 packed (y41p) to planar yuv 420 (yu12)
 * args:
 *    out: pointer to output buffer (yu12)
 *    in: pointer to input buffer containing y41p data frame
 *    width: picture width
 *    height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void y41p_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert yuv mono (grey) to yuv 420 planar (yu12)
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing grey (y only) data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void grey_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert y16 (16 bit greyscale format) to yu12
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing y16 (16 bit greyscale) data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void y16_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert y16x (16 bit greyscale format - be) to yu12
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing y16x (16 bit greyscale bigendian) data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void y16x_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert SPCA501 (s501) to yuv 420 planar (yu12)
 *   s501  |Y0..width..Y0|U..width/2..U|Y1..width..Y1|V..width/2..V|
 *   signed values (-128;+127) must be converted to unsigned (0; 255)
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing s501 data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void s501_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert SPCA505 (s505) to yuv 420 planar (yu12)
 *   s505  |Y0..width..Y0|Y1..width..Y1|U..width/2..U|V..width/2..V|
 *   signed values (-128;+127) must be converted to unsigned (0; 255)
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing s501 data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void s505_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert SPCA508 (s508) to yuv 420 planar (yu12)
 *   s508  |Y0..width..Y0|U..width/2..U|V..width/2..V|Y1..width..Y1|
 *   signed values (-128;+127) must be converted to unsigned (0; 255)
 * args:
 *   out: pointer to output buffer (yu12)
 *   in: pointer to input buffer containing s501 data frame
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void s508_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert rgb24 to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing rgb24 data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void rgb24_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert bgr24 to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing bgr24 data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void bgr24_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert rgb1 (rgb332) to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing rgb332 data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void rgb1_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert ar12 (argb444) to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing argb444 data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void ar12_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert ar15 (argb555) to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing argb555 data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void ar15_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert ar15_be (argb555X) to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing argb555X (be) data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void ar15x_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert rgbp (rgb565) to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing argb555 data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void rgbp_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert rgbr (rgb565X) to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing rgb565 bigendian data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void rgbr_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert bgrh to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing bgrh (bgr666) data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void bgrh_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert ar24 to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing ar24 (bgr32) data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void ar24_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert ba24 to yu12
 * args:
 *   out: pointer to output buffer containing yu12 data
 *   in: pointer to input buffer containing ba24 (rgb32) data
 *   width: picture width
 *   height: picture height
 *
 * asserts:
 *   out is not null
 *   in is not null
 *
 * returns: none
 */
void ba24_to_yu12(uint8_t *out, uint8_t *in, int width, int height);

/*
 * yu12 to rgb24
 * args:
 *    out - pointer to output rgb data buffer
 *    in - pointer to input yu12 data buffer
 *    width - buffer width (in pixels)
 *    height - buffer height (in pixels)
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void yu12_to_rgb24 (uint8_t *out, uint8_t *in, int width, int height);

/*
 * table_int initialize float calc 1.402 0.34414 0.71414 1.772
 * args:
 *    out - pointer to output rgb data buffer
 *    in - pointer to input yu12 data buffer
 *    width - buffer width (in pixels)
 *    height - buffer height (in pixels)
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void init_yuv2rgb_num_table();

/*
 * yu12 to rgb24 high efficiency, use table inquer improve excution efficency
 * args:
 *    out - pointer to output rgb data buffer
 *    in - pointer to input yu12 data buffer
 *    width - buffer width (in pixels)
 *    height - buffer height (in pixels)
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void yu12_to_rgb24_higheffic (uint8_t *out, uint8_t *in, int width, int height);

/*
 * FIXME:  yu12 to bgr24 with lines upsidedown
 *   used for bitmap files (DIB24)
 * args:
 *    out - pointer to output bgr data buffer
 *    in - pointer to input yu12 data buffer
 *    width - buffer width (in pixels)
 *    height - buffer height (in pixels)
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void yu12_to_dib24 (uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert yuv 420 planar (yu12) to yuv 422 (save_image_jpeg)
 * args:
 *    out- pointer to output buffer (yuyv)
 *    in- pointer to input buffer (yuv420 planar data frame (yu12))
 *    width- picture width
 *    height- picture height
 *
 * asserts:
 *    out is not null
 *    in is not null
 *
 * returns: none
 */
void yu12_to_yuyv (uint8_t *out, uint8_t *in, int width, int height);

/*
 * convert bayer raw data to rgb24
 * args:
 *   pBay: pointer to buffer containing Raw bayer data
 *   pRGB24: pointer to buffer containing rgb24 data
 *   width: picture width
 *   height: picture height
 *   pix_order: bayer pixel order (0=gb/rg   1=gr/bg  2=bg/gr  3=rg/bg)
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void bayer_to_rgb24(uint8_t *pBay, uint8_t *pRGB24, int width, int height, int pix_order);

#if MJPG_BUILTIN

/*
 * used for internal jpeg decoding  420 planar to 422
 * args:
 *   out: pointer to data output of idct (macroblocks yyyy u v)
 *   pic: pointer to picture buffer (yuyv)
 *   width: picture width
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void yuv420pto422(int *out, uint8_t *pic, int width);

/*
 * used for internal jpeg decoding 422 planar to 422
 * args:
 *   out: pointer to data output of idct (macroblocks yyyy u v)
 *   pic: pointer to picture buffer (yuyv)
 *   width: picture width
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void yuv422pto422(int *out, uint8_t *pic, int width);

/*
 * used for internal jpeg decoding 444 planar to 422
 * args:
 *   out: pointer to data output of idct (macroblocks yyyy u v)
 *   pic: pointer to picture buffer (yuyv)
 *   width: picture width
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void yuv444pto422(int *out, uint8_t *pic, int width);

/*
 * used for internal jpeg decoding 400 planar to 422
 * args:
 *   out: pointer to data output of idct (macroblocks yyyy )
 *   pic: pointer to picture buffer (yuyv)
 *   width: picture width
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void yuv400pto422(int *out, uint8_t *pic, int width);

#endif

#endif

