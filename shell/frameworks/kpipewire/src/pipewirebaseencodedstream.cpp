/*
    SPDX-FileCopyrightText: 2022-2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pipewirebaseencodedstream.h"

#include <logging_libav.h>
#include <logging_record.h>
#include <memory>
#include <va/va.h>

extern "C" {
#include <libavcodec/codec.h>
#include <libavutil/log.h>
}
#include <unistd.h>

#include "pipewireproduce_p.h"
#include "vaapiutils_p.h"

struct PipeWireEncodedStreamPrivate {
    uint m_nodeId = 0;
    std::optional<uint> m_fd;
    Fraction m_maxFramerate;
    int m_maxPendingFrames = 50;
    bool m_active = false;
    PipeWireBaseEncodedStream::Encoder m_encoder;
    std::optional<quint8> m_quality;
    PipeWireBaseEncodedStream::EncodingPreference m_encodingPreference;

    std::unique_ptr<QThread> m_produceThread;
    std::unique_ptr<PipeWireProduce> m_produce;
};

PipeWireBaseEncodedStream::State PipeWireBaseEncodedStream::state() const
{
    if (isActive()) {
        return Recording;
    } else if (d->m_produceThread && d->m_produce->m_deactivated && d->m_produceThread->isRunning()) {
        return Rendering;
    }

    return Idle;
}

PipeWireBaseEncodedStream::PipeWireBaseEncodedStream(QObject *parent)
    : QObject(parent)
    , d(new PipeWireEncodedStreamPrivate)
{
    d->m_encoder = suggestedEncoders().value(0, NoEncoder);

    const auto &category = PIPEWIRELIBAV_LOGGING();
    if (category.isDebugEnabled()) {
        av_log_set_level(AV_LOG_DEBUG);
    } else if (category.isInfoEnabled()) {
        av_log_set_level(AV_LOG_INFO);
    } else if (category.isWarningEnabled()) {
        av_log_set_level(AV_LOG_WARNING);
    } else {
        av_log_set_level(AV_LOG_ERROR);
    }
}

PipeWireBaseEncodedStream::~PipeWireBaseEncodedStream()
{
    setActive(false);

    if (d->m_fd) {
        close(*d->m_fd);
    }
}

void PipeWireBaseEncodedStream::setNodeId(uint nodeId)
{
    if (nodeId == d->m_nodeId)
        return;

    d->m_nodeId = nodeId;
    refresh();
    Q_EMIT nodeIdChanged(nodeId);
}

void PipeWireBaseEncodedStream::setFd(uint fd)
{
    if (fd == d->m_fd)
        return;

    if (d->m_fd) {
        close(*d->m_fd);
    }
    d->m_fd = fd;
    refresh();
    Q_EMIT fdChanged(fd);
}

Fraction PipeWireBaseEncodedStream::maxFramerate() const
{
    if (d->m_maxFramerate) {
        return d->m_maxFramerate;
    }
    return Fraction{60, 1};
}

void PipeWireBaseEncodedStream::setMaxFramerate(const Fraction &framerate)
{
    if (d->m_maxFramerate == framerate) {
        return;
    }
    d->m_maxFramerate = framerate;

    if (d->m_produce) {
        d->m_produce->setMaxFramerate(d->m_maxFramerate);
    }

    Q_EMIT maxFramerateChanged();
}

void PipeWireBaseEncodedStream::setMaxFramerate(quint32 numerator, quint32 denominator)
{
    setMaxFramerate({numerator, denominator});
}

void PipeWireBaseEncodedStream::setMaxPendingFrames(int maxPendingFrames)
{
    if (d->m_maxPendingFrames == maxPendingFrames) {
        return;
    }
    if (d->m_produce) {
        d->m_produce->setMaxPendingFrames(maxPendingFrames);
    }
    d->m_maxPendingFrames = maxPendingFrames;
    Q_EMIT maxPendingFramesChanged();
}

int PipeWireBaseEncodedStream::maxBufferSize() const
{
    return d->m_maxPendingFrames;
}

void PipeWireBaseEncodedStream::setActive(bool active)
{
    if (d->m_active == active)
        return;

    d->m_active = active;
    refresh();
    Q_EMIT activeChanged(active);
}

std::optional<quint8> PipeWireBaseEncodedStream::quality() const
{
    return d->m_quality;
}

void PipeWireBaseEncodedStream::setQuality(quint8 quality)
{
    d->m_quality = quality;
    if (d->m_produce) {
        d->m_produce->setQuality(d->m_quality);
    }
}

void PipeWireBaseEncodedStream::refresh()
{
    if (d->m_produceThread) {
        QMetaObject::invokeMethod(d->m_produce.get(), &PipeWireProduce::deactivate, Qt::QueuedConnection);
        d->m_produceThread->wait();

        d->m_produce.reset();
        d->m_produceThread.reset();
    }

    if (d->m_active && d->m_nodeId > 0) {
        d->m_produceThread = std::make_unique<QThread>();
        d->m_produceThread->setObjectName("PipeWireProduce::input");
        d->m_produce = makeProduce();
        d->m_produce->setQuality(d->m_quality);
        d->m_produce->setMaxPendingFrames(d->m_maxPendingFrames);
        d->m_produce->setEncodingPreference(d->m_encodingPreference);
        d->m_produce->moveToThread(d->m_produceThread.get());
        d->m_produceThread->start();
        QMetaObject::invokeMethod(d->m_produce.get(), &PipeWireProduce::initialize, Qt::QueuedConnection);
    }

    Q_EMIT stateChanged();
}

void PipeWireBaseEncodedStream::setEncoder(Encoder encoder)
{
    if (d->m_encoder == encoder || !suggestedEncoders().contains(encoder)) {
        return;
    }
    d->m_encoder = encoder;
    Q_EMIT encoderChanged();
}

PipeWireBaseEncodedStream::Encoder PipeWireBaseEncodedStream::encoder() const
{
    return d->m_encoder;
}

QList<PipeWireBaseEncodedStream::Encoder> PipeWireBaseEncodedStream::suggestedEncoders() const
{
    auto vaapi = VaapiUtils::instance();

    QList<PipeWireBaseEncodedStream::Encoder> ret = {PipeWireBaseEncodedStream::VP8,
                                                     PipeWireBaseEncodedStream::VP9,
                                                     PipeWireBaseEncodedStream::H264Main,
                                                     PipeWireBaseEncodedStream::H264Baseline};
    auto removeUnavailableEncoders = [&vaapi](const PipeWireBaseEncodedStream::Encoder &encoder) {
        switch (encoder) {
        case PipeWireBaseEncodedStream::VP8:
            if (vaapi->supportsProfile(VAProfileVP8Version0_3) && avcodec_find_encoder_by_name("vp8_vaapi")) {
                return false;
            } else {
                return !avcodec_find_encoder_by_name("libvpx");
            }
        case PipeWireBaseEncodedStream::VP9:
            return !avcodec_find_encoder_by_name("libvpx-vp9");
        case PipeWireBaseEncodedStream::H264Main:
        case PipeWireBaseEncodedStream::H264Baseline:
            if (vaapi->supportsProfile(encoder == PipeWireBaseEncodedStream::H264Main ? VAProfileH264Main : VAProfileH264ConstrainedBaseline)
                && avcodec_find_encoder_by_name("h264_vaapi")) {
                return false;
            } else {
                return !(avcodec_find_encoder_by_name("libx264") || avcodec_find_encoder_by_name("libopenh264"));
            }
        default:
            return true;
        }
    };
    ret.removeIf(removeUnavailableEncoders);
    return ret;
}

void PipeWireBaseEncodedStream::setEncodingPreference(PipeWireBaseEncodedStream::EncodingPreference preference)
{
    d->m_encodingPreference = preference;
    if (d->m_produce) {
        d->m_produce->setEncodingPreference(d->m_encodingPreference);
    }
}

PipeWireBaseEncodedStream::EncodingPreference PipeWireBaseEncodedStream::encodingPreference()
{
    return d->m_encodingPreference;
}

bool PipeWireBaseEncodedStream::isActive() const
{
    return d->m_active;
}

uint PipeWireBaseEncodedStream::nodeId() const
{
    return d->m_nodeId;
}

uint PipeWireBaseEncodedStream::fd() const
{
    return d->m_fd.value_or(0);
}

#include "moc_pipewirebaseencodedstream.cpp"
