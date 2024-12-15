/*
* Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     shicetu <shicetu@uniontech.com>
*             hujianbo <hujianbo@uniontech.com>
* Maintainer: shicetu <shicetu@uniontech.com>
*             hujianbo <hujianbo@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LOAD_LIBS_H
#define LOAD_LIBS_H
#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <pthread.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
//#include <libavutil/dict.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libffmpegthumbnailer/videothumbnailerc.h>
#include <libusb.h>
#include <portaudio.h>
#include <libv4l2.h>
#include <libudev.h>

//AVCodec *avcodec_find_encoder(enum AVCodecID id);
typedef AVCodec *(*uos_avcodec_find_encoder)(enum AVCodecID id);
//AVCodec *avcodec_find_decoder(enum AVCodecID id);
typedef AVCodec *(*uos_avcodec_find_decoder)(enum AVCodecID id);
typedef AVCodec *(*uos_avcodec_find_encoder_by_name)(const char *name);
typedef AVCodecContext *(*uos_avcodec_alloc_context)(const AVCodec *codec);
typedef void (*uos_avcodec_free_context)(AVCodecContext **avctx);
typedef AVCodecContext *(*uos_avcodec_alloc_context3)(const AVCodec *codec);
//int avcodec_get_context_defaults3(AVCodecContext *s, const AVCodec *codec);
typedef int (*uos_avcodec_get_context_defaults3)(AVCodecContext *s, const AVCodec *codec);
//int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
typedef int (*uos_avcodec_open2)(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
//void avcodec_flush_buffers(AVCodecContext *avctx);
typedef void (*uos_avcodec_flush_buffers)(AVCodecContext *avctx);
//int avcodec_close(AVCodecContext *avctx);
typedef int (*uos_avcodec_close)(AVCodecContext *avctx);
//int avcodec_parameters_from_context(AVCodecParameters *par,const AVCodecContext *codec);
typedef int (*uos_avcodec_parameters_from_context)(AVCodecParameters *par, const AVCodecContext *codec);
//int avcodec_parameters_to_context(AVCodecContext *codec, const AVCodecParameters *par);
typedef int (*uos_avcodec_parameters_to_context)(AVCodecContext *codec, const AVCodecParameters *par);
//int avcodec_send_packet(AVCodecContext *avctx, const AVPacket *avpkt);
typedef int (*uos_avcodec_send_packet)(AVCodecContext *avctx, const AVPacket *avpkt);
//int avcodec_receive_frame(AVCodecContext *avctx, AVFrame *frame);
typedef int (*uos_avcodec_receive_frame)(AVCodecContext *avctx, AVFrame *frame);
//int avcodec_receive_packet(AVCodecContext *avctx, AVPacket *avpkt);
typedef int (*uos_avcodec_receive_packet)(AVCodecContext *avctx, AVPacket *avpkt);
//void av_packet_free(AVPacket **pkt);
typedef void (*uos_av_packet_free)(AVPacket **pkt);
//int avcodec_fill_audio_frame(AVFrame *frame, int nb_channels,enum AVSampleFormat sample_fmt, const uint8_t *buf,int buf_size, int align);
typedef int (*uos_avcodec_fill_audio_frame)(AVFrame *frame, int nb_channels, enum AVSampleFormat sample_fmt, const uint8_t *buf, int buf_size, int align);
//void av_packet_unref(AVPacket *pkt);
typedef void (*uos_av_packet_unref)(AVPacket *pkt);
//int avcodec_send_frame(AVCodecContext *avctx, const AVFrame *frame);
typedef int (*uos_avcodec_send_frame)(AVCodecContext *avctx, const AVFrame *frame);
//void av_packet_rescale_ts(AVPacket *pkt, AVRational tb_src, AVRational tb_dst);
typedef void (*uos_av_packet_rescale_ts)(AVPacket *pkt, AVRational tb_src, AVRational tb_dst);
//void av_init_packet(AVPacket *pkt);
typedef void (*uos_av_init_packet)(AVPacket *pkt);
//AVPacket *av_packet_alloc(void);
typedef AVPacket *(*uos_av_packet_alloc)(void);


typedef video_thumbnailer *(*uos_video_thumbnailer)();
typedef void (*uos_video_thumbnailer_destroy)(video_thumbnailer *thumbnailer);
/* create image_data structure */
typedef image_data *(*uos_video_thumbnailer_create_image_data)(void);
/* destroy image_data structure */
typedef void (*uos_video_thumbnailer_destroy_image_data)(image_data *data);
typedef int (*uos_video_thumbnailer_generate_thumbnail_to_buffer)(video_thumbnailer *thumbnailer, const char *movie_filename, image_data *generated_image_data);

