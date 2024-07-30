/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "h264vaapiencoder_p.h"

#include <QSize>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

#include "logging_record.h"

H264VAAPIEncoder::H264VAAPIEncoder(H264Profile profile, PipeWireProduce *produce)
    : HardwareEncoder(produce)
    , m_profile(profile)
{
}

bool H264VAAPIEncoder::initialize(const QSize &size)
{
    if (!createDrmContext(size)) {
        return false;
    }

    m_avFilterGraph = avfilter_graph_alloc();
    if (!m_avFilterGraph) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Could not create filter graph";
        return false;
    }

    int ret = avfilter_graph_create_filter(&m_inputFilter,
                                           avfilter_get_by_name("buffer"),
                                           "in",
                                           "width=1:height=1:pix_fmt=drm_prime:time_base=1/1",
                                           nullptr,
                                           m_avFilterGraph);
    if (ret < 0) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Failed to create the buffer filter";
        return false;
    }

    auto parameters = av_buffersrc_parameters_alloc();
    if (!parameters) {
        qFatal("Failed to allocate memory");
    }

    parameters->format = AV_PIX_FMT_DRM_PRIME;
    parameters->width = size.width();
    parameters->height = size.height();
    parameters->time_base = {1, 1000};
    parameters->hw_frames_ctx = m_drmFramesContext;

    av_buffersrc_parameters_set(m_inputFilter, parameters);
    av_free(parameters);
    parameters = nullptr;

    ret = avfilter_graph_create_filter(&m_outputFilter, avfilter_get_by_name("buffersink"), "out", nullptr, nullptr, m_avFilterGraph);
    if (ret < 0) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Could not create buffer output filter";
        return false;
    }

    auto inputs = avfilter_inout_alloc();
    if (!inputs) {
        qFatal("Failed to allocate memory");
    }
    inputs->name = av_strdup("in");
    inputs->filter_ctx = m_inputFilter;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    auto outputs = avfilter_inout_alloc();
    if (!outputs) {
        qFatal("Failed to allocate memory");
    }
    outputs->name = av_strdup("out");
    outputs->filter_ctx = m_outputFilter;
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    ret = avfilter_graph_parse(m_avFilterGraph, "hwmap=mode=direct:derive_device=vaapi,scale_vaapi=format=nv12:mode=fast", outputs, inputs, NULL);
    if (ret < 0) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Failed creating filter graph";
        return false;
    }

    for (auto i = 0u; i < m_avFilterGraph->nb_filters; ++i) {
        m_avFilterGraph->filters[i]->hw_device_ctx = av_buffer_ref(m_drmContext);
    }

    ret = avfilter_graph_config(m_avFilterGraph, nullptr);
    if (ret < 0) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Failed configuring filter graph";
        return false;
    }

    auto codec = avcodec_find_encoder_by_name("h264_vaapi");
    if (!codec) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "h264_vaapi codec not found";
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
    m_avCodecContext->pix_fmt = AV_PIX_FMT_VAAPI;
    m_avCodecContext->time_base = AVRational{1, 1000};

    if (m_quality) {
        m_avCodecContext->global_quality = percentageToAbsoluteQuality(m_quality);
    } else {
        m_avCodecContext->global_quality = 35;
    }

    switch (m_profile) {
    case H264Profile::Baseline:
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
    // av_dict_set_int(&options, "threads", qMin(16, QThread::idealThreadCount()), 0);
    applyEncodingPreference(options);

    m_avCodecContext->hw_frames_ctx = av_buffer_ref(m_outputFilter->inputs[0]->hw_frames_ctx);

    if (int result = avcodec_open2(m_avCodecContext, codec, &options); result < 0) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Could not open codec" << av_err2str(ret);
        return false;
    }

    return true;
}

int H264VAAPIEncoder::percentageToAbsoluteQuality(const std::optional<quint8> &quality)
{
    if (!quality) {
        return -1;
    }

    constexpr int MinQuality = 51 + 6 * 6;
    return std::max(1, int(MinQuality - (m_quality.value() / 100.0) * MinQuality));
}

void H264VAAPIEncoder::applyEncodingPreference(AVDictionary *options)
{
    HardwareEncoder::applyEncodingPreference(options);
    // Disable motion estimation, not great while dragging windows but speeds up encoding by an order of magnitude
    av_dict_set(&options, "flags", "+mv4", 0);
    // Disable in-loop filtering
    av_dict_set(&options, "-flags", "+loop", 0);
}
