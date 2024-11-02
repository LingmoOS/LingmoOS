/*
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <me@vhanda.in>

    Code adapted from Strigi FFmpeg Analyzer -
    SPDX-FileCopyrightText: 2010 Evgeny Egorochkin <phreedom.stdin@gmail.com>
    SPDX-FileCopyrightText: 2011 Tirtha Chatterjee <tirtha.p.chatterjee@gmail.com>
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "ffmpeg-extractor.h"
#include "thumbnail-utils.h"

#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
# include <stdint.h>
#endif

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}
#include <QDebug>
#include <QImage>

using namespace LingmoUIFileMetadata;

static const QString VERSION = "1.0";
static const QString PLUGIN_NAME = "FFmpeg";

FFmpegExtractor::FFmpegExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{
}
const QStringList supportedMimeTypes = {
    QStringLiteral("video/mp4"),
    QStringLiteral("video/mpeg"),
    QStringLiteral("video/quicktime"),
    QStringLiteral("video/webm"),
    QStringLiteral("video/ogg"),
    QStringLiteral("video/mp2t"),
    QStringLiteral("video/x-flv"),
    QStringLiteral("video/x-matroska"),
    QStringLiteral("video/x-ms-wmv"),
    QStringLiteral("video/x-ms-asf"),
    QStringLiteral("video/x-msvideo"),
};

QStringList FFmpegExtractor::mimetypes() const
{
    return supportedMimeTypes;
}

void FFmpegExtractor::extract(ExtractionResult* result)
{
    AVFormatContext* fmt_ctx = nullptr;

#if LIBAVFORMAT_VERSION_MAJOR < 58
    av_register_all();
#endif

    QByteArray arr = result->inputUrl().toUtf8();

    fmt_ctx = avformat_alloc_context();
    if (int ret = avformat_open_input(&fmt_ctx, arr.data(), nullptr, nullptr)) {
        qWarning() << "avformat_open_input error: " << ret;
        return;
    }

    int ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        qWarning() << "avform_find_stream_info error: " << ret;
        return;
    }

    result->addType(Type::Video);

    if (result->inputFlags() & ExtractionResult::ExtractThumbnail) {
        if (ThumbnailUtils::needGenerateThumbnail(result, PLUGIN_NAME, VERSION)) {
            bool failed(true);
            //寻找视频流
            int videoStreamIndex = av_find_default_stream_index(fmt_ctx);
            if (videoStreamIndex >= 0) {
                AVCodecParameters *codec_par = fmt_ctx->streams[videoStreamIndex]->codecpar;
                if (codec_par->codec_type == AVMEDIA_TYPE_VIDEO) {
                    //寻找解码器
                    const AVCodec* codec = avcodec_find_decoder(codec_par->codec_id);
                    if (codec) {
                        //打开编码器上下文
                        AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
                        if (avcodec_parameters_to_context(codec_ctx, codec_par) >= 0) {
                            //打开解码器
                            if (avcodec_open2(codec_ctx, codec, nullptr) >= 0) {
                                AVPacket* packet = av_packet_alloc();
                                int64_t targetTime = 0;
                                int64_t timestamp = targetTime * AV_TIME_BASE;
                                av_seek_frame(fmt_ctx, -1, timestamp, AVSEEK_FLAG_BACKWARD);
                                //读取帧数据
                                while (av_read_frame(fmt_ctx, packet) == 0) {
                                    if (packet->stream_index == videoStreamIndex) {
                                        //发送数据包到解码器
                                        avcodec_send_packet(codec_ctx, packet);
                                        AVFrame* frame = av_frame_alloc();
                                        if (avcodec_receive_frame(codec_ctx, frame) == 0) {
                                            if (frame->key_frame && frame->pts >= timestamp) {
                                                //将帧数据转换成图片格式
                                                SwsContext* sws_ctx = sws_getContext(frame->width, frame->height, codec_ctx->pix_fmt,
                                                                                     frame->width, frame->height, AV_PIX_FMT_RGB24,
                                                                                     SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
                                                AVFrame* rgbFrame = av_frame_alloc();
                                                av_image_alloc(rgbFrame->data, rgbFrame->linesize, frame->width, frame->height, AV_PIX_FMT_RGB24, 1);
                                                sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, rgbFrame->data, rgbFrame->linesize);
                                                //使用frame数据构造缩略图
                                                QImage thumbnail(rgbFrame->data[0], frame->width, frame->height, rgbFrame->linesize[0], QImage::Format_RGB888);
                                                ThumbnailUtils::setThumbnail(result, thumbnail, PLUGIN_NAME, VERSION);
                                                failed = false;
                                                sws_freeContext(sws_ctx);
                                                av_frame_free(&rgbFrame);
                                                break;
                                            }
                                        }
                                        av_frame_unref(frame);
                                        av_frame_free(&frame);
                                    }
                                }
                                av_packet_free(&packet);
                            }
                        }
                        avcodec_free_context(&codec_ctx);
                    }
                }
            }
            if (failed) {
                ThumbnailUtils::handleFailureOfThumbnail(result->inputUrl(), PLUGIN_NAME, VERSION);
            }
        }
    }


    if (result->inputFlags() & ExtractionResult::ExtractMetaData) {
        int totalSecs = fmt_ctx->duration / AV_TIME_BASE;
        int bitrate = fmt_ctx->bit_rate;

        result->add(Property::Duration, totalSecs);
        result->add(Property::BitRate, bitrate);

        const int index_stream = av_find_default_stream_index(fmt_ctx);
        if (index_stream >= 0) {
            AVStream* stream = fmt_ctx->streams[index_stream];

            const AVCodecParameters* codec = stream->codecpar;

            if (codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                result->add(Property::Width, codec->width);
                result->add(Property::Height, codec->height);

                AVRational avSampleAspectRatio = av_guess_sample_aspect_ratio(fmt_ctx, stream, nullptr);
                AVRational avDisplayAspectRatio;
                av_reduce(&avDisplayAspectRatio.num, &avDisplayAspectRatio.den,
                          codec->width  * avSampleAspectRatio.num,
                          codec->height * avSampleAspectRatio.den,
                          1024*1024);
                double displayAspectRatio = avDisplayAspectRatio.num;
                if (avDisplayAspectRatio.den) {
                    displayAspectRatio /= avDisplayAspectRatio.den;
                }
                if (displayAspectRatio) {
                    result->add(Property::AspectRatio, displayAspectRatio);
                }

                AVRational avFrameRate = av_guess_frame_rate(fmt_ctx, stream, nullptr);
                double frameRate = avFrameRate.num;
                if (avFrameRate.den) {
                    frameRate /= avFrameRate.den;
                }
                if (frameRate) {
                    result->add(Property::FrameRate, frameRate);
                }
            }
        }

        AVDictionary* dict = fmt_ctx->metadata;
        AVDictionaryEntry* entry;

        entry = av_dict_get(dict, "title", nullptr, 0);
        if (entry) {
            result->add(Property::Title, QString::fromUtf8(entry->value));
        }


        entry = av_dict_get(dict, "author", nullptr, 0);
        if (entry) {
            result->add(Property::Author, QString::fromUtf8(entry->value));
        }

        entry = av_dict_get(dict, "copyright", nullptr, 0);
        if (entry) {
            result->add(Property::Copyright, QString::fromUtf8(entry->value));
        }

        entry = av_dict_get(dict, "comment", nullptr, 0);
        if (entry) {
            result->add(Property::Comment, QString::fromUtf8(entry->value));
        }

        entry = av_dict_get(dict, "album", nullptr, 0);
        if (entry) {
            result->add(Property::Album, QString::fromUtf8(entry->value));
        }

        entry = av_dict_get(dict, "genre", nullptr, 0);
        if (entry) {
            result->add(Property::Genre, QString::fromUtf8(entry->value));
        }

        entry = av_dict_get(dict, "track", nullptr, 0);
        if (entry) {
            QString value = QString::fromUtf8(entry->value);

            bool ok = false;
            int track = value.toInt(&ok);
            if (ok && track) {
                result->add(Property::TrackNumber, track);
            }
        }

        entry = av_dict_get(dict, "year", nullptr, 0);
        if (entry) {
            int year = QString::fromUtf8(entry->value).toInt();
            result->add(Property::ReleaseYear, year);
        }
    }

    avformat_close_input(&fmt_ctx);
}
