/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "libvpxencoder_p.h"

#include <QSize>
#include <QThread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/pixfmt.h>
}

#include "logging_record.h"

LibVpxEncoder::LibVpxEncoder(PipeWireProduce *produce)
    : SoftwareEncoder(produce)
{
}

bool LibVpxEncoder::initialize(const QSize &size)
{
    createFilterGraph(size);

    auto codec = avcodec_find_encoder_by_name("libvpx");
    if (!codec) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "libvpx codec not found";
        return false;
    }

    m_avCodecContext = avcodec_alloc_context3(codec);
    if (!m_avCodecContext) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Could not allocate video codec context";
        return false;
    }
    m_avCodecContext->bit_rate = size.width() * size.height() * 2;

    Q_ASSERT(!size.isEmpty());
    m_avCodecContext->width = size.width();
    m_avCodecContext->height = size.height();
    m_avCodecContext->max_b_frames = 0;
    m_avCodecContext->gop_size = 100;
    m_avCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    m_avCodecContext->time_base = AVRational{1, 1000};
    m_avCodecContext->global_quality = 35;

    if (m_quality) {
        m_avCodecContext->global_quality = percentageToAbsoluteQuality(m_quality);
    } else {
        m_avCodecContext->global_quality = 35;
    }

    AVDictionary *options = nullptr;
    applyEncodingPreference(options);

    if (int result = avcodec_open2(m_avCodecContext, codec, &options); result < 0) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Could not open codec" << av_err2str(result);
        return false;
    }

    return true;
}

int LibVpxEncoder::percentageToAbsoluteQuality(const std::optional<quint8> &quality)
{
    if (!quality) {
        return -1;
    }

    constexpr int MinQuality = 63;
    return std::max(1, int(MinQuality - (m_quality.value() / 100.0) * MinQuality));
}

void LibVpxEncoder::applyEncodingPreference(AVDictionary *options)
{
    av_dict_set_int(&options, "threads", qMin(16, QThread::idealThreadCount()), 0);
    av_dict_set(&options, "preset", "veryfast", 0);
    av_dict_set(&options, "tune-content", "screen", 0);
    av_dict_set(&options, "deadline", "realtime", 0);
    // In theory a lower number should be faster, but the opposite seems to be true
    // av_dict_set(&options, "quality", "40", 0);
    // Disable motion estimation, not great while dragging windows but speeds up encoding by an order of magnitude
    av_dict_set(&options, "flags", "+mv4", 0);
    // Disable in-loop filtering
    av_dict_set(&options, "-flags", "+loop", 0);
    av_dict_set(&options, "crf", "45", 0);
}
