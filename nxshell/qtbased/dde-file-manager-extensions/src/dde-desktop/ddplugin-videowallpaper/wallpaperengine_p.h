// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WALLPAPERENGINE_P_H
#define WALLPAPERENGINE_P_H

#include "wallpaperengine.h"
#include "videoproxy.h"
#include "videosurface.h"

#include <QFileSystemWatcher>
#include <QUrl>

#ifndef USE_LIBDMR
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtMultimedia/QMediaPlaylist>
#endif

namespace ddplugin_videowallpaper {

class WallpaperEnginePrivate
{
public:
    explicit WallpaperEnginePrivate(WallpaperEngine *qq);
    inline QRect relativeGeometry(const QRect &geometry)
    {
        return QRect(QPoint(0, 0), geometry.size());
    }
#ifndef USE_LIBDMR
    static QList<QMediaContent> getVideos(const QString &path);
#else
    static QList<QUrl> getVideos(const QString &path);
#endif
public:
    VideoProxyPointer createWidget(QWidget *root);
    void setBackgroundVisible(bool v);
    QString sourcePath() const;
    QMap<QString, VideoProxyPointer> widgets;

    QFileSystemWatcher *watcher = nullptr;
#ifndef USE_LIBDMR
    QList<QMediaContent> videos;
    QMediaPlaylist *playlist = nullptr;
    QMediaPlayer *player = nullptr;
    VideoSurface *surface = nullptr;
#else
    QList<QUrl> videos;
#endif
private:
    WallpaperEngine *q;
};

}

#endif // WALLPAPERENGINE_P_H
