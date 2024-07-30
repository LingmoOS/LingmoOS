/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#include "videoscannerframe_p.h"

#include <cstring>

using namespace Prison;

VideoScannerFrame::VideoScannerFrame() = default;

VideoScannerFrame::VideoScannerFrame(const QVideoFrame &frame, bool isVerticallyFlipped, Format::BarcodeFormats formats)
    : m_frame(frame)
    , m_formats(formats)
    , m_verticallyFlipped(isVerticallyFlipped)
{
}

VideoScannerFrame::~VideoScannerFrame() = default;

int VideoScannerFrame::width() const
{
    return m_frame.width();
}

int VideoScannerFrame::height() const
{
    return m_frame.height();
}

int VideoScannerFrame::bytesPerLine() const
{
    return m_frame.bytesPerLine(0);
}

QVideoFrameFormat::PixelFormat VideoScannerFrame::pixelFormat() const
{
    return m_frame.pixelFormat();
}

void VideoScannerFrame::map()
{
    if (!m_frameData && m_image.isNull()) {
        m_frame.map(QVideoFrame::ReadOnly);
    }
}

void VideoScannerFrame::unmap()
{
    if (m_frame.isMapped()) {
        m_frame.unmap();
    }
}

const uint8_t *VideoScannerFrame::bits() const
{
    if (m_frameData) {
        return m_frameData;
    }
    if (!m_image.isNull()) {
        return m_image.bits();
    }

    Q_ASSERT(m_frame.isMapped());
    return m_frame.bits(0);
}

bool VideoScannerFrame::copyRequired() const
{
    return m_frame.handleType() == QVideoFrame::RhiTextureHandle;
}

void VideoScannerFrame::copyFrameData(QByteArray &buffer)
{
    Q_ASSERT(m_frame.isMapped());

    const auto size = frameDataSize();
    if (buffer.size() != size) {
        buffer.resize(size);
    }
    std::memcpy(buffer.data(), m_frame.bits(0), size);
    m_frameData = reinterpret_cast<const uint8_t *>(buffer.constData());
}

int VideoScannerFrame::frameDataSize() const
{
    Q_ASSERT(m_frame.isMapped());

    switch (m_frame.pixelFormat()) {
    case QVideoFrameFormat::Format_YUV420P:
    case QVideoFrameFormat::Format_YUV422P:
    case QVideoFrameFormat::Format_YV12:
    case QVideoFrameFormat::Format_NV12:
    case QVideoFrameFormat::Format_NV21:
    case QVideoFrameFormat::Format_IMC1:
    case QVideoFrameFormat::Format_IMC2:
    case QVideoFrameFormat::Format_IMC3:
    case QVideoFrameFormat::Format_IMC4:
        return m_frame.mappedBytes(0) / 2;
    default:
        return m_frame.mappedBytes(0);
    }
}

bool VideoScannerFrame::needsConversion() const
{
    switch (m_frame.pixelFormat()) {
    case QVideoFrameFormat::Format_Jpeg:
    case QVideoFrameFormat::Format_SamplerExternalOES:
    case QVideoFrameFormat::Format_SamplerRect:
        return true;
    default:
        return false;
    }
}

void VideoScannerFrame::convertToImage()
{
    if (!m_image.isNull()) {
        return;
    }

    Q_ASSERT(m_frame.isMapped());
    m_image = m_frame.toImage();
}

QImage VideoScannerFrame::image() const
{
    return m_image;
}

bool VideoScannerFrame::isVerticallyFlipped() const
{
    return m_verticallyFlipped;
}

Format::BarcodeFormats VideoScannerFrame::formats() const
{
    return m_formats;
}
