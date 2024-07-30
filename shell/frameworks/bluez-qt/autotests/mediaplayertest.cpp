/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediaplayertest.h"
#include "autotests.h"
#include "initmanagerjob.h"
#include "pendingcall.h"
#include "services.h"

#include <QSignalSpy>
#include <QTest>

namespace BluezQt
{
extern void bluezqt_initFakeBluezTestRun();
}

using namespace BluezQt;

Q_DECLARE_METATYPE(MediaPlayer::Equalizer)
Q_DECLARE_METATYPE(MediaPlayer::Repeat)
Q_DECLARE_METATYPE(MediaPlayer::Shuffle)
Q_DECLARE_METATYPE(MediaPlayer::Status)

MediaPlayerTest::MediaPlayerTest()
    : m_manager(nullptr)
{
    Autotests::registerMetatypes();

    qRegisterMetaType<MediaPlayer::Equalizer>("Equalizer");
    qRegisterMetaType<MediaPlayer::Repeat>("Repeat");
    qRegisterMetaType<MediaPlayer::Shuffle>("Shuffle");
    qRegisterMetaType<MediaPlayer::Status>("Status");
    qRegisterMetaType<MediaPlayerTrack>("MediaPlayerTrack");
}

void MediaPlayerTest::initTestCase()
{
    bluezqt_initFakeBluezTestRun();

    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-standard"));

    // Create adapter
    QString adapter = QStringLiteral("/org/bluez/hci0");
    QVariantMap adapterProps;
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(adapter));
    adapterProps[QStringLiteral("Address")] = QStringLiteral("1C:E5:C3:BC:94:7E");
    adapterProps[QStringLiteral("Name")] = QStringLiteral("TestAdapter");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-adapter"), adapterProps);

    // Create devices
    QVariantMap deviceProps;
    QVariantMap mediaPlayerProps;
    QVariantMap trackProps;

    QString device1 = adapter + QLatin1String("/dev_40_79_6A_0C_39_75");
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(device1));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(QDBusObjectPath(adapter));
    deviceProps[QStringLiteral("Address")] = QStringLiteral("40:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice");
    deviceProps[QStringLiteral("UUIDs")] = QStringList(QStringLiteral("0000110E-0000-1000-8000-00805F9B34FB"));
    mediaPlayerProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(device1 + QLatin1String("/player0")));
    mediaPlayerProps[QStringLiteral("Name")] = QStringLiteral("Player1");
    mediaPlayerProps[QStringLiteral("Equalizer")] = QStringLiteral("on");
    mediaPlayerProps[QStringLiteral("Repeat")] = QStringLiteral("singletrack");
    mediaPlayerProps[QStringLiteral("Shuffle")] = QStringLiteral("group");
    mediaPlayerProps[QStringLiteral("Status")] = QStringLiteral("stopped");
    mediaPlayerProps[QStringLiteral("Position")] = QVariant::fromValue(quint32(150));
    mediaPlayerProps[QStringLiteral("Device")] = QVariant::fromValue(QDBusObjectPath(device1));
    trackProps[QStringLiteral("Title")] = QStringLiteral("Song1");
    trackProps[QStringLiteral("Artist")] = QStringLiteral("Band1");
    trackProps[QStringLiteral("Album")] = QStringLiteral("First Album");
    trackProps[QStringLiteral("Genre")] = QStringLiteral("Rock");
    trackProps[QStringLiteral("NumberOfTracks")] = 24;
    trackProps[QStringLiteral("TrackNumber")] = 4;
    trackProps[QStringLiteral("Duration")] = 12403;
    mediaPlayerProps[QStringLiteral("Track")] = trackProps;
    deviceProps[QStringLiteral("MediaPlayer")] = mediaPlayerProps;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    QString device2 = adapter + QLatin1String("/dev_50_79_6A_0C_39_75");
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(device2));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(QDBusObjectPath(adapter));
    deviceProps[QStringLiteral("Address")] = QStringLiteral("50:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice2");
    deviceProps[QStringLiteral("UUIDs")] = QStringList(QStringLiteral("0000110E-0000-1000-8000-00805F9B34FB"));
    mediaPlayerProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(device2 + QLatin1String("/player0")));
    mediaPlayerProps[QStringLiteral("Name")] = QStringLiteral("Player2");
    mediaPlayerProps[QStringLiteral("Equalizer")] = QStringLiteral("off");
    mediaPlayerProps[QStringLiteral("Repeat")] = QStringLiteral("alltracks");
    mediaPlayerProps[QStringLiteral("Shuffle")] = QStringLiteral("off");
    mediaPlayerProps[QStringLiteral("Status")] = QStringLiteral("paused");
    mediaPlayerProps[QStringLiteral("Position")] = QVariant::fromValue(quint32(2500));
    mediaPlayerProps[QStringLiteral("Device")] = QVariant::fromValue(QDBusObjectPath(device2));
    trackProps[QStringLiteral("Title")] = QStringLiteral("Song2");
    trackProps[QStringLiteral("Artist")] = QStringLiteral("Band2");
    trackProps[QStringLiteral("Album")] = QStringLiteral("Second Album");
    trackProps[QStringLiteral("Genre")] = QStringLiteral("Heavy Metal");
    trackProps[QStringLiteral("NumberOfTracks")] = 666;
    trackProps[QStringLiteral("TrackNumber")] = 22;
    trackProps[QStringLiteral("Duration")] = 22403;
    mediaPlayerProps[QStringLiteral("Track")] = trackProps;
    deviceProps[QStringLiteral("MediaPlayer")] = mediaPlayerProps;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    m_manager = new Manager();
    InitManagerJob *initJob = m_manager->init();
    initJob->exec();
    QVERIFY(!initJob->error());

    QCOMPARE(m_manager->adapters().count(), 1);
    QCOMPARE(m_manager->devices().count(), 2);
}

