/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_MEDIAPLAYERTRACK_H
#define BLUEZQT_MEDIAPLAYERTRACK_H

#include <QSharedPointer>
#include <QString>

#include "bluezqt_export.h"

namespace BluezQt
{
/**
 * @class BluezQt::MediaPlayerTrack mediaplayertrack.h <BluezQt/MediaPlayerTrack>
 *
 * Media player track.
 *
 * This class represents a track in media player.
 */
class BLUEZQT_EXPORT MediaPlayerTrack
{
public:
    /**
     * Creates a new invalid MediaPlayerTrack object.
     */
    explicit MediaPlayerTrack();

    /**
     * Destroys a MediaPlayerTrack object.
     */
    virtual ~MediaPlayerTrack();

    /**
     * Copy constructor.
     *
     * @param other
     */
    MediaPlayerTrack(const MediaPlayerTrack &other);

    /**
     * Copy assignment operator.
     *
     * @param other
     */
    MediaPlayerTrack &operator=(const MediaPlayerTrack &other);

    /**
     * Returns whether the track is valid.
     *
     * @return true if track is valid
     */
    bool isValid() const;

    /**
     * Returns a title of the track.
     *
     * @return track title
     */
    QString title() const;

    /**
     * Returns an artist of the track.
     *
     * @return track artist
     */
    QString artist() const;

    /**
     * Returns an album of the track.
     *
     * @return track album
     */
    QString album() const;

    /**
     * Returns a genre of the track.
     *
     * @return track genre
     */
    QString genre() const;

    /**
     * Returns a total number of tracks.
     *
     * @return total number of tracks
     */
    quint32 numberOfTracks() const;

    /**
     * Returns a track number of the track.
     *
     * @return track number
     */
    quint32 trackNumber() const;

    /**
     * Returns a duration of the track.
     *
     * @return track duration
     */
    quint32 duration() const;

private:
    BLUEZQT_NO_EXPORT explicit MediaPlayerTrack(const QVariantMap &properties);

    QSharedPointer<class MediaPlayerTrackPrivate> d;

    friend class MediaPlayerPrivate;
};

} // namespace BluezQt

Q_DECLARE_METATYPE(BluezQt::MediaPlayerTrack)

#endif // BLUEZQT_MEDIAPLAYERTRACK_H
