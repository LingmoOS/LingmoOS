// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avinputstream.h"
#include <unistd.h>
#include <stdio.h>
#include <qbuffer.h>
#include <QProcess>
#include <QTime>
#include <QDebug>
#include <QThread>
#include <QMutexLocker>
#include "recordadmin.h"

CAVInputStream::CAVInputStream(WaylandIntegration::WaylandIntegrationPrivate *context):
    m_context(context)
{
    m_bMix = false;
    setbWriteAmix(true);
    m_hMicAudioThread = 0;
    setbRunThread(true);
    //m_outPutType = MP4_MKV_;
    m_pMicAudioFormatContext = nullptr;
    m_pSysAudioFormatContext = nullptr;
    m_pAudioInputFormat = nullptr;
    m_pAudioCardInputFormat = nullptr;
    dec_pkt = nullptr;
    m_bMicAudio = false;
    m_bSysAudio = false;
    m_micAudioindex = -1;
    m_sysAudioindex = -1;
    m_left = 0;
    m_top = 0;
    m_right = 0;
    m_bottom = 0;
    m_fps = 0;
    m_screenDW = 0;
    m_screenDH = 0;
    m_selectWidth = 0;
    m_selectHeight = 0;
    m_start_time = 0;
}

CAVInputStream::~CAVInputStream(void)
{
    printf("Desctruction Input!\n");
    setbWriteAmix(false);
    setbRunThread(false);
}

void CAVInputStream::setMicAudioRecord(bool bRecord)
{
    m_bMicAudio = bRecord;
}

void CAVInputStream::setSysAudioRecord(bool bRecord)
{
    m_bSysAudio = bRecord;
}

bool CAVInputStream::openInputStream()
{
    //打开系统音频输入流，
    bool bSysAudio = openSysStream();
    //此处的逻辑：当外部选择需要录制系统音（即m_bSysAudio为false），但是系统音频的输出流打开失败时，内部会将可以录制改为不可录制
    if (bSysAudio != m_bSysAudio) {
        qWarning() << "sys audio stream open failure! " << "m_bSysAudio : " << m_bSysAudio << " to m_bSysAudio: " << bSysAudio;
        m_bSysAudio = bSysAudio;
    }
    //开麦克风音频输入流 注：录屏时不要求录制麦克风时（即m_bMicAudio为false），则此方法默认返回false
    bool bMicAudio = openMicStream();
    //此处的逻辑：录制视频时选择需要录制麦克风音频，但是麦克风音频的输入流打开失败时，内部会将可以录制改为不可录制
    if (bMicAudio != m_bMicAudio) {
        qWarning() << "sys audio stream open failure! " << "m_bMicAudio : " << m_bMicAudio << " to m_bMicAudio: " << bMicAudio;
        m_bMicAudio = bMicAudio;
    }
    //只有外部选择了系统音频、麦克风音频及打开系统音频输出流成功、打开麦克风音频输入流成功的情况下才录制混音
    if (bSysAudio && bMicAudio) {
        m_bMix = true;
    }
    return true;
}

//开麦克风音频输入流 注：录屏时不要求录制麦克风时，则此方法默认返回false
bool CAVInputStream::openMicStream()
{
    AVDictionary *device_param = nullptr;
    m_pAudioInputFormat = avlibInterface::m_av_find_input_format("pulse"); //alsa
    assert(m_pAudioInputFormat != nullptr);
    if (m_pAudioInputFormat == nullptr) {
        qWarning() << "did not find this audio input devices\n";
    }
    if (m_bMicAudio) {
        if (m_micDeviceName.isEmpty()) {
            qWarning() << "Error: The mic audio device name is empty!";
            return false;
        }
        //Set own audio device's name
        if (avlibInterface::m_avformat_open_input(&m_pMicAudioFormatContext, m_micDeviceName.toLatin1(), m_pAudioInputFormat, &device_param) != 0) {
            qWarning() << "Couldn't open input audio stream.（无法打开输入流）\n";
            return false;
        }
        qInfo() << "mic device's name is:" << m_micDeviceName;
        //input audio initialize
        if (avlibInterface::m_avformat_find_stream_info(m_pMicAudioFormatContext, nullptr) < 0) {
            qWarning() << "Couldn't find audio stream information.（无法获取流信息）\n";
            return false;
        }
        m_micAudioindex = -1;
        for (int i = 0; i < static_cast<int>(m_pMicAudioFormatContext->nb_streams); i++) {
            if (m_pMicAudioFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
                m_micAudioindex = i;
                break;
            }
        }
        if (m_micAudioindex == -1) {
            qWarning() << "Couldn't find a audio stream.（没有找到音频流）\n";
            return false;
        }
        if (avlibInterface::m_avcodec_open2(m_pMicAudioFormatContext->streams[m_micAudioindex]->codec, avlibInterface::m_avcodec_find_decoder(m_pMicAudioFormatContext->streams[m_micAudioindex]->codec->codec_id), nullptr) < 0) {
            qWarning() << "Could not open audio codec.（无法打开解码器）\n";
            return false;
        }
        /* print Video device information*/
        avlibInterface::m_av_dump_format(m_pMicAudioFormatContext, 0, "default", 0);
        return true;
    } else {
        return false;
    }
}

