/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <mutex>

#include <QObject>

#include "dmabufhandler.h"
#include "pipewireproduce_p.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
}

#undef av_err2str
// The one provided by libav fails to compile on GCC due to passing data from the function scope outside
char *av_err2str(int errnum);

struct PipeWireFrame;
class PipeWireProduce;

/**
 * Base class for objects that encapsulate encoder logic and state.
 */
class Encoder : public QObject
{
    Q_OBJECT
public:
    enum class H264Profile { Baseline, Main, High };

    /**
     * Constructor.
     *
     * @param produce The PipeWireProduce instance that owns this encoder.
     */
    Encoder(PipeWireProduce *produce);
    ~Encoder() override;

    /**
     * Initialize and setup the encoder.
     *
     * @param size The size of the stream being encoded.
     *
     * @return true if initailization was succesful, false if not.
     */
    virtual bool initialize(const QSize &size) = 0;
    /**
     * Process a PipeWire frame and pass it to libav for filtering.
     *
     * @param frame The frame to process.
     *
     * @note This method will be called on its own thread.
     */
    virtual bool filterFrame(const PipeWireFrame &frame) = 0;
    /**
     * Get the next finished frames from the libav filter chain and queue them for encoding.
     *
     * @param maximumFrames The maximum number of frames that can be queued for encoding.
     *
     * @return A pair with the number of frames removed from the filter chain as first entry
     *         and the number of frames queued for encoding as the second entry.
     *
     * @note This method will be called on its own thread.
     */
    virtual std::pair<int, int> encodeFrame(int maximumFrames);
    /**
     * Get the next encoded frames from libav and pass them to PipeWireProduce.
     *
     * @return The number of encoded frames that were received.
     *
     * @note This method will be called on its own thread.
     */
    virtual int receivePacket();
    /**
     * End encoding and perform any necessary cleanup.
     */
    virtual void finish();

    /**
     * Return the AVCodecContext for this encoder.
     */
    AVCodecContext *avCodecContext() const;

    /**
     * Set the quality level, from 0 (lowest) to 100 (highest).
     *
     * Internally this will be converted to an encoder-specific quality value.
     */
    void setQuality(std::optional<quint8> quality);

    static bool supportsHardwareEncoding();

    void setEncodingPreference(PipeWireBaseEncodedStream::EncodingPreference preference);

protected:
    virtual int percentageToAbsoluteQuality(const std::optional<quint8> &quality) = 0;

    PipeWireProduce *m_produce;

    AVCodecContext *m_avCodecContext = nullptr;
    std::mutex m_avCodecMutex;

    AVFilterGraph *m_avFilterGraph = nullptr;
    AVFilterContext *m_inputFilter = nullptr;
    AVFilterContext *m_outputFilter = nullptr;

    std::optional<quint8> m_quality;
    PipeWireBaseEncodedStream::EncodingPreference m_encodingPreference;
    virtual void applyEncodingPreference(AVDictionary *options);
};

/**
 * Encoder subclass that can be used as base class for software encoders.
 */
class SoftwareEncoder : public Encoder
{
public:
    SoftwareEncoder(PipeWireProduce *produce);

    bool filterFrame(const PipeWireFrame &frame) override;

protected:
    /**
     * Create a default filter graph that converts from RGBA to YUV420P.
     *
     * @param size The size of the stream to encode.
     */
    bool createFilterGraph(const QSize &size);

    DmaBufHandler m_dmaBufHandler;
};

/**
 * Encoder subclass that can be used as base class for hardware encoders.
 */
class HardwareEncoder : public Encoder
{
public:
    HardwareEncoder(PipeWireProduce *produce);
    ~HardwareEncoder() override;

    bool filterFrame(const PipeWireFrame &frame) override;

protected:
    /**
     * Check if VAAPI is supported for a given size stream.
     *
     * @param size The size of the stream to check.
     *
     * @return The path to a device node that can encode this stream. If
     *         the stream cannot be encoded by the current hardware, an
     *         empty QByteArray will be returned.
     */
    QByteArray checkVaapi(const QSize &size);
    /**
     * Create the libav contexts for the DRM subsystem.
     *
     * These contexts are used when doing import of dma-buf based frames.
     *
     * @param path The path to a device node where the frames are.
     * @param size The size of the frames.
     *
     * @return true if the contexts were successfully created, false if not.
     */
    bool createDrmContext(const QSize &size);

    AVBufferRef *m_drmContext = nullptr;
    AVBufferRef *m_drmFramesContext = nullptr;
};
