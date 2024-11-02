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

#include "player-item.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusPendingCall>
#include <QMimeDatabase>
#include <QUrl>
#include "dbusproperties.h"
#include "mprisplayer2player.h"
#include "mprisplayer2.h"

static const QString MPRIS2_PATH = QStringLiteral("/org/mpris/MediaPlayer2");
class MediaPlayer2Props
{
public:
    bool m_canQuit = false;
    bool m_fullScreen = false;
    bool m_canSetFullScreen = false;
    bool m_canRaise = false;
    bool m_hasTrackList = false;
    QString m_identity;
    QString m_desktopEntry;
    QStringList m_supportedUriSchemes;
    QStringList m_supportedMimeTypes;

};
class  MediaPlayer2PlayerProps
{
public:
    QString m_playbackStatus;
    QString m_loopStatus;
    double m_rate = 1;
    bool m_shuffle = false;
    QVariantMap m_metaData;
    double m_volume = 0;
    qint64 m_position = 0;
    double m_minimumRate = 0;
    double m_maximumRate = 0;
    bool m_canGoNext = true;
    bool m_canGoPrevious = true;
    bool m_canPlay = false;
    bool m_canPause = false;
    bool m_canSeek = false;
    bool m_canControl = false;
};
class PlayerItemPrivate: public QObject
{
    Q_OBJECT
public:
    PlayerItemPrivate(const QString &serviceName, QObject *parent = nullptr);

    void refresh();
    void updateMediaPlayer2Props(QDBusPendingCallWatcher *watcher);
    void updateMediaPlayer2PropsFromMap(const QVariantMap &map);
    void updateMediaPlayer2PlayerProps(QDBusPendingCallWatcher *watcher);
    void updateMediaPlayer2PlayerPropsFromMap(const QVariantMap &map);
    void propertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
    void onSeeked(qint64 position);

    OrgFreedesktopDBusPropertiesInterface *m_propsIface = nullptr;
    OrgMprisMediaPlayer2Interface *m_mprisIface = nullptr;
    OrgMprisMediaPlayer2PlayerInterface *m_mprisPlayerIface = nullptr;
    QString m_serviceName;
    uint m_pid = 0;
    bool m_mp2PropsReceived = false;
    bool m_mp2PlayerPropsReceived = false;
    MediaPlayer2Props m_mp2Props;
    MediaPlayer2PlayerProps m_mp2PlayerProps;
    bool m_valid = false;
    PlayerItem *q = nullptr;
};

PlayerItemPrivate::PlayerItemPrivate(const QString &serviceName, QObject *parent) : QObject(parent)
{
    if(serviceName.isEmpty() || !serviceName.startsWith(QLatin1String("org.mpris.MediaPlayer2."))) {
        qWarning() << "Invalid mpris2 service: " << serviceName;
        return;
    }
    q = qobject_cast<PlayerItem *>(parent);
    m_serviceName = serviceName;
    QDBusConnection conn = QDBusConnection::sessionBus();
    QDBusReply<uint> pidReply = conn.interface()->servicePid(serviceName);
    if(pidReply.isValid()) {
        m_pid = pidReply.value();
    }
    m_propsIface = new OrgFreedesktopDBusPropertiesInterface(serviceName, MPRIS2_PATH, conn, this);
    m_mprisIface = new OrgMprisMediaPlayer2Interface(serviceName, MPRIS2_PATH, conn, this);
    m_mprisPlayerIface = new OrgMprisMediaPlayer2PlayerInterface(serviceName, MPRIS2_PATH, conn, this);
    if(!m_propsIface->isValid() || !m_mprisIface->isValid() || !m_mprisPlayerIface->isValid()) {
        qWarning() << "Invalid mpris2 service: " << serviceName;
        return;
    }
    connect(m_propsIface, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged, this, &PlayerItemPrivate::propertiesChanged);
    connect(m_mprisPlayerIface, &OrgMprisMediaPlayer2PlayerInterface::Seeked, this, &PlayerItemPrivate::onSeeked);
    refresh();
    m_valid = true;
}

void PlayerItemPrivate::refresh()
{
    //fetch MediaPlayer2 properties
    QDBusPendingCall async = m_propsIface->GetAll(OrgMprisMediaPlayer2Interface::staticInterfaceName());
    auto watcher = new QDBusPendingCallWatcher(async, this);
    watcher->setProperty("fetch", true);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &PlayerItemPrivate::updateMediaPlayer2Props);
    //fetch MediaPlayer2.Player properties
    async = m_propsIface->GetAll(OrgMprisMediaPlayer2PlayerInterface::staticInterfaceName());
    watcher = new QDBusPendingCallWatcher(async, this);
    watcher->setProperty("fetch", true);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &PlayerItemPrivate::updateMediaPlayer2PlayerProps);
}