//打开系统音频输入流 注：录屏时不要求录制系统音时，则此方法默认返回false
bool CAVInputStream::openSysStream()
{
    AVDictionary *device_param = nullptr;
    m_pAudioCardInputFormat = avlibInterface::m_av_find_input_format("pulse"); //alsa
    assert(m_pAudioCardInputFormat != nullptr);
    if (m_pAudioCardInputFormat == nullptr) {
        qWarning() << "did not find this card audio input devices\n";
    }
    if (m_bSysAudio) {
        if (avlibInterface::m_avformat_open_input(&m_pSysAudioFormatContext, m_sysDeviceName.toLatin1(), m_pAudioCardInputFormat, &device_param) != 0) {
            qWarning() << "Couldn't open input audio stream.（无法打开输入流）\n";
            return false;
        }
        qInfo() << "sys device's name is:" << m_sysDeviceName;
        if (avlibInterface::m_avformat_find_stream_info(m_pSysAudioFormatContext, nullptr) < 0) {
            qWarning() << "Couldn't find audio stream information.（无法获取流信息）";
            return false;
        }
        fflush(stdout);
        m_sysAudioindex = -1;
        for (int i = 0; i < static_cast<int>(m_pSysAudioFormatContext->nb_streams); i++) {
            if (m_pSysAudioFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
                m_sysAudioindex = i;
                break;
            }
        }
        if (m_sysAudioindex == -1) {
            qWarning() << "Couldn't find a audio stream.（没有找到音频流）\n";
            return false;
        }
        ///Caution, m_pAudFmtCtx->streams[m_audioindex]->codec->codec_id =14, AV_CODEC_ID_RAWVIDEO
        if (avlibInterface::m_avcodec_open2(m_pSysAudioFormatContext->streams[m_sysAudioindex]->codec, avlibInterface::m_avcodec_find_decoder(m_pSysAudioFormatContext->streams[m_sysAudioindex]->codec->codec_id), nullptr) < 0) {
            qWarning() << "Could not open audio codec.（无法打开解码器）\n";
            return false;
        }
        /* print Video device information*/
        avlibInterface::m_av_dump_format(m_pSysAudioFormatContext, 0, m_sysDeviceName.toLatin1(), 0);
        return true;
    } else {
        return false;
    }
}

bool CAVInputStream::audioCapture()
{
    m_start_time = avlibInterface::m_av_gettime();
    if (m_bMix) {
        pthread_create(&m_hMicAudioThread, nullptr, captureMicToMixAudioThreadFunc, static_cast<void *>(this));
        pthread_create(&m_hSysAudioThread, nullptr, captureSysToMixAudioThreadFunc, static_cast<void *>(this));
        pthread_create(&m_hMixThread, nullptr, writeMixThreadFunc, static_cast<void *>(this));
    } else {
        if (m_bMicAudio) {
            pthread_create(&m_hMicAudioThread, nullptr, captureMicAudioThreadFunc, static_cast<void *>(this));
        }
        if (m_bSysAudio) {
            pthread_create(&m_hSysAudioThread, nullptr, captureSysAudioThreadFunc, static_cast<void *>(this));
        }
    }
    return true;
}

