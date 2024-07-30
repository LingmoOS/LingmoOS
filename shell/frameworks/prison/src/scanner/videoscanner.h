/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_VIDEOSCANNER_H
#define PRISON_VIDEOSCANNER_H

#include "prisonscanner_export.h"
#include "scanresult.h"

#include <QObject>
#include <QVideoSink>

#include <memory>

namespace Prison
{

class VideoScannerPrivate;

/** Scans a live video feed for barcodes.
 *
 *  In Qt5 this can be added as a video filter to a VideoOutput element.
 *  In Qt6 this can be connected to a QVideoSink object.
 *
 *  @since 5.94
 */
class PRISONSCANNER_EXPORT VideoScanner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Prison::ScanResult result READ result NOTIFY resultChanged)
    Q_PROPERTY(Prison::Format::BarcodeFormats formats READ formats WRITE setFormats NOTIFY formatsChanged)

    Q_PROPERTY(QVideoSink *videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

public:
    explicit VideoScanner(QObject *parent = nullptr);
    ~VideoScanner();

    /** The latest result of the barcode scan. */
    ScanResult result() const;

    /** The barcode formats the scanner should look for.
     *  By default all supported formats are enabled.
     */
    Format::BarcodeFormats formats() const;
    /**
     * Sets the barcode formats to detect.
     * @param formats can be OR'ed values from Format::BarcodeFormats.
     */
    void setFormats(Format::BarcodeFormats formats);

    /** The video sink being scanned for barcodes. */
    QVideoSink *videoSink() const;
    /** Sets the video sink to scan for barcodes. */
    void setVideoSink(QVideoSink *sink);

Q_SIGNALS:
    /** Emitted whenever we get a new scan result, as long as any
     *  property of the result changes. On a live video feed this can
     *  be very frequently due to the changes of the position of the detected
     *  barcode. This is therefore useful e.g. for marking the position
     *  of the detected barcode.
     *  @see resultContentChanged
     */
    void resultChanged(const Prison::ScanResult &scanResult);

    /** Emitted when a barcode with a new content has been detected, but
     *  not when merely the position of a barcode changes in the video stream.
     *  This is useful e.g. for continuously scanning multiple codes in one go.
     *  @see resultChanged
     */
    void resultContentChanged(const Prison::ScanResult &scanResult);

    void formatsChanged();
    void videoSinkChanged();

private:
    std::unique_ptr<VideoScannerPrivate> d;
};

}

#endif // PRISON_VIDEOSCANNER_H
