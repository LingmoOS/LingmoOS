// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mediacontroller.h"
#include "mediamonitor.h"
#include "mediamodel.h"
#include <qdbusinterface.h>

#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include <QDebug>

MediaController::MediaController()
    : m_mediaPlayer2(nullptr)
    , m_mediaMonitor(new MediaMonitor(this))
{
    connect(m_mediaMonitor, &MediaMonitor::mediaAcquired, this, &MediaController::loadMediaPath);
    connect(m_mediaMonitor, &MediaMonitor::mediaLost, this, &MediaController::removeMediaPath);
    m_mediaMonitor->init();
}

void MediaController::loadMediaPath(const QString &path)
{
    DBusMediaPlayer2 *newMediaPlayer = new DBusMediaPlayer2(path, "/org/mpris/MediaPlayer2", QDBusConnection::sessionBus(), this);

    if (!newMediaPlayer->canShowInUI()) {
        newMediaPlayer->deleteLater();
        return;
    }

    if (!m_mediaPlayer2)
        Q_EMIT mediaAcquired();
    m_path = path;
    MediaModel::ref().setPath(path);

    // save paths
    if (!m_mediaPaths.contains(path))
        m_mediaPaths.append(path);

    if (m_mediaPlayer2)
        m_mediaPlayer2->deleteLater();

    m_mediaPlayer2 = newMediaPlayer;

    connect(m_mediaPlayer2, &DBusMediaPlayer2::MetadataChanged, this, &MediaController::onMetaDataChanged);
    connect(m_mediaPlayer2, &DBusMediaPlayer2::PlaybackStatusChanged, this, &MediaController::onPlaybackStatusChanged);
    connect(m_mediaPlayer2, &DBusMediaPlayer2::CanControlChanged, &MediaModel::ref(), &MediaModel::onCanControlChanged);

    onMetaDataChanged();
    onPlaybackStatusChanged();
}

void MediaController::onMetaDataChanged()
{
    if (!m_mediaPlayer2) return;

    const auto &meta = m_mediaPlayer2->metadata();
    MediaModel::MediaInfo info;
    info.title = meta.value("xesam:title").toString();
    info.artist = meta.value("xesam:artist").toString();
    info.pixmap = QPixmap(QUrl(meta.value("mpris:artUrl").toString()).toLocalFile()).scaled(QSize(32, 32), Qt::IgnoreAspectRatio);
    MediaModel::ref().setMediaInfo(info);
}

void MediaController::onPlaybackStatusChanged()
{
    if (!m_mediaPlayer2)
        return;
    const QString &stat = m_mediaPlayer2->playbackStatus();
    MediaModel::ref().setPlayState(stat == "Playing");
}

void MediaController::removeMediaPath(const QString &path)
{
    m_mediaPaths.removeOne(path);

    if (m_path != path || !m_mediaPlayer2)
        return;

    if (!m_mediaPaths.isEmpty()) {
         loadMediaPath(m_mediaPaths.last());
         return;
    }

    m_mediaPlayer2->deleteLater();
    m_mediaPlayer2 = nullptr;

    Q_EMIT mediaLosted();
}

void MediaController::next()
{
    if (m_mediaPlayer2) {
        m_mediaPlayer2->Next();
    }
}

void MediaController::raise()
{
    if (!m_path.isEmpty()) {
        QDBusInterface inter(m_path, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", QDBusConnection::sessionBus());
        inter.asyncCall("Raise");
    }
}

void MediaController::pause()
{
    if (m_mediaPlayer2) {
        m_mediaPlayer2->Pause();
    }
}

void MediaController::play()
{
    if (m_mediaPlayer2) {
        m_mediaPlayer2->Play();
    }
}

bool MediaController::isWorking() const
{
    return m_mediaPlayer2 != nullptr;
}