//void CAVInputStream::writeToFrame(QImage *img, int64_t time){
//    if (m_exit_thread)
//        return;
//    //    if(dec_pkt == nullptr)
//    //    {
//    //        return;
//    //    }
//    if(m_start_time<=0){
//        return ;
//    }
//    int encode_video = 1;
//    int ret;
//    AVFrame * pframe = nullptr;
//    pframe = av_frame_alloc();
//    pframe->width = m_screenDW;
//    pframe->height = m_screenDH;
//    pframe->format =AV_PIX_FMT_BGR32;
//    ret = av_frame_get_buffer(pframe,32);
//    if(ret==0)
//    {

//        //pthread_mutex_lock(&mutexScreenD);
//        QImage* imageTempt = img;
//        int64_t timeStamp1 =av_gettime();

//        //pthread_mutex_unlock(&mutexScreenD);

//        pframe->width = imageTempt->width();
//        pframe->height = imageTempt->height();
//        pframe->format = m_ipix_fmt;
//        pframe->pts = 0;
//        pframe->crop_left = m_cl;
//        pframe->crop_top = m_ct;
//        pframe->crop_right = m_cr;
//        pframe->crop_bottom = m_cb;
//        pframe->data[0] = imageTempt->bits();
//        //int dec_got_frame = 0;
//        if(m_pVideoCBFunc)
//        {
//            int64_t timeStamp = timeStamp1 - m_start_time;
//            //if(timeStamp>=0){
//            //pthread_mutex_lock(&mutex);
//            static int s_flag = 0;
//            if (s_flag < time)
//            {
//                s_flag = time;
//            }
//            else {
//                qDebug() << "error=================";
//            }
//            //qDebug() << "+++++++++++++++++++++++++++++++++++:时间戳："  << time;
//            m_pVideoCBFunc(nullptr, m_ipix_fmt, pframe,time);
//            //pthread_mutex_unlock(&mutex);
//            //}
//        }
//        av_frame_free(&pframe);
//    }
//}

//void CAVInputStream::writeFrame(WaylandIntegration::WaylandIntegrationPrivate::waylandFrame &frame)
//{
//    //qDebug() << "22222222222222222222222:writeToFrame" << QThread::currentThreadId();
//    if (isExitThread() || m_start_time<=0)
//        return;
//    AVFrame * pframe = av_frame_alloc();
//    pframe->width = m_screenDW;
//    pframe->height = m_screenDH;
//    pframe->format =AV_PIX_FMT_BGR32;
//    if(0 == av_frame_get_buffer(pframe,32))
//    {
//        pframe->width = frame._width;
//        pframe->height = frame._height;
//        pframe->format = m_ipix_fmt;
//        pframe->crop_left = m_left;
//        pframe->crop_top = m_top;
//        pframe->crop_right = m_right;
//        pframe->crop_bottom = m_bottom;
//        pframe->linesize[0] = frame._stride;
//        pframe->data[0] = frame._frame;
////        if(m_pVideoCBFunc)
////        {
////            //qDebug() << "+++++++++++++++++++++++++++++++++++:时间戳："  << time;
////            m_pVideoCBFunc(nullptr, m_ipix_fmt, pframe,frame._time);
////        }
//        av_frame_free(&pframe);
//    }
//}
void CAVInputStream::initScreenData()
{

}

bool CAVInputStream::bWriteMix()
{
    QMutexLocker locker(&m_bWriteMixMutex);
    return m_bWriteMix;
}

void CAVInputStream::setbWriteAmix(bool bWriteMix)
{
    QMutexLocker locker(&m_bWriteMixMutex);
    m_bWriteMix = bWriteMix;
}

bool CAVInputStream::bRunThread()
{
    QMutexLocker locker(&m_bRunThreadMutex);
    return m_bRunThread;
}

void CAVInputStream::setbRunThread(bool bRunThread)
{
    QMutexLocker locker(&m_bRunThreadMutex);
    m_bRunThread = bRunThread;
}

void  CAVInputStream::onsFinisheStream()
{
    if (m_hMixThread) {
        m_hMixThread = 0;
    }
    if (m_hMicAudioThread) {
        m_hMicAudioThread = 0;
    }
    if (m_hSysAudioThread) {
        m_hSysAudioThread = 0;
    }
}

