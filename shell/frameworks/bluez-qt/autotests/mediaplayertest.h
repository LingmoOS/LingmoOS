/*
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef MEDIAPLAYERTEST_H
#define MEDIAPLAYERTEST_H

#include <QList>
#include <QObject>

#include "bluezdevice1_tst.h"
#include "bluezmediaplayer1_tst.h"
#include "dbusproperties_tst.h"

#include "device.h"
#include "manager.h"
#include "mediaplayer.h"

class MediaPlayerTest : public QObject
{
    Q_OBJECT

public:
    explicit MediaPlayerTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void connectTest();
    void disconnectTest();
    void connectProfileTest();

    void getPropertiesTest();
    void setEqualizerTest();
    void setRepeatTest();
    void setShuffleTest();
    void changeStatusTest();
    void changeTrackTest();

    void disconnectProfileTest();

    void bug403289();

private:
    QString equalizerString(const BluezQt::MediaPlayerPtr &mediaPlayer) const;
    QString repeatString(const BluezQt::MediaPlayerPtr &mediaPlayer) const;
    QString shuffleString(const BluezQt::MediaPlayerPtr &mediaPlayer) const;
    QString statusString(const BluezQt::MediaPlayerPtr &mediaPlayer) const;
    QVariantMap trackMap(const BluezQt::MediaPlayerPtr &mediaPlayer) const;

    struct MediaPlayerUnit {
        BluezQt::DevicePtr device;
        org::bluez::MediaPlayer1 *dbusMediaPlayer;
        org::freedesktop::DBus::Properties *dbusProperties;
    };

    BluezQt::Manager *m_manager;
    QList<MediaPlayerUnit> m_units;
};

#endif // MEDIAPLAYERTEST_H
