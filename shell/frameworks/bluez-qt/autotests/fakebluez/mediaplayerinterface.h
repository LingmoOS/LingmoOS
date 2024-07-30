/*
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef MEDIAPLAYERINTERFACE_H
#define MEDIAPLAYERINTERFACE_H

#include "object.h"

#include <QDBusAbstractAdaptor>

class QDBusObjectPath;

class MediaPlayerObject : public QObject
{
public:
    explicit MediaPlayerObject(const QDBusObjectPath &path, QObject *parent = nullptr);
};

class MediaPlayerInterface : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.MediaPlayer1")
    Q_PROPERTY(QString Name READ name)
    Q_PROPERTY(QString Equalizer READ equalizer WRITE setEqualizer)
    Q_PROPERTY(QString Repeat READ repeat WRITE setRepeat)
    Q_PROPERTY(QString Shuffle READ shuffle WRITE setShuffle)
    Q_PROPERTY(QString Status READ status)
    Q_PROPERTY(QVariantMap Track READ track)
    Q_PROPERTY(quint32 Position READ position)
    Q_PROPERTY(QDBusObjectPath Device READ device)

public:
    explicit MediaPlayerInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent = nullptr);

    QString name() const;

    QString equalizer() const;
    void setEqualizer(const QString &equalizer);

    QString repeat() const;
    void setRepeat(const QString &repeat);

    QString shuffle() const;
    void setShuffle(const QString &shuffle);

    QString status() const;

    QVariantMap track() const;

    quint32 position() const;

    QDBusObjectPath device() const;

public Q_SLOTS:
    void Play();
    void Pause();
    void Stop();
    void Next();
    void Previous();
    void FastForward();
    void Rewind();
};

#endif // MEDIAPLAYERINTERFACE_H
