/*
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef LINGMO_QUICK_PLAYER_ITEM_H
#define LINGMO_QUICK_PLAYER_ITEM_H

#include <QObject>
#include "properties.h"
class PlayerItemPrivate;
class PlayerItem : public QObject
{
    Q_OBJECT
public:

    explicit PlayerItem(const QString &service, QObject *parent = nullptr);

    uint pid();
    bool valid();
    //media player2 properties
    bool canQuit();
    bool fullScreen();
    bool canSetFullScreen();
    bool canRaise();
    bool hasTrackList();
    QString identity();
    QString desktopEntry();
    QStringList supportedUriSchemes();
    QStringList supportedMimeTypes();
    //media player2 methods
    void raise();
    void quit();

    //media player2 player properties
    QString playbackStatus();
    QString loopStatus();
    double rate();
    bool shuffle();
    QVariantMap metaData();
    bool isCurrentMediaVideo();
    bool isCurrentMediaAudio();
    double volume();
    quint64 position();
    double minimumRate();
    double maximumRate();
    bool canGoNext();
    bool canGoPrevious();
    bool canPlay();
    bool canPause();
    bool canSeek();
    bool canControl();
    //media player2 player methods
    void next();
    void previous();
    void pause();
    void playPause();
    void stop();
    void play();
    void seek(qint64 offset);
    void setPosition(const QString &trackID, qint64 position);
    void openUri(const QString &uri);

Q_SIGNALS:
    void dataChanged(const QString &service, QVector<int>);

private:
    PlayerItemPrivate *d;
};


#endif //LINGMO_QUICK_PLAYER_ITEM_H
