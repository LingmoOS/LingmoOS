/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "agentmanagertest.h"
#include "autotests.h"
#include "initmanagerjob.h"
#include "manager.h"
#include "pendingcall.h"
#include "services.h"

#include <QTest>

namespace BluezQt
{
extern void bluezqt_initFakeBluezTestRun();
}

using namespace BluezQt;

// TestAgent
TestAgent::TestAgent(QObject *parent)
    : Agent(parent)
    , m_pinRequested(false)
    , m_passkeyRequested(false)
    , m_authorizationRequested(false)
    , m_cancelCalled(false)
    , m_releaseCalled(false)
{
}

QDBusObjectPath TestAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/testagent"));
}

void TestAgent::requestPinCode(DevicePtr device, const BluezQt::Request<QString> &request)
{
    m_device = device;
    m_pinRequested = true;

    request.accept(QString());
}

void TestAgent::displayPinCode(DevicePtr device, const QString &pinCode)
{
    m_device = device;
    m_displayedPinCode = pinCode;
}

void TestAgent::requestPasskey(DevicePtr device, const BluezQt::Request<quint32> &request)
{
    m_device = device;
    m_passkeyRequested = true;

    request.accept(0);
}

void TestAgent::displayPasskey(DevicePtr device, const QString &passkey, const QString &entered)
{
    m_device = device;
    m_displayedPasskey = passkey;
    m_enteredPasskey = entered;
}

void TestAgent::requestConfirmation(DevicePtr device, const QString &passkey, const BluezQt::Request<> &request)
{
    m_device = device;
    m_requestedPasskey = passkey;

    request.accept();
}

void TestAgent::requestAuthorization(DevicePtr device, const BluezQt::Request<> &request)
{
    m_device = device;
    m_authorizationRequested = true;

    request.accept();
}

void TestAgent::authorizeService(DevicePtr device, const QString &uuid, const BluezQt::Request<> &request)
{
    m_device = device;
    m_authorizedUuid = uuid;

    request.accept();
}

void TestAgent::cancel()
{
    m_cancelCalled = true;
}

void TestAgent::release()
{
    m_releaseCalled = true;
}

// AgentManagerTest
void AgentManagerTest::initTestCase()
{
    bluezqt_initFakeBluezTestRun();

    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-standard"));

    // Create adapter
    QVariantMap adapterProps;
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(QStringLiteral("/org/bluez/hci0")));
    adapterProps[QStringLiteral("Address")] = QStringLiteral("1C:E5:C3:BC:94:7E");
    adapterProps[QStringLiteral("Name")] = QStringLiteral("TestAdapter");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-adapter"), adapterProps);

    // Create device
    m_device = QDBusObjectPath(QStringLiteral("/org/bluez/hci0/dev_40_79_6A_0C_39_75"));
    QVariantMap deviceProps;
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(m_device);
    deviceProps[QStringLiteral("Adapter")] = adapterProps.value(QStringLiteral("Path"));
    deviceProps[QStringLiteral("Address")] = QStringLiteral("40:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    Manager *manager = new Manager(this);
    InitManagerJob *job = manager->init();
    job->exec();

    QVERIFY(!job->error());
    QCOMPARE(manager->adapters().count(), 1);
    QCOMPARE(manager->devices().count(), 1);

    m_agent = new TestAgent(this);
    manager->registerAgent(m_agent)->waitForFinished();
}

void AgentManagerTest::cleanupTestCase()
{
    FakeBluez::stop();
}

void AgentManagerTest::requestPinCodeTest()
{
    QCOMPARE(m_agent->m_pinRequested, false);

    QVariantMap props;
    props.insert(QStringLiteral("Device"), QVariant::fromValue(m_device));
    FakeBluez::runAction(QStringLiteral("agentmanager"), QStringLiteral("request-pincode"), props);

    QTRY_COMPARE(m_agent->m_pinRequested, true);
    QTRY_COMPARE(m_agent->m_device->name(), QStringLiteral("TestDevice"));
}

void AgentManagerTest::displayPinCodeTest()
{
    QCOMPARE(m_agent->m_displayedPinCode, QString());

    QVariantMap props;
    props.insert(QStringLiteral("Device"), QVariant::fromValue(m_device));
    props.insert(QStringLiteral("PinCode"), QStringLiteral("123456"));
    FakeBluez::runAction(QStringLiteral("agentmanager"), QStringLiteral("display-pincode"), props);

    QTRY_COMPARE(m_agent->m_displayedPinCode, QStringLiteral("123456"));
    QTRY_COMPARE(m_agent->m_device->name(), QStringLiteral("TestDevice"));
}

