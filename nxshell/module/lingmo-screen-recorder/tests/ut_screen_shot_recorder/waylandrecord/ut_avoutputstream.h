// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QTimer>
#include <QTest>
#include <QMainWindow>
#include <QHBoxLayout>
#include  <QFont>
#include <QScreen>
#include <QDesktopWidget>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/waylandrecord/avoutputstream.h"

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
#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include "libavdevice/avdevice.h"
#include "libavfilter/buffersink.h"
#include "libavutil/avutil.h"
#include "libavutil/fifo.h"
}
using namespace testing;

//打桩替换ffmpeg中的一些方法,由于ffmpeg的方法其实现对需要构造特别的数据，而进行某一个方法的单元测试，是无法构建完整的数据
int avformat_alloc_output_context2_stub(AVFormatContext **ctx, AVOutputFormat *oformat,
                                        const char *format_name, const char *filename)
{
    qDebug() << "替换ffmpeg: avformat_alloc_output_context2!";
    AVFormatContext *ctxx = new AVFormatContext();
    ctxx->oformat = new  AVOutputFormat();
    *ctx = ctxx ;
    return 1;
}

AVCodec *avcodec_find_encoder_stub(enum AVCodecID id)
{
    qDebug() << "替换ffmpeg: avcodec_find_encoder!";
    AVCodec *codec = new AVCodec();
    codec->id = id;
    AVSampleFormat *sample_fmts = new AVSampleFormat();
    codec->sample_fmts = sample_fmts;
    return codec;
}

AVCodecContext *avcodec_alloc_context3_stub(const AVCodec *codec)
{
    qDebug() << "替换ffmpeg: avcodec_alloc_context3!";
    AVCodecContext *codecCtx = new AVCodecContext();
    codecCtx->codec = codec;
    return codecCtx;

}

int av_dict_set_stub(AVDictionary **pm, const char *key, const char *value, int flags)
{
    qDebug() << "替换ffmpeg: av_dict_set!";
    return 1;
}