//lswresample
//void swr_free(struct SwrContext **s);
typedef void (*uos_swr_free)(struct SwrContext **s);

//lswscale
//void sws_freeContext(struct SwsContext *swsContext);
typedef void (*uos_sws_freeContext)(struct SwsContext *swsContext);

typedef struct _LoadLibNames {
    char *chAvcodec;
    char *chFfmpegthumbnailer;
    char *chSwresample;
    char *chSwscale;
    char *chAvformat;
    char *chAvutil;
    char *chUdev;
    char *chUsb;
    char *chPortaudio;
    char *chV4l2;
} LoadLibNames;

void setLibNames(LoadLibNames tmp);

typedef struct _LoadLibs {
    uos_avcodec_find_encoder m_avcodec_find_encoder;
    uos_avcodec_find_decoder m_avcodec_find_decoder;
    uos_avcodec_find_encoder_by_name m_avcodec_find_encoder_by_name;
    uos_avcodec_alloc_context m_avcodec_alloc_context;
    uos_avcodec_free_context m_avcodec_free_context;
    uos_avcodec_alloc_context3 m_avcodec_alloc_context3;
    uos_avcodec_get_context_defaults3 m_avcodec_get_context_defaults3;
    uos_avcodec_open2 m_avcodec_open2;
    uos_avcodec_flush_buffers m_avcodec_flush_buffers;
    uos_avcodec_close m_avcodec_close;
    uos_avcodec_parameters_from_context m_avcodec_parameters_from_context;
    uos_avcodec_parameters_to_context m_avcodec_parameters_to_context;
    uos_avcodec_send_packet m_avcodec_send_packet;
    uos_avcodec_receive_frame m_avcodec_receive_frame;
    uos_avcodec_receive_packet m_avcodec_receive_packet;
    uos_av_packet_free m_av_packet_free;
    uos_avcodec_fill_audio_frame m_avcodec_fill_audio_frame;
    uos_av_packet_unref m_av_packet_unref;
    uos_avcodec_send_frame m_avcodec_send_frame;
    uos_av_packet_rescale_ts m_av_packet_rescale_ts;
    uos_av_init_packet m_av_init_packet;
    uos_av_packet_alloc m_av_packet_alloc;


    uos_video_thumbnailer m_video_thumbnailer;
    uos_video_thumbnailer_destroy m_video_thumbnailer_destroy;
    uos_video_thumbnailer_create_image_data m_video_thumbnailer_create_image_data;
    uos_video_thumbnailer_destroy_image_data m_video_thumbnailer_destroy_image_data;
    uos_video_thumbnailer_generate_thumbnail_to_buffer m_video_thumbnailer_generate_thumbnail_to_buffer;


    uos_swr_free m_swr_free;

    uos_sws_freeContext m_sws_freeContext;

} LoadLibs;

LoadLibs *getLoadLibsInstance();//饿汉式不支持延迟加载


