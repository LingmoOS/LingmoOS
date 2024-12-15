// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MEDIACONTROLLER_H
#define MEDIACONTROLLER_H

#include "dbusmediaplayer2.h"
#include "mediamonitor.h"

#include <QObject>
#include <QDBusInterface>

class MediaController : public QObject
{
    Q_OBJECT
public:
    MediaController();
    ~MediaController() {}

    bool isWorking() const;
    void next();
    void pause();
    void play();
    void raise();
    const QString &mediaPath() const { return m_path; }

Q_SIGNALS:
    void mediaAcquired() const;
    void mediaChanged() const;
    void mediaLosted() const;

public Q_SLOTS:
    void loadMediaPath(const QString &path);
    void removeMediaPath(const QString &path);
    void onMetaDataChanged();
    void onPlaybackStatusChanged();

private:
    DBusMediaPlayer2 *m_mediaPlayer2;
    MediaMonitor *m_mediaMonitor;

    QString     m_path;
    QStringList m_mediaPaths;
};

#endif
