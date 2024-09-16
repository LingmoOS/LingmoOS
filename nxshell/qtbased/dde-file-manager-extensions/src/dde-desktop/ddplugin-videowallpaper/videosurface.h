// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#ifndef USE_LIBDMR
#include <QAbstractVideoSurface>

namespace ddplugin_videowallpaper {

class VideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit VideoSurface(QObject *parent = nullptr);
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const override;
    bool present(const QVideoFrame &frame) override;
signals:
    void pushImage(const QImage &img);
protected:

};

}
#endif
#endif // VIDEOSURFACE_H
