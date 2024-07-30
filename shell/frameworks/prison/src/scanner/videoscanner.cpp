/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#include "videoscanner.h"
#include "videoscannerframe_p.h"
#include "videoscannerworker_p.h"

#include <QDebug>

using namespace Prison;

namespace Prison
{
class VideoScannerPrivate
{
public:
    void newFrame(const QVideoFrame &videoFrame, bool verticallyFlipped);
    void setResult(VideoScanner *q, const ScanResult &result);

    QVideoSink *m_sink = nullptr;
    VideoScannerThread m_thread;
    VideoScannerWorker m_worker;
    QByteArray m_frameDataBuffer; // reused memory when we have to copy frame data
    ScanResult m_result;
    QVariant m_previousContent;
    Format::BarcodeFormats m_formats = Format::NoFormat;
    bool m_workerBusy = false;
};
}

void VideoScannerPrivate::newFrame(const QVideoFrame &videoFrame, bool verticallyFlipped)
{
    // NOTE: this runs in the render thread
    if (!m_workerBusy && videoFrame.isValid()) {
        m_workerBusy = true;

        VideoScannerFrame frame(videoFrame, verticallyFlipped, m_formats);
        // check if we are only allowed to access this in the render thread
        if (frame.copyRequired()) {
            frame.map();
            if (frame.needsConversion()) {
                frame.convertToImage();
            } else {
                frame.copyFrameData(m_frameDataBuffer);
            }
            frame.unmap();
        }

        Q_EMIT m_worker.scanFrameRequest(frame);
    }
}

void VideoScannerPrivate::setResult(VideoScanner *q, const ScanResult &result)
{
    m_workerBusy = false;
    if (m_result == result) {
        return;
    }

    m_result = result;
    Q_EMIT q->resultChanged(result);

    if (m_previousContent == result.content()) {
        return;
    }
    m_previousContent = result.content();
    Q_EMIT q->resultContentChanged(result);
}

VideoScanner::VideoScanner(QObject *parent)
    : QObject(parent)
    , d(new VideoScannerPrivate)
{
    d->m_worker.moveToThread(&d->m_thread);
    connect(
        &d->m_worker,
        &VideoScannerWorker::result,
        this,
        [this](const ScanResult &result) {
            d->setResult(this, result);
        },
        Qt::QueuedConnection);

    d->m_thread.setObjectName(QStringLiteral("Prison Barcode Scanner Worker"));
    d->m_thread.start();
}

VideoScanner::~VideoScanner()
{
    d->m_thread.quit();
    d->m_thread.wait();
}

ScanResult VideoScanner::result() const
{
    return d->m_result;
}

Format::BarcodeFormats VideoScanner::formats() const
{
    return d->m_formats;
}

void VideoScanner::setFormats(Format::BarcodeFormats formats)
{
    if (d->m_formats == formats) {
        return;
    }

    d->m_formats = formats;
    Q_EMIT formatsChanged();
}

QVideoSink *VideoScanner::videoSink() const
{
    return d->m_sink;
}

void VideoScanner::setVideoSink(QVideoSink *sink)
{
    if (d->m_sink == sink) {
        return;
    }

    if (d->m_sink) {
        disconnect(d->m_sink, nullptr, this, nullptr);
    }
    d->m_sink = sink;
    connect(d->m_sink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &frame) {
        d->newFrame(frame, frame.surfaceFormat().scanLineDirection() == QVideoFrameFormat::BottomToTop);
    });
    Q_EMIT videoSinkChanged();
}

#include "moc_videoscanner.cpp"
