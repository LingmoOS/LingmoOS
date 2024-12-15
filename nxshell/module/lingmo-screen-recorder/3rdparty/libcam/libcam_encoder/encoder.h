/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           George Sedov <radist.morse@gmail.com>                               #
#                  - Threaded encoding                                          #
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

#ifndef ENCODER_H
#define ENCODER_H

#include <inttypes.h>
#include <sys/types.h>

#include "cameraconfig.h"

#ifdef HAVE_FFMPEG_AVCODEC_H
#include <ffmpeg/avcodec.h>
#else
#include <libavcodec/avcodec.h>
#ifdef HAVE_LIBAVUTIL_VERSION_H
#include <libavutil/version.h>
#endif
#include <libavutil/avutil.h>
#endif

#define LIBAVCODEC_VER_AT_LEAST(major,minor)  (LIBAVCODEC_VERSION_MAJOR > major || \
                                              (LIBAVCODEC_VERSION_MAJOR == major && \
                                               LIBAVCODEC_VERSION_MINOR >= minor))

#ifdef LIBAVUTIL_VERSION_MAJOR
#define LIBAVUTIL_VER_AT_LEAST(major,minor)  (LIBAVUTIL_VERSION_MAJOR > major || \
                                              (LIBAVUTIL_VERSION_MAJOR == major && \
                                               LIBAVUTIL_VERSION_MINOR >= minor))
#else
#define LIBAVUTIL_VER_AT_LEAST(major,minor) 0
#endif

#ifndef X264_ME_HEX
#define X264_ME_HEX 1
#endif

#if !LIBAVUTIL_VER_AT_LEAST(52,0)
#define AV_PIX_FMT_NONE     PIX_FMT_NONE
#define AV_PIX_FMT_YUVJ420P PIX_FMT_YUVJ420P
#define AV_PIX_FMT_YUV420P  PIX_FMT_YUV420P
#endif


/* Possible Audio formats */
#define WAVE_FORMAT_UNKNOWN             (0x0000)
#define WAVE_FORMAT_PCM                 (0x0001)
#define WAVE_FORMAT_ADPCM               (0x0002)
#define WAVE_FORMAT_IEEE_FLOAT          (0x0003)
#define WAVE_FORMAT_IBM_CVSD            (0x0005)
#define WAVE_FORMAT_ALAW                (0x0006)
#define WAVE_FORMAT_MULAW               (0x0007)
#define WAVE_FORMAT_OKI_ADPCM           (0x0010)
#define WAVE_FORMAT_DVI_ADPCM           (0x0011)
#define WAVE_FORMAT_DIGISTD             (0x0015)
#define WAVE_FORMAT_DIGIFIX             (0x0016)
#define WAVE_FORMAT_YAMAHA_ADPCM        (0x0020)
#define WAVE_FORMAT_DSP_TRUESPEECH      (0x0022)
#define WAVE_FORMAT_GSM610              (0x0031)
#define WAVE_FORMAT_MP3                 (0x0055)
#define WAVE_FORMAT_MPEG12              (0x0050)
#define WAVE_FORMAT_AAC                 (0x00ff)
#define WAVE_FORMAT_IBM_MULAW           (0x0101)
#define WAVE_FORMAT_IBM_ALAW            (0x0102)
#define WAVE_FORMAT_IBM_ADPCM           (0x0103)
#define WAVE_FORMAT_AC3                 (0x2000)
/*extra audio formats (codecs)*/
#define ANTEX_FORMAT_ADPCME     (0x0033)
#define AUDIO_FORMAT_APTX       (0x0025)
#define AUDIOFILE_FORMAT_AF10       (0x0026)
#define AUDIOFILE_FORMAT_AF36       (0x0024)
#define BROOKTREE_FORMAT_BTVD       (0x0400)
#define CANOPUS_FORMAT_ATRAC        (0x0063)
#define CIRRUS_FORMAT_CIRRUS        (0x0060)
#define CONTROL_FORMAT_CR10     (0x0037)
#define CONTROL_FORMAT_VQLPC        (0x0034)
#define CREATIVE_FORMAT_ADPCM       (0x0200)
#define CREATIVE_FORMAT_FASTSPEECH10    (0x0203)
#define CREATIVE_FORMAT_FASTSPEECH8 (0x0202)
#define IMA_FORMAT_ADPCM        (0x0039)
#define CONSISTENT_FORMAT_CS2       (0x0260)
#define HP_FORMAT_CU            (0x0019)
#define DEC_FORMAT_G723         (0x0123)
#define DF_FORMAT_G726          (0x0085)
#define DSP_FORMAT_ADPCM        (0x0036)
#define DOLBY_FORMAT_AC2        (0x0030)
#define DOLBY_FORMAT_AC3_SPDIF      (0x0092)
#define ESS_FORMAT_ESPCM        (0x0061)
#define IEEE_FORMAT_FLOAT       (0x0003)
#define MS_FORMAT_MSAUDIO1_DIVX     (0x0160)
#define MS_FORMAT_MSAUDIO2_DIVX     (0x0161)
#define OGG_FORMAT_VORBIS       (0x566f)
#define OGG_FORMAT_VORBIS1      (0x674f)
#define OGG_FORMAT_VORBIS1P     (0x676f)
#define OGG_FORMAT_VORBIS2      (0x6750)
#define OGG_FORMAT_VORBIS2P     (0x6770)
#define OGG_FORMAT_VORBIS3      (0x6751)
#define OGG_FORMAT_VORBIS3P     (0x6771)
#define MS_FORMAT_WMA9          (0x0163)
#define MS_FORMAT_WMA9_PRO      (0x0162)