typedef int (*uos_avformat_open_input)(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options);
typedef int (*uos_avformat_find_stream_info)(AVFormatContext *ic, AVDictionary **options);
typedef int (*uos_av_find_best_stream)(AVFormatContext *ic, enum AVMediaType type, int wanted_stream_nb, int related_stream, AVCodec **decoder_ret, int flags);
typedef void (*uos_av_dump_format)(AVFormatContext *ic, int index, const char *url, int is_output);
typedef void (*uos_avformat_close_input)(AVFormatContext **s);
//int avformat_alloc_output_context2(AVFormatContext **ctx, AVOutputFormat *oformat,const char *format_name, const char *filename);
typedef int (*uos_avformat_alloc_output_context2)(AVFormatContext **ctx, AVOutputFormat *oformat, const char *format_name, const char *filename);
//AVStream *avformat_new_stream(AVFormatContext *s, const AVCodec *c);
typedef AVStream *(*uos_avformat_new_stream)(AVFormatContext *s, const AVCodec *c);
//void avformat_free_context(AVFormatContext *s);
typedef void (*uos_avformat_free_context)(AVFormatContext *s);
//int avformat_write_header(AVFormatContext *s, AVDictionary **options);
typedef int (*uos_avformat_write_header)(AVFormatContext *s, AVDictionary **options);
//int avio_open(AVIOContext **s, const char *url, int flags);
typedef int (*uos_avio_open)(AVIOContext **s, const char *url, int flags);
//int av_write_frame(AVFormatContext *s, AVPacket *pkt);
typedef int (*uos_av_write_frame)(AVFormatContext *s, AVPacket *pkt);
//int av_write_trailer(AVFormatContext *s);
typedef int (*uos_av_write_trailer)(AVFormatContext *s);
//int avio_closep(AVIOContext **s);
typedef int (*uos_avio_closep)(AVIOContext **s);

typedef struct _LoadAvformat {
    uos_avformat_open_input m_avformat_open_input;
    uos_avformat_find_stream_info m_avformat_find_stream_info;
    uos_av_find_best_stream m_av_find_best_stream;
    uos_av_dump_format m_av_dump_format;
    uos_avformat_close_input m_avformat_close_input;
    uos_avformat_alloc_output_context2 m_avformat_alloc_output_context2;
    uos_avformat_new_stream m_avformat_new_stream;
    uos_avformat_free_context m_avformat_free_context;
    uos_avformat_write_header m_avformat_write_header;
    uos_avio_open m_avio_open;
    uos_av_write_frame m_av_write_frame;
    uos_av_write_trailer m_av_write_trailer;
    uos_avio_closep m_avio_closep;
} LoadAvformat;
LoadAvformat *getAvformat();

//libavutil不到1M
typedef AVDictionaryEntry *(*uos_av_dict_get)(const AVDictionary *m, const char *key, const AVDictionaryEntry *prev, int flags);
//int av_strerror(int errnum, char *errbuf, size_t errbuf_size);
typedef int (*uos_av_strerror)(int errnum, char *errbuf, size_t errbuf_size);
//int av_dict_copy(AVDictionary **dst, const AVDictionary *src, int flags);
typedef int (*uos_av_dict_copy)(AVDictionary **dst, const AVDictionary *src, int flags);
//void av_dict_free(AVDictionary **m);
typedef void (*uos_av_dict_free)(AVDictionary **m);
//int av_dict_set(AVDictionary **pm, const char *key, const char *value, int flags);
typedef int (*uos_av_dict_set)(AVDictionary **pm, const char *key, const char *value, int flags);
//int av_dict_set_int(AVDictionary **pm, const char *key, int64_t value, int flags);
typedef int (*uos_av_dict_set_int)(AVDictionary **pm, const char *key, int64_t value, int flags);
//void av_log_set_level(int level);
typedef void (*uos_av_log_set_level)(int level);
/*int av_image_copy_to_buffer(uint8_t *dst, int dst_size,
const uint8_t * const src_data[4], const int src_linesize[4],
enum AVPixelFormat pix_fmt, int width, int height, int align);*/
typedef int (*uos_av_image_copy_to_buffer)(uint8_t *dst, int dst_size,
                                           const uint8_t *const src_data[4], const int src_linesize[4],
                                           enum AVPixelFormat pix_fmt, int width, int height, int align);
//void av_frame_free(AVFrame **frame);
typedef void (*uos_av_frame_free)(AVFrame **frame);
//AVFrame *av_frame_alloc(void);
typedef AVFrame *(*uos_av_frame_alloc)(void);
//void av_freep(void *ptr);
typedef void (*uos_av_freep)(void *ptr);
//void av_frame_unref(AVFrame *frame);
typedef void (*uos_av_frame_unref)(AVFrame *frame);
//void av_free(void *ptr);
typedef void (*uos_av_free)(void *ptr);
/*int av_samples_get_buffer_size(int *linesize, int nb_channels, int nb_samples,
                               enum AVSampleFormat sample_fmt, int align);*/