void PlayerItemPrivate::updateMediaPlayer2Props(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QVariantMap> propsReply = *watcher;
    watcher->deleteLater();
    //已收到更新信号后忽略主动查询返回
    bool fetch = watcher->property("fetch").toBool();
    if(m_mp2PropsReceived && fetch) {
        return;
    }

    if (propsReply.isError()) {
        qWarning() << m_serviceName << "update Media player2 prop error:" << "Error message:"
                   << propsReply.error().name() << propsReply.error().message();
        return;
    }
    updateMediaPlayer2PropsFromMap(propsReply.value());
    if(!fetch) {
        m_mp2PropsReceived = true;
    }
}

void PlayerItemPrivate::updateMediaPlayer2PropsFromMap(const QVariantMap &map)
{
    QVector<int> updateProperties;
    if(map.find(QStringLiteral("CanQuit")) != map.constEnd()) {
        m_mp2Props.m_canQuit = map.value(QStringLiteral("CanQuit")).toBool();
        updateProperties << MprisProperties::CanQuit;
    }
    if(map.find(QStringLiteral("Fullscreen")) != map.constEnd()) {
        m_mp2Props.m_fullScreen = map.value(QStringLiteral("Fullscreen")).toBool();
        updateProperties << MprisProperties::FullScreen;
    }
    if(map.find(QStringLiteral("CanSetFullscreen")) != map.constEnd()) {
        m_mp2Props.m_canSetFullScreen = map.value(QStringLiteral("CanSetFullscreen")).toBool();
        updateProperties << MprisProperties::CanSetFullScreen;
    }
    if(map.find(QStringLiteral("CanRaise")) != map.constEnd()) {
        m_mp2Props.m_canRaise = map.value(QStringLiteral("CanRaise")).toBool();
        updateProperties << MprisProperties::CanRaise;
    }
    if(map.find(QStringLiteral("HasTrackList")) != map.constEnd()) {
        m_mp2Props.m_hasTrackList = map.value(QStringLiteral("HasTrackList")).toBool();
        updateProperties << MprisProperties::HasTrackList;
    }
    if(map.find(QStringLiteral("Identity")) != map.constEnd()) {
        m_mp2Props.m_identity = map.value(QStringLiteral("Identity")).toString();
        updateProperties << MprisProperties::Identity;
    }
    if(map.find(QStringLiteral("DesktopEntry")) != map.constEnd()) {
        m_mp2Props.m_desktopEntry = map.value(QStringLiteral("DesktopEntry")).toString();
        updateProperties << MprisProperties::DesktopEntry;
    }
    if(map.find(QStringLiteral("SupportedUriSchemes")) != map.constEnd()) {
        m_mp2Props.m_supportedUriSchemes = map.value(QStringLiteral("SupportedUriSchemes")).toStringList();
        updateProperties << MprisProperties::SupportedUriSchemes;
    }
    if(map.find(QStringLiteral("SupportedMimeTypes")) != map.constEnd()) {
        m_mp2Props.m_supportedMimeTypes = map.value(QStringLiteral("SupportedMimeTypes")).toStringList();
        updateProperties << MprisProperties::SupportedMimeTypes;
    }
    if(!updateProperties.isEmpty()) {
        Q_EMIT q->dataChanged(m_serviceName, updateProperties);
    }
}

void PlayerItemPrivate::updateMediaPlayer2PlayerProps(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QVariantMap> propsReply = *watcher;
    watcher->deleteLater();
    bool fetch = watcher->property("fetch").toBool();
    if(m_mp2PlayerPropsReceived && fetch) {
        return;
    }

    if (propsReply.isError()) {
        qWarning() << m_serviceName << "update Media player2 player prop error:" << "Error message:"
                   << propsReply.error().name() << propsReply.error().message();
        return;
    }
    updateMediaPlayer2PlayerPropsFromMap(propsReply.value());
    if(!fetch) {
        m_mp2PlayerPropsReceived = true;
    }
}