int avcodec_open2_stub1(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
{
    qDebug() << "替换ffmpeg: avcodec_open2!";

    return 1;
}

AVStream *avformat_new_stream_stub(AVFormatContext *s, const AVCodec *c)
{
    qDebug() << "替换ffmpeg: avformat_new_stream!";
    AVStream *stream = new AVStream();
    stream->id = 0;
    stream->index = 0;
    return stream;
}

AVFrame *av_frame_alloc_stub1(void)
{
    qDebug() << "替换ffmpeg: av_frame_alloc!";
    AVFrame *frame = new AVFrame();
    return frame;
}

void av_frame_free_stub1(AVFrame **frame)
{
    qDebug() << "替换ffmpeg: av_frame_free!";
    delete frame;
}

int avpicture_get_size_stub(enum AVPixelFormat pix_fmt, int width, int height)
{
    qDebug() << "替换ffmpeg: avpicture_get_size!";
    return 1;
}


int av_get_channel_layout_nb_channels_stub(uint64_t channel_layout)
{
    qDebug() << "替换ffmpeg: av_get_channel_layout_nb_channels!";
    return 1;

}

int init_filters_stub()
{
    qDebug() << "替换ffmpeg: init_filters!";
    return 1;
}

int avio_open_stub(AVIOContext **s, const char *url, int flags)
{
    qDebug() << "替换ffmpeg: avio_open!";
    return 1;

}

void av_dump_format_stub1(AVFormatContext *ic,
                          int index,
                          const char *url,
                          int is_output)
{
    qDebug() << "替换ffmpeg: av_dump_format!";

}

int avformat_write_header_stub(AVFormatContext *s, AVDictionary **options)
{
    qDebug() << "替换ffmpeg: avformat_write_header!";
    return 1;
}

int avcodec_encode_video2_stub(AVCodecContext *avctx, AVPacket *avpkt,
                               const AVFrame *frame, int *got_packet_ptr)
{
    qDebug() << "替换ffmpeg: avcodec_encode_video2!";
    got_packet_ptr = 0;
    return 1;
}

int av_samples_alloc_stub(uint8_t **audio_data, int *linesize, int nb_channels,
                          int nb_samples, enum AVSampleFormat sample_fmt, int align)
{
    qDebug() << "替换ffmpeg: av_samples_alloc!";
    return 1;
}

int swr_convert_stub(struct SwrContext *s, uint8_t **out, int out_count,
                     const uint8_t **in, int in_count)
{

    qDebug() << "替换ffmpeg: swr_convert!";
    return 1;
}

int64_t av_rescale_q_stub(int64_t a, AVRational bq, AVRational cq)
{
    qDebug() << "替换ffmpeg: av_rescale_q!";
    return 1;
}

int av_frame_get_buffer_stub(AVFrame *frame, int align)
{
    qDebug() << "替换ffmpeg: av_frame_get_buffer!";
    return 1;
}

int avcodec_encode_audio2_stub(AVCodecContext *avctx, AVPacket *avpkt,
                               const AVFrame *frame, int *got_packet_ptr)
{
    qDebug() << "替换ffmpeg: avcodec_encode_audio2!";
    got_packet_ptr = 0;
    return 1;
}

int avfilter_graph_create_filter_stub(AVFilterContext **filt_ctx, const AVFilter *filt,
                                      const char *name, const char *args, void *opaque,
                                      AVFilterGraph *graph_ctx)
{
    qDebug() << "替换ffmpeg: avfilter_graph_set_auto_convert!";


    return 1;
}

int av_opt_set_bin_stub(void *obj, const char *name, const uint8_t *val, int size, int search_flags)
{
    qDebug() << "替换ffmpeg: av_opt_set_bin!";

    return 1;
}

int avfilter_graph_parse_ptr_stub(AVFilterGraph *graph, const char *filters,
                                  AVFilterInOut **inputs, AVFilterInOut **outputs,
                                  void *log_ctx)
{
    qDebug() << "替换ffmpeg: avfilter_graph_parse_ptr!";

    return 1;
}

int avfilter_graph_config_stub(AVFilterGraph *graphctx, void *log_ctx)
{
    qDebug() << "替换ffmpeg: avfilter_graph_config!";
    return 1;
}

void av_get_channel_layout_string_stub(char *buf, int buf_size, int nb_channels, uint64_t channel_layout)
{
    qDebug() << "替换ffmpeg: av_get_channel_layout_string!";

}

int av_buffersrc_add_frame_flags_stub(AVFilterContext *buffer_src,
                                      AVFrame *frame, int flags)
{
    qDebug() << "替换ffmpeg: av_buffersrc_add_frame_flags!";
    return 1;
}

int av_buffersink_get_frame_stub(AVFilterContext *ctx, AVFrame *frame)
{

    qDebug() << "替换ffmpeg: av_buffersink_get_frame!";
    return 1;
}

int av_audio_fifo_read_stub(AVAudioFifo *af, void **data, int nb_samples)
{
    qDebug() << "替换ffmpeg: av_audio_fifo_read!";
    return 1;
}

int av_audio_fifo_write_stub(AVAudioFifo *af, void **data, int nb_samples)
{
    qDebug() << "替换ffmpeg: av_audio_fifo_write!";
    return 1;
}

int av_interleaved_write_frame_stub(AVFormatContext *s, AVPacket *pkt)
{
    qDebug() << "替换ffmpeg: av_audio_fifo_write!";
    return 1;
}

int av_write_trailer_stub(AVFormatContext *s)
{
    qDebug() << "替换ffmpeg: av_write_trailer!";
    return 1;
}

int av_audio_fifo_space_stub(AVAudioFifo *af)
{
    qDebug() << "替换ffmpeg: av_audio_fifo_space!";
    return 1;
}

int av_audio_fifo_size_stub(AVAudioFifo *af)
{
    qDebug() << "替换ffmpeg: av_audio_fifo_size!";
    return 1;

}
class CAVOutputStreamTest: public testing::Test
{

public:
    Stub stub;
    CAVOutputStream *m_avOutputStream;
    WaylandIntegration::WaylandIntegrationPrivate *m_context;
    virtual void SetUp() override
    {
        std::cout << "start UtilsTest" << std::endl;
        m_context   = new  WaylandIntegration::WaylandIntegrationPrivate();
        avlibInterface::initFunctions();
        m_avOutputStream = new CAVOutputStream(m_context);
    }

    virtual void TearDown() override
    {
        delete m_avOutputStream;
        delete m_context;
        avlibInterface::unloadFunctions();
        std::cout << "end UtilsTest" << std::endl;
    }
};

static int g_first2 = 0;

ACCESS_PRIVATE_FIELD(CAVOutputStream, AVCodecID, m_videoCodecID);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVCodecID, m_micAudioCodecID);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVCodecID, m_sysAudioCodecID);
ACCESS_PRIVATE_FIELD(CAVOutputStream, uint8_t **, m_convertedMicSamples);
ACCESS_PRIVATE_FIELD(CAVOutputStream, uint8_t **, m_convertedSysSamples);
ACCESS_PRIVATE_FIELD(CAVOutputStream, SwsContext *, m_pVideoSwsContext);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVCodecContext *, pCodecCtx);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVFrame *, pFrameYUV);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVStream *, m_micAudioStream);
ACCESS_PRIVATE_FIELD(CAVOutputStream, SwrContext *, m_pMicAudioSwrContext);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVCodecContext *, m_pMicCodecContext);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVAudioFifo *, m_micAudioFifo);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVStream *, m_sysAudioStream);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVCodecContext *, m_pSysCodecContext);
ACCESS_PRIVATE_FIELD(CAVOutputStream, SwrContext *, m_pSysAudioSwrContext);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVAudioFifo *, m_sysAudioFifo);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVCodecContext *, pCodecCtx_amix);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVFilterContext *, buffersink_ctx);
ACCESS_PRIVATE_FIELD(CAVOutputStream,   AVFormatContext *, m_videoFormatContext);
ACCESS_PRIVATE_FIELD(CAVOutputStream, int, is_fifo_scardinit);
ACCESS_PRIVATE_FIELD(CAVOutputStream, int64_t, m_start_mix_time);
ACCESS_PRIVATE_FIELD(CAVOutputStream, int64_t, m_nLastAudioMixPresentationTime);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVStream *, audio_amix_st);
ACCESS_PRIVATE_FIELD(CAVOutputStream, bool, m_isOverWrite);
ACCESS_PRIVATE_FIELD(CAVOutputStream, bool, m_isWriteFrame);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVStream *, m_videoStream);
ACCESS_PRIVATE_FIELD(CAVOutputStream, uint8_t *, m_out_buffer);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVFilterGraph *, filter_graph);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVFrame *, mMic_frame);
ACCESS_PRIVATE_FIELD(CAVOutputStream, AVFrame *, mSpeaker_frame);

