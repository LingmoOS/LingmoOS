// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _DMR_FFMPEG_PROBE
#define _DMR_FFMPEG_PROBE

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/hwcontext.h>
}

#include <QtCore>

#define RESULT_CONTINUE(n) {if(n) continue;}

namespace dmr {


    typedef int (*ffmAvHwdeviceCtxCreate)(AVBufferRef **device_ctx, enum AVHWDeviceType type,
                                           const char *device, AVDictionary *opts, int flags);

    typedef enum AVHWDeviceType (*ffmAvHwdeviceIterateTypes)(enum AVHWDeviceType prev);

    typedef const char* (*ffmAvHwdeviceGetTypeName)(enum AVHWDeviceType type);

    typedef int (*ffmAvformatOpenInput)(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options);

    typedef int (*ffmAvformatFindStreamInfo)(AVFormatContext *ic, AVDictionary **options);

    typedef AVCodec * (*ffmAvcodecFindDecoder)(enum AVCodecID id);

    typedef AVCodecParserContext *(*ffmAvParserInit)(int codec_id);

    typedef const AVCodecHWConfig *(*ffmAvcodecGetHwConfig)(const AVCodec *codec, int index);

    typedef AVCodecContext *(*ffmAvcodecAllocContext3)(const AVCodec *codec);

    typedef int (*ffmAvcodecParametersToContext)(AVCodecContext *codec, const AVCodecParameters *par);

    typedef void (*ffmAvformatCloseInput)(AVFormatContext **s);

    typedef int (*ffmAvcodecOpen2)(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);

    typedef int (*ffmAvReadFrame)(AVFormatContext *s, AVPacket *pkt);

    typedef AVFrame *(*ffmAvFrameAlloc)(void);

    typedef int (*ffmAvcodecSendPacket)(AVCodecContext *avctx, const AVPacket *avpkt);

    typedef int (*ffmAvHwframeTransferData)(AVFrame *dst, const AVFrame *src, int flags);

    typedef int (*ffmAvImageGetBufferSize)(enum AVPixelFormat pix_fmt, int width, int height, int align);

    typedef void (*ffmAvFrameFree)(AVFrame **frame);

    typedef int (*ffmAvImageCopyToBuffer)(uint8_t *dst, int dst_size,
                                const uint8_t * const src_data[4], const int src_linesize[4],
                                enum AVPixelFormat pix_fmt, int width, int height, int align);

    typedef int (*ffmAvcodecReceiveFrame)(AVCodecContext *avctx, AVFrame *frame);

    typedef AVBufferRef *(*ffmAvBufferRef)(AVBufferRef *buf);

    typedef void *(*ffmAvMalloc)(size_t size);

    typedef void (*ffmAvcodecFreeContext)(AVCodecContext **avctx);

    typedef int (*ffmAvcodecClose)(AVCodecContext *avctx);

    typedef void (*ffmAvBufferUnref)(AVBufferRef **buf);

    /**
     * @file 用于硬解探测的单例类
     */
    class HwdecProbe
    {
    public:
        /**
         * @brief 获取对象指针
         * @return 对象指针
         */
        static HwdecProbe& get();

        /**
         * @brief 判断文件是否可以用硬解播放
         * @param url 文件路径
         * @param out hwList 可以使用的硬解名字List
         * @return 是返回true,否则返回false
         */
        bool isFileCanHwdec(const QUrl& url, QList<QString>& hwList);
    private:
        HwdecProbe();

        /**
         * @brief 初始化接口
         */
        void initffmpegInterface();

        /**
         * @brief 初始化硬解
         * @param ctx 解码器上下文
         * @param type 解码类型
         * @return 成功0 失败小于0
         */
        int hwDecoderInit(AVCodecContext *ctx, int type);

        /**
         * @brief 获取硬解的所有类型
         */
        void getHwTypes();

        /**
         * @brief 解码器是否是某种类型的硬解码
         * @param pDec 解码器
         * @param type 解码类型
         * @return 是true 否false
         */
        bool isTypeHaveHwdec(const AVCodec *pDec, AVHWDeviceType type);

    private:
        //单例指针
        static HwdecProbe             m_ffmpegProbe;
        //硬解设备上下文
        AVBufferRef                     *m_hwDeviceCtx;
        //所有硬解类型
        QList<AVHWDeviceType>           m_hwTypeList;
        // av_hwdevice_ctx_create 函数指针
        ffmAvHwdeviceCtxCreate          m_avHwdeviceCtxCreate;
        // av_hwdevice_iterate_types 函数指针
        ffmAvHwdeviceIterateTypes       m_avHwdeviceIterateTypes;
        //av_hwdevice_get_type_name 函数指针
        ffmAvHwdeviceGetTypeName        m_avHwdeviceGetTypeName;
        // avformat_open_input 函数指针
        ffmAvformatOpenInput            m_avformatOpenInput;
        // avformat_find_stream_info 函数指针
        ffmAvformatFindStreamInfo       m_avformatFindStreamInfo;
        // avcodec_find_decoder 函数指针
        ffmAvcodecFindDecoder           m_avcodecFindDecoder;
        // av_parser_init 函数指针
        ffmAvParserInit                 m_avParserInit;
        // avcodec_get_hw_config 函数指针
        ffmAvcodecGetHwConfig           m_avcodecGetHwConfig;
        // avcodec_alloc_context3 函数指针
        ffmAvcodecAllocContext3         m_avcodecAllocContext3;
        // avcodec_parameters_to_context 函数指针
        ffmAvcodecParametersToContext   m_avcodecParametersToContext;
        // avformat_close_input 函数指针
        ffmAvformatCloseInput           m_avformatCloseInput;
        // avcodec_open2 函数指针
        ffmAvcodecOpen2                 m_avcodecOpen2;
        // av_read_frame 函数指针
        ffmAvReadFrame                  m_avReadFrame;
        // av_frame_alloc 函数指针
        ffmAvFrameAlloc                 m_avFrameAlloc;
        // avcodec_send_packet 函数指针
        ffmAvcodecSendPacket            m_avcodecSendPacket;
        // av_hwframe_transfer_data 函数指针
        ffmAvHwframeTransferData        m_avHwframeTransferData;
        // av_image_get_buffer_size; 函数指针
        ffmAvImageGetBufferSize         m_avImageGetBufferSize;
        // av_frame_free 函数指针
        ffmAvFrameFree                  m_avFrameFree;
        // av_image_copy_to_buffer 函数指针
        ffmAvImageCopyToBuffer          m_avImageCopyToBuffer;
        // m_avcodec_receive_frame; 函数指针
        ffmAvcodecReceiveFrame          m_avcodecReceiveFrame;
        // v_buffer_ref 函数指针
        ffmAvBufferRef                  m_avBufferRef;
        // av_malloc 函数指针
        ffmAvMalloc                     m_avMalloc;
        // avcodec_free_context 函数指针
        ffmAvcodecFreeContext           m_avcodecFreeContext;
        // avcodec_close 函数指针
        ffmAvcodecClose                 m_avcodecClose;
        // av_buffer_unref 函数指针
        ffmAvBufferUnref                m_avBufferUnref;
    };
}

#endif