/*video buffer flags*/
#define VIDEO_BUFF_FREE    (0)
#define VIDEO_BUFF_USED    (1)

/*
 * set pause timestamp
 * args:
 *   value - timestamp value
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void set_video_pause_timestamp(int64_t timestamp);

/*
 * get pause timestamp
 * args:
 *   value: nome
 *
 * asserts:
 *    none
 *
 * returns: pause timestamp
 */
void /*__attribute__ ((constructor)) */gviewencoder_init();
int64_t get_video_pause_timestamp(void);


/*
 * codec data struct used for encoder context
 * we set all avcodec stuff here so that we don't
 * need to export any of it's symbols in the public API
 */
typedef struct _encoder_codec_data_t {
    AVCodec *codec;
    AVDictionary *private_options;
    AVCodecContext *codec_context;
    AVFrame *frame;
    AVPacket *outpkt;
} encoder_codec_data_t;

typedef struct _bmp_info_header_t {
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
 * get default mkv_codecPriv
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: pointer to bmp_info_header_t
 */
bmp_info_header_t *get_default_mkv_codecPriv();

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
    int header_len[3]);

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
void prepare_video_frame(encoder_codec_data_t *encoder_ctx, uint8_t *inp, int width, int height);


/*
 * returns the real codec array index
 * args:
 *   codec_id - codec id
 *
 * asserts:
 *   none
 *
 * returns: real index or -1 if none
 */
int get_video_codec_index(int codec_id);

/*
 * returns the list codec index
 * args:
 *   codec_id - codec id
 *
 * asserts:
 *   none
 *
 * returns: list index or -1 if none
 */
int get_video_codec_list_index(int codec_id);

/*
 * returns the real codec array index
 * args:
 *   codec_id - codec id
 *
 * asserts:
 *   none
 *
 * returns: real index or -1 if none
 */
int get_audio_codec_index(int codec_id);

/*
 * returns the list codec index
 * args:
 *   codec_id - codec id
 *
 * asserts:
 *   none
 *
 * returns: real index or -1 if none
 */
int get_audio_codec_list_index(int codec_id);

/*
 * get audio mkv codec
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: mkv codec entry or NULL if none
 */
const char *encoder_get_audio_mkv_codec(int codec_ind);

/*
 * get video mkv codec
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: mkv codec entry or NULL if none
 */
const char *encoder_get_video_mkv_codec(int codec_ind);

/*
 * get video compressor (avi 4cc code)
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: compressor codec entry or NULL if none
 */
const char *encoder_get_video_codec_4cc(int codec_ind);

/*
 * get audio codec bits
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: bits entry from audio codec list
 */
int encoder_get_audio_bits(int codec_ind);

/*
 * get audio codec bit rate
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: bit_rate entry from audio codec list
 */
int encoder_get_audio_bit_rate(int codec_ind);

#endif
