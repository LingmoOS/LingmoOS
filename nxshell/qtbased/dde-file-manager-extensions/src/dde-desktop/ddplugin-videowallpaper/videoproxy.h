// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOPROXY_H
#define VIDEOPROXY_H

#include "ddplugin_videowallpaper_global.h"

#ifdef USE_LIBDMR
#include <player_widget.h>
#include <player_engine.h>
#include <compositing_manager.h>
#else
#include <QWidget>
#include <QtMultimedia/QMediaContent>

class QMediaPlayer;
class QVideoWidget;
class QMediaPlaylist;
#endif

namespace ddplugin_videowallpaper {

#ifdef USE_LIBDMR
class VideoProxy : public dmr::PlayerWidget
{
    Q_OBJECT
public:
    explicit VideoProxy(QWidget *parent = nullptr);
    ~VideoProxy();
    void setPlayList(const QList<QUrl> &list);
    void play();
    void stop();
protected slots:
    void playNext();
private:
    QList<QUrl> playList;
    QUrl current;
    bool run = false;
};

#else
class VideoProxy : public QWidget
{
    Q_OBJECT
public:
    explicit VideoProxy(QWidget *parent = nullptr);
    ~VideoProxy();
    void updateImage(const QImage &img);
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QImage image;
};
#endif
typedef QSharedPointer<VideoProxy> VideoProxyPointer;

}

#endif // VIDEOPROXY_H
