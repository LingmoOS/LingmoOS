/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_VIDEOSCANNERFRAME_P_H
#define PRISON_VIDEOSCANNERFRAME_P_H

#include "format.h"

#include <QImage>
#include <QMetaType>
#include <QVideoFrame>

namespace Prison
{

/**
 *  A single frame from a video feed handed over to the barcode scanner worker thread.
 *
 *  This abstracts three possible states:
 *  - direct data access to the original frame data, when possible
 *  - access to an internal copy of at least the luminescence part of the raw frame data
 *    if access is only possible in the render thread and the raw format is consumeable
 *    by ZXing
 *  - a decoded 8bit grayscale QImage of the frame content, if the raw frame data is only
 *    accessible in the render thread and the native frame format is not consumeable by
 *    ZXing directly
 *
 *  @internal
 */
class VideoScannerFrame
{
public:
    explicit VideoScannerFrame();
    explicit VideoScannerFrame(const QVideoFrame &frame, bool verticallyFlipped, Format::BarcodeFormats formats);
    ~VideoScannerFrame();

    int width() const;
    int height() const;
    int bytesPerLine() const;
    QVideoFrameFormat::PixelFormat pixelFormat() const;

    /** Map/unmap the frame if needed, ie. if we don't already have a copy. */
    void map();
    void unmap();

    /** Raw frame data, either zero copy or from an internal buffer
     *  if we were forced to copy.
     *  Possibly truncated to just the subset that contains all the luminescence
     *  channel, e.g. for planar formats.
     */
    const uint8_t *bits() const;

    /** Decides based on the input frame format whether an immediate
     *  copy in the reader thread is necessary.
     */
    bool copyRequired() const;
    /** Copy the required subset of the frame data to our internal buffer.
     *  @note Requires the frame to be mapped.
     */
    void copyFrameData(QByteArray &buffer);

    /** Returns whether we have a format that ZXing cannot consume without
     *  prior conversion. These are the formats we need to let Qt convert
     *  to a QImage first, and in case frame data access is only allowed in
     *  the render thread, this also has to happen there.
     */
    bool needsConversion() const;
    /** Convert to grayscale QImage.
     *  @note Requires the frame to be mapped.
     */
    void convertToImage();

    /** Returns the frame as QImage.
     *  Result is only valid after a call to convertToImage().
     */
    QImage image() const;

    /** The requested barcode formats. */
    Format::BarcodeFormats formats() const;

    /** Returns @c true if the raw frame data is vertically flipped compared
     *  to how it's displayed. This doesn't impact barcode detection as such,
     *  but it requires corresponding adjustments to the coordinates at which
     *  a barcode has been detected.
     */
    bool isVerticallyFlipped() const;

private:
    /** The amount of data to copy. This can be less than the entire frame
     *  size for planar formats.
     *  @note Requires the frame to be mapped.
     */
    int frameDataSize() const;

    QVideoFrame m_frame;
    const uint8_t *m_frameData = nullptr;
    QImage m_image;
    Format::BarcodeFormats m_formats = {};
    bool m_verticallyFlipped = false;
};

}

Q_DECLARE_METATYPE(Prison::VideoScannerFrame)

#endif // PRISON_VIDEOSCANNERFRAME_P_H