TEST_F(CAVOutputStreamTest, SetVideoCodecProp)
{
    m_avOutputStream->SetVideoCodecProp(AVCodecID::AV_CODEC_ID_H264, 24, 48000, 100, 1920, 1080);
}

TEST_F(CAVOutputStreamTest, SetAudioCodecProp)
{
    m_avOutputStream->SetAudioCodecProp(AVCodecID::AV_CODEC_ID_MP3, 48000, 2, 1, 48000);
}

TEST_F(CAVOutputStreamTest, SetAudioCardCodecProp)
{
    m_avOutputStream->SetAudioCardCodecProp(AVCodecID::AV_CODEC_ID_MP3, 48000, 2, 1, 48000);
}

int init_context_amix_stub(int channel, uint64_t channel_layout, int sample_rate, int64_t bit_rate)
{
    return 1;
}


TEST_F(CAVOutputStreamTest, open)
{
    stub.set(avformat_alloc_output_context2, avformat_alloc_output_context2_stub);
//    stub.set(avcodec_find_encoder, avcodec_find_encoder_stub);
    stub.set(avcodec_alloc_context3, avcodec_alloc_context3_stub);
    stub.set(av_dict_set, av_dict_set_stub);
    stub.set(av_frame_alloc, av_frame_alloc_stub1);
    stub.set(avpicture_get_size, avpicture_get_size_stub);
    stub.set(ADDR(CAVOutputStream, init_context_amix), init_context_amix_stub);
    stub.set(av_get_channel_layout_nb_channels, av_get_channel_layout_nb_channels_stub);
    stub.set(ADDR(CAVOutputStream, init_filters), init_filters_stub);
    stub.set(avcodec_open2, avcodec_open2_stub1);
    stub.set(avformat_new_stream, avformat_new_stream_stub);
    stub.set(av_dump_format, av_dump_format_stub1);
    stub.set(avformat_write_header, avformat_write_header_stub);

    access_private_field::CAVOutputStreamm_videoCodecID(*m_avOutputStream) = AVCodecID::AV_CODEC_ID_H264;
    access_private_field::CAVOutputStreamm_micAudioCodecID(*m_avOutputStream) = AVCodecID::AV_CODEC_ID_AAC;
    access_private_field::CAVOutputStreamm_sysAudioCodecID(*m_avOutputStream) = AVCodecID::AV_CODEC_ID_AAC;

    int channel = 2;
    uint64_t channel_layout = 2;
    int sample_rate = 48000;
    int64_t bit_rate = 48000;
    m_avOutputStream->SetAudioCodecProp(AV_CODEC_ID_AAC, sample_rate, channel, channel_layout, bit_rate);
    m_avOutputStream->open("tmp/test.mp4");

    stub.reset(avformat_alloc_output_context2);
    stub.reset(avcodec_find_encoder);
    stub.reset(avcodec_alloc_context3);
    stub.reset(av_dict_set);
    stub.reset(av_frame_alloc);
    stub.reset(avpicture_get_size);
    stub.reset(ADDR(CAVOutputStream, init_context_amix));
    stub.reset(av_get_channel_layout_nb_channels);
    stub.reset(ADDR(CAVOutputStream, init_filters));
    stub.reset(avio_open);
    stub.reset(avformat_new_stream);
    stub.reset(av_dump_format);
    stub.reset(avformat_write_header);

    av_free(access_private_field::CAVOutputStreamm_out_buffer(*m_avOutputStream));
    free(access_private_field::CAVOutputStreamm_convertedMicSamples(*m_avOutputStream));
    free(access_private_field::CAVOutputStreamm_convertedSysSamples(*m_avOutputStream));
    delete  access_private_field::CAVOutputStreamm_videoStream(*m_avOutputStream);
    delete  access_private_field::CAVOutputStreamm_micAudioStream(*m_avOutputStream);
    delete  access_private_field::CAVOutputStreamm_sysAudioStream(*m_avOutputStream);
    delete  access_private_field::CAVOutputStreampFrameYUV(*m_avOutputStream);
    delete  access_private_field::CAVOutputStreampCodecCtx(*m_avOutputStream);
    delete  access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream);
    delete  access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream);
    delete  access_private_field::CAVOutputStreamm_videoFormatContext(*m_avOutputStream)->oformat;
    delete  access_private_field::CAVOutputStreamm_videoFormatContext(*m_avOutputStream);
}

