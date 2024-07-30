/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_VIDEOSCANNERWORKER_H
#define PRISON_VIDEOSCANNERWORKER_H

#include "scanresult.h"

#include <QObject>
#include <QThread>
#include <QVideoFrame>

namespace Prison
{

class VideoScannerFrame;

/** Contains the actual barcode detecting/decoding work,
 *  to be run in a secondary thread.
 */
class VideoScannerWorker : public QObject
{
    Q_OBJECT
public:
    explicit VideoScannerWorker(QObject *parent = nullptr);

Q_SIGNALS:
    void scanFrameRequest(const VideoScannerFrame &frame);
    void result(const Prison::ScanResult &result);

public Q_SLOTS:
    void slotScanFrame(VideoScannerFrame frame);
};

/** Thread for executing the VideoScannerWorker. */
class VideoScannerThread : public QThread
{
public:
    void run() override;
};

}

#endif // PRISON_VIDEOSCANNERWORKER_H