void PlayerItemPrivate::updateMediaPlayer2PlayerPropsFromMap(const QVariantMap &map)
{
    QVector<int> updateProperties;
    if(map.find(QStringLiteral("PlaybackStatus")) != map.constEnd()) {
        m_mp2PlayerProps.m_playbackStatus = map.value(QStringLiteral("PlaybackStatus")).toString();
        updateProperties << MprisProperties::PlaybackStatus;
    }
    if(map.find(QStringLiteral("LoopStatus")) != map.constEnd()) {
        m_mp2PlayerProps.m_loopStatus = map.value(QStringLiteral("LoopStatus")).toString();
        updateProperties << MprisProperties::LoopStatus;
    }
    if(map.find(QStringLiteral("Rate")) != map.constEnd()) {
        m_mp2PlayerProps.m_rate = map.value(QStringLiteral("Rate")).toDouble();
        updateProperties << MprisProperties::Rate;
    }
    if(map.find(QStringLiteral("Shuffle")) != map.constEnd()) {
        m_mp2PlayerProps.m_shuffle = map.value(QStringLiteral("Shuffle")).toBool();
        updateProperties << MprisProperties::Shuffle;
    }
    if(map.find(QStringLiteral("Metadata")) != map.constEnd()) {
        QDBusArgument arg = map.value(QStringLiteral("Metadata")).value<QDBusArgument>();
        m_mp2PlayerProps.m_metaData.clear();
        arg >> m_mp2PlayerProps.m_metaData;
        updateProperties << MprisProperties::MetaData;
        updateProperties << MprisProperties::IsCurrentMediaVideo;
        updateProperties << MprisProperties::IsCurrentMediaAudio;
    }
    if(map.find(QStringLiteral("Volume")) != map.constEnd()) {
        m_mp2PlayerProps.m_volume = map.value(QStringLiteral("Volume")).toDouble();
        updateProperties << MprisProperties::Volume;
    }
    if(map.find(QStringLiteral("Position")) != map.constEnd()) {
        m_mp2PlayerProps.m_position = map.value(QStringLiteral("Position")).toLongLong();
        updateProperties << MprisProperties::Position;
    }
    if(map.find(QStringLiteral("MinimumRate")) != map.constEnd()) {
        m_mp2PlayerProps.m_minimumRate = map.value(QStringLiteral("MinimumRate")).toDouble();
        updateProperties << MprisProperties::MinimumRate;
    }
    if(map.find(QStringLiteral("MaximumRate")) != map.constEnd()) {
        m_mp2PlayerProps.m_maximumRate = map.value(QStringLiteral("MaximumRate")).toDouble();
        updateProperties << MprisProperties::MaximumRate;
    }
    if(map.find(QStringLiteral("CanGoNext")) != map.constEnd()) {
        m_mp2PlayerProps.m_canGoNext = map.value(QStringLiteral("CanGoNext")).toBool();
        updateProperties << MprisProperties::CanGoNext;
    }
    if(map.find(QStringLiteral("CanGoPrevious")) != map.constEnd()) {
        m_mp2PlayerProps.m_canGoPrevious = map.value(QStringLiteral("CanGoPrevious")).toBool();
        updateProperties << MprisProperties::CanGoPrevious;
    }
    if(map.find(QStringLiteral("CanPlay")) != map.constEnd()) {
        m_mp2PlayerProps.m_canPlay = map.value(QStringLiteral("CanPlay")).toBool();
        updateProperties << MprisProperties::CanPlay;
    }
    if(map.find(QStringLiteral("CanPause")) != map.constEnd()) {
        m_mp2PlayerProps.m_canPause = map.value(QStringLiteral("CanPause")).toBool();
        updateProperties << MprisProperties::CanPause;
    }
    if(map.find(QStringLiteral("CanSeek")) != map.constEnd()) {
        m_mp2PlayerProps.m_canSeek = map.value(QStringLiteral("CanSeek")).toBool();
        updateProperties << MprisProperties::CanSeek;
    }
    if(map.find(QStringLiteral("CanControl")) != map.constEnd()) {
        m_mp2PlayerProps.m_canControl = map.value(QStringLiteral("CanControl")).toBool();
        updateProperties << MprisProperties::CanControl;
    }
    if(!updateProperties.isEmpty()) {
        Q_EMIT q->dataChanged(m_serviceName, updateProperties);
    }
}

void PlayerItemPrivate::propertiesChanged(const QString &interface, const QVariantMap &changedProperties,
                                          const QStringList &invalidatedProperties)
{
    if(interface == OrgMprisMediaPlayer2Interface::staticInterfaceName()) {
        if(invalidatedProperties.isEmpty()) {
            updateMediaPlayer2PropsFromMap(changedProperties);
        } else {
            QDBusPendingCall async = m_propsIface->GetAll(OrgMprisMediaPlayer2Interface::staticInterfaceName());
            auto watcher = new QDBusPendingCallWatcher(async, this);
            connect(watcher, &QDBusPendingCallWatcher::finished, this, &PlayerItemPrivate::updateMediaPlayer2Props);
        }
    } else if (interface == OrgMprisMediaPlayer2PlayerInterface::staticInterfaceName()) {
        if(invalidatedProperties.isEmpty()) {
            updateMediaPlayer2PlayerPropsFromMap(changedProperties);
        } else {
            QDBusPendingCall async = m_propsIface->GetAll(OrgMprisMediaPlayer2PlayerInterface::staticInterfaceName());
            auto watcher = new QDBusPendingCallWatcher(async, this);
            connect(watcher, &QDBusPendingCallWatcher::finished, this, &PlayerItemPrivate::updateMediaPlayer2PlayerProps);
        }
    }
}

