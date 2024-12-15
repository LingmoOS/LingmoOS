/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
#                             Add UYVY color support(Macbook iSight)            #
#           Flemming Frandsen <dren.dk@gmail.com>                               #
#                             Add VU meter OSD                                  #
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
#  Encoder library                                                                #
#                                                                               #
********************************************************************************/

#ifndef GVIEWENCODER_H
#define GVIEWENCODER_H

#include <features.h>

#include <inttypes.h>
#include <sys/types.h>

/*make sure we support c++*/
__BEGIN_DECLS

/*encoder modes*/
#define ENCODER_MODE_NONE   (0)
#define ENCODER_MODE_RAW    (1)

/*Muxer defs*/
#define ENCODER_MUX_MKV        (0)
#define ENCODER_MUX_WEBM       (1)
#define ENCODER_MUX_AVI        (2)
#define ENCODER_MUX_MP4        (3)

/*Scheduler Modes*/
#define ENCODER_SCHED_LIN  (0)
#define ENCODER_SCHED_EXP  (1)

/*audio sample format*/
#ifndef GV_SAMPLE_TYPE_INT16
#define GV_SAMPLE_TYPE_INT16  (0) //interleaved
#define GV_SAMPLE_TYPE_FLOAT  (1) //interleaved
#define GV_SAMPLE_TYPE_INT16P (2) //planar
#define GV_SAMPLE_TYPE_FLOATP (3) //planar
#endif

#define MAX_DELAYED_FRAMES 20  /*Maximum supported delayed frames*/

__attribute__((unused)) static int my_video_codec_ind = 0;

/*video buffer*/
typedef struct _video_buffer_t {
    uint8_t *frame;  /*uncompressed*/
    int frame_size;
    int64_t timestamp;
    int keyframe;  /* 1-keyframe; 0-non keyframe (only for direct input)*/
    int flag;      /*VIDEO_BUFF_FREE | VIDEO_BUFF_USED*/
} video_buffer_t;

/*video codec properties*/
typedef struct _video_codec_t {
    int valid;                //the encoding codec exists in libav
    char compressor[5];       //fourcc - upper case
    int mkv_4cc;              //fourcc WORD value
    char mkv_codec[25];       //mkv codecID
    void *mkv_codecPriv;      //mkv codec private data
    char description[35];     //codec description
    int pix_fmt;              //codec input pixel format
    int fps;                  // encoder frame rate (used for time base)
    int bit_rate;             //lavc default bit rate
    int qmax;                 //lavc qmax
    int qmin;                 //lavc qmin
    int max_qdiff;            //lavc qmin
    int dia;                  //lavc dia_size
    int pre_dia;              //lavc pre_dia_size
    int pre_me;               //lavc pre_me
    int me_pre_cmp;           //lavc me_pre_cmp
    int me_cmp;               //lavc me_cmp
    int me_sub_cmp;           //lavc me_sub_cmp
    int last_pred;            //lavc last_predictor_count
    int gop_size;             //lavc gop_size
    float qcompress;          //lavc qcompress
    float qblur;              //lavc qblur
    int subq;                 //lavc subq
    int framerefs;            //lavc refs
    int codec_id;             //lavc codec_id
    char codec_name[20];      //lavc codec_name
    int mb_decision;          //lavc mb_decision
    int trellis;              //lavc trellis quantization
    int me_method;            //lavc motion estimation method
    int mpeg_quant;           //lavc mpeg quantization
    int max_b_frames;         //lavc max b frames
    int num_threads;          //lavc num threads
    int flags;                //lavc flags
    int monotonic_pts;        //use monotonic pts instead of timestamp based
} video_codec_t;