TEST_F(CAVOutputStreamTest, close)
{
    m_avOutputStream->close();
}

QPixmap grabEntireDesktop1()
{
    QPixmap g_tempPixmap;
    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    g_tempPixmap = t_primaryScreen->grabWindow(QApplication::desktop()->winId(), 0, 0, 1920, 1080);
    return g_tempPixmap;
}


TEST_F(CAVOutputStreamTest, writeVideoFrame)
{
    QImage image = grabEntireDesktop1().toImage();
    WaylandIntegration::WaylandIntegrationPrivate::waylandFrame frame ;
    frame._frame = image.bits();
    frame._width = image.width();
    frame._height = image.height();
    frame._stride = image.bytesPerLine();
    access_private_field::CAVOutputStreampCodecCtx(*m_avOutputStream) = new AVCodecContext() ;

    access_private_field::CAVOutputStreampCodecCtx(*m_avOutputStream)->width = image.width();
    access_private_field::CAVOutputStreampCodecCtx(*m_avOutputStream)->height = image.height();
    access_private_field::CAVOutputStreampFrameYUV(*m_avOutputStream) = av_frame_alloc();

    stub.set(avcodec_encode_video2, avcodec_encode_video2_stub);

    m_avOutputStream->SetVideoCodecProp(AVCodecID::AV_CODEC_ID_H264, 24, 48000, 100, 1920, 1080);
    m_avOutputStream->writeVideoFrame(frame);

    stub.reset(avcodec_encode_video2);

    delete access_private_field::CAVOutputStreampCodecCtx(*m_avOutputStream);
    sws_freeContext(access_private_field::CAVOutputStreamm_pVideoSwsContext(*m_avOutputStream));
}

int audioWrite_stub(AVAudioFifo *af, void **data, int nb_samples)
{
    return 49000;
}
AVAudioFifo *audioFifoAlloc_stub(AVSampleFormat sample_fmt, int channels, int nb_samples)
{
    return av_audio_fifo_alloc(sample_fmt, channels, nb_samples);
}
int audioFifoRealloc_stub(AVAudioFifo *af, int nb_samples)
{

    return 1;
}

int audioRead_stub(AVAudioFifo *af, void **data, int nb_samples)
{
    return nb_samples + 1;

}
int audioFifoSize_stub(AVAudioFifo *af)
{
    g_first2++;
    if (g_first2 == 1) {
        return 1153;
    }
    g_first2 = 0;
    return 1;

}

TEST_F(CAVOutputStreamTest, writeMicAudioFrame)
{
    access_private_field::CAVOutputStreamm_micAudioStream(*m_avOutputStream) = new AVStream();
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream) = new AVCodecContext();
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->frame_size = 1152;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->channels = 2;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_U8P;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->sample_rate = 48000;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->channel_layout = 2;
    access_private_field::CAVOutputStreamm_pMicAudioSwrContext(*m_avOutputStream) = nullptr;
    access_private_field::CAVOutputStreamm_micAudioFifo(*m_avOutputStream) = nullptr;
    AVStream *stream = new AVStream();
    stream->codec = new AVCodecContext();
    stream->codec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_U8P;
    stream->codec->sample_rate = 48000;

    AVFrame *inputFrame = new AVFrame();
    inputFrame->nb_samples = 48000;

    stub.set(av_samples_alloc, av_samples_alloc_stub);
    stub.set(swr_convert, swr_convert_stub);
    stub.set(ADDR(CAVOutputStream, audioFifoAlloc), audioFifoAlloc_stub);
    stub.set(ADDR(CAVOutputStream, audioFifoRealloc), audioFifoRealloc_stub);
    stub.set(ADDR(CAVOutputStream, audioWrite), audioWrite_stub);
    stub.set(av_rescale_q, av_rescale_q_stub);
    stub.set(av_frame_get_buffer, av_frame_get_buffer_stub);
    stub.set(ADDR(CAVOutputStream, audioRead), audioRead_stub);
    stub.set(avcodec_encode_audio2, avcodec_encode_audio2_stub);
    stub.set(ADDR(CAVOutputStream, audioFifoSize), audioFifoSize_stub);
    //stub.set(av_frame_alloc, av_frame_alloc_stub1);
    //stub.set(av_frame_free, av_frame_free_stub1);

    //正式执行需测试的方法
    m_avOutputStream->writeMicAudioFrame(stream, inputFrame, 1111);

    stub.reset(av_samples_alloc);
    stub.reset(swr_convert);
    stub.reset(ADDR(CAVOutputStream, audioFifoRealloc));
    stub.reset(ADDR(CAVOutputStream, audioWrite));
    stub.reset(av_rescale_q);
    stub.reset(av_frame_get_buffer);
    stub.reset(ADDR(CAVOutputStream, audioRead));
    stub.reset(avcodec_encode_audio2);
    stub.reset(ADDR(CAVOutputStream, audioFifoSize));
    //stub.reset(av_frame_alloc);
    //stub.reset(av_frame_free);

    delete inputFrame;
    delete stream->codec;
    delete stream;
    swr_free(&access_private_field::CAVOutputStreamm_pMicAudioSwrContext(*m_avOutputStream));
    av_audio_fifo_free(access_private_field::CAVOutputStreamm_micAudioFifo(*m_avOutputStream));
    delete access_private_field::CAVOutputStreamm_convertedMicSamples(*m_avOutputStream);
    delete access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream);
    delete access_private_field::CAVOutputStreamm_micAudioStream(*m_avOutputStream);

}

