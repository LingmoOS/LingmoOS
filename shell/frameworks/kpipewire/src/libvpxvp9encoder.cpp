/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2023 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "libvpxvp9encoder_p.h"

#include "pipewireproduce_p.h"

#include <QSize>
#include <QThread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/pixfmt.h>
}

#include "logging_record.h"

LibVpxVp9Encoder::LibVpxVp9Encoder(PipeWireProduce *produce)
    : SoftwareEncoder(produce)
{
}

bool LibVpxVp9Encoder::initialize(const QSize &size)
{
    createFilterGraph(size);

    auto codec = avcodec_find_encoder_by_name("libvpx-vp9");
    if (!codec) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "libvpx-vp9 codec not found";
        return false;
    }

    m_avCodecContext = avcodec_alloc_context3(codec);
    if (!m_avCodecContext) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Could not allocate video codec context";
        return false;
    }

    Q_ASSERT(!size.isEmpty());
    m_avCodecContext->width = size.width();
    m_avCodecContext->height = size.height();
    m_avCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    m_avCodecContext->time_base = AVRational{1, 1000};

    AVDictionary *options = nullptr;

    applyEncodingPreference(options);

    const auto area = size.width() * size.height();
    // m_avCodecContext->framerate is not set, so we use m_produce->maxFramerate() instead.
    const auto maxFramerate = m_produce->maxFramerate();
    const auto fps = qreal(maxFramerate.numerator) / std::max(quint32(1), maxFramerate.denominator);

    m_avCodecContext->gop_size = fps * 2;

    // TODO: Make bitrate depend on the framerate? More frames is more data.
    // maxFramerate can apparently be changed while recording, so keep that in mind.
    m_avCodecContext->bit_rate = std::round(area * 2);
    m_avCodecContext->rc_min_rate = std::round(area);
    m_avCodecContext->rc_max_rate = std::round(area * 3);

    m_avCodecContext->rc_buffer_size = m_avCodecContext->bit_rate;

    m_avCodecContext->thread_count = QThread::idealThreadCount();

    if (int result = avcodec_open2(m_avCodecContext, codec, &options); result < 0) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Could not open codec" << av_err2str(result);
        return false;
    }

    return true;
}

int LibVpxVp9Encoder::percentageToAbsoluteQuality(const std::optional<quint8> &quality)
{
    if (!quality) {
        return -1;
    }

    constexpr int MinQuality = 63;
    return std::max(1, int(MinQuality - (m_quality.value() / 100.0) * MinQuality));
}

void LibVpxVp9Encoder::applyEncodingPreference(AVDictionary *options)
{
    // We're probably capturing a screen
    av_dict_set(&options, "tune-content", "screen", 0);

    // Lower crf is higher quality. Max 0, min 63. libvpx-vp9 doesn't use global_quality.
    int crf = 31;
    if (m_quality) {
        crf = percentageToAbsoluteQuality(m_quality);
    }
    m_avCodecContext->qmin = std::clamp(crf / 2, 0, crf);
    m_avCodecContext->qmax = std::clamp(qRound(crf * 1.5), crf, 63);
    av_dict_set_int(&options, "crf", crf, 0);

    // 0-4 are for Video-On-Demand with the good or best deadline.
    // Don't use best, it's not worth it.
    // 5-8 are for streaming with the realtime deadline.
    // Lower is higher quality.
    int cpuUsed = 5 + std::max(1, int(3 - std::round(m_quality.value_or(50) / 100.0 * 3)));
    av_dict_set_int(&options, "cpu-used", cpuUsed, 0);
    av_dict_set(&options, "deadline", "realtime", 0);

    // The value is interpreted as being equivalent to log2(realNumberOfColumns),
    // so 3 is 8 columns. 6 is the max amount of columns. 2 is the max amount of rows.
    av_dict_set(&options, "tile-columns", "6", 0);
    av_dict_set(&options, "tile-rows", "2", 0);

    // This should make things faster, but it only seems to consume 100MB more RAM.
    // av_dict_set(&options, "row-mt", "1", 0);
    av_dict_set(&options, "frame-parallel", "1", 0);
}
