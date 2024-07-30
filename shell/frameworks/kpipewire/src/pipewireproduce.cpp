/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pipewireproduce_p.h"

#include <QMutex>
#include <QPainter>
#include <QThreadPool>
#include <logging_record.h>

#include <QDateTime>
#include <memory>
#include <qstringliteral.h>

#include "h264vaapiencoder_p.h"
#include "libopenh264encoder_p.h"
#include "libvpxencoder_p.h"
#include "libvpxvp9encoder_p.h"
#include "libx264encoder_p.h"

extern "C" {
#include <fcntl.h>
}

Q_DECLARE_METATYPE(std::optional<int>);
Q_DECLARE_METATYPE(std::optional<std::chrono::nanoseconds>);

PipeWireProduce::PipeWireProduce(PipeWireBaseEncodedStream::Encoder encoderType, uint nodeId, uint fd, const Fraction &framerate)
    : QObject()
    , m_nodeId(nodeId)
    , m_encoderType(encoderType)
    , m_fd(fd)
    , m_frameRate(framerate)
{
    qRegisterMetaType<std::optional<int>>();
    qRegisterMetaType<std::optional<std::chrono::nanoseconds>>();
}

PipeWireProduce::~PipeWireProduce()
{
}

void PipeWireProduce::initialize()
{
    m_stream.reset(new PipeWireSourceStream(nullptr));
    m_stream->setMaxFramerate(m_frameRate);

    // The check in supportsHardwareEncoding() is insufficient to fully
    // determine if we actually support hardware encoding the current stream,
    // but to determine that we need the stream size, which we don't get until
    // after we've created the stream, but creating the stream sets important
    // parameters that require the correct usage hint to be set. So use the
    // insufficient check to set the hint, assuming that we still get a working
    // stream when we use the wrong hint with software encoding.
    m_stream->setUsageHint(Encoder::supportsHardwareEncoding() ? PipeWireSourceStream::UsageHint::EncodeHardware
                                                               : PipeWireSourceStream::UsageHint::EncodeSoftware);

    bool created = m_stream->createStream(m_nodeId, m_fd);
    if (!created || !m_stream->error().isEmpty()) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "failed to set up stream for" << m_nodeId << m_stream->error();
        m_error = m_stream->error();
        m_stream.reset(nullptr);
        return;
    }
    connect(m_stream.get(), &PipeWireSourceStream::streamParametersChanged, this, &PipeWireProduce::setupStream);
}

Fraction PipeWireProduce::maxFramerate() const
{
    return m_maxFramerate;
}

void PipeWireProduce::setMaxFramerate(const Fraction &framerate)
{
    m_maxFramerate = framerate;
    m_stream->setMaxFramerate(framerate);
}

int PipeWireProduce::maxPendingFrames() const
{
    return m_maxPendingFrames;
}

void PipeWireProduce::setMaxPendingFrames(int newMaxBufferSize)
{
    if (newMaxBufferSize < 3) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Maxmimum pending frame count of " << newMaxBufferSize << " requested. Value must be 3 or higher.";
        newMaxBufferSize = 3;
    }
    m_maxPendingFrames = newMaxBufferSize;
}

void PipeWireProduce::setupStream()
{
    qCDebug(PIPEWIRERECORD_LOGGING) << "Setting up stream";
    disconnect(m_stream.get(), &PipeWireSourceStream::streamParametersChanged, this, &PipeWireProduce::setupStream);

    m_encoder = makeEncoder();
    if (!m_encoder) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "No encoder could be created";
        return;
    }

    connect(m_stream.get(), &PipeWireSourceStream::stateChanged, this, &PipeWireProduce::stateChanged);
    if (!setupFormat()) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Could not set up the producing thread";
        return;
    }

    connect(m_stream.data(), &PipeWireSourceStream::frameReceived, this, &PipeWireProduce::processFrame);

    m_passthroughThread = std::thread([this]() {
        m_passthroughRunning = true;
        while (m_passthroughRunning) {
            std::unique_lock<std::mutex> lock(m_frameReceivedMutex);
            m_frameReceivedCondition.wait(lock);

            if (!m_passthroughRunning) {
                break;
            }

            auto [filtered, queued] = m_encoder->encodeFrame(m_maxPendingFrames - m_pendingEncodeFrames);
            m_pendingFilterFrames -= filtered;
            m_pendingEncodeFrames += queued;

            m_frameReceivedCondition.notify_all();
        }
    });
    pthread_setname_np(m_passthroughThread.native_handle(), "PipeWireProduce::passthrough");

    m_outputThread = std::thread([this]() {
        m_outputRunning = true;
        while (m_outputRunning) {
            std::unique_lock<std::mutex> lock(m_frameReceivedMutex);
            m_frameReceivedCondition.wait(lock);

            if (!m_outputRunning) {
                break;
            }

            auto received = m_encoder->receivePacket();
            m_pendingEncodeFrames -= received;
        }
    });
    pthread_setname_np(m_outputThread.native_handle(), "PipeWireProduce::output");
}

void PipeWireProduce::deactivate()
{
    m_deactivated = true;

    auto streamState = m_stream->state();

    m_stream->setActive(false);

    // If we have not been initialized properly before, ensure we still run any
    // cleanup code and exit the thread, otherwise we risk applications not closing
    // properly.
    if (!m_encoder || streamState != PW_STREAM_STATE_STREAMING) {
        QMetaObject::invokeMethod(this, &PipeWireProduce::destroy, Qt::QueuedConnection);
    }
}