bool isWriteFrame_stub()
{
    return false;
}

TEST_F(CAVOutputStreamTest, writeMicToMixAudioFrame)
{

    access_private_field::CAVOutputStreamm_micAudioStream(*m_avOutputStream) = new AVStream();
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream) = new AVCodecContext();
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->frame_size = 1152;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->channels = 2;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_U8P;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->sample_rate = 48000;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->channel_layout = 2;
    access_private_field::CAVOutputStreamm_pMicAudioSwrContext(*m_avOutputStream) = nullptr;
    access_private_field::CAVOutputStreamm_micAudioFifo(*m_avOutputStream) = nullptr;
    AVStream *stream = new AVStream();
    stream->codec = new AVCodecContext();
    stream->codec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_U8P;
    stream->codec->sample_rate = 48000;

    AVFrame *inputFrame = new AVFrame();
    inputFrame->nb_samples = 48000;

    stub.set(av_samples_alloc, av_samples_alloc_stub);
    stub.set(swr_convert, swr_convert_stub);
    stub.set(ADDR(CAVOutputStream, audioWrite), audioWrite_stub);
    stub.set(av_rescale_q, av_rescale_q_stub);
    stub.set(av_frame_get_buffer, av_frame_get_buffer_stub);
    stub.set(ADDR(CAVOutputStream, isWriteFrame), isWriteFrame_stub);

    //正式执行需测试的方法
    m_avOutputStream->writeMicToMixAudioFrame(stream, inputFrame, 1111);

    stub.reset(av_samples_alloc);
    stub.reset(swr_convert);
    stub.reset(ADDR(CAVOutputStream, audioWrite));
    stub.reset(av_rescale_q);
    stub.reset(av_frame_get_buffer);
    stub.reset(ADDR(CAVOutputStream, isWriteFrame));

    delete inputFrame;
    delete stream->codec;
    delete stream;
    delete access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream);
    delete access_private_field::CAVOutputStreamm_micAudioStream(*m_avOutputStream);
    swr_free(&access_private_field::CAVOutputStreamm_pMicAudioSwrContext(*m_avOutputStream));
}


TEST_F(CAVOutputStreamTest, writeSysAudioFrame)
{
    access_private_field::CAVOutputStreamm_sysAudioStream(*m_avOutputStream) = new AVStream();
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream) = new AVCodecContext();
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->frame_size = 1152;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->channels = 2;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_U8P;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->sample_rate = 48000;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->channel_layout = 2;
    access_private_field::CAVOutputStreamm_pSysAudioSwrContext(*m_avOutputStream) = nullptr;
    access_private_field::CAVOutputStreamm_sysAudioFifo(*m_avOutputStream) = nullptr;
    AVStream *stream = new AVStream();
    stream->codec = new AVCodecContext();
    stream->codec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_U8P;
    stream->codec->sample_rate = 48000;

    AVFrame *inputFrame = new AVFrame();
    inputFrame->nb_samples = 48000;

    stub.set(av_samples_alloc, av_samples_alloc_stub);
    stub.set(swr_convert, swr_convert_stub);
    stub.set(ADDR(CAVOutputStream, audioWrite), audioWrite_stub);
    stub.set(ADDR(CAVOutputStream, audioFifoSize), audioFifoSize_stub);
    stub.set(av_rescale_q, av_rescale_q_stub);
    stub.set(av_frame_get_buffer, av_frame_get_buffer_stub);
    stub.set(ADDR(CAVOutputStream, audioRead), audioRead_stub);
    stub.set(avcodec_encode_audio2, avcodec_encode_audio2_stub);

    //正式执行需测试的方法
    m_avOutputStream->writeSysAudioFrame(stream, inputFrame, 1111);

    stub.reset(av_samples_alloc);
    stub.reset(swr_convert);
    stub.reset(ADDR(CAVOutputStream, audioWrite));
    stub.reset(ADDR(CAVOutputStream, audioFifoSize));
    stub.reset(av_rescale_q);
    stub.reset(av_frame_get_buffer);
    stub.reset(ADDR(CAVOutputStream, audioRead));
    stub.reset(avcodec_encode_audio2);

    delete inputFrame;
    delete stream->codec;
    delete stream;
    delete access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream);
    delete access_private_field::CAVOutputStreamm_sysAudioStream(*m_avOutputStream);
    swr_free(&access_private_field::CAVOutputStreamm_pSysAudioSwrContext(*m_avOutputStream));
}
TEST_F(CAVOutputStreamTest, writeSysToMixAudioFrame)
{
    access_private_field::CAVOutputStreamm_sysAudioStream(*m_avOutputStream) = new AVStream();
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream) = new AVCodecContext();
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->frame_size = 1152;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->channels = 2;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_U8P;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->sample_rate = 48000;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->channel_layout = 2;
    access_private_field::CAVOutputStreamm_pSysAudioSwrContext(*m_avOutputStream) = nullptr;
    access_private_field::CAVOutputStreamm_sysAudioFifo(*m_avOutputStream) = nullptr;
    AVStream *stream = new AVStream();
    stream->codec = new AVCodecContext();
    stream->codec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_U8P;
    stream->codec->sample_rate = 48000;

    AVFrame *inputFrame = new AVFrame();
    inputFrame->nb_samples = 48000;

    stub.set(av_samples_alloc, av_samples_alloc_stub);
    stub.set(swr_convert, swr_convert_stub);
    stub.set(ADDR(CAVOutputStream, audioWrite), audioWrite_stub);
    stub.set(av_rescale_q, av_rescale_q_stub);
    stub.set(av_frame_get_buffer, av_frame_get_buffer_stub);
    stub.set(ADDR(CAVOutputStream, isWriteFrame), isWriteFrame_stub);

    //正式执行需测试的方法
    m_avOutputStream->writeSysToMixAudioFrame(stream, inputFrame, 1111);

    stub.reset(av_samples_alloc);
    stub.reset(swr_convert);
    stub.reset(ADDR(CAVOutputStream, audioWrite));
    stub.reset(av_rescale_q);
    stub.reset(av_frame_get_buffer);
    stub.reset(ADDR(CAVOutputStream, isWriteFrame));

    delete inputFrame;
    delete stream->codec;
    delete stream;
    delete access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream);
    delete access_private_field::CAVOutputStreamm_sysAudioStream(*m_avOutputStream);
    swr_free(&access_private_field::CAVOutputStreamm_pSysAudioSwrContext(*m_avOutputStream));
}

