/*
    SPDX-FileCopyrightText: 2018-2020 Red Hat Inc
    SPDX-FileCopyrightText: 2020-2021 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileContributor: Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QDebug>
#include <QHash>
#include <QImage>
#include <QObject>
#include <QPoint>
#include <QSharedPointer>
#include <QSize>
#include <optional>

#include <pipewire/pipewire.h>
#include <spa/param/format-utils.h>
#include <spa/param/props.h>
#include <spa/param/video/format-utils.h>

#include <kpipewire_export.h>

#undef Status

class PipeWireFrameCleanupFunction;
class PipeWireCore;
struct gbm_device;

typedef void *EGLDisplay;

struct DmaBufPlane {
    int fd; ///< The dmabuf file descriptor
    uint32_t offset; ///< The offset from the start of buffer
    uint32_t stride; ///< The distance from the start of a row to the next row in bytes
};

struct DmaBufAttributes {
    int width = 0;
    int height = 0;
    uint32_t format = 0;
    uint64_t modifier = 0; ///< The layout modifier

    QList<DmaBufPlane> planes;
};

struct PipeWireCursor {
    QPoint position;
    QPoint hotspot;
    QImage texture;
    bool operator!=(const PipeWireCursor &other) const
    {
        return !operator==(other);
    };
    bool operator==(const PipeWireCursor &other) const
    {
        return position == other.position && hotspot == other.hotspot && texture == other.texture;
    }
};

class KPIPEWIRE_EXPORT PipeWireFrameData
{
    Q_DISABLE_COPY(PipeWireFrameData)
public:
    PipeWireFrameData(spa_video_format format, void *data, QSize size, qint32 stride, PipeWireFrameCleanupFunction *cleanup);
    ~PipeWireFrameData();

    QImage toImage() const;
    std::shared_ptr<PipeWireFrameData> copy() const;

    const spa_video_format format;
    void *const data = nullptr;
    const QSize size;
    const qint32 stride = 0;
    PipeWireFrameCleanupFunction *const cleanup = nullptr;
};

struct KPIPEWIRE_EXPORT PipeWireFrame {
    spa_video_format format;
    std::optional<quint64> sequential;
    std::optional<std::chrono::nanoseconds> presentationTimestamp;
    std::optional<DmaBufAttributes> dmabuf;
    std::optional<QRegion> damage;
    std::optional<PipeWireCursor> cursor;
    std::shared_ptr<PipeWireFrameData> dataFrame;
};

struct Fraction {
    bool operator==(const Fraction &other) const
    {
        return numerator == other.numerator && denominator == other.denominator;
    }
    explicit operator bool() const
    {
        return isValid();
    }
    bool isValid() const
    {
        return denominator > 0;
    }
    quint32 numerator = 0;
    quint32 denominator = 0;
};

struct PipeWireSourceStreamPrivate;

class KPIPEWIRE_EXPORT PipeWireSourceStream : public QObject
{
    Q_OBJECT
public:
    /**
     * A hint to indicate how this stream will be used.
     *
     * By default this will be set to Render.
     */
    enum class UsageHint {
        Render, ///< Stream is intended mainly for reading pixel data and rendering directly to screen.
        EncodeSoftware, ///< Stream is intended mainly for encoding video using software encoding.
        EncodeHardware, ///< Stream is intended mainly for encoding video using hardware encoding.
    };

    explicit PipeWireSourceStream(QObject *parent = nullptr);
    ~PipeWireSourceStream();

    Fraction framerate() const;
    void setMaxFramerate(const Fraction &framerate);
    uint nodeId();
    QString error() const;

    QSize size() const;
    pw_stream_state state() const;
    bool createStream(uint nodeid, int fd);
    void setActive(bool active);
    void setDamageEnabled(bool withDamage);

    UsageHint usageHint() const;
    void setUsageHint(UsageHint hint);

    void handleFrame(struct pw_buffer *buffer);
    void process();
    void renegotiateModifierFailed(spa_video_format format, quint64 modifier);

    std::optional<std::chrono::nanoseconds> currentPresentationTimestamp() const;

    static uint32_t spaVideoFormatToDrmFormat(spa_video_format spa_format);

    bool usingDmaBuf() const;
    bool allowDmaBuf() const;
    void setAllowDmaBuf(bool allowed);

Q_SIGNALS:
    void streamReady();
    void startStreaming();
    void stopStreaming();
    void streamParametersChanged();
    void frameReceived(const PipeWireFrame &frame);
    void stateChanged(pw_stream_state state, pw_stream_state oldState);

private:
    static void onStreamParamChanged(void *data, uint32_t id, const struct spa_pod *format);
    static void onStreamStateChanged(void *data, pw_stream_state old, pw_stream_state state, const char *error_message);
    static void onRenegotiate(void *data, uint64_t);
    QList<const spa_pod *> createFormatsParams(spa_pod_builder podBuilder);

    void coreFailed(const QString &errorMessage);
    QScopedPointer<PipeWireSourceStreamPrivate> d;
};