void AgentManagerTest::requestPasskeyTest()
{
    QCOMPARE(m_agent->m_passkeyRequested, false);

    QVariantMap props;
    props.insert(QStringLiteral("Device"), QVariant::fromValue(m_device));
    FakeBluez::runAction(QStringLiteral("agentmanager"), QStringLiteral("request-passkey"), props);

    QTRY_COMPARE(m_agent->m_passkeyRequested, true);
    QTRY_COMPARE(m_agent->m_device->name(), QStringLiteral("TestDevice"));
}

void AgentManagerTest::displayPasskeyTest()
{
    QCOMPARE(m_agent->m_displayedPasskey, QString());
    QCOMPARE(m_agent->m_enteredPasskey, QString());

    QVariantMap props;
    props.insert(QStringLiteral("Device"), QVariant::fromValue(m_device));
    props.insert(QStringLiteral("Passkey"), QVariant::fromValue(quint32(654321)));
    props.insert(QStringLiteral("EnteredPasskey"), QVariant::fromValue(quint16(43)));
    FakeBluez::runAction(QStringLiteral("agentmanager"), QStringLiteral("display-passkey"), props);

    QTRY_COMPARE(m_agent->m_displayedPasskey, QStringLiteral("654321"));
    QTRY_COMPARE(m_agent->m_enteredPasskey, QStringLiteral("43"));
    QTRY_COMPARE(m_agent->m_device->name(), QStringLiteral("TestDevice"));
}

void AgentManagerTest::requestConfirmationTest()
{
    QCOMPARE(m_agent->m_requestedPasskey, QString());

    QVariantMap props;
    props.insert(QStringLiteral("Device"), QVariant::fromValue(m_device));
    props.insert(QStringLiteral("Passkey"), QVariant::fromValue(quint32(12)));
    FakeBluez::runAction(QStringLiteral("agentmanager"), QStringLiteral("request-confirmation"), props);

    QTRY_COMPARE(m_agent->m_requestedPasskey, QStringLiteral("000012"));
    QTRY_COMPARE(m_agent->m_device->name(), QStringLiteral("TestDevice"));
}

void AgentManagerTest::requestAuthorizationTest()
{
    QCOMPARE(m_agent->m_authorizationRequested, false);

    QVariantMap props;
    props.insert(QStringLiteral("Device"), QVariant::fromValue(m_device));
    FakeBluez::runAction(QStringLiteral("agentmanager"), QStringLiteral("request-authorization"), props);

    QTRY_COMPARE(m_agent->m_authorizationRequested, true);
    QTRY_COMPARE(m_agent->m_device->name(), QStringLiteral("TestDevice"));
}

void AgentManagerTest::authorizeServiceTest()
{
    QCOMPARE(m_agent->m_authorizedUuid, QString());

    QVariantMap props;
    props.insert(QStringLiteral("Device"), QVariant::fromValue(m_device));
    props.insert(QStringLiteral("UUID"), Services::ObexFileTransfer);
    FakeBluez::runAction(QStringLiteral("agentmanager"), QStringLiteral("authorize-service"), props);

    QTRY_COMPARE(m_agent->m_authorizedUuid, Services::ObexFileTransfer);
    QTRY_COMPARE(m_agent->m_device->name(), QStringLiteral("TestDevice"));
}

void AgentManagerTest::cancelTest()
{
    QCOMPARE(m_agent->m_cancelCalled, false);

    FakeBluez::runAction(QStringLiteral("agentmanager"), QStringLiteral("cancel"));

    QTRY_COMPARE(m_agent->m_cancelCalled, true);
    QTRY_COMPARE(m_agent->m_device->name(), QStringLiteral("TestDevice"));
}

void AgentManagerTest::releaseTest()
{
    QCOMPARE(m_agent->m_releaseCalled, false);

    FakeBluez::runAction(QStringLiteral("agentmanager"), QStringLiteral("release"));

    QTRY_COMPARE(m_agent->m_releaseCalled, true);
    QTRY_COMPARE(m_agent->m_device->name(), QStringLiteral("TestDevice"));
}

QTEST_MAIN(AgentManagerTest)

#include "moc_agentmanagertest.cpp"
