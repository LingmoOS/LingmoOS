/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pwhelpers.h"
#include "logging.h"

QImage::Format SpaToQImageFormat(quint32 format)
{
    switch (format) {
    case SPA_VIDEO_FORMAT_BGRx:
    case SPA_VIDEO_FORMAT_BGRA:
        return QImage::Format_RGBA8888_Premultiplied; // Handled in SpaBufferToQImage
    case SPA_VIDEO_FORMAT_ABGR:
    case SPA_VIDEO_FORMAT_xBGR:
        return QImage::Format_ARGB32; // Handled in SpaBufferToQImage
    case SPA_VIDEO_FORMAT_BGR:
        return QImage::Format_BGR888;
    case SPA_VIDEO_FORMAT_RGBx:
        return QImage::Format_RGBX8888;
    case SPA_VIDEO_FORMAT_RGB:
        return QImage::Format_RGB888;
    case SPA_VIDEO_FORMAT_RGBA:
        return QImage::Format_RGBA8888_Premultiplied;
    case SPA_VIDEO_FORMAT_GRAY8:
        return QImage::Format_Grayscale8;
    default:
        qCWarning(PIPEWIRE_LOGGING) << "cannot convert spa format to QImage" << format;
        return QImage::Format_RGB32;
    }
}

QImage PWHelpers::SpaBufferToQImage(const uchar *data, int width, int height, qsizetype bytesPerLine, spa_video_format format, PipeWireFrameCleanupFunction *c)
{
    c->ref();
    switch (format) {
    case SPA_VIDEO_FORMAT_BGRx:
    case SPA_VIDEO_FORMAT_BGRA:
    case SPA_VIDEO_FORMAT_xBGR:
    case SPA_VIDEO_FORMAT_ABGR: {
        // This is needed because QImage does not support BGRA
        // This is obviously a much slower path, it makes sense to avoid it as much as possible
        return QImage(data, width, height, bytesPerLine, SpaToQImageFormat(format), &PipeWireFrameCleanupFunction::unref, c).rgbSwapped();
    }
    case SPA_VIDEO_FORMAT_GRAY8:
    case SPA_VIDEO_FORMAT_RGBx:
    case SPA_VIDEO_FORMAT_RGB:
    case SPA_VIDEO_FORMAT_RGBA:
    default:
        return QImage(data, width, height, bytesPerLine, SpaToQImageFormat(format), &PipeWireFrameCleanupFunction::unref, c);
    }
}

QImage PipeWireFrameData::toImage() const
{
    return PWHelpers::SpaBufferToQImage(static_cast<uchar *>(data), size.width(), size.height(), stride, format, cleanup);
}

std::shared_ptr<PipeWireFrameData> PipeWireFrameData::copy() const
{
    const uint bufferSize = size.height() * stride * 4;
    auto newMap = malloc(bufferSize);
    memcpy(newMap, data, bufferSize);
    return std::make_shared<PipeWireFrameData>(format, newMap, size, stride, new PipeWireFrameCleanupFunction([newMap] {
                                                   free(newMap);
                                               }));
}