typedef int (*uos_av_samples_get_buffer_size)(int *linesize, int nb_channels, int nb_samples,
                                              enum AVSampleFormat sample_fmt, int align);
//const char *av_get_media_type_string(enum AVMediaType media_type);
typedef const char *(*uos_av_get_media_type_string)(enum AVMediaType media_type);
//int av_image_get_buffer_size(enum AVPixelFormat pix_fmt, int width, int height, int align);
typedef int (*uos_av_image_get_buffer_size)(enum AVPixelFormat pix_fmt, int width, int height, int align);

typedef struct _LoadAvutil {
    uos_av_dict_get m_av_dict_get;
    uos_av_strerror m_av_strerror;
    uos_av_dict_copy m_av_dict_copy;
    uos_av_dict_free m_av_dict_free;
    uos_av_dict_set m_av_dict_set;
    uos_av_dict_set_int m_av_dict_set_int;
    uos_av_log_set_level m_av_log_set_level;
    uos_av_image_copy_to_buffer m_av_image_copy_to_buffer;
    uos_av_frame_free m_av_frame_free;
    uos_av_frame_alloc m_av_frame_alloc;
    uos_av_freep m_av_freep;
    uos_av_frame_unref m_av_frame_unref;
    uos_av_free m_av_free;
    uos_av_samples_get_buffer_size m_av_samples_get_buffer_size;
    uos_av_get_media_type_string m_av_get_media_type_string;//
    uos_av_image_get_buffer_size m_av_image_get_buffer_size;

} LoadAvutil;
LoadAvutil *getAvutil();

//struct udev_enumerate *udev_enumerate_new(struct udev *udev);
typedef struct udev_enumerate *(*uos_udev_enumerate_new)(struct udev *udev);
//struct udev *udev_enumerate_get_udev(struct udev_enumerate *udev_enumerate);
typedef struct udev *(*uos_udev_enumerate_get_udev)(struct udev_enumerate *udev_enumerate);
//int udev_enumerate_add_match_subsystem(struct udev_enumerate *udev_enumerate, const char *subsystem);
typedef int (*uos_udev_enumerate_add_match_subsystem)(struct udev_enumerate *udev_enumerate, const char *subsystem);
//int udev_enumerate_scan_devices(struct udev_enumerate *udev_enumerate);
typedef int (*uos_udev_enumerate_scan_devices)(struct udev_enumerate *udev_enumerate);
//struct udev_list_entry *udev_enumerate_get_list_entry(struct udev_enumerate *udev_enumerate);
typedef struct udev_list_entry *(*uos_udev_enumerate_get_list_entry)(struct udev_enumerate *udev_enumerate);
//const char *udev_list_entry_get_name(struct udev_list_entry *list_entry);
typedef const char *(*uos_udev_list_entry_get_name)(struct udev_list_entry *list_entry);
//struct udev_device *udev_device_new_from_syspath(struct udev *udev, const char *syspath);
typedef struct udev_device *(*uos_udev_device_new_from_syspath)(struct udev *udev, const char *syspath);
//const char *udev_device_get_devnode(struct udev_device *udev_device);
typedef const char *(*uos_udev_device_get_devnode)(struct udev_device *udev_device);
/*struct udev_device *udev_device_get_parent_with_subsystem_devtype(struct udev_device *udev_device,
                                                                  const char *subsystem, const char *devtype);*/
typedef struct udev_device *(*uos_udev_device_get_parent_with_subsystem_devtype)(struct udev_device *udev_device,
                                                                                 const char *subsystem, const char *devtype);
//const char *udev_device_get_sysattr_value(struct udev_device *udev_device, const char *sysattr);
typedef const char *(*uos_udev_device_get_sysattr_value)(struct udev_device *udev_device, const char *sysattr);
//struct udev_device *udev_device_unref(struct udev_device *udev_device);
typedef struct udev_device *(*uos_udev_device_unref)(struct udev_device *udev_device);
//struct udev_enumerate *udev_enumerate_unref(struct udev_enumerate *udev_enumerate);
typedef struct udev_enumerate *(*uos_udev_enumerate_unref)(struct udev_enumerate *udev_enumerate);
//struct udev *udev_new(void);
typedef struct udev *(*uos_udev_new)(void);
//struct udev_monitor *udev_monitor_new_from_netlink(struct udev *udev, const char *name);
typedef struct udev_monitor *(*uos_udev_monitor_new_from_netlink)(struct udev *udev, const char *name);
/*int udev_monitor_filter_add_match_subsystem_devtype(struct udev_monitor *udev_monitor,
                                                    const char *subsystem, const char *devtype);*/