void *CAVInputStream::writeMixThreadFunc(void *param)
{
    CAVInputStream *inputStream = static_cast<CAVInputStream *>(param);
    inputStream->writMixAudio();
    return nullptr;
}

void CAVInputStream::writMixAudio()
{
    //    while ((bWriteMix() || m_context->m_recordAdmin->m_pOutputStream->isNotAudioFifoEmty()) && m_bMix) {
    while (bWriteMix() && m_bMix) {
        m_context->m_recordAdmin->m_pOutputStream->writeMixAudio();
    }
}

void *CAVInputStream::captureMicAudioThreadFunc(void *param)
{
    CAVInputStream *inputStream = static_cast<CAVInputStream *>(param);
    inputStream->readMicAudioPacket();
    return nullptr;
}

void *CAVInputStream::captureMicToMixAudioThreadFunc(void *param)
{
    CAVInputStream *inputStream = static_cast<CAVInputStream *>(param);
    inputStream->readMicToMixAudioPacket();
    return nullptr;
}

//ReadAudioPackets + ReadVideoPackets
int CAVInputStream::readMicAudioPacket()
{
    int got_frame_ptr = 0;
    //start decode and encode
    while (bRunThread()) {
        int ret;
        AVFrame *inputFrame = avlibInterface::m_av_frame_alloc();
        if (!inputFrame)
            return AVERROR(ENOMEM);
        /** Decode one frame worth of audio samples. */
        /** Packet used for temporary storage. */
        AVPacket inputPacket;
        avlibInterface::m_av_init_packet(&inputPacket);
        inputPacket.data = nullptr;
        inputPacket.size = 0;
        /** Read one audio frame from the input file into a temporary packet. */
        if ((ret = avlibInterface::m_av_read_frame(m_pMicAudioFormatContext, &inputPacket)) < 0) {
            /** If we are at the end of the file, flush the decoder below. */
            if (ret == AVERROR_EOF) {
                setbRunThread(false);
            } else {
                continue;
            }
        }
        //AVPacket -> AVFrame
        if ((ret = avlibInterface::m_avcodec_decode_audio4(m_pMicAudioFormatContext->streams[m_micAudioindex]->codec, inputFrame, &got_frame_ptr, &inputPacket)) < 0) {
            printf("Could not decode audio frame\n");
            return ret;
        }
        avlibInterface::m_av_packet_unref(&inputPacket);
        /** If there is decoded data, convert and store it */
        if (got_frame_ptr) {
            m_context->m_recordAdmin->m_pOutputStream->writeMicAudioFrame(m_pMicAudioFormatContext->streams[m_micAudioindex], inputFrame, avlibInterface::m_av_gettime() - m_start_time);
        }
        avlibInterface::m_av_frame_free(&inputFrame);
        fflush(stdout);
    }// -- while end
    if (nullptr != m_pMicAudioFormatContext) {
        avlibInterface::m_avformat_close_input(&m_pMicAudioFormatContext);
    }
    if (nullptr != m_pMicAudioFormatContext) {
        avlibInterface::m_avformat_free_context(m_pMicAudioFormatContext);
        m_pMicAudioFormatContext = nullptr;
    }
    m_micAudioindex = -1;
    return 0;
}