/*audio codec properties*/
typedef struct _audio_codec_t {
    int valid;                //the encoding codec exists in ffmpeg
    int bits;                 //bits per sample (pcm only)
    int monotonic_pts;
    uint16_t avi_4cc;         //fourcc value (4 bytes)
    char mkv_codec[25];       //mkv codecID
    char description[35];     //codec description
    int bit_rate;             //lavc default bit rate
    int codec_id;             //lavc codec_id
    char codec_name[20];      //lavc codec name
    int sample_format;        //lavc sample format
    int profile;              //for AAC only
    void *mkv_codpriv;        //pointer for codec private data
    int codpriv_size;         //size in bytes of private data
    int flags;                //lavc flags
    char name[5];             //common 4cc name (lowercase)
} audio_codec_t;

/*video*/
typedef struct _encoder_video_context_t {
    void *codec_data;

    int monotonic_pts;

    /*delayed frames handling*/
    int write_df; /*index of delayed frame pts for write;*/
    int read_df; /*index of delayed frame pts for read;*/
    int64_t delayed_pts[MAX_DELAYED_FRAMES]; /*delayed frames pts*/
    int flush_delayed_frames;
    int flushed_buffers;
    int flush_done;

    uint8_t *priv_data;

    uint8_t *tmpbuf;

    int outbuf_size;
    uint8_t *outbuf;
    int outbuf_coded_size;

    int64_t framecount;

    int64_t pts;
    int64_t dts;
    int flags;
    int duration;

} encoder_video_context_t;

/*Audio*/
typedef struct _encoder_audio_context_t {
    void *codec_data;

    int avi_4cc;

    int monotonic_pts;

    /*delayed frames handling*/
    int flush_delayed_frames;
    int flushed_buffers;
    int flush_done;

    uint8_t *priv_data;

    int outbuf_size;
    uint8_t *outbuf;
    int outbuf_coded_size;

    int64_t pts;
    int64_t dts;
    int flags;
    int duration;

} encoder_audio_context_t;


typedef struct _encoder_context_t {
    int muxer_id;

    int input_format;
    int video_codec_ind;
    int audio_codec_ind;

    int video_width;
    int video_height;

    int fps_num;
    int fps_den;

    int audio_channels;
    int audio_samprate;

    encoder_video_context_t *enc_video_ctx;
    encoder_audio_context_t *enc_audio_ctx;

    /*external h264 encoder data*/
    int h264_pps_size;
    uint8_t *h264_pps;
    int h264_sps_size;
    uint8_t *h264_sps;

} encoder_context_t;

/*
 * set verbosity
 * args:
 *   value - verbosity value
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void encoder_set_verbosity(int value);

/*
 * get valid video codec count
 * args:
 *   none
 *
 * asserts:
 *    none
 *
 * returns: number of valid video codecs
 */
int encoder_get_valid_video_codecs();

/*
 * get valid audio codec count
 * args:
 *   none
 *
 * asserts:
 *    none
 *
 * returns: number of valid audio codecs
 */
int encoder_get_valid_audio_codecs();

/*
 * sets the valid flag in the video codecs list
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: number of valid video codecs in list
 */
int encoder_set_valid_video_codec_list ();

/*
 * sets the valid flag in the audio codecs list
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: number of valid audio codecs in list
 */
int encoder_set_valid_audio_codec_list ();

/*
 * get video list codec description
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: list codec entry or NULL if none
 */
const char *encoder_get_video_codec_description(int codec_ind);

/*
 * get audio list codec description
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: list codec entry or NULL if none
 */
const char *encoder_get_audio_codec_description(int codec_ind);

/*
 * encoder initialization
 * args:
 *   input_format - input v4l2 format (yuyv for encoding)
 *   video_codec_ind - video codec list index
 *   audio_codec_ind - audio codec list index
 *   muxer_id - file muxer:
 *        ENCODER_MUX_MKV; ENCODER_MUX_WEBM; ENCODER_MUX_AVI
 *   video_width - video frame width
 *   video_height - video frame height
 *   fps_num - fps numerator
 *   fps_den - fps denominator
 *   audio_channels- audio channels
 *   audio_samprate- audio sample rate
 *
 * asserts:
 *   none
 *
 * returns: pointer to encoder context (NULL on error)
 */
