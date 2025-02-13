/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "declarativemediaplayer.h"

DeclarativeMediaPlayer::DeclarativeMediaPlayer(BluezQt::MediaPlayerPtr mediaPlayer, QObject *parent)
    : QObject(parent)
    , m_mediaPlayer(mediaPlayer)
{
    connect(m_mediaPlayer.data(), &BluezQt::MediaPlayer::nameChanged, this, &DeclarativeMediaPlayer::nameChanged);
    connect(m_mediaPlayer.data(), &BluezQt::MediaPlayer::equalizerChanged, this, &DeclarativeMediaPlayer::equalizerChanged);
    connect(m_mediaPlayer.data(), &BluezQt::MediaPlayer::repeatChanged, this, &DeclarativeMediaPlayer::repeatChanged);
    connect(m_mediaPlayer.data(), &BluezQt::MediaPlayer::shuffleChanged, this, &DeclarativeMediaPlayer::shuffleChanged);
    connect(m_mediaPlayer.data(), &BluezQt::MediaPlayer::statusChanged, this, &DeclarativeMediaPlayer::statusChanged);
    connect(m_mediaPlayer.data(), &BluezQt::MediaPlayer::positionChanged, this, &DeclarativeMediaPlayer::positionChanged);

    connect(m_mediaPlayer.data(), &BluezQt::MediaPlayer::trackChanged, this, [this]() {
        updateTrack();
        Q_EMIT trackChanged(m_track);
    });

    updateTrack();
}

QString DeclarativeMediaPlayer::name() const
{
    return m_mediaPlayer->name();
}

BluezQt::MediaPlayer::Equalizer DeclarativeMediaPlayer::equalizer() const
{
    return m_mediaPlayer->equalizer();
}

void DeclarativeMediaPlayer::setEqualizer(BluezQt::MediaPlayer::Equalizer equalizer)
{
    m_mediaPlayer->setEqualizer(equalizer);
}

BluezQt::MediaPlayer::Repeat DeclarativeMediaPlayer::repeat() const
{
    return m_mediaPlayer->repeat();
}

void DeclarativeMediaPlayer::setRepeat(BluezQt::MediaPlayer::Repeat repeat)
{
    m_mediaPlayer->setRepeat(repeat);
}

BluezQt::MediaPlayer::Shuffle DeclarativeMediaPlayer::shuffle() const
{
    return m_mediaPlayer->shuffle();
}

void DeclarativeMediaPlayer::setShuffle(BluezQt::MediaPlayer::Shuffle shuffle)
{
    m_mediaPlayer->setShuffle(shuffle);
}

BluezQt::MediaPlayer::Status DeclarativeMediaPlayer::status() const
{
    return m_mediaPlayer->status();
}

QJsonObject DeclarativeMediaPlayer::track() const
{
    return m_track;
}

quint32 DeclarativeMediaPlayer::position() const
{
    return m_mediaPlayer->position();
}

BluezQt::PendingCall *DeclarativeMediaPlayer::play()
{
    return m_mediaPlayer->play();
}

BluezQt::PendingCall *DeclarativeMediaPlayer::pause()
{
    return m_mediaPlayer->pause();
}

BluezQt::PendingCall *DeclarativeMediaPlayer::stop()
{
    return m_mediaPlayer->stop();
}

BluezQt::PendingCall *DeclarativeMediaPlayer::next()
{
    return m_mediaPlayer->next();
}

BluezQt::PendingCall *DeclarativeMediaPlayer::previous()
{
    return m_mediaPlayer->previous();
}

BluezQt::PendingCall *DeclarativeMediaPlayer::fastForward()
{
    return m_mediaPlayer->fastForward();
}

BluezQt::PendingCall *DeclarativeMediaPlayer::rewind()
{
    return m_mediaPlayer->rewind();
}

void DeclarativeMediaPlayer::updateTrack()
{
    m_track[QStringLiteral("valid")] = m_mediaPlayer->track().isValid();
    m_track[QStringLiteral("title")] = m_mediaPlayer->track().title();
    m_track[QStringLiteral("artist")] = m_mediaPlayer->track().artist();
    m_track[QStringLiteral("album")] = m_mediaPlayer->track().album();
    m_track[QStringLiteral("genre")] = m_mediaPlayer->track().genre();
    m_track[QStringLiteral("numberOfTracks")] = qint64(m_mediaPlayer->track().numberOfTracks());
    m_track[QStringLiteral("trackNumber")] = qint64(m_mediaPlayer->track().trackNumber());
    m_track[QStringLiteral("duration")] = qint64(m_mediaPlayer->track().duration());
}
