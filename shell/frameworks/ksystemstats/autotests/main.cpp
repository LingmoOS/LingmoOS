/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QTest>
#include <QSignalSpy>

#include "../src/daemon.h"

#include <systemstats/SensorContainer.h>
#include <systemstats/SensorObject.h>
#include <systemstats/SensorPlugin.h>
#include <systemstats/DBusInterface.h>

#include <QDBusConnection>
#include <QDBusMetaType>

class TestPlugin : public KSysGuard::SensorPlugin
{
public:
    TestPlugin(QObject *parent)
        : SensorPlugin(parent, {})
    {
        m_testContainer = new KSysGuard::SensorContainer("testContainer", "Test Container", this);
        m_testObject = new KSysGuard::SensorObject("testObject", "Test Object", m_testContainer);
        m_property1 = new KSysGuard::SensorProperty("property1", m_testObject);
        m_property1->setMin(0);
        m_property1->setMax(100);
        m_property1->setShortName("Some Sensor 1");
        m_property1->setName("Some Sensor Name 1");

        m_property2 = new KSysGuard::SensorProperty("property2", m_testObject);
    }
    QString providerName() const override
    {
        return "testPlugin";
    }
    void update() override
    {
        m_updateCount++;
    }
    KSysGuard::SensorContainer *m_testContainer;
    KSysGuard::SensorObject *m_testObject;
    KSysGuard::SensorProperty *m_property1;
    KSysGuard::SensorProperty *m_property2;
    int m_updateCount = 0;
};

class KStatsTest : public Daemon
{
    Q_OBJECT
public:
    KStatsTest();

protected:
    void loadProviders() override;
private Q_SLOTS:
    void initTestCase();
    void findById();
    void update();
    void subscription();
    void changes();
    void dbusApi();

private:
    TestPlugin *m_testPlugin = nullptr;
};

KStatsTest::KStatsTest()
{
    qDBusRegisterMetaType<KSysGuard::SensorData>();
    qDBusRegisterMetaType<KSysGuard::SensorInfo>();
    qDBusRegisterMetaType<KSysGuard::SensorDataList>();
    qDBusRegisterMetaType<QHash<QString, KSysGuard::SensorInfo>>();
    qDBusRegisterMetaType<QStringList>();
}

void KStatsTest::loadProviders()
{
    m_testPlugin = new TestPlugin(this);
    registerProvider(m_testPlugin);

    // Sensor registration in containers uses queued connections, trigger those
    QTest::qWait(0);
}

void KStatsTest::initTestCase()
{
    QDBusConnection::sessionBus().registerObject(KSysGuard::SystemStats::ObjectPath, this, QDBusConnection::ExportAdaptors);
    loadProviders();
}

void KStatsTest::findById()
{
    QVERIFY(findSensor("testContainer/testObject/property1"));
    QVERIFY(findSensor("testContainer/testObject/property2"));
    QVERIFY(!findSensor("testContainer/asdfasdfasfs/property1"));
}

void KStatsTest::update()
{
    QCOMPARE(m_testPlugin->m_updateCount, 0);
    sendFrame();
    QCOMPARE(m_testPlugin->m_updateCount, 1);
}

void KStatsTest::subscription()
{
    QSignalSpy property1Subscribed(m_testPlugin->m_property1, &KSysGuard::SensorProperty::subscribedChanged);
    QSignalSpy property2Subscribed(m_testPlugin->m_property2, &KSysGuard::SensorProperty::subscribedChanged);
    QSignalSpy objectSubscribed(m_testPlugin->m_testObject, &KSysGuard::SensorObject::subscribedChanged);

    m_testPlugin->m_property1->subscribe();
    QCOMPARE(property1Subscribed.count(), 1);
    QCOMPARE(objectSubscribed.count(), 1);

    m_testPlugin->m_property1->subscribe();
    QCOMPARE(property1Subscribed.count(), 1);
    QCOMPARE(objectSubscribed.count(), 1);

    m_testPlugin->m_property2->subscribe();
    QCOMPARE(objectSubscribed.count(), 1);

    m_testPlugin->m_property1->unsubscribe();
    QCOMPARE(property1Subscribed.count(), 1);
    m_testPlugin->m_property1->unsubscribe();
    QCOMPARE(property1Subscribed.count(), 2);
}

void KStatsTest::changes()
{
    QSignalSpy property1Changed(m_testPlugin->m_property1, &KSysGuard::SensorProperty::valueChanged);
    m_testPlugin->m_property1->setValue(14);
    QCOMPARE(property1Changed.count(), 1);
    QCOMPARE(m_testPlugin->m_property1->value(), QVariant(14));
}

void KStatsTest::dbusApi()
{
    KSysGuard::SystemStats::DBusInterface iface(QDBusConnection::sessionBus().baseService(),
        KSysGuard::SystemStats::ObjectPath,
        QDBusConnection::sessionBus(),
        this);
    // list all objects
    auto pendingSensors = iface.allSensors();
    pendingSensors.waitForFinished();
    auto sensors = pendingSensors.value();
    QVERIFY(sensors.count() == 4);

    // test metadata
    QCOMPARE(sensors["testContainer/testObject/property1"].name, "Some Sensor Name 1");

    // query value
    m_testPlugin->m_property1->setValue(100);

    auto pendingValues = iface.sensorData({ "testContainer/testObject/property1" });
    pendingValues.waitForFinished();
    QCOMPARE(pendingValues.value().first().sensorProperty, "testContainer/testObject/property1");
    QCOMPARE(pendingValues.value().first().payload.toInt(), 100);

    // change updates
    QSignalSpy changesSpy(&iface, &KSysGuard::SystemStats::DBusInterface::newSensorData);

    iface.subscribe({ "testContainer/testObject/property1" });

    sendFrame();
    // a frame with no changes, does nothing
    QVERIFY(!changesSpy.wait(20));

    m_testPlugin->m_property1->setValue(101);
    // an update does nothing till it gets a frame, in order to batch
    QVERIFY(!changesSpy.wait(20));
    sendFrame();

    QVERIFY(changesSpy.wait(20));
    QCOMPARE(changesSpy.first().first().value<KSysGuard::SensorDataList>().first().sensorProperty, "testContainer/testObject/property1");
    QCOMPARE(changesSpy.first().first().value<KSysGuard::SensorDataList>().first().payload, QVariant(101));

    // we're not subscribed to property 2 so if that updates we should not get anything
    m_testPlugin->m_property2->setValue(102);
    sendFrame();
    QVERIFY(!changesSpy.wait(20));
}

QTEST_GUILESS_MAIN(KStatsTest)

#include "main.moc"