void MediaPlayerTest::cleanupTestCase()
{
    for (const MediaPlayerUnit &unit : m_units) {
        delete unit.dbusMediaPlayer;
        delete unit.dbusProperties;
    }

    delete m_manager;

    FakeBluez::stop();
}

void MediaPlayerTest::connectTest()
{
    for (DevicePtr device : m_manager->devices()) {
        QVERIFY(!device->ubi().isEmpty());
        QVERIFY(!device->mediaPlayer());

        QSignalSpy deviceSpy(device.data(), SIGNAL(mediaPlayerChanged(MediaPlayerPtr)));

        device->connectToDevice();
        QTRY_COMPARE(deviceSpy.count(), 1);
        QVERIFY(device->mediaPlayer());
    }
}

void MediaPlayerTest::disconnectTest()
{
    for (DevicePtr device : m_manager->devices()) {
        QVERIFY(device->mediaPlayer());

        QSignalSpy deviceSpy(device.data(), SIGNAL(mediaPlayerChanged(MediaPlayerPtr)));

        device->disconnectFromDevice();
        QTRY_COMPARE(deviceSpy.count(), 1);
        QVERIFY(!device->mediaPlayer());
    }
}

void MediaPlayerTest::connectProfileTest()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    QString service = QStringLiteral("org.kde.bluezqt.fakebluez");

    for (DevicePtr device : m_manager->devices()) {
        QVERIFY(!device->ubi().isEmpty());
        QVERIFY(!device->mediaPlayer());

        QSignalSpy deviceSpy(device.data(), SIGNAL(mediaPlayerChanged(MediaPlayerPtr)));

        device->connectProfile(Services::AudioVideoRemoteControl);
        QTRY_COMPARE(deviceSpy.count(), 1);
        QVERIFY(device->mediaPlayer());

        QString path = device->ubi() + QLatin1String("/player0");

        MediaPlayerUnit u;
        u.device = device;
        u.dbusMediaPlayer = new org::bluez::MediaPlayer1(service, path, connection, this);
        u.dbusProperties = new org::freedesktop::DBus::Properties(service, path, connection, this);
        m_units.append(u);
    }
}

void MediaPlayerTest::getPropertiesTest()
{
    for (const MediaPlayerUnit &unit : m_units) {
        QCOMPARE(unit.device->mediaPlayer()->name(), unit.dbusMediaPlayer->name());
        QCOMPARE(equalizerString(unit.device->mediaPlayer()), unit.dbusMediaPlayer->equalizer());
        QCOMPARE(repeatString(unit.device->mediaPlayer()), unit.dbusMediaPlayer->repeat());
        QCOMPARE(shuffleString(unit.device->mediaPlayer()), unit.dbusMediaPlayer->shuffle());
        QCOMPARE(statusString(unit.device->mediaPlayer()), unit.dbusMediaPlayer->status());
        QCOMPARE(trackMap(unit.device->mediaPlayer()), unit.dbusMediaPlayer->track());
        QCOMPARE(unit.device->mediaPlayer()->position(), unit.dbusMediaPlayer->position());
    }
}

