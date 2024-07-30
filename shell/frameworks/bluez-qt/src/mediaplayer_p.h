/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_MEDIAPLAYER_P_H
#define BLUEZQT_MEDIAPLAYER_P_H

#include <QObject>

#include "bluezmediaplayer1.h"
#include "dbusproperties.h"
#include "mediaplayer.h"

namespace BluezQt
{
typedef org::bluez::MediaPlayer1 BluezMediaPlayer;
typedef org::freedesktop::DBus::Properties DBusProperties;

class MediaPlayerPrivate : public QObject
{
    Q_OBJECT

public:
    explicit MediaPlayerPrivate(const QString &path, const QVariantMap &properties);

    void init(const QVariantMap &properties);

    QDBusPendingReply<> setDBusProperty(const QString &name, const QVariant &value);
    void propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

    MediaPlayerTrack variantToTrack(const QVariant &variant) const;

    QWeakPointer<MediaPlayer> q;
    BluezMediaPlayer *m_bluezMediaPlayer;
    DBusProperties *m_dbusProperties;

    QString m_path;
    QString m_name;
    MediaPlayer::Equalizer m_equalizer;
    MediaPlayer::Repeat m_repeat;
    MediaPlayer::Shuffle m_shuffle;
    MediaPlayer::Status m_status;
    MediaPlayerTrack m_track;
    quint32 m_position;
};

} // namespace BluezQt

#endif // BLUEZQT_MEDIAPLAYER_P_H
