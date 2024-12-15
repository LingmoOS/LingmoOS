// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USE_LIBDMR

#include "videosurface.h"

#include <QDebug>
#include <QTime>

using namespace ddplugin_videowallpaper;

VideoSurface::VideoSurface(QObject *parent) : QAbstractVideoSurface(parent)
{

}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const
{
    if (type == QAbstractVideoBuffer::NoHandle) {
        return QList<QVideoFrame::PixelFormat> {
            QVideoFrame::Format_ARGB32,
            QVideoFrame::Format_ARGB32,
            QVideoFrame::Format_ARGB32_Premultiplied,
            QVideoFrame::Format_RGB32
        };
    }

    return {};
}

bool VideoSurface::present(const QVideoFrame &frame)
{
    QVideoFrame clone(frame);
    if (!clone.map(QAbstractVideoBuffer::ReadOnly))
        return false;

    QImage img = QImage(clone.bits(), clone.width(), clone.height(), QVideoFrame::imageFormatFromPixelFormat(clone.pixelFormat())).copy();
    clone.unmap();
    emit pushImage(img);
    return true;
}
#endif