void MediaPlayerTest::setEqualizerTest()
{
    for (const MediaPlayerUnit &unit : m_units) {
        MediaPlayerPtr mediaPlayer = unit.device->mediaPlayer();
        MediaPlayer::Equalizer value = mediaPlayer->equalizer() == MediaPlayer::EqualizerOff ? MediaPlayer::EqualizerOn : MediaPlayer::EqualizerOff;

        QSignalSpy equalizerSpy(mediaPlayer.data(), SIGNAL(equalizerChanged(Equalizer)));

        mediaPlayer->setEqualizer(value);
        QTRY_COMPARE(equalizerSpy.count(), 1);
        QCOMPARE(equalizerSpy.at(0).at(0).value<MediaPlayer::Equalizer>(), value);
        QCOMPARE(mediaPlayer->equalizer(), value);
    }
}

void MediaPlayerTest::setRepeatTest()
{
    for (const MediaPlayerUnit &unit : m_units) {
        MediaPlayerPtr mediaPlayer = unit.device->mediaPlayer();
        MediaPlayer::Repeat value = mediaPlayer->repeat() == MediaPlayer::RepeatGroup ? MediaPlayer::RepeatSingleTrack : MediaPlayer::RepeatGroup;

        QSignalSpy equalizerSpy(mediaPlayer.data(), SIGNAL(repeatChanged(Repeat)));

        mediaPlayer->setRepeat(value);
        QTRY_COMPARE(equalizerSpy.count(), 1);
        QCOMPARE(equalizerSpy.at(0).at(0).value<MediaPlayer::Repeat>(), value);
        QCOMPARE(mediaPlayer->repeat(), value);
    }
}

void MediaPlayerTest::setShuffleTest()
{
    for (const MediaPlayerUnit &unit : m_units) {
        MediaPlayerPtr mediaPlayer = unit.device->mediaPlayer();
        MediaPlayer::Shuffle value = mediaPlayer->shuffle() == MediaPlayer::ShuffleAllTracks ? MediaPlayer::ShuffleOff : MediaPlayer::ShuffleAllTracks;

        QSignalSpy equalizerSpy(mediaPlayer.data(), SIGNAL(shuffleChanged(Shuffle)));

        mediaPlayer->setShuffle(value);
        QTRY_COMPARE(equalizerSpy.count(), 1);
        QCOMPARE(equalizerSpy.at(0).at(0).value<MediaPlayer::Shuffle>(), value);
        QCOMPARE(mediaPlayer->shuffle(), value);
    }
}

void MediaPlayerTest::changeStatusTest()
{
    for (const MediaPlayerUnit &unit : m_units) {
        MediaPlayerPtr mediaPlayer = unit.device->mediaPlayer();

        QSignalSpy statusSpy(mediaPlayer.data(), SIGNAL(statusChanged(Status)));

        if (mediaPlayer->status() != MediaPlayer::Stopped) {
            mediaPlayer->stop();
            QTRY_COMPARE(statusSpy.count(), 1);
            QCOMPARE(statusSpy.at(0).at(0).value<MediaPlayer::Status>(), MediaPlayer::Stopped);
            QCOMPARE(mediaPlayer->status(), MediaPlayer::Stopped);
        }

        statusSpy.clear();
        mediaPlayer->play();
        QTRY_COMPARE(statusSpy.count(), 1);
        QCOMPARE(statusSpy.at(0).at(0).value<MediaPlayer::Status>(), MediaPlayer::Playing);
        QCOMPARE(mediaPlayer->status(), MediaPlayer::Playing);

        statusSpy.clear();
        mediaPlayer->pause();
        QTRY_COMPARE(statusSpy.count(), 1);
        QCOMPARE(statusSpy.at(0).at(0).value<MediaPlayer::Status>(), MediaPlayer::Paused);
        QCOMPARE(mediaPlayer->status(), MediaPlayer::Paused);

        statusSpy.clear();
        mediaPlayer->fastForward();
        QTRY_COMPARE(statusSpy.count(), 1);
        QCOMPARE(statusSpy.at(0).at(0).value<MediaPlayer::Status>(), MediaPlayer::ForwardSeek);
        QCOMPARE(mediaPlayer->status(), MediaPlayer::ForwardSeek);

        statusSpy.clear();
        mediaPlayer->rewind();
        QTRY_COMPARE(statusSpy.count(), 1);
        QCOMPARE(statusSpy.at(0).at(0).value<MediaPlayer::Status>(), MediaPlayer::ReverseSeek);
        QCOMPARE(mediaPlayer->status(), MediaPlayer::ReverseSeek);

        statusSpy.clear();
        mediaPlayer->stop();
        QTRY_COMPARE(statusSpy.count(), 1);
        QCOMPARE(statusSpy.at(0).at(0).value<MediaPlayer::Status>(), MediaPlayer::Stopped);
        QCOMPARE(mediaPlayer->status(), MediaPlayer::Stopped);
    }
}

