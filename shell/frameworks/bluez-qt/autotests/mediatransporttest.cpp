/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediatransporttest.h"
#include "a2dp-codecs.h"
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

Q_DECLARE_METATYPE(MediaTransport::State)

static a2dp_sbc_t sbcConfiguration;
static a2dp_aac_t aacConfiguration;

static AudioCodec charToCodec(uchar value)
{
    switch (value) {
    case A2DP_CODEC_SBC:
        return AudioCodec::Sbc;
        break;
    case A2DP_CODEC_MPEG24:
        return AudioCodec::Aac;
        break;
    }

    return AudioCodec::Invalid;
}

static AudioSampleRate byteArrayToSampleRate(AudioCodec codec, const QByteArray &buffer)
{
    switch (codec) {
    case AudioCodec::Sbc: {
        if (buffer.size() != sizeof(a2dp_sbc_t)) {
            return AudioSampleRate::Invalid;
        }

        a2dp_sbc_t sbcConfig = *reinterpret_cast<const a2dp_sbc_t *>(buffer.constData());
        switch (sbcConfig.frequency) {
        case SBC_SAMPLING_FREQ_44100:
            return AudioSampleRate::Rate44100;
            break;
        case SBC_SAMPLING_FREQ_48000:
            return AudioSampleRate::Rate48000;
            break;
        }
        break;
    }
    case AudioCodec::Aac: {
        if (buffer.size() != sizeof(a2dp_aac_t)) {
            return AudioSampleRate::Invalid;
        }

        a2dp_aac_t aacConfig = *reinterpret_cast<const a2dp_aac_t *>(buffer.constData());
        switch (AAC_GET_FREQUENCY(aacConfig)) {
        case AAC_SAMPLING_FREQ_44100:
            return AudioSampleRate::Rate44100;
            break;
        case AAC_SAMPLING_FREQ_48000:
            return AudioSampleRate::Rate48000;
            break;
        }
        break;
    }
    }

    return AudioSampleRate::Invalid;
}

QString stateString(MediaTransport::State state)
{
    switch (state) {
    case MediaTransport::State::Idle:
        return QStringLiteral("idle");
    case MediaTransport::State::Pending:
        return QStringLiteral("pending");
    case MediaTransport::State::Active:
        return QStringLiteral("active");
    }

    return {};
}

MediaTransportTest::MediaTransportTest()
    : m_manager(nullptr)
{
    Autotests::registerMetatypes();

    qRegisterMetaType<MediaTransport::State>("State");

    sbcConfiguration.frequency = SBC_SAMPLING_FREQ_44100;
    AAC_SET_FREQUENCY(aacConfiguration, AAC_SAMPLING_FREQ_48000);
}

void MediaTransportTest::initTestCase()
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
    QVariantMap mediaTransportProps;

    QString device1 = adapter + QLatin1String("/dev_40_79_6A_0C_39_75");
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(device1));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(QDBusObjectPath(adapter));
    deviceProps[QStringLiteral("Address")] = QStringLiteral("40:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice");
    deviceProps[QStringLiteral("UUIDs")] = QStringList(Services::AudioSink);
    mediaTransportProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(device1 + QLatin1String("/fd0")));
    mediaTransportProps[QStringLiteral("Device")] = QVariant::fromValue(QDBusObjectPath(device1));
    mediaTransportProps[QStringLiteral("UUID")] = Services::AudioSink;
    mediaTransportProps[QStringLiteral("Codec")] = QVariant::fromValue(quint8(A2DP_CODEC_SBC)); // SBC
    mediaTransportProps[QStringLiteral("Configuration")] =
        QVariant::fromValue(QByteArray(reinterpret_cast<const char *>(&sbcConfiguration), sizeof(sbcConfiguration)));
    mediaTransportProps[QStringLiteral("State")] = QStringLiteral("pending");
    mediaTransportProps[QStringLiteral("Volume")] = QVariant::fromValue(quint16(63));
    deviceProps[QStringLiteral("MediaTransport")] = mediaTransportProps;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    QString device2 = adapter + QLatin1String("/dev_50_79_6A_0C_39_75");
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(device2));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(QDBusObjectPath(adapter));
    deviceProps[QStringLiteral("Address")] = QStringLiteral("50:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice2");
    deviceProps[QStringLiteral("UUIDs")] = QStringList(Services::AudioSink);
    mediaTransportProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(device2 + QLatin1String("/fd0")));
    mediaTransportProps[QStringLiteral("Device")] = QVariant::fromValue(QDBusObjectPath(device2));
    mediaTransportProps[QStringLiteral("UUID")] = Services::AudioSink;
    mediaTransportProps[QStringLiteral("Codec")] = QVariant::fromValue(quint8(A2DP_CODEC_MPEG24)); // AAC
    mediaTransportProps[QStringLiteral("Configuration")] =
        QVariant::fromValue(QByteArray(reinterpret_cast<const char *>(&aacConfiguration), sizeof(aacConfiguration)));
    mediaTransportProps[QStringLiteral("State")] = QStringLiteral("active");
    mediaTransportProps[QStringLiteral("Volume")] = QVariant::fromValue(quint16(127));
    deviceProps[QStringLiteral("MediaTransport")] = mediaTransportProps;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    m_manager = new Manager();
    InitManagerJob *initJob = m_manager->init();
    initJob->exec();
    QVERIFY(!initJob->error());

    QCOMPARE(m_manager->adapters().count(), 1);
    QCOMPARE(m_manager->devices().count(), 2);
}

