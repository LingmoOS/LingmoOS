/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediaplayer_p.h"
#include "macros.h"
#include "utils.h"

namespace BluezQt
{
static MediaPlayer::Equalizer stringToEqualizer(const QString &equalizer)
{
    if (equalizer == QLatin1String("on")) {
        return MediaPlayer::EqualizerOn;
    }
    return MediaPlayer::EqualizerOff;
}

static MediaPlayer::Repeat stringToRepeat(const QString &repeat)
{
    if (repeat == QLatin1String("singletrack")) {
        return MediaPlayer::RepeatSingleTrack;
    } else if (repeat == QLatin1String("alltracks")) {
        return MediaPlayer::RepeatAllTracks;
    } else if (repeat == QLatin1String("group")) {
        return MediaPlayer::RepeatGroup;
    }
    return MediaPlayer::RepeatOff;
}

static MediaPlayer::Shuffle stringToShuffle(const QString &shuffle)
{
    if (shuffle == QLatin1String("alltracks")) {
        return MediaPlayer::ShuffleAllTracks;
    } else if (shuffle == QLatin1String("group")) {
        return MediaPlayer::ShuffleGroup;
    }
    return MediaPlayer::ShuffleOff;
}

static MediaPlayer::Status stringToStatus(const QString &status)
{
    if (status == QLatin1String("playing")) {
        return MediaPlayer::Playing;
    } else if (status == QLatin1String("stopped")) {
        return MediaPlayer::Stopped;
    } else if (status == QLatin1String("paused")) {
        return MediaPlayer::Paused;
    } else if (status == QLatin1String("forward-seek")) {
        return MediaPlayer::ForwardSeek;
    } else if (status == QLatin1String("reverse-seek")) {
        return MediaPlayer::ReverseSeek;
    }
    return MediaPlayer::Error;
}

MediaPlayerPrivate::MediaPlayerPrivate(const QString &path, const QVariantMap &properties)
    : QObject()
    , m_dbusProperties(nullptr)
    , m_path(path)
    , m_equalizer(MediaPlayer::EqualizerOff)
    , m_repeat(MediaPlayer::RepeatOff)
    , m_shuffle(MediaPlayer::ShuffleOff)
    , m_status(MediaPlayer::Error)
    , m_position(0)
{
    m_bluezMediaPlayer = new BluezMediaPlayer(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);

    init(properties);
}

void MediaPlayerPrivate::init(const QVariantMap &properties)
{
    m_dbusProperties = new DBusProperties(Strings::orgBluez(), m_bluezMediaPlayer->path(), DBusConnection::orgBluez(), this);

    // Init properties
    m_name = properties.value(QStringLiteral("Name")).toString();
    m_equalizer = stringToEqualizer(properties.value(QStringLiteral("Equalizer")).toString());
    m_repeat = stringToRepeat(properties.value(QStringLiteral("Repeat")).toString());
    m_shuffle = stringToShuffle(properties.value(QStringLiteral("Shuffle")).toString());
    m_status = stringToStatus(properties.value(QStringLiteral("Status")).toString());
    m_track = variantToTrack(properties.value(QStringLiteral("Track")));
    m_position = properties.value(QStringLiteral("Position")).toUInt();
}

QDBusPendingReply<> MediaPlayerPrivate::setDBusProperty(const QString &name, const QVariant &value)
{
    return m_dbusProperties->Set(Strings::orgBluezMediaPlayer1(), name, QDBusVariant(value));
}

void MediaPlayerPrivate::propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    if (interface != Strings::orgBluezMediaPlayer1()) {
        return;
    }

    QVariantMap::const_iterator i;
    for (i = changed.constBegin(); i != changed.constEnd(); ++i) {
        const QVariant &value = i.value();
        const QString &property = i.key();

        if (property == QLatin1String("Name")) {
            PROPERTY_CHANGED(m_name, toString, nameChanged);
        } else if (property == QLatin1String("Equalizer")) {
            PROPERTY_CHANGED2(m_equalizer, stringToEqualizer(value.toString()), equalizerChanged);
        } else if (property == QLatin1String("Repeat")) {
            PROPERTY_CHANGED2(m_repeat, stringToRepeat(value.toString()), repeatChanged);
        } else if (property == QLatin1String("Shuffle")) {
            PROPERTY_CHANGED2(m_shuffle, stringToShuffle(value.toString()), shuffleChanged);
        } else if (property == QLatin1String("Status")) {
            PROPERTY_CHANGED2(m_status, stringToStatus(value.toString()), statusChanged);
        } else if (property == QLatin1String("Position")) {
            PROPERTY_CHANGED(m_position, toUInt, positionChanged);
        } else if (property == QLatin1String("Track")) {
            m_track = variantToTrack(value);
            Q_EMIT q.lock()->trackChanged(m_track);
        }
    }

    for (const QString &property : invalidated) {
        if (property == QLatin1String("Name")) {
            PROPERTY_INVALIDATED(m_name, QString(), nameChanged);
        } else if (property == QLatin1String("Equalizer")) {
            PROPERTY_INVALIDATED(m_equalizer, MediaPlayer::EqualizerOff, equalizerChanged);
        } else if (property == QLatin1String("Repeat")) {
            PROPERTY_INVALIDATED(m_repeat, MediaPlayer::RepeatOff, repeatChanged);
        } else if (property == QLatin1String("Shuffle")) {
            PROPERTY_INVALIDATED(m_shuffle, MediaPlayer::ShuffleOff, shuffleChanged);
        } else if (property == QLatin1String("Status")) {
            PROPERTY_INVALIDATED(m_status, MediaPlayer::Error, statusChanged);
        } else if (property == QLatin1String("Position")) {
            PROPERTY_INVALIDATED(m_position, 0, positionChanged);
        } else if (property == QLatin1String("Track")) {
            m_track = variantToTrack(QVariant());
            Q_EMIT q.lock()->trackChanged(m_track);
        }
    }
}

MediaPlayerTrack MediaPlayerPrivate::variantToTrack(const QVariant &variant) const
{
    const QVariantMap &properties = qdbus_cast<QVariantMap>(variant);
    return MediaPlayerTrack(properties);
}

} // namespace BluezQt

#include "moc_mediaplayer_p.cpp"
