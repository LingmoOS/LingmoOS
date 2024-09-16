// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVOUTPUTSTREAM_H
#define AVOUTPUTSTREAM_H

#include <string>
#include <assert.h>
#include <QMutex>
#include "avlibinterface.h"
#include "waylandintegration_p.h"

using namespace std;

class CAVOutputStream
{
public:
    explicit CAVOutputStream(WaylandIntegration::WaylandIntegrationPrivate *context);
    ~CAVOutputStream(void);

    //初始化视频编码器
    void SetVideoCodecProp(AVCodecID codec_id, int framerate, int bitrate, int gopsize, int width, int height);

    //初始化音频编码器
    void SetAudioCodecProp(AVCodecID codec_id, int samplerate, int channels, int layout, int bitrate);
    void SetAudioCardCodecProp(AVCodecID codec_id, int samplerate, int channels, int layout, int bitrate);

    //创建编码器和混合器，打开输出
    bool open(QString path);

    /**
     * @brief close:关闭输出
     */
    void close();

    //写入一帧图像
    //int  writeVideoFrame(AVStream *st, enum AVPixelFormat pix_fmt, AVFrame *pframe, int64_t lTimeStamp);

    /**
     * @brief writeVideoFrame:写视频帧
     * @param frame:rgb帧
     * @return
     */
    int writeVideoFrame(WaylandIntegration::WaylandIntegrationPrivate::waylandFrame &frame);
    int writeMicAudioFrame(AVStream *stream, AVFrame *inputFrame, int64_t lTimeStamp);
    int writeMicToMixAudioFrame(AVStream *stream, AVFrame *inputFrame, int64_t lTimeStamp);
    int writeSysAudioFrame(AVStream *stream, AVFrame *inputFrame, int64_t lTimeStamp);
    int writeSysToMixAudioFrame(AVStream *stream, AVFrame *inputFrame, int64_t lTimeStamp);
    int write_filter_audio_frame(AVStream *&outst, AVCodecContext *&codecCtx_audio, AVFrame *&outframe);
    int init_filters();
    int init_context_amix(int channel, uint64_t channel_layout, int sample_rate, int64_t bit_rate);
    void writeMixAudio();
    void setIsOverWrite(bool isCOntinue);

    /**
     * @brief audioRead:读音频，自动锁
     * @param af
     * @param data
     * @param nb_samples
     * @return
     */
    int audioRead(AVAudioFifo *af, void **data, int nb_samples);

    /**
     * @brief audioWrite:写音频，自动锁
     * @param af
     * @param data
     * @param nb_samples
     * @return
     */

    int audioWrite(AVAudioFifo *af, void **data, int nb_samples);

    /**
     * @brief writeFrame:写视频帧，自动锁
     * @param s
     * @param pkt
     * @return
     */
    int writeFrame(AVFormatContext *s, AVPacket *pkt);

    /**
     * @brief writeTrailer:写视频文件尾，自动锁
     * @param s
     * @return
     */
    int writeTrailer(AVFormatContext *s);
    int audioFifoSpace(AVAudioFifo *af);
    int audioFifoSize(AVAudioFifo *af);
    int audioFifoRealloc(AVAudioFifo *af, int nb_samples);
    AVAudioFifo *audioFifoAlloc(enum AVSampleFormat sample_fmt, int channels, int nb_samples);
    void audioFifoFree(AVAudioFifo *af);
    bool isWriteFrame();
    void setIsWriteFrame(bool isWriteFrame);
    /**
     * @brief 设置电脑类型
     * @param boardVendorType
     */
    void setBoardVendor(int boardVendorType);
    /**
     * @brief 音频缓冲区是否还有数据
     * @return
     */
    bool isNotAudioFifoEmty();
protected:
    void freeSwrContext(struct SwrContext *swrContext);
public:
    //截图区域
    int m_left;
    int m_top;
    int m_right;
    int m_bottom;
    /**
     * @brief 视频类型
     */
    int m_videoType;
private:
    /**
     * @brief 电脑类型
     */
    int m_boardVendorType = 0;
    char *m_path;
    QMutex m_audioReadWriteMutex;
    QMutex m_writeFrameMutex;
    bool m_isWriteFrame;
    QMutex m_isWriteFrameMutex;
    WaylandIntegration::WaylandIntegrationPrivate *m_context;
    AVStream *m_videoStream;
    /**
     * @brief 麦克风音频流
     */
    AVStream *m_micAudioStream;
    /**
     * @brief 系统音频流
     */
    AVStream *m_sysAudioStream;
    /**
     * @brief 音频混合流
     */
    AVStream *audio_amix_st;
    AVFormatContext *m_videoFormatContext;
    /**
     * @brief 视频上下文
     */
    AVCodecContext *pCodecCtx;  //video
    /**
     * @brief 麦克风音频编码上下文
     */
    AVCodecContext *m_pMicCodecContext;  //audio
    /**
     * @brief 系统音频编码上下文
     */
    AVCodecContext *m_pSysCodecContext;

