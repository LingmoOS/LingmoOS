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
#include "../../src/waylandrecord/avinputstream.h"

#include "stub.h"
#include "addr_pri.h"


using namespace testing;

//打桩替换ffmpeg中的一些方法,由于ffmpeg的方法其实现对需要构造特别的数据，而进行某一个方法的单元测试，是无法构建完整的数据
//替换av_find_input_format函数
AVInputFormat *av_find_input_format_stub(const char *short_name)
{
    Q_UNUSED(short_name);
    return new AVInputFormat();
}
//替换avformat_open_input函数
int avformat_open_input_stub(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options)
{
    Q_UNUSED(ps);
    Q_UNUSED(url);
    Q_UNUSED(fmt);
    Q_UNUSED(options);
    return 0;
}
//替换avformat_find_stream_info函数
int avformat_find_stream_info_stub(AVFormatContext *ic, AVDictionary **options)
{
    return 1;
}
//替换avcodec_open2函数
int avcodec_open2_stub(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
{
    return 1;
}
//替换av_dump_format函数
void av_dump_format_stub(AVFormatContext *ic,
                         int index,
                         const char *url,
                         int is_output)
{
}

int64_t av_gettime_stub(void)
{
    return 1;
}
AVFrame *av_frame_alloc_stub(void)
{

    return new AVFrame();
}
void av_init_packet_stub(AVPacket *pkt)
{
    pkt = new AVPacket();

}
int av_read_frame_stub(AVFormatContext *s, AVPacket *pkt)
{
    return AVERROR_EOF;

}
int avcodec_decode_audio4_stub(AVCodecContext *avctx, AVFrame *frame,
                               int *got_frame_ptr, const AVPacket *avpkt)
{
    int p = 0;
    got_frame_ptr = &p;
    return 1;
}

void av_packet_unref_stub(AVPacket *pkt)
{

}
void av_free_packet_stub(AVPacket *pkt)
{
    delete pkt;
}
void av_frame_free_stub(AVFrame **frame)
{

    delete *frame;
}

void avformat_close_input_stub(AVFormatContext **s)
{

}

void avformat_free_context_stub(AVFormatContext *s)
{

}


class CAVInputStreamTest: public testing::Test
{

public:
    Stub stub;
    CAVInputStream *m_avInputStream;
    WaylandIntegration::WaylandIntegrationPrivate *m_context;
    virtual void SetUp() override
    {
        std::cout << "start UtilsTest" << std::endl;
        avlibInterface::initFunctions();
        m_context  = new  WaylandIntegration::WaylandIntegrationPrivate();
        m_avInputStream = new CAVInputStream(m_context);

    }

    virtual void TearDown() override
    {
        delete m_avInputStream;
        delete m_context;
        avlibInterface::unloadFunctions();
        std::cout << "end UtilsTest" << std::endl;
    }
};

TEST_F(CAVInputStreamTest, setMicAudioRecord)
{

    m_avInputStream->setMicAudioRecord(false);
    EXPECT_EQ(m_avInputStream->m_bMicAudio, false);
}

TEST_F(CAVInputStreamTest, setSysAudioRecord)
{
    m_avInputStream->setSysAudioRecord(false);
    EXPECT_EQ(m_avInputStream->m_bSysAudio, false);
}


TEST_F(CAVInputStreamTest, openInputStream)
{
    stub.set(av_find_input_format, av_find_input_format_stub);
    stub.set(avformat_open_input, avformat_open_input_stub);
    stub.set(avformat_find_stream_info, avformat_find_stream_info_stub);
    stub.set(avcodec_open2, avcodec_open2_stub);
    stub.set(av_dump_format, av_dump_format_stub);
    //构造数据
    m_avInputStream->m_bMicAudio = true;
    m_avInputStream->m_pMicAudioFormatContext = new AVFormatContext() ;
    m_avInputStream->m_pMicAudioFormatContext->nb_streams = 2;
    AVStream *temp1Stream = new AVStream();
    temp1Stream->codec = new  AVCodecContext();
    temp1Stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    m_avInputStream->m_pMicAudioFormatContext->streams = &temp1Stream;
    m_avInputStream->m_bSysAudio = true;
    m_avInputStream->m_pSysAudioFormatContext = new AVFormatContext() ;
    m_avInputStream->m_pSysAudioFormatContext->nb_streams = 2;
    m_avInputStream->m_pSysAudioFormatContext->streams = &temp1Stream;

    //执行方法
    m_avInputStream->openInputStream();
    stub.reset(av_find_input_format);
    stub.reset(avformat_open_input);
    stub.reset(avformat_find_stream_info);
    stub.reset(avcodec_open2);
    stub.reset(av_dump_format);

    delete  m_avInputStream->m_pSysAudioFormatContext;
    delete  temp1Stream->codec;
    delete  temp1Stream;
    delete  m_avInputStream->m_pMicAudioFormatContext;
    delete  m_avInputStream->m_pAudioCardInputFormat;
    delete  m_avInputStream->m_pAudioInputFormat;
}

TEST_F(CAVInputStreamTest, onsFinisheStream)
{
    m_avInputStream->m_hMixThread = 1;

    m_avInputStream->m_hMicAudioThread = 1;

    m_avInputStream->m_hSysAudioThread = 1;

    m_avInputStream->onsFinisheStream();

    EXPECT_EQ(m_avInputStream->m_hMixThread, 0);
    EXPECT_EQ(m_avInputStream->m_hMicAudioThread, 0);
    EXPECT_EQ(m_avInputStream->m_hSysAudioThread, 0);

}


int pthread_create_stub()
{

    return 1;
}

TEST_F(CAVInputStreamTest, audioCapture)
{

    stub.set(av_gettime, av_gettime_stub);
    stub.set(pthread_create, pthread_create_stub);
    m_avInputStream->m_bMix = true;
    m_avInputStream->m_bMicAudio = false;
    m_avInputStream->m_bSysAudio = false;

    m_avInputStream->audioCapture();
    stub.reset(av_gettime);
    stub.reset(pthread_create);
}

TEST_F(CAVInputStreamTest, GetVideoInputInfo)
{

    m_avInputStream->m_screenDW = 1000;
    m_avInputStream->m_left = 50;
    m_avInputStream->m_right = 50;

    m_avInputStream->m_screenDH = 1000;
    m_avInputStream->m_top = 50;
    m_avInputStream->m_bottom = 50;

    m_avInputStream->m_ipix_fmt = AVPixelFormat::AV_PIX_FMT_NB;
    m_avInputStream->m_fps = 50;

    int width = 0;
    int height = 0;
    AVPixelFormat pixFmt;
    int frame_rate = 0;
    bool flag = m_avInputStream->GetVideoInputInfo(width, height, frame_rate, pixFmt);

    EXPECT_EQ(flag, true);
}

TEST_F(CAVInputStreamTest, GetAudioInputInfo)
{
    m_avInputStream->m_micAudioindex = 0;
    m_avInputStream->m_pMicAudioFormatContext = new AVFormatContext() ;

    AVStream *temp1Stream = new AVStream();
    temp1Stream->codec = new  AVCodecContext();
    temp1Stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    temp1Stream->codec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_NB;
    temp1Stream->codec->sample_rate = 1000;
    temp1Stream->codec->channels = 2;
    temp1Stream->codec->channel_layout = 2;
    m_avInputStream->m_pMicAudioFormatContext->streams = &temp1Stream;

    int sample_rate = 0;
    int channels = 0;
    AVSampleFormat sample_fmt;
    int layout = 0;
    bool flag = m_avInputStream->GetAudioInputInfo(sample_fmt, sample_rate, channels, layout);
    EXPECT_EQ(flag, true);

    delete temp1Stream->codec;
    delete temp1Stream;
    delete m_avInputStream->m_pMicAudioFormatContext;

}

TEST_F(CAVInputStreamTest, GetAudioSCardInputInfo)
{
    m_avInputStream->m_sysAudioindex = 0;
    m_avInputStream->m_pSysAudioFormatContext = new AVFormatContext() ;
    AVStream *temp1Stream = new AVStream();
    temp1Stream->codec = new  AVCodecContext();
    temp1Stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    temp1Stream->codec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_NB;
    temp1Stream->codec->sample_rate = 1000;
    temp1Stream->codec->channels = 2;
    temp1Stream->codec->channel_layout = 2;
    m_avInputStream->m_pSysAudioFormatContext->streams = &temp1Stream;

    int sample_rate = 0;
    int channels = 0;
    AVSampleFormat sample_fmt;
    int layout = 0;
    bool flag = m_avInputStream->GetAudioSCardInputInfo(sample_fmt, sample_rate, channels, layout);
    EXPECT_EQ(flag, true);
    delete temp1Stream->codec;
    delete temp1Stream;
    delete m_avInputStream->m_pSysAudioFormatContext;

}

TEST_F(CAVInputStreamTest, currentAudioChannel)
{
    m_avInputStream->currentAudioChannel();
}

static int g_first = 0;
bool bRunThread_stub()
{
    g_first++;
    if (g_first == 1) {
        return true;
    } else {
        g_first = 0;
        return false;
    }
}

ACCESS_PRIVATE_FUN(CAVInputStream, int(), readMicAudioPacket);
TEST_F(CAVInputStreamTest, readMicAudioPacket)
{
    stub.set(ADDR(CAVInputStream, bRunThread), bRunThread_stub);
    stub.set(av_frame_alloc, av_frame_alloc_stub);
    //stub.set(av_init_packet, av_init_packet_stub);
    stub.set(av_read_frame, av_read_frame_stub);
    stub.set(avcodec_decode_audio4, avcodec_decode_audio4_stub);
    stub.set(av_packet_unref, av_packet_unref_stub);
    stub.set(av_free_packet, av_free_packet_stub);
    stub.set(av_frame_free, av_frame_free_stub);
    stub.set(avformat_close_input, avformat_close_input_stub);
    stub.set(avformat_free_context, avformat_free_context_stub);


    m_avInputStream->m_pMicAudioFormatContext = avformat_alloc_context();
    m_avInputStream->m_micAudioindex = 0 ;

    AVStream *temp1Stream = new AVStream();
    temp1Stream->codec = new  AVCodecContext();
    temp1Stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    temp1Stream->codec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_NB;
    temp1Stream->codec->sample_rate = 1000;
    temp1Stream->codec->channels = 2;
    temp1Stream->codec->channel_layout = 2;
    m_avInputStream->m_pMicAudioFormatContext->streams = &temp1Stream;
    auto ret = call_private_fun::CAVInputStreamreadMicAudioPacket(*m_avInputStream);

    stub.reset(bRunThread_stub);
    stub.reset(av_frame_alloc_stub);
//    stub.reset(av_init_packet_stub);
    stub.reset(av_read_frame_stub);
    stub.reset(avcodec_decode_audio4_stub);
    stub.reset(av_packet_unref_stub);
    stub.reset(av_free_packet);
    stub.reset(av_frame_free_stub);
    stub.reset(avformat_close_input_stub);
    stub.reset(avformat_free_context_stub);

    EXPECT_EQ(ret, 0);

    delete temp1Stream->codec;
    delete temp1Stream;

    avformat_free_context(m_avInputStream->m_pMicAudioFormatContext);

}

ACCESS_PRIVATE_FUN(CAVInputStream, int(), readMicToMixAudioPacket);
TEST_F(CAVInputStreamTest, readMicToMixAudioPacket)
{
    stub.set(ADDR(CAVInputStream, bRunThread), bRunThread_stub);
    stub.set(av_frame_alloc, av_frame_alloc_stub);
//    stub.set(av_init_packet, av_init_packet_stub);
    stub.set(av_read_frame, av_read_frame_stub);
    stub.set(avcodec_decode_audio4, avcodec_decode_audio4_stub);
    stub.set(av_packet_unref, av_packet_unref_stub);
    stub.set(av_free_packet, av_free_packet_stub);
    stub.set(av_frame_free, av_frame_free_stub);
    stub.set(avformat_close_input, avformat_close_input_stub);
    stub.set(avformat_free_context, avformat_free_context_stub);


    m_avInputStream->m_pMicAudioFormatContext = avformat_alloc_context();
    m_avInputStream->m_micAudioindex = 0 ;

    AVStream *temp1Stream = new AVStream();
    temp1Stream->codec = new  AVCodecContext();
    temp1Stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    temp1Stream->codec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_NB;
    temp1Stream->codec->sample_rate = 1000;
    temp1Stream->codec->channels = 2;
    temp1Stream->codec->channel_layout = 2;
    m_avInputStream->m_pMicAudioFormatContext->streams = &temp1Stream;
    auto ret = call_private_fun::CAVInputStreamreadMicToMixAudioPacket(*m_avInputStream);

    stub.reset(bRunThread_stub);
    stub.reset(av_frame_alloc_stub);
//    stub.reset(av_init_packet_stub);
    stub.reset(av_read_frame_stub);
    stub.reset(avcodec_decode_audio4_stub);
    stub.reset(av_packet_unref_stub);
    stub.reset(av_free_packet);
    stub.reset(av_frame_free_stub);
    stub.reset(avformat_close_input_stub);
    stub.reset(avformat_free_context_stub);

    EXPECT_EQ(ret, 0);

    delete temp1Stream->codec;
    delete temp1Stream;
    avformat_free_context(m_avInputStream->m_pMicAudioFormatContext);

}

ACCESS_PRIVATE_FUN(CAVInputStream, int(), readSysAudioPacket);
TEST_F(CAVInputStreamTest, readSysAudioPacket)
{
    if (QSysInfo::currentCpuArchitecture().startsWith("arm"))
        return;
    stub.set(ADDR(CAVInputStream, bRunThread), bRunThread_stub);
    stub.set(av_frame_alloc, av_frame_alloc_stub);
//    stub.set(av_init_packet, av_init_packet_stub);
    stub.set(av_read_frame, av_read_frame_stub);
    stub.set(avcodec_decode_audio4, avcodec_decode_audio4_stub);
    stub.set(av_packet_unref, av_packet_unref_stub);
    stub.set(av_frame_free, av_frame_free_stub);
    stub.set(avformat_close_input, avformat_close_input_stub);
    stub.set(avformat_free_context, avformat_free_context_stub);

    m_avInputStream->m_bSysAudio = true ;
    m_avInputStream->m_pSysAudioFormatContext = avformat_alloc_context();
    m_avInputStream->m_sysAudioindex = 0 ;

    AVStream *temp1Stream = new AVStream();
    temp1Stream->codec = new  AVCodecContext();
    temp1Stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    temp1Stream->codec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_NB;
    temp1Stream->codec->sample_rate = 1000;
    temp1Stream->codec->channels = 2;
    temp1Stream->codec->channel_layout = 2;
    m_avInputStream->m_pSysAudioFormatContext->streams = &temp1Stream;
    auto ret = call_private_fun::CAVInputStreamreadSysAudioPacket(*m_avInputStream);

    stub.reset(ADDR(CAVInputStream, bRunThread));
    stub.reset(av_frame_alloc);
//    stub.reset(av_init_packet);
    stub.reset(av_read_frame);
    stub.reset(avcodec_decode_audio4);
    stub.reset(av_packet_unref);
    stub.reset(av_frame_free);
    stub.reset(avformat_close_input);
    stub.reset(avformat_free_context);

    EXPECT_EQ(ret, 0);
    delete temp1Stream->codec ;
    delete temp1Stream ;
    avformat_free_context(m_avInputStream->m_pSysAudioFormatContext);

}

ACCESS_PRIVATE_FUN(CAVInputStream, int(), readSysToMixAudioPacket);
TEST_F(CAVInputStreamTest, readSysToMixAudioPacket)
{
    if (QSysInfo::currentCpuArchitecture().startsWith("arm"))
        return;
    stub.set(ADDR(CAVInputStream, bRunThread), bRunThread_stub);
    stub.set(av_frame_alloc, av_frame_alloc_stub);
//    stub.set(av_init_packet, av_init_packet_stub);
    stub.set(av_read_frame, av_read_frame_stub);
    stub.set(avcodec_decode_audio4, avcodec_decode_audio4_stub);
    stub.set(av_packet_unref, av_packet_unref_stub);
    stub.set(av_frame_free, av_frame_free_stub);
    stub.set(avformat_close_input, avformat_close_input_stub);
    stub.set(avformat_free_context, avformat_free_context_stub);

    m_avInputStream->m_bSysAudio = true ;
    m_avInputStream->m_pSysAudioFormatContext = avformat_alloc_context();
    m_avInputStream->m_sysAudioindex = 0 ;

    AVStream *temp1Stream = new AVStream();
    temp1Stream->codec = new  AVCodecContext();
    temp1Stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    temp1Stream->codec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_NB;
    temp1Stream->codec->sample_rate = 1000;
    temp1Stream->codec->channels = 2;
    temp1Stream->codec->channel_layout = 2;
    m_avInputStream->m_pSysAudioFormatContext->streams = &temp1Stream;


    auto ret = call_private_fun::CAVInputStreamreadSysToMixAudioPacket(*m_avInputStream);


    stub.reset(ADDR(CAVInputStream, bRunThread));
    stub.reset(av_frame_alloc);
//    stub.reset(av_init_packet);
    stub.reset(av_read_frame);
    stub.reset(avcodec_decode_audio4);
    stub.reset(av_packet_unref);
    stub.reset(av_frame_free);
    stub.reset(avformat_close_input);
    stub.reset(avformat_free_context);

    EXPECT_EQ(ret, 0);

    delete temp1Stream->codec;
    delete temp1Stream;
//    delete m_avInputStream->m_pSysAudioFormatContext;
    avformat_free_context(m_avInputStream->m_pSysAudioFormatContext);
}

ACCESS_PRIVATE_FUN(CAVInputStream, void(), initScreenData);
TEST_F(CAVInputStreamTest, initScreenData)
{
    call_private_fun::CAVInputStreaminitScreenData(*m_avInputStream);
}

ACCESS_PRIVATE_FUN(CAVInputStream, bool(), bWriteMix);
ACCESS_PRIVATE_FIELD(CAVInputStream, bool, m_bWriteMix);
TEST_F(CAVInputStreamTest, bWriteMix)
{
    access_private_field::CAVInputStreamm_bWriteMix(*m_avInputStream) = true;
    bool res = call_private_fun::CAVInputStreambWriteMix(*m_avInputStream);

    EXPECT_EQ(res, true);

}

ACCESS_PRIVATE_FUN(CAVInputStream, bool(), bRunThread);
ACCESS_PRIVATE_FIELD(CAVInputStream, bool, m_bRunThread);
TEST_F(CAVInputStreamTest, bRunThread)
{
    access_private_field::CAVInputStreamm_bRunThread(*m_avInputStream) = true;
    bool res = call_private_fun::CAVInputStreambRunThread(*m_avInputStream);

    EXPECT_EQ(res, true);
}

ACCESS_PRIVATE_FUN(CAVInputStream, void (), writMixAudio);
static int first = 0;
bool bWriteMix_stub()
{
    first ++;
    if (first == 1) {
        return true;
    } else {
        first = 0;
        return false;
    }
}

ACCESS_PRIVATE_FIELD(CAVInputStream, bool, m_bMix);
TEST_F(CAVInputStreamTest, writMixAudio)
{
    access_private_field::CAVInputStreamm_bMix(*m_avInputStream) = false;
    stub.set(ADDR(CAVInputStream, bWriteMix), bWriteMix_stub);
    call_private_fun::CAVInputStreamwritMixAudio(*m_avInputStream);

    stub.reset(ADDR(CAVInputStream, bWriteMix));
}


