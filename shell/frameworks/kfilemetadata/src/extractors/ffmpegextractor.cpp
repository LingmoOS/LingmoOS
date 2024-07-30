/*
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <me@vhanda.in>

    Code adapted from Strigi FFmpeg Analyzer -
    SPDX-FileCopyrightText: 2010 Evgeny Egorochkin <phreedom.stdin@gmail.com>
    SPDX-FileCopyrightText: 2011 Tirtha Chatterjee <tirtha.p.chatterjee@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "ffmpegextractor.h"
#include "kfilemetadata_debug.h"

#include "config-kfilemetadata.h"

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
#include <libavcodec/avcodec.h>
}

using namespace KFileMetaData;

FFmpegExtractor::FFmpegExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{
}

const QStringList supportedMimeTypes = {
    QStringLiteral("video/mp2t"),
    QStringLiteral("video/mp4"),
    QStringLiteral("video/mpeg"),
    QStringLiteral("video/ogg"),
    QStringLiteral("video/quicktime"),
    QStringLiteral("video/vnd.avi"),
    QStringLiteral("video/webm"),
    QStringLiteral("video/x-flv"),
    QStringLiteral("video/x-matroska"),
    QStringLiteral("video/x-ms-asf"),
    QStringLiteral("video/x-ms-wmv"),
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
        qCWarning(KFILEMETADATA_LOG) << "avformat_open_input error: " << ret;
        return;
    }

    int ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        qCWarning(KFILEMETADATA_LOG) << "avform_find_stream_info error: " << ret;
        return;
    }

    result->addType(Type::Video);

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
        // In Ogg, the internal comment metadata headers are attached to a single content stream.
        // By convention, it's the first logical bitstream occuring.
        if (!dict && fmt_ctx->nb_streams > 0) {
            dict = fmt_ctx->streams[0]->metadata;
        }

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

#include "moc_ffmpegextractor.cpp"