encoder_context_t *encoder_init(
    int input_format,
    int video_codec_ind,
    int audio_codec_ind,
    int muxer_id,
    int video_width,
    int video_height,
    int fps_num,
    int fps_den,
    int audio_channels,
    int audio_samprate);

/*
 * initialization of the file muxer
 * args:
 *   encoder_ctx - pointer to encoder context
 *   filename - video filename
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: none
 */
void encoder_muxer_init(encoder_context_t *encoder_ctx, const char *filename);

/*
 * close the file muxer
 * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void encoder_muxer_close(encoder_context_t *encoder_ctx);

/*
 * get video list codec entry for codec index
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: list codec entry or NULL if none
 */
video_codec_t *encoder_get_video_codec_defaults(int codec_ind);

/*
 * get audio list codec entry for codec index
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: audio list codec entry or NULL if none
 */
audio_codec_t *encoder_get_audio_codec_defaults(int codec_ind);

/*
 * checks if the video codec index corresponds to VP8 or VP9 (webm) codec
 * args:
 *    codec_ind - video codec list index
 *
 * asserts:
 *    none
 *
 * returns: 1 true; 0 false
 */
int encoder_check_webm_video_codec(int codec_ind);

/*
 * get video codec list index for avi 4cc
 * args:
 *   codec_4cc - codec 4cc
 *
 * asserts:
 *   none
 *
 * returns: codec index or -1 if error
 */
int encoder_get_video_codec_ind_4cc(const char *codec_4cc);

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
 * get audio codec name
 * args:
 *   codec_ind - codec list index
 *
 * asserts:
 *   none
 *
 * returns: codec name entry
 */
const char *encoder_get_audio_codec_name(int codec_ind);

/*
 * get audio sample format max value
 * args:
 *   none
 *
 * returns the maximum audio sample format value
 */
int encoder_get_max_audio_sample_fmt();

/*
 * get audio codec list index for codec name
 * args:
 *   codec_name - codec common name
 *
 * asserts:
 *   none
 *
 * returns: codec index or -1 if error
 */
int encoder_get_audio_codec_ind_name(const char *codec_name);

/*
 * get the audio encoder frame size
 * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: audio encoder frame size
 */
int encoder_get_audio_frame_size(encoder_context_t *encoder_ctx);

/*
 * get the audio encoder input sample format
 * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: audio encoder sample format
 */
int encoder_get_audio_sample_fmt(encoder_context_t *encoder_ctx);

/*
 * get the video codec index for VP9 (webm) codec
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: index for VP9 codec or -1 if error
 */
int encoder_get_webm_video_codec_index();

/*
 * checks if the audio codec index corresponds to Vorbis (webm) codec
 * args:
 *    codec_ind - audio codec list index
 *
 * asserts:
 *    none
 *
 * returns: 1 true; 0 false
 */
int encoder_check_webm_audio_codec(int codec_ind);

/*
 * get the audio codec index for Vorbis (webm) codec
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: index for Vorbis codec or -1 if error
 */
int encoder_get_webm_audio_codec_index();

/*
 * get the mkv codec private data
 * args:
 *    codec_ind - codec list index
 *
 * asserts:
 *    none
 *
 * returns: pointer to mkvCodecPriv data
 */
void *encoder_get_video_mkvCodecPriv(int codec_ind);

/*
 * set the video codec mkv private data
 * args:
 *    encoder_ctx - pointer to encoder context
 *
 * asserts:
 *    encoder_ctx is not null
 *
 * returns: mkvCodecPriv size
 */
int encoder_set_video_mkvCodecPriv(encoder_context_t *encoder_ctx);

/*
 * get the mkv codec private data
 * args:
 *    codec_ind - codec list index
 *
 * asserts:
 *    none
 *
 * returns: pointer to mkvCodecPriv data
 */