int CAVInputStream::readMicToMixAudioPacket()
{
    int got_frame_ptr = 0;
    //start decode and encode
    while (bRunThread()) {
        int ret;
        AVFrame *inputFrame = avlibInterface::m_av_frame_alloc();
        if (!inputFrame)
            return AVERROR(ENOMEM);
        /** Decode one frame worth of audio samples.解码一帧值的音频样本。 */
        /** Packet used for temporary storage. 临时存储用的小包。*/
        AVPacket inputPacket;
        avlibInterface::m_av_init_packet(&inputPacket);
        inputPacket.data = nullptr;
        inputPacket.size = 0;
        /** Read one audio frame from the input file into a temporary packet. 从输入文件中读取一个音频帧到一个临时包中。*/
        if ((ret = avlibInterface::m_av_read_frame(m_pMicAudioFormatContext, &inputPacket)) < 0) {
            /** If we are at the end of the file, flush the decoder below. 如果我们在文件的末尾，刷新下面的解码器。*/
            if (ret == AVERROR_EOF) {
                setbRunThread(false);
            } else {
                continue;
            }
        }
        //AVPacket -> AVFrame
        if ((ret = avlibInterface::m_avcodec_decode_audio4(m_pMicAudioFormatContext->streams[m_micAudioindex]->codec, inputFrame, &got_frame_ptr, &inputPacket)) < 0) {
            printf("Could not decode audio frame\n");
            return ret;
        }
        avlibInterface::m_av_packet_unref(&inputPacket);
        /** If there is decoded data, convert and store it 如果有解码的数据，转换并存储它*/
        if (got_frame_ptr) {
            m_context->m_recordAdmin->m_pOutputStream->writeMicToMixAudioFrame(m_pMicAudioFormatContext->streams[m_micAudioindex], inputFrame, avlibInterface::m_av_gettime() - m_start_time);
        }
        avlibInterface::m_av_frame_free(&inputFrame);
        fflush(stdout);
    }// -- while end
    if (nullptr != m_pMicAudioFormatContext) {
        avlibInterface::m_avformat_close_input(&m_pMicAudioFormatContext);
    }
    if (nullptr != m_pMicAudioFormatContext) {
        avlibInterface::m_avformat_free_context(m_pMicAudioFormatContext);
        m_pMicAudioFormatContext = nullptr;
    }
    m_micAudioindex = -1;
    return 0;
}

void *CAVInputStream::captureSysAudioThreadFunc(void *param)
{
    CAVInputStream *inputStream = static_cast<CAVInputStream *>(param);
    inputStream->readSysAudioPacket();
    return nullptr;
}

void *CAVInputStream::captureSysToMixAudioThreadFunc(void *param)
{
    CAVInputStream *inputStream = static_cast<CAVInputStream *>(param);
    inputStream->readSysToMixAudioPacket();
    return nullptr;
}

//ReadAudioPackets + ReadVideoPackets
int CAVInputStream::readSysAudioPacket()
{
    if (!m_bSysAudio)
        return 1;
    int got_frame_ptr = 0;
    //start decode and encode
    while (bRunThread()) {
        int ret;
        AVFrame *input_frame = avlibInterface::m_av_frame_alloc();
        if (!input_frame) {
            return AVERROR(ENOMEM);
        }
        AVPacket inputPacket;
        avlibInterface::m_av_init_packet(&inputPacket);
        inputPacket.data = nullptr;
        inputPacket.size = 0;
        /** Read one audio frame from the input file into a temporary packet. */
        if ((ret = avlibInterface::m_av_read_frame(m_pSysAudioFormatContext, &inputPacket)) < 0) {
            /** If we are at the end of the file, flush the decoder below. */
            if (ret == AVERROR_EOF) {
                setbRunThread(false);
            } else {
                continue;
            }
        }
        if ((ret = avlibInterface::m_avcodec_decode_audio4(m_pSysAudioFormatContext->streams[m_sysAudioindex]->codec, input_frame, &got_frame_ptr, &inputPacket)) < 0) {
            printf("Could not decode audio frame\n");
            return ret;
        }
        avlibInterface::m_av_packet_unref(&inputPacket);
        /** If there is decoded data, convert and store it */
        if (got_frame_ptr) {
            m_context->m_recordAdmin->m_pOutputStream->writeSysAudioFrame(m_pSysAudioFormatContext->streams[m_sysAudioindex], input_frame, avlibInterface::m_av_gettime() - m_start_time);
        }
        avlibInterface::m_av_frame_free(&input_frame);
    }// -- while end
    if (nullptr != m_pSysAudioFormatContext) {
        avlibInterface::m_avformat_close_input(&m_pSysAudioFormatContext);
    }
    if (nullptr != m_pSysAudioFormatContext) {
        avlibInterface::m_avformat_free_context(m_pSysAudioFormatContext);
        m_pSysAudioFormatContext = nullptr;
    }
    m_sysAudioindex = -1;
    return 0;
}

