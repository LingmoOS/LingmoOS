// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVLIBINTERFACE_H
#define AVLIBINTERFACE_H

#include <QObject>
#include <QLibrary>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}


class avlibInterface
{
public:
    avlibInterface();
    typedef int64_t (*p_av_gettime)(void); // libavutil
    typedef AVFrame* (*p_av_frame_alloc)(void);// libavutil
    typedef void (*p_av_frame_free)(AVFrame **);
    typedef int64_t (*p_av_get_default_channel_layout)(int);
    typedef const char *(*p_av_get_sample_fmt_name)(enum AVSampleFormat);
    typedef char *(*p_av_strdup)(const char *);
    typedef void (*p_av_get_channel_layout_string)(char *, int, int, uint64_t);
    typedef int (*p_av_get_channel_layout_nb_channels)(uint64_t);
    typedef int (*p_av_audio_fifo_read)(AVAudioFifo *, void **, int );
    typedef int (*p_av_audio_fifo_write)(AVAudioFifo *, void **, int );
    typedef int (*p_av_frame_get_buffer)(AVFrame *, int );
    typedef int (*p_av_frame_apply_cropping)(AVFrame *, int );
    typedef void (*p_av_frame_unref)(AVFrame *);
    typedef void (*p_av_free)(void *);
    typedef int (*p_av_audio_fifo_space)(AVAudioFifo *);
    typedef int (*p_av_audio_fifo_size)(AVAudioFifo *);
    typedef int (*p_av_audio_fifo_realloc)(AVAudioFifo *, int );
    typedef AVAudioFifo *(*p_av_audio_fifo_alloc)(enum AVSampleFormat , int , int );
    typedef int (*p_av_samples_alloc)(uint8_t **, int *, int , int , enum AVSampleFormat , int );
    typedef int64_t (*p_av_rescale_q)(int64_t, AVRational, AVRational) av_const;
    typedef int64_t (*p_av_rescale_q_rnd)(int64_t , AVRational , AVRational , enum AVRounding) av_const;
    typedef int (*p_av_dict_set)(AVDictionary **, const char *, const char *, int );
    typedef void (*p_av_freep)(void *);
    typedef void (*p_av_audio_fifo_free)(AVAudioFifo *);
    typedef void *(*p_av_malloc)(size_t );
    typedef int (*p_av_opt_set_bin)(void *, const char *, const uint8_t *, int , int );
    typedef unsigned (*p_av_int_list_length_for_size)(unsigned, const void *, uint64_t) av_pure;


    typedef void (*p_avcodec_register_all)(void); //libavcodec
    typedef void (*p_av_init_packet)(AVPacket*);
    typedef int (*p_avcodec_decode_audio4)(AVCodecContext*, AVFrame*, int*, const AVPacket*);
    typedef void (*p_av_packet_unref)(AVPacket*);
    typedef int (*p_avcodec_open2)(AVCodecContext *, const AVCodec *, AVDictionary **);
    typedef AVCodec *(*p_avcodec_find_decoder)(enum AVCodecID );
    typedef AVCodecContext *(*p_avcodec_alloc_context3)(const AVCodec *);
    typedef int (*p_avcodec_encode_video2)(AVCodecContext *, AVPacket *, const AVFrame *, int *);
    typedef void (*p_av_free_packet)(AVPacket *);
    typedef int (*p_avcodec_encode_audio2)(AVCodecContext *, AVPacket *, const AVFrame *, int *);
    typedef int (*p_avcodec_close)(AVCodecContext *);
    typedef AVCodec *(*p_avcodec_find_encoder)(enum AVCodecID id);
    typedef int (*p_avpicture_get_size)(enum AVPixelFormat, int, int);
    typedef int (*p_avpicture_fill)(AVPicture *, const uint8_t *, enum AVPixelFormat , int, int);

