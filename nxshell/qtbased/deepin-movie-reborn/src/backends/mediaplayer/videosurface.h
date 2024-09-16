// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QAbstractVideoSurface>

class VideoSurface : public QAbstractVideoSurface
{
        Q_OBJECT

public:
        explicit VideoSurface(QObject *parent = Q_NULLPTR);
        ~VideoSurface();

        QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
        bool present(const QVideoFrame &frame);

signals:
        void frameAvailable(QVideoFrame &frame);

};

#endif // VIDEOSURFACE_H