typedef int (*uos_udev_monitor_filter_add_match_subsystem_devtype)(struct udev_monitor *udev_monitor,
                                                                   const char *subsystem, const char *devtype);
//int udev_monitor_enable_receiving(struct udev_monitor *udev_monitor);
typedef int (*uos_udev_monitor_enable_receiving)(struct udev_monitor *udev_monitor);
//int udev_monitor_get_fd(struct udev_monitor *udev_monitor);
typedef int (*uos_udev_monitor_get_fd)(struct udev_monitor *udev_monitor);
//struct udev_device *udev_monitor_receive_device(struct udev_monitor *udev_monitor);
typedef struct udev_device *(*uos_udev_monitor_receive_device)(struct udev_monitor *udev_monitor);
//const char *udev_device_get_subsystem(struct udev_device *udev_device);
typedef const char *(*uos_udev_device_get_subsystem)(struct udev_device *udev_device);
//const char *udev_device_get_devtype(struct udev_device *udev_device);
typedef const char *(*uos_udev_device_get_devtype)(struct udev_device *udev_device);
//const char *udev_device_get_action(struct udev_device *udev_device);
typedef const char *(*uos_udev_device_get_action)(struct udev_device *udev_device);
//struct udev *udev_unref(struct udev *udev);
typedef struct udev *(*uos_udev_unref)(struct udev *udev);
//struct udev_list_entry *udev_list_entry_get_next(struct udev_list_entry *list_entry);
typedef struct udev_list_entry *(*uos_udev_list_entry_get_next)(struct udev_list_entry *list_entry);

typedef struct _LoadUdev {
    uos_udev_enumerate_new m_udev_enumerate_new;
    uos_udev_enumerate_get_udev m_udev_enumerate_get_udev;
    uos_udev_enumerate_add_match_subsystem m_udev_enumerate_add_match_subsystem;
    uos_udev_enumerate_scan_devices m_udev_enumerate_scan_devices;
    uos_udev_enumerate_get_list_entry m_udev_enumerate_get_list_entry;
    uos_udev_list_entry_get_name m_udev_list_entry_get_name;
    uos_udev_device_new_from_syspath m_udev_device_new_from_syspath;
    uos_udev_device_get_devnode m_udev_device_get_devnode;
    uos_udev_device_get_parent_with_subsystem_devtype m_udev_device_get_parent_with_subsystem_devtype;
    uos_udev_device_get_sysattr_value m_udev_device_get_sysattr_value;
    uos_udev_device_unref m_udev_device_unref;
    uos_udev_enumerate_unref m_udev_enumerate_unref;
    uos_udev_new m_udev_new;
    uos_udev_monitor_new_from_netlink m_udev_monitor_new_from_netlink;
    uos_udev_monitor_filter_add_match_subsystem_devtype m_udev_monitor_filter_add_match_subsystem_devtype;
    uos_udev_monitor_enable_receiving m_udev_monitor_enable_receiving;
    uos_udev_monitor_get_fd m_udev_monitor_get_fd;
    uos_udev_monitor_receive_device m_udev_monitor_receive_device;
    uos_udev_device_get_subsystem m_udev_device_get_subsystem;
    uos_udev_device_get_devtype m_udev_device_get_devtype;
    uos_udev_device_get_action m_udev_device_get_action;
    uos_udev_unref m_udev_unref;
    uos_udev_list_entry_get_next m_udev_list_entry_get_next;
} LoadUdev;
LoadUdev *getUdev();


//int LIBUSB_CALL libusb_init(libusb_context **ctx);
typedef int LIBUSB_CALL (*uos_libusb_init)(libusb_context **ctx);
/*ssize_t LIBUSB_CALL libusb_get_device_list(libusb_context *ctx,
    libusb_device ***list);*/
typedef ssize_t LIBUSB_CALL (*uos_libusb_get_device_list)(libusb_context *ctx,
                                                          libusb_device ***list);