    typedef void (*p_av_register_all)(void); // libavformat
    typedef  int (*p_av_read_frame)(AVFormatContext*, AVPacket*);
    typedef void (*p_avformat_close_input)(AVFormatContext**);
    typedef void (*p_avformat_free_context)(AVFormatContext*);
    typedef AVInputFormat* (*p_av_find_input_format)(const char *);
    typedef int (*p_avformat_open_input)(AVFormatContext **, const char *, AVInputFormat *, AVDictionary **);
    typedef int (*p_avformat_find_stream_info)(AVFormatContext *, AVDictionary **);
    typedef void (*p_av_dump_format)(AVFormatContext *, int, const char *, int);
    typedef AVStream *(*p_avformat_new_stream)(AVFormatContext *, const AVCodec *);
    typedef int (*p_av_interleaved_write_frame)(AVFormatContext *, AVPacket *);
    typedef int (*p_av_write_trailer)(AVFormatContext *);
    typedef int (*p_avio_close)(AVIOContext *);
    typedef int (*p_avformat_write_header)(AVFormatContext *s, AVDictionary **options);
    typedef int (*p_avio_open)(AVIOContext **s, const char *url, int flags);
    typedef int (*p_avformat_alloc_output_context2)(AVFormatContext **, AVOutputFormat *, const char *, const char *);

    typedef void (*p_avdevice_register_all)(void);//libavdevice

    typedef const AVFilter *(*p_avfilter_get_by_name)(const char *); // libavfilter
    typedef AVFilterInOut *(*p_avfilter_inout_alloc)(void);
    typedef AVFilterGraph *(*p_avfilter_graph_alloc)(void);
    typedef int (*p_avfilter_graph_create_filter)(AVFilterContext **, const AVFilter *, const char *, const char *, void *, AVFilterGraph *);
    typedef int (*p_avfilter_graph_parse_ptr)(AVFilterGraph *, const char *,AVFilterInOut **, AVFilterInOut **, void *);
    typedef int (*p_avfilter_graph_config)(AVFilterGraph *, void *);
    typedef void (*p_avfilter_inout_free)(AVFilterInOut **);
    typedef void (*p_avfilter_graph_free)(AVFilterGraph **);
    typedef int (*p_av_buffersrc_add_frame_flags)(AVFilterContext *,AVFrame *, int);
    typedef int (*p_av_buffersink_get_frame)(AVFilterContext *, AVFrame *);

    typedef int (*p_sws_scale)(struct SwsContext *, const uint8_t *const [],const int [], int, int , uint8_t *const [], const int []); // libswscale
    typedef struct SwsContext *(*p_sws_getContext)(int , int , enum AVPixelFormat ,int , int , enum AVPixelFormat ,int , SwsFilter *,SwsFilter *, const double *);

    typedef int (*p_swr_convert)(struct SwrContext *, uint8_t **, int , const uint8_t ** , int);// libswresample
    typedef struct SwrContext *(*p_swr_alloc_set_opts)(struct SwrContext *, int64_t, enum AVSampleFormat, int, int64_t, enum AVSampleFormat, int, int, void *);
    typedef int (*p_swr_init)(struct SwrContext *);
    typedef void (*p_swr_free)(SwrContext **); //新增,解决内存泄露

    static p_av_gettime m_av_gettime; // libavutil
    static p_av_frame_alloc m_av_frame_alloc;
    static p_av_frame_free m_av_frame_free;
    static p_av_get_default_channel_layout m_av_get_default_channel_layout;
    static p_av_get_sample_fmt_name m_av_get_sample_fmt_name;
    static p_av_strdup m_av_strdup;
    static p_av_get_channel_layout_string m_av_get_channel_layout_string;
    static p_av_get_channel_layout_nb_channels m_av_get_channel_layout_nb_channels;
    static p_av_audio_fifo_read m_av_audio_fifo_read;
    static p_av_audio_fifo_write m_av_audio_fifo_write;
    static p_av_frame_get_buffer m_av_frame_get_buffer;
    static p_av_frame_apply_cropping m_av_frame_apply_cropping;
    static p_av_frame_unref m_av_frame_unref;
    static p_av_free m_av_free;
    static p_av_audio_fifo_space m_av_audio_fifo_space;
    static p_av_audio_fifo_size m_av_audio_fifo_size;
    static p_av_audio_fifo_realloc m_av_audio_fifo_realloc;
    static p_av_audio_fifo_alloc m_av_audio_fifo_alloc;
    static p_av_samples_alloc m_av_samples_alloc;
    static p_av_rescale_q m_av_rescale_q;
    static p_av_rescale_q_rnd m_av_rescale_q_rnd;
    static p_av_dict_set m_av_dict_set;
    static p_av_freep m_av_freep;
    static p_av_audio_fifo_free m_av_audio_fifo_free;
    static p_av_malloc m_av_malloc;
    static p_av_opt_set_bin m_av_opt_set_bin;
    static p_av_int_list_length_for_size m_av_int_list_length_for_size;


