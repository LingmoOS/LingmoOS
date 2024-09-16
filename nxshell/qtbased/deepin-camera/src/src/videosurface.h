// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <QImage>
#include <QRect>
#include <QVideoFrame>

class VideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT

public:
    VideoSurface(QObject *parent = 0);
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const override;
    bool isFormatSupported(const QVideoSurfaceFormat &format) const override;
    bool start(const QVideoSurfaceFormat &format) override;
    bool present(const QVideoFrame &frame) override;
    void stop() override;

signals:
    void presentImage(QImage&);

private:
    QImage::Format imageFormat_;
    QSize imageSize_;
    QVideoFrame currentFrame_;
};

#endif // VIDEOSURFACE_H