//uint8_t LIBUSB_CALL libusb_get_bus_number(libusb_device *dev);
typedef uint8_t LIBUSB_CALL (*uos_libusb_get_bus_number)(libusb_device *dev);
//uint8_t LIBUSB_CALL libusb_get_device_address(libusb_device *dev);
typedef uint8_t LIBUSB_CALL (*uos_libusb_get_device_address)(libusb_device *dev);
//libusb_device * LIBUSB_CALL libusb_ref_device(libusb_device *dev);
typedef libusb_device *LIBUSB_CALL (*uos_libusb_ref_device)(libusb_device *dev);
/*void LIBUSB_CALL libusb_free_device_list(libusb_device **list,
    int unref_devices);*/
typedef void LIBUSB_CALL (*uos_libusb_free_device_list)(libusb_device **list,
                                                        int unref_devices);
/*int LIBUSB_CALL libusb_get_device_descriptor(libusb_device *dev,
    struct libusb_device_descriptor *desc);*/
typedef int LIBUSB_CALL (*uos_libusb_get_device_descriptor)(libusb_device *dev,
                                                            struct libusb_device_descriptor *desc);
/*int LIBUSB_CALL libusb_get_config_descriptor(libusb_device *dev,
    uint8_t config_index, struct libusb_config_descriptor **config);*/
typedef int LIBUSB_CALL (*uos_libusb_get_config_descriptor)(libusb_device *dev,
                                                            uint8_t config_index, struct libusb_config_descriptor **config);
//void LIBUSB_CALL libusb_unref_device(libusb_device *dev);
typedef void LIBUSB_CALL (*uos_libusb_unref_device)(libusb_device *dev);

typedef struct _LoadUSB {
    uos_libusb_init m_libusb_init;
    uos_libusb_get_device_list m_libusb_get_device_list;
    uos_libusb_get_bus_number m_libusb_get_bus_number;
    uos_libusb_get_device_address m_libusb_get_device_address;
    uos_libusb_ref_device m_libusb_ref_device;
    uos_libusb_free_device_list m_libusb_free_device_list;
    uos_libusb_get_device_descriptor m_libusb_get_device_descriptor;
    uos_libusb_get_config_descriptor m_libusb_get_config_descriptor;
    uos_libusb_unref_device m_libusb_unref_device;
} LoadUSB;

LoadUSB *getUSB();

//PaDeviceIndex Pa_GetDeviceCount( void );
typedef PaDeviceIndex (*uos_Pa_GetDeviceCount)( void );
//PaError Pa_IsStreamStopped( PaStream *stream );
typedef PaError (*uos_Pa_IsStreamStopped)( PaStream *stream );
//const PaDeviceInfo* Pa_GetDeviceInfo( PaDeviceIndex device );
typedef const PaDeviceInfo *(*uos_Pa_GetDeviceInfo)( PaDeviceIndex device );
//PaError Pa_Initialize( void );
typedef PaError (*uos_Pa_Initialize)( void );
//PaError Pa_IsStreamActive( PaStream *stream );
typedef PaError (*uos_Pa_IsStreamActive)( PaStream *stream );
//PaError Pa_Terminate( void );
typedef PaError (*uos_Pa_Terminate)( void );
//PaError Pa_AbortStream( PaStream *stream );
typedef PaError (*uos_Pa_AbortStream)( PaStream *stream );
//PaError Pa_StopStream( PaStream *stream );
typedef PaError (*uos_Pa_StopStream)( PaStream *stream );
//PaError Pa_CloseStream( PaStream *stream );
typedef PaError (*uos_Pa_CloseStream)( PaStream *stream );
//PaDeviceIndex Pa_GetDefaultInputDevice( void );
typedef PaDeviceIndex (*uos_Pa_GetDefaultInputDevice)( void );
//const PaHostApiInfo * Pa_GetHostApiInfo( PaHostApiIndex hostApi );
typedef const PaHostApiInfo *(*uos_Pa_GetHostApiInfo)( PaHostApiIndex hostApi );
//PaDeviceIndex Pa_GetDefaultOutputDevice( void );
typedef PaDeviceIndex (*uos_Pa_GetDefaultOutputDevice)( void );
/*PaError Pa_OpenStream( PaStream** stream,
                       const PaStreamParameters *inputParameters,
                       const PaStreamParameters *outputParameters,
                       double sampleRate,
                       unsigned long framesPerBuffer,
                       PaStreamFlags streamFlags,
                       PaStreamCallback *streamCallback,
                       void *userData );*/
