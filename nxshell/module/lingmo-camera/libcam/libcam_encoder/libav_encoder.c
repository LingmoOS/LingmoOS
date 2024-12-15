/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           George Sedov <radist.morse@gmail.com>                               #
#                   - Threaded encoding                                         #
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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
/* support for internationalization - i18n */
#include <locale.h>
#include <libintl.h>

#include "gviewencoder.h"
#include "encoder.h"

#define AV_RB16(x)                           \
    ((((const uint8_t*)(x))[0] << 8) |          \
      ((const uint8_t*)(x))[1])


/*
 * set yu12 frame in codec data frame
 * args:
 *    video_codec_data - pointer to video codec data
 *    inp - input data (yu12)
 *    width - frame width
 *    height - frame height 
 *
 * asserts:
 *    video_codec_data is not null
 *    inp is not null
 *
 * returns: none
 */
void prepare_video_frame(encoder_codec_data_t *video_codec_data, uint8_t *inp, int width, int height)
{
	/*assertions*/
	assert(video_codec_data);
	assert(inp);

	int size = width * height;

	video_codec_data->frame->format = AV_PIX_FMT_YUV420P;
	video_codec_data->frame->width = width;
	video_codec_data->frame->height = height;
	
	video_codec_data->frame->data[0] = inp; //Y
	video_codec_data->frame->data[1] = inp + size; //U
	video_codec_data->frame->data[2] = video_codec_data->frame->data[1] + size/4; //V
	video_codec_data->frame->linesize[0] = width;
	video_codec_data->frame->linesize[1] = width / 2;
	video_codec_data->frame->linesize[2] = width / 2;
}

/*
 * split xiph headers from libav private data
 * args:
 *    extradata - libav codec private data
 *    extradata_size - codec private data size
 *    first_header_size - first header size
 *    header_start - first 3 bytes of header
 *    header_len - header length
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
int avpriv_split_xiph_headers(
		uint8_t *extradata,
		int extradata_size,
		int first_header_size,
		uint8_t *header_start[3],
        int header_len[3])
{
    int i;

     if (extradata_size >= 6 && AV_RB16(extradata) == first_header_size) {
        int overall_len = 6;
        for (i=0; i<3; i++) {
            header_len[i] = AV_RB16(extradata);
            extradata += 2;
            header_start[i] = extradata;
            extradata += header_len[i];
            if (overall_len > extradata_size - header_len[i])
                return -1;
            overall_len += header_len[i];
        }
    } else if (extradata_size >= 3 && extradata_size < INT_MAX - 0x1ff && extradata[0] == 2) {
        int overall_len = 3;
        extradata++;
        for (i=0; i<2; i++, extradata++) {
            header_len[i] = 0;
            for (; overall_len < extradata_size && *extradata==0xff; extradata++) {
                header_len[i] += 0xff;
                overall_len   += 0xff + 1;
            }
            header_len[i] += *extradata;
            overall_len   += *extradata;
            if (overall_len > extradata_size)
                return -1;
        }
        header_len[2] = extradata_size - overall_len;
        header_start[0] = extradata;
        header_start[1] = header_start[0] + header_len[0];
        header_start[2] = header_start[1] + header_len[1];
    } else {
        return -1;
    }
    return 0;
}

