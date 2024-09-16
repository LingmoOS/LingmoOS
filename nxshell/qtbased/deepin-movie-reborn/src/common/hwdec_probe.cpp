// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hwdec_probe.h"
#include "compositing_manager.h"
namespace dmr {

HwdecProbe HwdecProbe::m_ffmpegProbe;

HwdecProbe::HwdecProbe():m_hwDeviceCtx(nullptr)
{
//    m_ffmpegProbe.initffmpegInterface();
//    m_ffmpegProbe.getHwTypes();
}

HwdecProbe& HwdecProbe::get()
{


    return m_ffmpegProbe;
}

bool HwdecProbe::isFileCanHwdec(const QUrl& url, QList<QString>& hwList)
{
    hwList.clear();
    AVFormatContext *input_ctx = nullptr;
    int ret = 0;

    // open the input file
    if (m_avformatOpenInput(&input_ctx, url.toString().toStdString().c_str(), nullptr, nullptr) != 0) {// Cannot open input file

        return false;
    }

    if (m_avformatFindStreamInfo(input_ctx, nullptr) < 0) { // Cannot find input stream information
        m_avformatCloseInput(&input_ctx);
        return false;
    }

    for (AVHWDeviceType type : m_hwTypeList) {
        for (size_t i = 0; i < input_ctx->nb_streams; i++) {

            AVStream *stream = input_ctx->streams[i];
            AVCodec *dec = m_avcodecFindDecoder(stream->codecpar->codec_id);
            RESULT_CONTINUE((nullptr == dec))

            RESULT_CONTINUE(!isTypeHaveHwdec(dec, type))

            AVCodecContext *codec_ctx = nullptr;
            codec_ctx = m_avcodecAllocContext3(dec);
            RESULT_CONTINUE((nullptr == codec_ctx)) // Failed to allocate the decoder context for stream

            ret = m_avcodecParametersToContext(codec_ctx, stream->codecpar);
            RESULT_CONTINUE((ret < 0)) // Failed to copy decoder parameters to input decoder context for stream

            ret = hwDecoderInit(codec_ctx, type);
            RESULT_CONTINUE((ret < 0))

            if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                // Open decoder. we think it can decodec when oepn decoder success
                ret = m_avcodecOpen2(codec_ctx, dec, nullptr);
                RESULT_CONTINUE((ret < 0))

                hwList.push_back(m_avHwdeviceGetTypeName(type));
                m_avcodecClose(codec_ctx);
            }
            m_avcodecFreeContext(&codec_ctx);
        }
    }

    free(input_ctx->streams);
    input_ctx->streams = nullptr;
    input_ctx->nb_streams = 0;
    m_avformatCloseInput(&input_ctx);

    if(nullptr != m_hwDeviceCtx)
        m_avBufferUnref(&m_hwDeviceCtx);

    return hwList.size() > 0;
}

