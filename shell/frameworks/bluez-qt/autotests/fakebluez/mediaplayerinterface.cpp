/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediaplayerinterface.h"
#include "objectmanager.h"

#include <QDBusArgument>
#include <QDBusConnection>

// MediaPlayerObject
MediaPlayerObject::MediaPlayerObject(const QDBusObjectPath &path, QObject *parent)
    : QObject(parent)
{
    QDBusConnection::sessionBus().registerObject(path.path(), this);
}

// MediaPlayerInterface
MediaPlayerInterface::MediaPlayerInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setPath(path);
    setObjectParent(parent);
    setProperties(properties);
    setName(QStringLiteral("org.bluez.MediaPlayer1"));
}

QString MediaPlayerInterface::name() const
{
    return Object::property(QStringLiteral("Name")).toString();
}

QString MediaPlayerInterface::equalizer() const
{
    return Object::property(QStringLiteral("Equalizer")).toString();
}

void MediaPlayerInterface::setEqualizer(const QString &equalizer)
{
    Object::changeProperty(QStringLiteral("Equalizer"), equalizer);
}

QString MediaPlayerInterface::repeat() const
{
    return Object::property(QStringLiteral("Repeat")).toString();
}

void MediaPlayerInterface::setRepeat(const QString &repeat)
{
    Object::changeProperty(QStringLiteral("Repeat"), repeat);
}

QString MediaPlayerInterface::shuffle() const
{
    return Object::property(QStringLiteral("Shuffle")).toString();
}

void MediaPlayerInterface::setShuffle(const QString &shuffle)
{
    Object::changeProperty(QStringLiteral("Shuffle"), shuffle);
}

QString MediaPlayerInterface::status() const
{
    return Object::property(QStringLiteral("Status")).toString();
}

QVariantMap MediaPlayerInterface::track() const
{
    return qdbus_cast<QVariantMap>(Object::property(QStringLiteral("Track")));
}

quint32 MediaPlayerInterface::position() const
{
    return Object::property(QStringLiteral("Position")).toUInt();
}

QDBusObjectPath MediaPlayerInterface::device() const
{
    return Object::property(QStringLiteral("Device")).value<QDBusObjectPath>();
}

void MediaPlayerInterface::Play()
{
    Object::changeProperty(QStringLiteral("Status"), QStringLiteral("playing"));
}

void MediaPlayerInterface::Pause()
{
    Object::changeProperty(QStringLiteral("Status"), QStringLiteral("paused"));
}

void MediaPlayerInterface::Stop()
{
    Object::changeProperty(QStringLiteral("Status"), QStringLiteral("stopped"));
}

void MediaPlayerInterface::Next()
{
    QVariantMap t = track();
    t[QStringLiteral("Title")] = QVariant(t[QStringLiteral("Title")].toString() + QLatin1String("_next"));
    t[QStringLiteral("TrackNumber")] = t[QStringLiteral("TrackNumber")].toUInt() + 1;

    Object::changeProperty(QStringLiteral("Track"), t);
}

void MediaPlayerInterface::Previous()
{
    QVariantMap t = track();
    t[QStringLiteral("Title")] = QVariant(t[QStringLiteral("Title")].toString() + QLatin1String("_prev"));
    t[QStringLiteral("TrackNumber")] = t[QStringLiteral("TrackNumber")].toUInt() - 1;

    Object::changeProperty(QStringLiteral("Track"), t);
}

void MediaPlayerInterface::FastForward()
{
    Object::changeProperty(QStringLiteral("Status"), QStringLiteral("forward-seek"));
}

void MediaPlayerInterface::Rewind()
{
    Object::changeProperty(QStringLiteral("Status"), QStringLiteral("reverse-seek"));
}

#include "moc_mediaplayerinterface.cpp"