void MediaTransportTest::cleanupTestCase()
{
    for (const MediaTransportUnit &unit : m_units) {
        delete unit.dbusMediaTransport;
        delete unit.dbusProperties;
    }

    delete m_manager;

    FakeBluez::stop();
}

void MediaTransportTest::connectTest()
{
    for (DevicePtr device : m_manager->devices()) {
        QVERIFY(!device->ubi().isEmpty());
        QVERIFY(!device->mediaTransport());

        QSignalSpy deviceSpy(device.data(), SIGNAL(mediaTransportChanged(MediaTransportPtr)));

        device->connectToDevice();
        QTRY_COMPARE(deviceSpy.count(), 1);
        QVERIFY(device->mediaTransport());
    }
}

void MediaTransportTest::disconnectTest()
{
    for (DevicePtr device : m_manager->devices()) {
        QVERIFY(device->mediaTransport());

        QSignalSpy deviceSpy(device.data(), SIGNAL(mediaTransportChanged(MediaTransportPtr)));

        device->disconnectFromDevice();
        QTRY_COMPARE(deviceSpy.count(), 1);
        QVERIFY(!device->mediaTransport());
    }
}

void MediaTransportTest::connectProfileTest()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    QString service = QStringLiteral("org.kde.bluezqt.fakebluez");

    for (DevicePtr device : m_manager->devices()) {
        QVERIFY(!device->ubi().isEmpty());
        QVERIFY(!device->mediaTransport());

        QSignalSpy deviceSpy(device.data(), SIGNAL(mediaTransportChanged(MediaTransportPtr)));

        device->connectProfile(Services::AudioSink);
        QTRY_COMPARE(deviceSpy.count(), 1);
        QVERIFY(device->mediaTransport());

        QString path = device->ubi() + QLatin1String("/fd0");

        MediaTransportUnit unit;
        unit.device = device;
        unit.dbusMediaTransport = new org::bluez::MediaTransport1(service, path, connection, this);
        unit.dbusProperties = new org::freedesktop::DBus::Properties(service, path, connection, this);
        m_units.append(unit);
    }
}

void MediaTransportTest::getPropertiesTest()
{
    for (const MediaTransportUnit &unit : m_units) {
        QCOMPARE(unit.device->mediaTransport()->audioConfiguration().codec, charToCodec(unit.dbusMediaTransport->codec()));
        QCOMPARE(unit.device->mediaTransport()->audioConfiguration().sampleRate,
                 byteArrayToSampleRate(unit.device->mediaTransport()->audioConfiguration().codec, unit.dbusMediaTransport->configuration()));
        QCOMPARE(stateString(unit.device->mediaTransport()->state()), unit.dbusMediaTransport->state());
        QCOMPARE(unit.device->mediaTransport()->volume(), unit.dbusMediaTransport->volume());
    }
}

void MediaTransportTest::disconnectProfileTest()
{
    for (const MediaTransportUnit &unit : m_units) {
        QVERIFY(unit.device->mediaTransport());

        QSignalSpy deviceSpy(unit.device.data(), SIGNAL(mediaTransportChanged(MediaTransportPtr)));

        unit.device->disconnectProfile(Services::AudioSink);
        QTRY_COMPARE(deviceSpy.count(), 1);
        QVERIFY(!unit.device->mediaTransport());
    }
}

QTEST_MAIN(MediaTransportTest)

#include "moc_mediatransporttest.cpp"