int CAVInputStream::readSysToMixAudioPacket()
{
    int got_frame_ptr = 0;
    while (bRunThread()) {
        int ret;
        AVFrame *inputFrame = avlibInterface::m_av_frame_alloc();
        if (!inputFrame) {
            return AVERROR(ENOMEM);
        }
        AVPacket inputPacket;
        avlibInterface::m_av_init_packet(&inputPacket);
        inputPacket.data = nullptr;
        inputPacket.size = 0;
        /** Read one audio frame from the input file into a temporary packet.将一个音频帧从输入文件插入一个临时包 */
        if ((ret = avlibInterface::m_av_read_frame(m_pSysAudioFormatContext, &inputPacket)) < 0) {
            /** If we are at the end of the file, flush the decoder below. */
            if (ret == AVERROR_EOF) {
                setbRunThread(false);
            } else {
                continue;
            }
        }
        if ((ret = avlibInterface::m_avcodec_decode_audio4(m_pSysAudioFormatContext->streams[m_sysAudioindex]->codec, inputFrame, &got_frame_ptr, &inputPacket)) < 0) {
            printf("Could not decode audio frame\n");
            return ret;
        }
        avlibInterface::m_av_packet_unref(&inputPacket);
        if (got_frame_ptr) {
            m_context->m_recordAdmin->m_pOutputStream->writeSysToMixAudioFrame(m_pSysAudioFormatContext->streams[m_sysAudioindex], inputFrame, avlibInterface::m_av_gettime() - m_start_time);
        }
        avlibInterface::m_av_frame_free(&inputFrame);
    }// -- while end
    if (nullptr != m_pSysAudioFormatContext) {
        avlibInterface::m_avformat_close_input(&m_pSysAudioFormatContext);
    }
    if (nullptr != m_pSysAudioFormatContext) {
        avlibInterface::m_avformat_free_context(m_pSysAudioFormatContext);
        m_pSysAudioFormatContext = nullptr;
    }
    m_sysAudioindex = -1;
    return 0;
}

bool CAVInputStream::GetVideoInputInfo(int &width, int &height, int &frame_rate, AVPixelFormat &pixFmt)
{
    width = m_screenDW - m_left - m_right;
    height = m_screenDH - m_top - m_bottom;
    pixFmt = m_ipix_fmt;
    frame_rate = m_fps;
    return true;
}

bool  CAVInputStream::GetAudioInputInfo(AVSampleFormat &sample_fmt, int &sample_rate, int &channels, int &layout)
{
    if (m_micAudioindex != -1) {
        sample_fmt = m_pMicAudioFormatContext->streams[m_micAudioindex]->codec->sample_fmt;
        sample_rate = m_pMicAudioFormatContext->streams[m_micAudioindex]->codec->sample_rate;
        channels = m_pMicAudioFormatContext->streams[m_micAudioindex]->codec->channels;
        layout = static_cast<int>(m_pMicAudioFormatContext->streams[m_micAudioindex]->codec->channel_layout);
        return true;
    }

    return false;
}
bool  CAVInputStream::GetAudioSCardInputInfo(AVSampleFormat &sample_fmt, int &sample_rate, int &channels, int &layout)
{
    if (m_sysAudioindex != -1) {
        sample_fmt = m_pSysAudioFormatContext->streams[m_sysAudioindex]->codec->sample_fmt;
        sample_rate = m_pSysAudioFormatContext->streams[m_sysAudioindex]->codec->sample_rate;
        channels = m_pSysAudioFormatContext->streams[m_sysAudioindex]->codec->channels;
        layout = static_cast<int>(m_pSysAudioFormatContext->streams[m_sysAudioindex]->codec->channel_layout);
        return true;
    }
    return false;
}

QString CAVInputStream::currentAudioChannel()
{
    QStringList options;
    options << QString(QStringLiteral("-c"));
    options << QString(QStringLiteral("pacmd list-sources | grep -PB 1 'analog.*monitor>' | head -n 1 | perl -pe 's/.* //g'"));
    QProcess process;
    process.start(QString(QStringLiteral("bash")), options);
    process.waitForFinished();
    process.waitForReadyRead();
    QByteArray tempArray =  process.readAllStandardOutput();
    char *charTemp = tempArray.data();
    QString str_output = QString(QLatin1String(charTemp));
    process.close();
    return str_output;
}