TEST_F(CAVOutputStreamTest, write_filter_audio_frame)
{
    stub.set(avcodec_encode_audio2, avcodec_encode_audio2_stub);
    AVCodecContext *codecCtx_audio = new AVCodecContext();

    AVStream *stream = new AVStream();
    stream->codec = new AVCodecContext();
    stream->codec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_U8P;
    stream->codec->sample_rate = 48000;

    AVFrame *output_frame = new AVFrame();
    output_frame->nb_samples = 48000;

    //正式执行需测试的方法
    m_avOutputStream->write_filter_audio_frame(stream, codecCtx_audio, output_frame);

    stub.reset(avcodec_encode_audio2);

    delete output_frame;
    delete stream->codec;
    delete stream;
    delete codecCtx_audio;

}


TEST_F(CAVOutputStreamTest, init_filters)
{
    access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream) = new AVCodecContext();
    access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream)->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
    access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream)->channel_layout = 2;
    access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream)->sample_rate = 48000;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream) = new AVCodecContext();
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->channel_layout = 2;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream) = new AVCodecContext();
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->channel_layout = 2;
    AVFilterContext *filtdd = new AVFilterContext();
    AVFilterLink *filterLink = new AVFilterLink();
    AVFilterLink **filterLinks = nullptr;
    filterLinks = &filterLink;
    filtdd->inputs = filterLinks;
    access_private_field::CAVOutputStreambuffersink_ctx(*m_avOutputStream) = filtdd;

    stub.set(avfilter_graph_create_filter, avfilter_graph_create_filter_stub);
    stub.set(av_opt_set_bin, av_opt_set_bin_stub);
    stub.set(avfilter_graph_parse_ptr, avfilter_graph_parse_ptr_stub);
    stub.set(avfilter_graph_config, avfilter_graph_config_stub);
    stub.set(av_get_channel_layout_string, av_get_channel_layout_string_stub);

    //正式执行需测试的方法
    m_avOutputStream->init_filters();

    stub.reset(avfilter_graph_create_filter);
    stub.reset(avcodec_encode_audio2);
    stub.reset(avfilter_graph_parse_ptr);
    stub.reset(avfilter_graph_config);
    stub.reset(av_get_channel_layout_string);

    delete filterLink;
    delete filtdd;
    avfilter_graph_free(&access_private_field::CAVOutputStreamfilter_graph(*m_avOutputStream));
    av_frame_free(&access_private_field::CAVOutputStreammMic_frame(*m_avOutputStream));
    av_frame_free(&access_private_field::CAVOutputStreammSpeaker_frame(*m_avOutputStream));
    delete access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream);
    delete access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream);
    delete access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream);

}