void PlayerItemPrivate::onSeeked(qint64 position)
{
    m_mp2PlayerProps.m_position = position;
    Q_EMIT q->dataChanged(m_serviceName, {MprisProperties::Position});

}

PlayerItem::PlayerItem(const QString &service, QObject *parent) : QObject(parent), d(new PlayerItemPrivate(service, this))
{
}

uint PlayerItem::pid()
{
    return d->m_pid;
}

bool PlayerItem::canQuit()
{
    return d->m_mp2Props.m_canQuit;
}

bool PlayerItem::fullScreen()
{
    return d->m_mp2Props.m_fullScreen;
}

bool PlayerItem::canSetFullScreen()
{
    return d->m_mp2Props.m_canSetFullScreen;
}

bool PlayerItem::canRaise()
{
    return d->m_mp2Props.m_canRaise;
}

bool PlayerItem::hasTrackList()
{
    return d->m_mp2Props.m_hasTrackList;
}

QString PlayerItem::identity()
{
    return d->m_mp2Props.m_identity;
}

QString PlayerItem::desktopEntry()
{
    return d->m_mp2Props.m_desktopEntry;
}

QStringList PlayerItem::supportedUriSchemes()
{
    return d->m_mp2Props.m_supportedUriSchemes;
}

QStringList PlayerItem::supportedMimeTypes()
{
    return d->m_mp2Props.m_supportedMimeTypes;
}

void PlayerItem::raise()
{
    d->m_mprisIface->Raise();
}

void PlayerItem::quit()
{
    d->m_mprisIface->Quit();
}

QString PlayerItem::playbackStatus()
{
    return d->m_mp2PlayerProps.m_playbackStatus;
}

QString PlayerItem::loopStatus()
{
    return d->m_mp2PlayerProps.m_loopStatus;
}

double PlayerItem::rate()
{
    return d->m_mp2PlayerProps.m_rate;
}

bool PlayerItem::shuffle()
{
    return d->m_mp2PlayerProps.m_shuffle;
}

QVariantMap PlayerItem::metaData()
{
    return d->m_mp2PlayerProps.m_metaData;
}

bool PlayerItem::isCurrentMediaVideo()
{
    QUrl url(d->m_mp2PlayerProps.m_metaData.value("xesam:url").toString());
    QMimeDatabase mdb;
    return mdb.mimeTypeForUrl(url).name().startsWith("video");
}

bool PlayerItem::isCurrentMediaAudio()
{
    QUrl url(d->m_mp2PlayerProps.m_metaData.value("xesam:url").toUrl());
    QMimeDatabase mdb;
    return mdb.mimeTypeForUrl(url).name().startsWith("audio");
}

double PlayerItem::volume()
{
    return d->m_mp2PlayerProps.m_volume;
}

quint64 PlayerItem::position()
{
    return d->m_mp2PlayerProps.m_position;
}

double PlayerItem::minimumRate()
{
    return d->m_mp2PlayerProps.m_minimumRate;
}

double PlayerItem::maximumRate()
{
    return d->m_mp2PlayerProps.m_maximumRate;
}

bool PlayerItem::canGoNext()
{
    return d->m_mp2PlayerProps.m_canGoNext;
}

bool PlayerItem::canGoPrevious()
{
    return d->m_mp2PlayerProps.m_canGoPrevious;
}

bool PlayerItem::canPlay()
{
    return d->m_mp2PlayerProps.m_canPlay;
}

bool PlayerItem::canPause()
{
    return d->m_mp2PlayerProps.m_canPause;
}

bool PlayerItem::canSeek()
{
    return d->m_mp2PlayerProps.m_canSeek;
}

bool PlayerItem::canControl()
{
    return d->m_mp2PlayerProps.m_canControl;
}

void PlayerItem::next()
{
    d->m_mprisPlayerIface->Next();
}

void PlayerItem::previous()
{
    d->m_mprisPlayerIface->Previous();
}

void PlayerItem::pause()
{
    d->m_mprisPlayerIface->Pause();
}

void PlayerItem::playPause()
{
    d->m_mprisPlayerIface->PlayPause();
}

void PlayerItem::stop()
{
    d->m_mprisPlayerIface->Stop();
}

void PlayerItem::play()
{
    d->m_mprisPlayerIface->Play();
}

void PlayerItem::seek(qint64 offset)
{
    d->m_mprisPlayerIface->Seek(offset);
}

void PlayerItem::setPosition(const QString &trackID, qint64 position)
{
    d->m_mprisPlayerIface->SetPosition(QDBusObjectPath(trackID), position);
}

void PlayerItem::openUri(const QString &uri)
{
    d->m_mprisPlayerIface->OpenUri(uri);
}

bool PlayerItem::valid()
{
    return d->m_valid;
}

#include "player-item.moc"