void MediaPlayerTest::changeTrackTest()
{
    for (const MediaPlayerUnit &unit : m_units) {
        MediaPlayerPtr mediaPlayer = unit.device->mediaPlayer();

        QSignalSpy trackSpy(mediaPlayer.data(), SIGNAL(trackChanged(MediaPlayerTrack)));

        trackSpy.clear();
        mediaPlayer->next();
        QTRY_COMPARE(trackSpy.count(), 1);
        QCOMPARE(mediaPlayer->track().isValid(), true);

        trackSpy.clear();
        mediaPlayer->previous();
        QTRY_COMPARE(trackSpy.count(), 1);
        QCOMPARE(mediaPlayer->track().isValid(), true);
    }
}

void MediaPlayerTest::disconnectProfileTest()
{
    for (const MediaPlayerUnit &unit : m_units) {
        QVERIFY(unit.device->mediaPlayer());

        QSignalSpy deviceSpy(unit.device.data(), SIGNAL(mediaPlayerChanged(MediaPlayerPtr)));

        unit.device->disconnectProfile(Services::AudioVideoRemoteControl);
        QTRY_COMPARE(deviceSpy.count(), 1);
        QVERIFY(!unit.device->mediaPlayer());
    }
}

void MediaPlayerTest::bug403289()
{
    // Bug 403289: MediaPlayer interface path is not checked in InterfacesRemoved signal

    QDBusConnection connection = QDBusConnection::sessionBus();
    QString service = QStringLiteral("org.kde.bluezqt.fakebluez");

    for (DevicePtr device : m_manager->devices()) {
        QVERIFY(!device->mediaPlayer());

        QSignalSpy deviceSpy(device.data(), SIGNAL(mediaPlayerChanged(MediaPlayerPtr)));

        const QVariantMap props = {{QStringLiteral("DevicePath"), QVariant::fromValue(QDBusObjectPath(device->ubi()))}};
        FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("bug403289"), props);

        QTRY_COMPARE(deviceSpy.count(), 1);
        QVERIFY(deviceSpy.at(0).at(0).value<MediaPlayerPtr>());
        QTest::qWait(100);
        QCOMPARE(deviceSpy.count(), 1);
        device->disconnectProfile(Services::AudioVideoRemoteControl);
        QTRY_COMPARE(deviceSpy.count(), 2);
        QVERIFY(!deviceSpy.at(1).at(0).value<MediaPlayerPtr>());
    }
}

QString MediaPlayerTest::equalizerString(const MediaPlayerPtr &mediaPlayer) const
{
    switch (mediaPlayer->equalizer()) {
    case MediaPlayer::EqualizerOn:
        return QStringLiteral("on");
    default:
        return QStringLiteral("off");
    }
}

QString MediaPlayerTest::repeatString(const MediaPlayerPtr &mediaPlayer) const
{
    switch (mediaPlayer->repeat()) {
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

QString MediaPlayerTest::shuffleString(const MediaPlayerPtr &mediaPlayer) const
{
    switch (mediaPlayer->shuffle()) {
    case MediaPlayer::ShuffleAllTracks:
        return QStringLiteral("alltracks");
    case MediaPlayer::ShuffleGroup:
        return QStringLiteral("group");
    default:
        return QStringLiteral("off");
    }
}

QString MediaPlayerTest::statusString(const MediaPlayerPtr &mediaPlayer) const
{
    switch (mediaPlayer->status()) {
    case MediaPlayer::Playing:
        return QStringLiteral("playing");
    case MediaPlayer::Stopped:
        return QStringLiteral("stopped");
    case MediaPlayer::Paused:
        return QStringLiteral("paused");
    case MediaPlayer::ForwardSeek:
        return QStringLiteral("forward-seek");
    case MediaPlayer::ReverseSeek:
        return QStringLiteral("reverse-seek");
    default:
        return QStringLiteral("error");
    }
}

QVariantMap MediaPlayerTest::trackMap(const MediaPlayerPtr &mediaPlayer) const
{
    QVariantMap out;

    if (!mediaPlayer->track().isValid()) {
        return out;
    }

    out[QStringLiteral("Title")] = mediaPlayer->track().title();
    out[QStringLiteral("Artist")] = mediaPlayer->track().artist();
    out[QStringLiteral("Album")] = mediaPlayer->track().album();
    out[QStringLiteral("Genre")] = mediaPlayer->track().genre();
    out[QStringLiteral("NumberOfTracks")] = mediaPlayer->track().numberOfTracks();
    out[QStringLiteral("TrackNumber")] = mediaPlayer->track().trackNumber();
    out[QStringLiteral("Duration")] = mediaPlayer->track().duration();

    return out;
}

QTEST_MAIN(MediaPlayerTest)

#include "moc_mediaplayertest.cpp"