void *encoder_get_audio_mkvCodecPriv(int codec_ind);

/*
 * set the audio codec mkv private data
 * args:
 *    encoder_ctx - pointer to encoder context
 *
 * asserts:
 *    encoder_ctx is not null
 *
 * returns: mkvCodecPriv size
 */
int encoder_set_audio_mkvCodecPriv(encoder_context_t *encoder_ctx);

/*
 * get an estimated write loop sleep time to avoid a ring buffer overrun
 * args:
 *   mode: scheduler mode:
 *      0 - linear funtion; 1 - exponencial funtion
 *   thresh: ring buffer threshold in wich scheduler becomes active:
 *      [0.2 (20%) - 0.9 (90%)]
 *   max_time - maximum scheduler time (in ms)
 *
 * asserts:
 *   none
 *
 * returns: estimate sleep time (milisec)
 */
double encoder_buff_scheduler(int mode, double thresh, double max_time);

/*
 * store unprocessed input video frame in video ring buffer
 * args:
 *   frame - pointer to unprocessed frame data
 *   size - frame size (in bytes)
 *   timestamp - frame timestamp (in nanosec)
 *   isKeyframe - flag if it's a key(IDR) frame
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int encoder_add_video_frame(uint8_t *frame, int size, int64_t timestamp, int isKeyframe);

/*
 * process next video frame on the ring buffer (encode and mux to file)
 * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: error code
 */
int encoder_process_next_video_buffer(encoder_context_t *encoder_ctx);

/*
 * process all used video frames from buffer
  * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: error code
 */
int encoder_flush_video_buffer(encoder_context_t *encoder_ctx);

/*
 * process audio buffer (encode and mux to file)
 * args:
 *   encoder_ctx - pointer to encoder context
 *   data - audio buffer
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: error code
 */
int encoder_process_audio_buffer(encoder_context_t *encoder_ctx, void *data);

/*
 * process all delayed audio frames from libavcodec
  * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null
 *
 * returns: error code
 */
int encoder_flush_audio_buffer(encoder_context_t *encoder_ctx);

/*
 * encode video frame
 * args:
 *   encoder_ctx - pointer to encoder context
 *   yuv_frame - yuyv input frame
 *
 * asserts:
 *   none
 *
 * returns: encoded buffer size
 */
int encoder_encode_video(encoder_context_t *encoder_ctx, void *yuv_frame);

/*
 * encode audio
 * args:
 *   encoder_ctx - pointer to encoder context
 *   audio_data - pointer to audio pcm data
 *
 * asserts:
 *   none
 *
 * returns: encoded buffer size
 */
int encoder_encode_audio(encoder_context_t *encoder_ctx, void *audio_data);

/*
 * close and clean encoder context
 * args:
 *   encoder_ctx - pointer to encoder context data
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void encoder_close(encoder_context_t *encoder_ctx);

/*
 * mux a video frame
 * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null;
 *
 * returns: none
 */
int encoder_write_video_data(encoder_context_t *encoder_ctx);

/*
 * mux a audio frame
 * args:
 *   encoder_ctx - pointer to encoder context
 *
 * asserts:
 *   encoder_ctx is not null;
 *
 * returns: none
 */
int encoder_write_audio_data(encoder_context_t *encoder_ctx);

/*
 * function to determine if enought free space is available
 * args:
 *   treshold: limit treshold in Kbytes (min. free space)
 *
 * asserts:
 *   none
 *
 * returns: 1 if still enough free space left on disk
 *          0 otherwise
 */
int encoder_disk_supervisor(int treshold, const char *path);

/*
 * sets the current video codec index
 * args:
 *   index - codec index
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void set_video_codec_ind(int index);

/*
 * converts string to lowercase
 * args:
 *   str - string pointer
 *
 * asserts:
 *   none
 *
 * returns: pointer to converted string
 */
char *lowercase(char *str);

__END_DECLS

#endif