static QString libPath(const QString &sLib)
{
    QDir dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (sLib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib
    if (list.contains(sLib)) {
        return sLib;
    } else {
        list.sort();
    }

    Q_ASSERT(list.size() > 0);
    return list.last();
}

void HwdecProbe::initffmpegInterface()
{
    QLibrary avcodecLibrary(CompositingManager::libPath("libavcodec.so"));
    QLibrary avformatLibrary(CompositingManager::libPath("libavformat.so"));
    QLibrary avutilLibrary(CompositingManager::libPath("libavutil.so"));

    m_avHwdeviceCtxCreate  = reinterpret_cast<ffmAvHwdeviceCtxCreate>(avutilLibrary.resolve("av_hwdevice_ctx_create"));
    m_avHwdeviceIterateTypes = reinterpret_cast<ffmAvHwdeviceIterateTypes>(avutilLibrary.resolve("av_hwdevice_iterate_types"));
    m_avHwdeviceGetTypeName = reinterpret_cast<ffmAvHwdeviceGetTypeName>(avutilLibrary.resolve("av_hwdevice_get_type_name"));
    m_avformatOpenInput = reinterpret_cast<ffmAvformatOpenInput>(avformatLibrary.resolve("avformat_open_input"));
    m_avformatFindStreamInfo = reinterpret_cast<ffmAvformatFindStreamInfo>(avformatLibrary.resolve("avformat_find_stream_info"));
    m_avcodecFindDecoder = reinterpret_cast<ffmAvcodecFindDecoder>(avcodecLibrary.resolve("avcodec_find_decoder"));
    m_avParserInit = reinterpret_cast<ffmAvParserInit>(avcodecLibrary.resolve("av_parser_init"));
    m_avcodecGetHwConfig = reinterpret_cast<ffmAvcodecGetHwConfig>(avcodecLibrary.resolve("avcodec_get_hw_config"));
    m_avcodecAllocContext3 = reinterpret_cast<ffmAvcodecAllocContext3>(avcodecLibrary.resolve("avcodec_alloc_context3"));
    m_avcodecParametersToContext = reinterpret_cast<ffmAvcodecParametersToContext>(avcodecLibrary.resolve("avcodec_parameters_to_context"));
    m_avformatCloseInput = reinterpret_cast<ffmAvformatCloseInput>(avformatLibrary.resolve("avformat_close_input"));
    m_avcodecOpen2 = reinterpret_cast<ffmAvcodecOpen2>(avcodecLibrary.resolve("avcodec_open2"));
    m_avReadFrame = reinterpret_cast<ffmAvReadFrame>(avformatLibrary.resolve("av_read_frame"));
    m_avFrameAlloc = reinterpret_cast<ffmAvFrameAlloc>(avutilLibrary.resolve("av_frame_alloc"));
    m_avcodecSendPacket = reinterpret_cast<ffmAvcodecSendPacket>(avcodecLibrary.resolve("avcodec_send_packet"));
    m_avHwframeTransferData = reinterpret_cast<ffmAvHwframeTransferData>(avutilLibrary.resolve("av_hwframe_transfer_data"));
    m_avImageGetBufferSize = reinterpret_cast<ffmAvImageGetBufferSize>(avutilLibrary.resolve("av_image_get_buffer_size"));
    m_avFrameFree = reinterpret_cast<ffmAvFrameFree>(avutilLibrary.resolve("av_frame_free"));
    m_avImageCopyToBuffer = reinterpret_cast<ffmAvImageCopyToBuffer>(avutilLibrary.resolve("av_image_copy_to_buffer"));
    m_avcodecReceiveFrame = reinterpret_cast<ffmAvcodecReceiveFrame>(avcodecLibrary.resolve("avcodec_receive_frame"));
    m_avBufferRef = reinterpret_cast<ffmAvBufferRef>(avutilLibrary.resolve("av_buffer_ref"));
    m_avMalloc = reinterpret_cast<ffmAvMalloc>(avutilLibrary.resolve("av_malloc"));
    m_avcodecFreeContext = reinterpret_cast<ffmAvcodecFreeContext>(avcodecLibrary.resolve("avcodec_free_context"));
    m_avcodecClose = reinterpret_cast<ffmAvcodecClose>(avcodecLibrary.resolve("avcodec_close"));
    m_avBufferUnref = reinterpret_cast<ffmAvBufferUnref>(avutilLibrary.resolve("av_buffer_unref"));
}

void HwdecProbe::getHwTypes()
{
    m_hwTypeList.clear();
    AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
    // find hwdevies
    while ((type = m_avHwdeviceIterateTypes(type)) != AV_HWDEVICE_TYPE_NONE) {
        m_hwTypeList.append(type);
    }

}

int HwdecProbe::hwDecoderInit(AVCodecContext *ctx, const int type)
{
    int err = 0;
    if(nullptr != m_hwDeviceCtx)
        m_avBufferUnref(&m_hwDeviceCtx);

    if ((err = m_avHwdeviceCtxCreate(&m_hwDeviceCtx, static_cast<AVHWDeviceType>(type),
                                      nullptr, nullptr, 0)) < 0) {
        fprintf(stderr, "Failed to create specified HW device.\n");
        return err;
    }
    ctx->hw_device_ctx = m_avBufferRef(m_hwDeviceCtx);

    return err;
}

bool HwdecProbe::isTypeHaveHwdec(const AVCodec *pDec, AVHWDeviceType type)
{
    bool rs = true;
    //is have tmpType hwdec config
    for (int j = 0;; j++) {
        const AVCodecHWConfig *config = m_avcodecGetHwConfig(pDec, j);
        if (nullptr == config) {
            rs = false;
            break;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                config->device_type == type) {
            break;
        }
    }

    return rs;
}
}