TEST_F(CAVOutputStreamTest, init_context_amix)
{
    //access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream) = new AVCodecContext();
    access_private_field::CAVOutputStreamm_sysAudioCodecID(*m_avOutputStream) = AV_CODEC_ID_AAC;
    access_private_field::CAVOutputStreamm_videoFormatContext(*m_avOutputStream) = new AVFormatContext();
    access_private_field::CAVOutputStreamm_videoFormatContext(*m_avOutputStream)->oformat = new AVOutputFormat();
    access_private_field::CAVOutputStreamm_videoFormatContext(*m_avOutputStream)->oformat->flags = false;
//    stub.set(avcodec_find_encoder, avcodec_find_encoder_stub);
    //stub.set(avcodec_alloc_context3, avcodec_alloc_context3_stub);
    stub.set(avcodec_open2, avcodec_open2_stub1);
    stub.set(avformat_new_stream, avformat_new_stream_stub);

    int channel = 2;
    uint64_t channel_layout = 2;
    int sample_rate = 48000;
    int64_t bit_rate = 48000;
    m_avOutputStream->SetAudioCodecProp(AV_CODEC_ID_AAC, sample_rate, channel, channel_layout, bit_rate);
    //正式执行需测试的方法
    m_avOutputStream->init_context_amix(channel, channel_layout, sample_rate, bit_rate);

    //stub.reset(avcodec_alloc_context3);
    stub.reset(avcodec_open2);
    stub.reset(avformat_new_stream);

    delete access_private_field::CAVOutputStreamaudio_amix_st(*m_avOutputStream);
    delete access_private_field::CAVOutputStreamm_videoFormatContext(*m_avOutputStream)->oformat;
    delete access_private_field::CAVOutputStreamm_videoFormatContext(*m_avOutputStream);
//    delete access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream);
    avcodec_free_context(&access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream));

}
static int g_first3 = 0;
bool isWriteFrame_stub1()
{

    g_first3++;
    if (g_first3 == 1) {
        return true;
    } else {
        g_first3 = 0;
        return false;
    }
}
int writeFrame_stub(AVFormatContext *s, AVPacket *pkt)
{
    return 1;
}
int audioFifoSize_stub1(AVAudioFifo *af)
{
    g_first2++;
    if (g_first2 == 1 || g_first2 == 2) {
        return 1153;
    }
    g_first2 = 0;
    return 1;

}


TEST_F(CAVOutputStreamTest, writeMixAudio)
{
    access_private_field::CAVOutputStreamis_fifo_scardinit(*m_avOutputStream) = 1152;
    access_private_field::CAVOutputStreamm_sysAudioFifo(*m_avOutputStream) = av_audio_fifo_alloc(AVSampleFormat::AV_SAMPLE_FMT_S16, 2, 20 * 1152);
    access_private_field::CAVOutputStreamm_micAudioFifo(*m_avOutputStream) = av_audio_fifo_alloc(AVSampleFormat::AV_SAMPLE_FMT_S16, 2, 20 * 1152);

    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream) = new AVCodecContext();
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->frame_size = 1152;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->channels = 2;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_U8P;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->sample_rate = 48000;
    access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream)->channel_layout = 2;

    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream) = new AVCodecContext();
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->frame_size = 1152;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->channels = 2;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_U8P;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->sample_rate = 48000;
    access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream)->channel_layout = 2;

    access_private_field::CAVOutputStreamm_start_mix_time(*m_avOutputStream) = -1;
    access_private_field::CAVOutputStreamm_nLastAudioMixPresentationTime(*m_avOutputStream) = 1;

    access_private_field::CAVOutputStreamaudio_amix_st(*m_avOutputStream) = new AVStream();

    access_private_field::CAVOutputStreamaudio_amix_st(*m_avOutputStream)->codec = new AVCodecContext();
    access_private_field::CAVOutputStreamaudio_amix_st(*m_avOutputStream)->codec->sample_rate = 48000;

    access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream) = new AVCodecContext();
    access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream)->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
    access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream)->channel_layout = 2;
    access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream)->sample_rate = 48000;
    access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream)->frame_size = 1152;

    stub.set(ADDR(CAVOutputStream, audioFifoSize), audioFifoSize_stub1);
    stub.set(ADDR(CAVOutputStream, audioRead), audioRead_stub);
    stub.set(av_rescale_q, av_rescale_q_stub);
    stub.set(av_buffersrc_add_frame_flags, av_buffersrc_add_frame_flags_stub);
    stub.set(ADDR(CAVOutputStream, isWriteFrame), isWriteFrame_stub1);
    stub.set(avcodec_encode_audio2, avcodec_encode_audio2_stub);
    stub.set(ADDR(CAVOutputStream, writeFrame), writeFrame_stub);
    stub.set(av_buffersink_get_frame, av_buffersink_get_frame_stub);

    //正式执行需测试的方法
    m_avOutputStream->writeMixAudio();

    stub.reset(ADDR(CAVOutputStream, audioFifoSize));
    stub.reset(ADDR(CAVOutputStream, audioRead));
    stub.reset(av_rescale_q);
    stub.reset(av_buffersrc_add_frame_flags);
    stub.reset(ADDR(CAVOutputStream, isWriteFrame));
    stub.reset(avcodec_encode_audio2);
    stub.reset(ADDR(CAVOutputStream, writeFrame));
    stub.reset(av_buffersink_get_frame);

    delete access_private_field::CAVOutputStreampCodecCtx_amix(*m_avOutputStream);
    delete access_private_field::CAVOutputStreamaudio_amix_st(*m_avOutputStream)->codec;
    delete access_private_field::CAVOutputStreamaudio_amix_st(*m_avOutputStream) ;
    delete access_private_field::CAVOutputStreamm_pSysCodecContext(*m_avOutputStream);
    delete access_private_field::CAVOutputStreamm_pMicCodecContext(*m_avOutputStream);

}