    /**
     * @brief 混合音频编码上下文
     */
    AVCodecContext *pCodecCtx_amix;
    /**
     * @brief 视频编码器
     */
    AVCodec *pCodec;  //videos
    /**
     * @brief 麦克风音频编码器
     */
    AVCodec *pCodec_a;  //audio
    /**
     * @brief 系统音频编码器
     */
    AVCodec *pCodec_aCard;
    /**
     * @brief 混合音频编码器
     */
    AVCodec *pCodec_amix;
    AVFrame *pFrameYUV;   ///转换为YUV420P保存的图像
    struct SwsContext *m_pVideoSwsContext;
    struct SwrContext *m_pMicAudioSwrContext;
    struct SwrContext *m_pSysAudioSwrContext;
    /**
     * @brief 麦克风音频缓冲区
     */
    AVAudioFifo *m_micAudioFifo;
    /**
     * @brief 系统音频缓冲区
     */
    AVAudioFifo *m_sysAudioFifo;
    int m_initFifoSpace = 0;
    int is_fifo_scardinit;
    int  m_nb_samples;
    //int64_t m_first_vid_time1, m_first_vid_time2; //前者是采集视频的第一帧的时间，后者是编码器输出的第一帧的时间
    int64_t m_start_mix_time; //第一个音频帧的时间
    int64_t m_next_vid_time;
    int64_t m_next_aud_time;
    int64_t  m_nLastAudioPresentationTime; //记录上一帧的音频时间戳
    int64_t  m_nLastAudioCardPresentationTime;
    int64_t  m_nLastAudioMixPresentationTime;
    /**
     * @brief 混合音频帧的数量
     */
    int64_t  m_mixCount;
    /**
     * @brief 单音音频帧编码数量
     */
    int64_t m_singleCount = 0;
    /**
     * @brief 写入视频帧的数量
     */
    int m_videoFrameCount = 0;

    /**
     * @brief 写入第一帧视频的时间戳
     */
    int64_t m_fristVideoFramePts = 0;

    uint8_t **m_convertedMicSamples;
    uint8_t **m_convertedSysSamples;
    uint8_t *m_out_buffer;
    AVFilterGraph *filter_graph;
    /**
     * @brief 过滤器上下文
     */
    AVFilterContext *buffersink_ctx;
    /**
     * @brief 麦克风音频过滤器上下文
     */
    AVFilterContext *buffersrc_ctx1;
    /**
     * @brief 系统音频过滤器上下文
     */
    AVFilterContext *buffersrc_ctx2;
    /**
     * @brief 写混合音频时，用来计数是否可以读取缓冲区的次数
     * 每当写混合音频时，会比较fifo缓冲区中的可读取帧数是否大于编码可用来编码的最小帧数，
     * 当fifo缓冲区中的可读取帧数是大于编码可用来编码的最小帧数，可进行写操作
     * 否则执行等待tmpFifoFailed开始计数，当连续300次不能进行写操作
     */
    int tmpFifoFailed;
    bool m_isOverWrite;
    AVCodecID  m_videoCodecID;
    AVCodecID  m_micAudioCodecID;
    AVCodecID  m_sysAudioCodecID;
    bool m_bMix;
    AVFrame *mMic_frame;
    AVFrame *mSpeaker_frame;
    int m_width, m_height;
    int m_framerate;
    int m_video_bitrate;
    int m_gopsize;
    int m_samplerate;
    int m_channels;
    int m_channels_layout;
    int m_audio_bitrate;
    int m_samplerate_card;
    int m_channels_card;
    int m_channels_card_layout;
    int m_audio_bitrate_card;
};

#endif //AVOUTPUTSTREAM_H
