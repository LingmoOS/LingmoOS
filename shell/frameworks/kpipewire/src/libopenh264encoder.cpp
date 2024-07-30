/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2024 Fabian Vogt <fabian@ritter-vogt.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "libopenh264encoder_p.h"

#include <QSize>
#include <QThread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/pixfmt.h>
}

#include "logging_record.h"

LibOpenH264Encoder::LibOpenH264Encoder(H264Profile profile, PipeWireProduce *produce)
    : SoftwareEncoder(produce)
    , m_profile(profile)
{
}

bool LibOpenH264Encoder::initialize(const QSize &size)
{
    createFilterGraph(size);

    auto codec = avcodec_find_encoder_by_name("libopenh264");
    if (!codec) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "libopenh264 codec not found";
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
    m_avCodecContext->max_b_frames = 0;
    m_avCodecContext->gop_size = 100;
    m_avCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    m_avCodecContext->time_base = AVRational{1, 1000};

    if (m_quality) {
        // "q" here stands for "quantization", but that effectively impacts quality.
        m_avCodecContext->qmin = m_avCodecContext->qmax = percentageToAbsoluteQuality(m_quality);
    }

    switch (m_profile) {
    case H264Profile::Baseline:
        // libopenh264 only does constrained baseline.
        // There's a bug in the ffmpeg -> openh264 interface though:
        // ffmpeg expects CONSTRAINED_BASELINE from the application and
        // passes that through, but libopenh264 only allows BASELINE.
        // Until that bug is fixed there'll always be a warning that the
        // profile is not supported (https://github.com/cisco/openh264/issues/3613)
        m_avCodecContext->profile = FF_PROFILE_H264_CONSTRAINED_BASELINE;
        break;
    case H264Profile::Main:
        m_avCodecContext->profile = FF_PROFILE_H264_MAIN;
        break;
    case H264Profile::High:
        m_avCodecContext->profile = FF_PROFILE_H264_HIGH;
        break;
    }

    AVDictionary *options = nullptr;
    av_dict_set_int(&options, "threads", qMin(16, QThread::idealThreadCount()), 0);
    applyEncodingPreference(options);

    if (int result = avcodec_open2(m_avCodecContext, codec, &options); result < 0) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Could not open codec" << av_err2str(result);
        return false;
    }

    return true;
}

int LibOpenH264Encoder::percentageToAbsoluteQuality(const std::optional<quint8> &quality)
{
    if (!quality) {
        return -1;
    }

    // 1-51 (incl.), lower is better
    return 51 - (m_quality.value() / 100.0) * 50;
}

void LibOpenH264Encoder::applyEncodingPreference(AVDictionary *options)
{
    SoftwareEncoder::applyEncodingPreference(options);
    // Disable motion estimation, not great while dragging windows but speeds up encoding by an order of magnitude
    av_dict_set(&options, "flags", "+mv4", 0);
    // Disable in-loop filtering
    av_dict_set_int(&options, "loopfilter", 0, 0);
}