typedef PaError (*uos_Pa_OpenStream)( PaStream **stream,
                                      const PaStreamParameters *inputParameters,
                                      const PaStreamParameters *outputParameters,
                                      double sampleRate,
                                      unsigned long framesPerBuffer,
                                      PaStreamFlags streamFlags,
                                      PaStreamCallback *streamCallback,
                                      void *userData );
//PaError Pa_StartStream( PaStream *stream );
typedef PaError (*uos_Pa_StartStream)( PaStream *stream );
//const PaStreamInfo* Pa_GetStreamInfo( PaStream *stream );
typedef const PaStreamInfo *(*uos_Pa_GetStreamInfo)( PaStream *stream );
//const char *Pa_GetErrorText( PaError errorCode );
typedef const char *(*uos_Pa_GetErrorText)( PaError errorCode );

typedef struct _LoadPortAudio {
    uos_Pa_GetDeviceCount m_Pa_GetDeviceCount;
    uos_Pa_IsStreamStopped m_Pa_IsStreamStopped;
    uos_Pa_GetDeviceInfo m_Pa_GetDeviceInfo;
    uos_Pa_Initialize m_Pa_Initialize;
    uos_Pa_IsStreamActive m_Pa_IsStreamActive;
    uos_Pa_Terminate m_Pa_Terminate;
    uos_Pa_AbortStream m_Pa_AbortStream;
    uos_Pa_StopStream m_Pa_StopStream;
    uos_Pa_CloseStream m_Pa_CloseStream;
    uos_Pa_GetDefaultInputDevice m_Pa_GetDefaultInputDevice;
    uos_Pa_GetHostApiInfo m_Pa_GetHostApiInfo;
    uos_Pa_GetDefaultOutputDevice m_Pa_GetDefaultOutputDevice;
    uos_Pa_OpenStream m_Pa_OpenStream;
    uos_Pa_StartStream m_Pa_StartStream;
    uos_Pa_GetStreamInfo m_Pa_GetStreamInfo;
    uos_Pa_GetErrorText m_Pa_GetErrorText;
} LoadPortAudio;

LoadPortAudio *getPortAudio();


//LIBV4L_PUBLIC int v4l2_ioctl(int fd, unsigned long int request, ...);
typedef LIBV4L_PUBLIC int (*uos_v4l2_ioctl)(int fd, unsigned long int request, ...);
//LIBV4L_PUBLIC int v4l2_munmap(void *_start, size_t length);
typedef LIBV4L_PUBLIC int (*uos_v4l2_munmap)(void *_start, size_t length);
/*LIBV4L_PUBLIC void *v4l2_mmap(void *start, size_t length, int prot, int flags,
int fd, int64_t offset);*/
typedef LIBV4L_PUBLIC void *(*uos_v4l2_mmap)(void *start, size_t length, int prot, int flags,
                                             int fd, int64_t offset);
//LIBV4L_PUBLIC ssize_t v4l2_read(int fd, void *buffer, size_t n);
typedef LIBV4L_PUBLIC ssize_t (*uos_v4l2_read)(int fd, void *buffer, size_t n);
//LIBV4L_PUBLIC int v4l2_close(int fd);
typedef LIBV4L_PUBLIC int (*uos_v4l2_close)(int fd);
//LIBV4L_PUBLIC int v4l2_open(const char *file, int oflag, ...);
typedef LIBV4L_PUBLIC int (*uos_v4l2_open)(const char *file, int oflag, ...);

typedef struct _LoadV4l2 {
    uos_v4l2_ioctl m_v4l2_ioctl;
    uos_v4l2_munmap m_v4l2_munmap;
    uos_v4l2_mmap m_v4l2_mmap;
    uos_v4l2_read m_v4l2_read;
    uos_v4l2_close m_v4l2_close;
    uos_v4l2_open m_v4l2_open;
} LoadV4l2;

LoadV4l2 *getV4l2();
#endif//LOAD_LIBS_H
