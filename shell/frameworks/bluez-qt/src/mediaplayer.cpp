/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediaplayer.h"
#include "mediaplayer_p.h"
#include "pendingcall.h"

namespace BluezQt
{
static QString equalizerToString(MediaPlayer::Equalizer equalizer)
{
    switch (equalizer) {
    case MediaPlayer::EqualizerOn:
        return QStringLiteral("on");
    default:
        return QStringLiteral("off");
    }
}

static QString repeatToString(MediaPlayer::Repeat repeat)
{
    switch (repeat) {
    case MediaPlayer::RepeatSingleTrack:
        return QStringLiteral("singletrack");
    case MediaPlayer::RepeatAllTracks:
        return QStringLiteral("alltracks");
    case MediaPlayer::RepeatGroup:
        return QStringLiteral("group");
    default:
        return QStringLiteral("off");
    }
}

static QString shuffleToString(MediaPlayer::Shuffle shuffle)
{
    switch (shuffle) {
    case MediaPlayer::ShuffleAllTracks:
        return QStringLiteral("alltracks");
    case MediaPlayer::ShuffleGroup:
        return QStringLiteral("group");
    default:
        return QStringLiteral("off");
    }
}

MediaPlayer::MediaPlayer(const QString &path, const QVariantMap &properties)
    : QObject()
    , d(new MediaPlayerPrivate(path, properties))
{
}

MediaPlayer::~MediaPlayer() = default;

MediaPlayerPtr MediaPlayer::toSharedPtr() const
{
    return d->q.toStrongRef();
}

QString MediaPlayer::name() const
{
    return d->m_name;
}

MediaPlayer::Equalizer MediaPlayer::equalizer() const
{
    return d->m_equalizer;
}

PendingCall *MediaPlayer::setEqualizer(MediaPlayer::Equalizer equalizer)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Equalizer"), equalizerToString(equalizer)), PendingCall::ReturnVoid, this);
}

MediaPlayer::Repeat MediaPlayer::repeat() const
{
    return d->m_repeat;
}

PendingCall *MediaPlayer::setRepeat(MediaPlayer::Repeat repeat)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Repeat"), repeatToString(repeat)), PendingCall::ReturnVoid, this);
}

MediaPlayer::Shuffle MediaPlayer::shuffle() const
{
    return d->m_shuffle;
}

PendingCall *MediaPlayer::setShuffle(MediaPlayer::Shuffle shuffle)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Shuffle"), shuffleToString(shuffle)), PendingCall::ReturnVoid, this);
}

MediaPlayer::Status MediaPlayer::status() const
{
    return d->m_status;
}

MediaPlayerTrack MediaPlayer::track() const
{
    return d->m_track;
}

quint32 MediaPlayer::position() const
{
    return d->m_position;
}

PendingCall *MediaPlayer::play()
{
    return new PendingCall(d->m_bluezMediaPlayer->Play(), PendingCall::ReturnVoid, this);
}

PendingCall *MediaPlayer::pause()
{
    return new PendingCall(d->m_bluezMediaPlayer->Pause(), PendingCall::ReturnVoid, this);
}

PendingCall *MediaPlayer::stop()
{
    return new PendingCall(d->m_bluezMediaPlayer->Stop(), PendingCall::ReturnVoid, this);
}

PendingCall *MediaPlayer::next()
{
    return new PendingCall(d->m_bluezMediaPlayer->Next(), PendingCall::ReturnVoid, this);
}

PendingCall *MediaPlayer::previous()
{
    return new PendingCall(d->m_bluezMediaPlayer->Previous(), PendingCall::ReturnVoid, this);
}

PendingCall *MediaPlayer::fastForward()
{
    return new PendingCall(d->m_bluezMediaPlayer->FastForward(), PendingCall::ReturnVoid, this);
}

PendingCall *MediaPlayer::rewind()
{
    return new PendingCall(d->m_bluezMediaPlayer->Rewind(), PendingCall::ReturnVoid, this);
}

} // namespace BluezQt

#include "moc_mediaplayer.cpp"