    static p_avcodec_register_all m_avcodec_register_all; //libavcodec
    static p_av_init_packet m_av_init_packet;
    static p_avcodec_decode_audio4 m_avcodec_decode_audio4;
    static p_av_packet_unref m_av_packet_unref;
    static p_avcodec_open2 m_avcodec_open2;
    static p_avcodec_find_decoder m_avcodec_find_decoder;
    static p_avcodec_alloc_context3 m_avcodec_alloc_context3;
    static p_avcodec_encode_video2 m_avcodec_encode_video2;
    static p_av_free_packet m_av_free_packet;
    static p_avcodec_encode_audio2 m_avcodec_encode_audio2;
    static p_avcodec_close m_avcodec_close;
    static p_avcodec_find_encoder m_avcodec_find_encoder;
    static p_avpicture_get_size m_avpicture_get_size;
    static p_avpicture_fill m_avpicture_fill;

    static p_av_register_all m_av_register_all; // libavformat
    static p_av_read_frame m_av_read_frame;
    static p_avformat_close_input m_avformat_close_input;
    static p_avformat_free_context m_avformat_free_context;
    static p_av_find_input_format m_av_find_input_format;
    static p_avformat_open_input m_avformat_open_input;
    static p_avformat_find_stream_info m_avformat_find_stream_info;
    static p_av_dump_format m_av_dump_format;
    static p_avformat_new_stream m_avformat_new_stream;
    static p_av_interleaved_write_frame m_av_interleaved_write_frame;
    static p_av_write_trailer m_av_write_trailer;
    static p_avio_close m_avio_close;
    static p_avformat_write_header m_avformat_write_header;
    static p_avio_open m_avio_open;
    static p_avformat_alloc_output_context2 m_avformat_alloc_output_context2;

    static p_avdevice_register_all m_avdevice_register_all;//libavdevice

    static p_avfilter_get_by_name m_avfilter_get_by_name; // libavfilter
    static p_avfilter_inout_alloc m_avfilter_inout_alloc;
    static p_avfilter_graph_alloc m_avfilter_graph_alloc;
    static p_avfilter_graph_create_filter m_avfilter_graph_create_filter;
    static p_avfilter_graph_parse_ptr m_avfilter_graph_parse_ptr;
    static p_avfilter_graph_config m_avfilter_graph_config;
    static p_avfilter_inout_free m_avfilter_inout_free;
    static p_avfilter_graph_free m_avfilter_graph_free;
    static p_av_buffersrc_add_frame_flags m_av_buffersrc_add_frame_flags;
    static p_av_buffersink_get_frame m_av_buffersink_get_frame;

    static p_sws_scale m_sws_scale;
    static p_sws_getContext m_sws_getContext;

    static p_swr_convert m_swr_convert;
    static p_swr_alloc_set_opts m_swr_alloc_set_opts;
    static p_swr_init m_swr_init;
    static p_swr_free m_swr_free; //新增，解决内存泄露

    static bool m_isInitFunction;


    static QLibrary m_libavutil;
    static QLibrary m_libavcodec;
    static QLibrary m_libavformat;
    static QLibrary m_libavfilter;
    static QLibrary m_libswscale;
    static QLibrary m_libswresample;
    static QLibrary m_libavdevice;

    static void initFunctions();
    static void unloadFunctions();
    static QString libPath(const QString &sLib);
};

#endif // AVLIBINTERFACE_H
