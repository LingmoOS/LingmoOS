/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_MEDIAPLAYER_H
#define BLUEZQT_MEDIAPLAYER_H

#include <QObject>

#include "bluezqt_export.h"
#include "mediaplayertrack.h"
#include "types.h"

#include <memory>

namespace BluezQt
{
class PendingCall;

/**
 * @class BluezQt::MediaPlayer mediaplayer.h <BluezQt/MediaPlayer>
 *
 * Media player.
 *
 * This class represents a media player interface.
 */
class BLUEZQT_EXPORT MediaPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(Equalizer equalizer READ equalizer WRITE setEqualizer NOTIFY equalizerChanged)
    Q_PROPERTY(Repeat repeat READ repeat WRITE setRepeat NOTIFY repeatChanged)
    Q_PROPERTY(Shuffle shuffle READ shuffle WRITE setShuffle NOTIFY shuffleChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(MediaPlayerTrack track READ track NOTIFY trackChanged)
    Q_PROPERTY(quint32 position READ position NOTIFY positionChanged)

public:
    /** Equalizer state. */
    enum Equalizer {
        /** Equalizer on. */
        EqualizerOn,
        /** Equalizer off. */
        EqualizerOff,
    };
    Q_ENUM(Equalizer)

    /** Repeat state. */
    enum Repeat {
        /** Repeat off. */
        RepeatOff,
        /** Repeat single track. */
        RepeatSingleTrack,
        /** Repeat all tracks. */
        RepeatAllTracks,
        /** Repeat group. */
        RepeatGroup,
    };
    Q_ENUM(Repeat)

    /** Shuffle state. */
    enum Shuffle {
        /** Shuffle off. */
        ShuffleOff,
        /** Shuffle all tracks. */
        ShuffleAllTracks,
        /** Shuffle group. */
        ShuffleGroup,
    };
    Q_ENUM(Shuffle)

    /** Player status. */
    enum Status {
        /** Player is playing. */
        Playing,
        /** Player is stopped. */
        Stopped,
        /** Player is paused. */
        Paused,
        /** Player is forward seeking. */
        ForwardSeek,
        /** Player is reverse seeking. */
        ReverseSeek,
        /** Error */
        Error,
    };
    Q_ENUM(Status)

    /**
     * Destroys a MediaPlayer object.
     */
    ~MediaPlayer() override;

    /**
     * Returns a shared pointer from this.
     *
     * @return MediaPlayerPtr
     */
    MediaPlayerPtr toSharedPtr() const;

    /**
     * Returns the name of the player.
     *
     * @return name of player
     */
    QString name() const;

    /**
     * Returns the equalizer state of the player.
     *
     * @return equalizer state of player
     */
    Equalizer equalizer() const;

    /**
     * Sets the equalizer state of the player.
     *
     * @param equalizer equalizer state
     * @return void pending call
     */
    PendingCall *setEqualizer(Equalizer equalizer);

    /**
     * Returns the repeat state of the player.
     *
     * @return repeat state of player
     */
    Repeat repeat() const;

    /**
     * Sets the repeat state of the player.
     *
     * @param repeat repeat state
     * @return void pending call
     */
    PendingCall *setRepeat(Repeat repeat);

    /**
     * Returns the shuffle state of the player.
     *
     * @return shuffle state of player
     */
    Shuffle shuffle() const;

    /**
     * Sets the shuffle state of the player.
     *
     * @param shuffle shuffle state
     * @return void pending call
     */
    PendingCall *setShuffle(Shuffle shuffle);

    /**
     * Returns the status of the player.
     *
     * @return status of player
     */
    Status status() const;

    /**
     * Returns the current track.
     *
     * @return current track
     */
    MediaPlayerTrack track() const;

    /**
     * Returns the playback position in milliseconds.
     *
     * @return playback position
     */
    quint32 position() const;

public Q_SLOTS:
    /**
     * Resumes playback.
     *
     * Possible errors: PendingCall::NotSupported, PendingCall::Failed
     *
     * @return void pending call
     */
    PendingCall *play();

    /**
     * Pauses playback.
     *
     * Possible errors: PendingCall::NotSupported, PendingCall::Failed
     *
     * @return void pending call
     */
    PendingCall *pause();

    /**
     * Stops playback.
     *
     * Possible errors: PendingCall::NotSupported, PendingCall::Failed
     *
     * @return void pending call
     */
    PendingCall *stop();

    /**
     * Switch to next track.
     *
     * Possible errors: PendingCall::NotSupported, PendingCall::Failed
     *
     * @return void pending call
     */
    PendingCall *next();

    /**
     * Switch to previous track.
     *
     * Possible errors: PendingCall::NotSupported, PendingCall::Failed
     *
     * @return void pending call
     */
    PendingCall *previous();

    /**
     * Fast forwards playback.
     *
     * Possible errors: PendingCall::NotSupported, PendingCall::Failed
     *
     * @return void pending call
     */
    PendingCall *fastForward();

    /**
     * Rewinds playback.
     *
     * Possible errors: PendingCall::NotSupported, PendingCall::Failed
     *
     * @return void pending call
     */
    PendingCall *rewind();

Q_SIGNALS:
    /**
     * Indicates that player's name have changed.
     */
    void nameChanged(const QString &name);

    /**
     * Indicates that player's equalizer state have changed.
     */
    void equalizerChanged(Equalizer equalizer);

    /**
     * Indicates that player's repeat state have changed.
     */
    void repeatChanged(Repeat repeat);

    /**
     * Indicates that player's shuffle state have changed.
     */
    void shuffleChanged(Shuffle shuffle);

    /**
     * Indicates that player's status have changed.
     */
    void statusChanged(Status status);

    /**
     * Indicates that player's current track have changed.
     */
    void trackChanged(MediaPlayerTrack track);

    /**
     * Indicates that player's playback position have changed.
     */
    void positionChanged(quint32 position);

private:
    BLUEZQT_NO_EXPORT explicit MediaPlayer(const QString &path, const QVariantMap &properties);

    std::unique_ptr<class MediaPlayerPrivate> const d;

    friend class MediaPlayerPrivate;
    friend class DevicePrivate;
};

} // namespace BluezQt

#endif // BLUEZQT_MEDIAPLAYER_H