TEST_F(CAVOutputStreamTest, setIsOverWrite)
{
    //正式执行需测试的方法
    m_avOutputStream->setIsOverWrite(false);
    bool flag = access_private_field::CAVOutputStreamm_isOverWrite(*m_avOutputStream);

    EXPECT_EQ(flag, false);
}


TEST_F(CAVOutputStreamTest, audioRead)
{
    stub.set(av_audio_fifo_read, av_audio_fifo_read_stub);

    AVAudioFifo *fifo = av_audio_fifo_alloc(AVSampleFormat::AV_SAMPLE_FMT_S16, 2, 20 * 1152);
    int index = m_avOutputStream->audioRead(fifo, nullptr, 48000);
    EXPECT_EQ(index, 1);
    stub.reset(av_audio_fifo_read);
}

TEST_F(CAVOutputStreamTest, audioWrite)
{
    stub.set(av_audio_fifo_write, av_audio_fifo_write_stub);
    AVAudioFifo *fifo = av_audio_fifo_alloc(AVSampleFormat::AV_SAMPLE_FMT_S16, 2, 20 * 1152);
    int index = m_avOutputStream->audioWrite(fifo, nullptr, 48000);
    EXPECT_EQ(index, 1);
    stub.reset(av_audio_fifo_write);
}


TEST_F(CAVOutputStreamTest, writeFrame)
{
    stub.set(av_interleaved_write_frame, av_interleaved_write_frame_stub);
    AVFormatContext *for_ctx = new AVFormatContext();
    AVPacket *pkt = new AVPacket();
    int index = m_avOutputStream->writeFrame(for_ctx, pkt);
    EXPECT_EQ(index, 1);
    stub.reset(av_interleaved_write_frame);
    delete pkt;
    delete for_ctx;
}

TEST_F(CAVOutputStreamTest, writeTrailer)
{
    stub.set(av_write_trailer, av_write_trailer_stub);
    AVFormatContext *for_ctx = new AVFormatContext();
    int index = m_avOutputStream->writeTrailer(for_ctx);
    EXPECT_EQ(index, 1);
    stub.reset(av_write_trailer);
    delete for_ctx;
}

TEST_F(CAVOutputStreamTest, audioFifoSpace)
{
    stub.set(av_audio_fifo_space, av_audio_fifo_space_stub);
    AVAudioFifo *fifo = av_audio_fifo_alloc(AVSampleFormat::AV_SAMPLE_FMT_S16, 2, 20 * 1152);
    int index = m_avOutputStream->audioFifoSpace(fifo);
    EXPECT_EQ(index, 1);
    stub.reset(av_audio_fifo_space);
}

TEST_F(CAVOutputStreamTest, audioFifoSize)
{
    stub.set(av_audio_fifo_size, av_audio_fifo_size_stub);
    AVAudioFifo *fifo = av_audio_fifo_alloc(AVSampleFormat::AV_SAMPLE_FMT_S16, 2, 20 * 1152);
    int index = m_avOutputStream->audioFifoSize(fifo);
    EXPECT_EQ(index, 1);
    stub.reset(av_audio_fifo_size);
}

TEST_F(CAVOutputStreamTest, audioFifoRealloc)
{
    AVAudioFifo *fifo = av_audio_fifo_alloc(AVSampleFormat::AV_SAMPLE_FMT_S16, 2, 20 * 1152);
    int index = m_avOutputStream->audioFifoRealloc(fifo, 48000);
    EXPECT_GT(index, -1);
    av_audio_fifo_free(fifo);
}

TEST_F(CAVOutputStreamTest, audioFifoAlloc)
{
    AVAudioFifo *fifo = m_avOutputStream->audioFifoAlloc(AVSampleFormat::AV_SAMPLE_FMT_S16, 2, 20 * 1152);
    EXPECT_NE(fifo, nullptr);
    av_audio_fifo_free(fifo);
}

TEST_F(CAVOutputStreamTest, audioFifoFree)
{
    AVAudioFifo *fifo = av_audio_fifo_alloc(AVSampleFormat::AV_SAMPLE_FMT_S16, 2, 20 * 1152);
    m_avOutputStream->audioFifoFree(fifo);

}

TEST_F(CAVOutputStreamTest, isWriteFrame)
{
    access_private_field::CAVOutputStreamm_isWriteFrame(*m_avOutputStream) = true;
    bool index = m_avOutputStream->isWriteFrame();
    EXPECT_EQ(index, true);
}

TEST_F(CAVOutputStreamTest, setIsWriteFrame)
{
    m_avOutputStream->setIsWriteFrame(false);
    EXPECT_EQ(access_private_field::CAVOutputStreamm_isWriteFrame(*m_avOutputStream), false);
}
