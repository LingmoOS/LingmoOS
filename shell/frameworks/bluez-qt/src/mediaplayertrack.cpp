/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediaplayertrack.h"

#include <QVariant>

namespace BluezQt
{
class MediaPlayerTrackPrivate
{
public:
    bool m_valid;
    QString m_title;
    QString m_artist;
    QString m_album;
    QString m_genre;
    quint32 m_numberOfTracks;
    quint32 m_trackNumber;
    quint32 m_duration;
};

MediaPlayerTrack::MediaPlayerTrack()
    : d(new MediaPlayerTrackPrivate)
{
    d->m_valid = false;
    d->m_numberOfTracks = 0;
    d->m_trackNumber = 0;
    d->m_duration = 0;
}

MediaPlayerTrack::MediaPlayerTrack(const QVariantMap &properties)
    : d(new MediaPlayerTrackPrivate)
{
    d->m_valid = !properties.isEmpty();
    d->m_title = properties.value(QStringLiteral("Title")).toString();
    d->m_artist = properties.value(QStringLiteral("Artist")).toString();
    d->m_album = properties.value(QStringLiteral("Album")).toString();
    d->m_genre = properties.value(QStringLiteral("Genre")).toString();
    d->m_numberOfTracks = properties.value(QStringLiteral("NumberOfTracks")).toUInt();
    d->m_trackNumber = properties.value(QStringLiteral("TrackNumber")).toUInt();
    d->m_duration = properties.value(QStringLiteral("Duration")).toUInt();
}

MediaPlayerTrack::~MediaPlayerTrack()
{
}

MediaPlayerTrack::MediaPlayerTrack(const MediaPlayerTrack &other)
    : d(other.d)
{
}

MediaPlayerTrack &MediaPlayerTrack::operator=(const MediaPlayerTrack &other)
{
    if (d != other.d) {
        d = other.d;
    }
    return *this;
}

bool MediaPlayerTrack::isValid() const
{
    return d->m_valid;
}

QString MediaPlayerTrack::title() const
{
    return d->m_title;
}

QString MediaPlayerTrack::artist() const
{
    return d->m_artist;
}

QString MediaPlayerTrack::album() const
{
    return d->m_album;
}

QString MediaPlayerTrack::genre() const
{
    return d->m_genre;
}

quint32 MediaPlayerTrack::numberOfTracks() const
{
    return d->m_numberOfTracks;
}

quint32 MediaPlayerTrack::trackNumber() const
{
    return d->m_trackNumber;
}

quint32 MediaPlayerTrack::duration() const
{
    return d->m_duration;
}

} // namespace BluezQt