void PipeWireProduce::destroy()
{
    // Ensure we cleanup the PipeWireSourceStream while in the same thread we
    // created it in.
    Q_ASSERT_X(QThread::currentThread() == thread(), "PipeWireProduce", "destroy() called from a different thread than PipeWireProduce's thread");

    if (m_passthroughThread.joinable()) {
        m_passthroughRunning = false;
        m_frameReceivedCondition.notify_all();
        m_passthroughThread.join();
    }

    if (m_outputThread.joinable()) {
        m_outputRunning = false;
        m_frameReceivedCondition.notify_all();
        m_outputThread.join();
    }

    m_stream.reset();

    qCDebug(PIPEWIRERECORD_LOGGING) << "finished";
    cleanup();
    QThread::currentThread()->quit();
}

void PipeWireProduce::setQuality(const std::optional<quint8> &quality)
{
    m_quality = quality;
    if (m_encoder) {
        m_encoder->setQuality(quality);
    }
}

void PipeWireProduce::setEncodingPreference(const PipeWireBaseEncodedStream::EncodingPreference &encodingPreference)
{
    m_encodingPreference = encodingPreference;

    if (m_encoder) {
        m_encoder->setEncodingPreference(encodingPreference);
    }
}

void PipeWireProduce::processFrame(const PipeWireFrame &frame)
{
    auto f = frame;

    if (frame.cursor) {
        m_cursor.position = frame.cursor->position;
        m_cursor.hotspot = frame.cursor->hotspot;
        if (!frame.cursor->texture.isNull()) {
            m_cursor.dirty = true;
            m_cursor.texture = frame.cursor->texture;
        }
    }

    auto pts = framePts(frame.presentationTimestamp);
    if (m_previousPts >= 0 && pts <= m_previousPts) {
        return;
    }

    auto frameTime = 1000.0 / (m_maxFramerate.numerator / m_maxFramerate.denominator);
    if ((pts - m_previousPts) < frameTime) {
        return;
    }

    if (m_pendingFilterFrames + 1 > m_maxPendingFrames) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Filter queue is full, dropping frame" << pts;
        return;
    }

    aboutToEncode(f);
    if (!m_encoder->filterFrame(f)) {
        return;
    }

    m_pendingFilterFrames++;
    m_previousPts = pts;

    m_frameReceivedCondition.notify_all();
}

void PipeWireProduce::stateChanged(pw_stream_state state)
{
    if (state != PW_STREAM_STATE_PAUSED || !m_deactivated) {
        return;
    }
    if (!m_stream) {
        qCDebug(PIPEWIRERECORD_LOGGING) << "finished without a stream";
        return;
    }

    disconnect(m_stream.data(), &PipeWireSourceStream::frameReceived, this, &PipeWireProduce::processFrame);

    m_encoder->finish();

    // We want to clean up the source stream while in the input thread, but we
    // need to do so while not handling any PipeWire callback as that risks
    // crashing because we're stil executing PipeWire handling code.
    QMetaObject::invokeMethod(this, &PipeWireProduce::destroy, Qt::QueuedConnection);
}

std::unique_ptr<Encoder> PipeWireProduce::makeEncoder()
{
    auto forcedEncoder = qEnvironmentVariable("KPIPEWIRE_FORCE_ENCODER");
    if (!forcedEncoder.isNull()) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Forcing encoder to" << forcedEncoder;
    }

    auto size = m_stream->size();

    switch (m_encoderType) {
    case PipeWireBaseEncodedStream::H264Baseline:
    case PipeWireBaseEncodedStream::H264Main: {
        auto profile = m_encoderType == PipeWireBaseEncodedStream::H264Baseline ? Encoder::H264Profile::Baseline : Encoder::H264Profile::Main;

        if (forcedEncoder.isNull() || forcedEncoder == u"h264_vaapi") {
            auto hardwareEncoder = std::make_unique<H264VAAPIEncoder>(profile, this);
            hardwareEncoder->setQuality(m_quality);
            hardwareEncoder->setEncodingPreference(m_encodingPreference);
            if (hardwareEncoder->initialize(size)) {
                return hardwareEncoder;
            }
        }

        if (forcedEncoder.isNull() || forcedEncoder == u"libx264") {
            auto softwareEncoder = std::make_unique<LibX264Encoder>(profile, this);
            softwareEncoder->setQuality(m_quality);
            softwareEncoder->setEncodingPreference(m_encodingPreference);
            if (softwareEncoder->initialize(size)) {
                return softwareEncoder;
            }
        }

        // Try libopenh264 last, it's slower and has less features.
        if (forcedEncoder.isNull() || forcedEncoder == u"libopenh264") {
            auto softwareEncoder = std::make_unique<LibOpenH264Encoder>(profile, this);
            softwareEncoder->setQuality(m_quality);
            softwareEncoder->setEncodingPreference(m_encodingPreference);
            if (softwareEncoder->initialize(size)) {
                return softwareEncoder;
            }
        }
        break;
    }
    case PipeWireBaseEncodedStream::VP8: {
        if (forcedEncoder.isNull() || forcedEncoder == u"libvpx") {
            auto encoder = std::make_unique<LibVpxEncoder>(this);
            encoder->setQuality(m_quality);
            if (encoder->initialize(size)) {
                return encoder;
            }
        }
        break;
    }
    case PipeWireBaseEncodedStream::VP9: {
        if (forcedEncoder.isNull() || forcedEncoder == u"libvpx-vp9") {
            auto encoder = std::make_unique<LibVpxVp9Encoder>(this);
            encoder->setQuality(m_quality);
            if (encoder->initialize(size)) {
                return encoder;
            }
        }
        break;
    }
    default:
        qCWarning(PIPEWIRERECORD_LOGGING) << "Unknown encoder type" << m_encoderType;
    }

    return nullptr;
}

#include "moc_pipewireproduce_p.cpp"
