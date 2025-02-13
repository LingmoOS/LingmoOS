/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef DECLARATIVEMEDIAPLAYER_H
#define DECLARATIVEMEDIAPLAYER_H

#include <QJsonObject>

#include <BluezQt/MediaPlayer>

class DeclarativeMediaPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(BluezQt::MediaPlayer::Equalizer equalizer READ equalizer WRITE setEqualizer NOTIFY equalizerChanged)
    Q_PROPERTY(BluezQt::MediaPlayer::Repeat repeat READ repeat WRITE setRepeat NOTIFY repeatChanged)
    Q_PROPERTY(BluezQt::MediaPlayer::Shuffle shuffle READ shuffle WRITE setShuffle NOTIFY shuffleChanged)
    Q_PROPERTY(BluezQt::MediaPlayer::Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QJsonObject track READ track NOTIFY trackChanged)
    Q_PROPERTY(quint32 position READ position NOTIFY positionChanged)

public:
    explicit DeclarativeMediaPlayer(BluezQt::MediaPlayerPtr mediaPlayer, QObject *parent = nullptr);

    QString name() const;

    BluezQt::MediaPlayer::Equalizer equalizer() const;
    void setEqualizer(BluezQt::MediaPlayer::Equalizer equalizer);

    BluezQt::MediaPlayer::Repeat repeat() const;
    void setRepeat(BluezQt::MediaPlayer::Repeat repeat);

    BluezQt::MediaPlayer::Shuffle shuffle() const;
    void setShuffle(BluezQt::MediaPlayer::Shuffle shuffle);

    BluezQt::MediaPlayer::Status status() const;

    QJsonObject track() const;

    quint32 position() const;

public Q_SLOTS:
    BluezQt::PendingCall *play();
    BluezQt::PendingCall *pause();
    BluezQt::PendingCall *stop();
    BluezQt::PendingCall *next();
    BluezQt::PendingCall *previous();
    BluezQt::PendingCall *fastForward();
    BluezQt::PendingCall *rewind();

Q_SIGNALS:
    void nameChanged(const QString &name);
    void equalizerChanged(BluezQt::MediaPlayer::Equalizer equalizer);
    void repeatChanged(BluezQt::MediaPlayer::Repeat repeat);
    void shuffleChanged(BluezQt::MediaPlayer::Shuffle shuffle);
    void statusChanged(BluezQt::MediaPlayer::Status status);
    void trackChanged(const QJsonObject &track);
    void positionChanged(quint32 position);

private:
    void updateTrack();

    BluezQt::MediaPlayerPtr m_mediaPlayer;
    QJsonObject m_track;
};

#endif // DECLARATIVEMEDIAPLAYER_H
