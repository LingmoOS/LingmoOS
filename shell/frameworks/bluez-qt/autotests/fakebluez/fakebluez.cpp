/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "fakebluez.h"
#include "agentmanager.h"
#include "devicemanager.h"
#include "obexagentmanager.h"
#include "obexclient.h"
#include "objectmanager.h"
#include "profilemanager.h"
#include "testinterface.h"

#include <QDBusConnection>
#include <QDebug>
#include <QTimer>

// ObexObject
class ObexObject : public QObject
{
public:
    explicit ObexObject(QObject *parent = nullptr)
        : QObject(parent)
    {
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/bluez/obex"), this);
    }
};

// FakeBluez
FakeBluez::FakeBluez(QObject *parent)
    : QObject(parent)
    , m_testInterface(new TestInterface(this))
    , m_objectManager(nullptr)
    , m_agentManager(nullptr)
    , m_profileManager(nullptr)
    , m_deviceManager(nullptr)
    , m_obexObject(nullptr)
    , m_obexAgentManager(nullptr)
    , m_obexClient(nullptr)
{
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/"), this);
}

void FakeBluez::runTest(const QString &testName)
{
    if (testName == QLatin1String("bluez-not-exporting-interfaces")) {
        runBluezNotExportingInterfacesTest();
    } else if (testName == QLatin1String("bluez-empty-managed-objects")) {
        runBluezEmptyManagedObjectsTest();
    } else if (testName == QLatin1String("bluez-no-adapters")) {
        runBluezNoAdaptersTest();
    } else if (testName == QLatin1String("bluez-standard")) {
        runBluezStandardTest();
    }

    if (testName == QLatin1String("obex-not-exporting-interfaces")) {
        runObexNotExportingInterfacesTest();
    } else if (testName == QLatin1String("obex-empty-managed-objects")) {
        runObexEmptyManagedObjectsTest();
    } else if (testName == QLatin1String("obex-no-client")) {
        runObexNoClientTest();
    } else if (testName == QLatin1String("obex-no-agentmanager")) {
        runObexNoAgentManagerTest();
    } else if (testName == QLatin1String("obex-standard")) {
        runObexStandardTest();
    }

    if (!QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.bluezqt.fakebluez"))) {
        qWarning() << "Cannot register org.kde.bluezqt.fakebluez service!";
    }
}

void FakeBluez::runAction(const QString &object, const QString &actionName, const QVariantMap &properties)
{
    m_actionObject = object;
    m_actionName = actionName;
    m_actionProperties = properties;

    QTimer::singleShot(0, this, SLOT(doRunAction()));
}

void FakeBluez::doRunAction()
{
    if (m_actionObject == QLatin1String("agentmanager")) {
        m_agentManager->runAction(m_actionName, m_actionProperties);
    } else if (m_actionObject == QLatin1String("devicemanager")) {
        m_deviceManager->runAction(m_actionName, m_actionProperties);
    }

    QTimer::singleShot(0, m_testInterface, SLOT(emitActionFinished()));
}

void FakeBluez::clear()
{
    // Everything is parented to ObjectManager
    delete m_objectManager;
    m_objectManager = nullptr;
}

void FakeBluez::createObjectManager()
{
    m_objectManager = new ObjectManager(this);
}

void FakeBluez::createAgentManager()
{
    m_agentManager = new AgentManager(m_objectManager);
    m_objectManager->addObject(m_agentManager);
}

void FakeBluez::createProfileManager()
{
    m_profileManager = new ProfileManager(m_objectManager);
    m_objectManager->addObject(m_profileManager);
}

void FakeBluez::createDeviceManager()
{
    m_deviceManager = new DeviceManager(m_objectManager);
}

void FakeBluez::createObexObjectManager()
{
    createObjectManager();
    m_obexObject = new ObexObject(this);
    m_objectManager->addAutoDeleteObject(m_obexObject);
}

void FakeBluez::createObexAgentManager()
{
    m_obexAgentManager = new ObexAgentManager(m_obexObject);
    m_objectManager->addObject(m_obexAgentManager);
}

void FakeBluez::createObexClient()
{
    m_obexClient = new ObexClient(m_obexObject);
    m_objectManager->addObject(m_obexClient);
}

void FakeBluez::runBluezNotExportingInterfacesTest()
{
    clear();
}

void FakeBluez::runBluezEmptyManagedObjectsTest()
{
    clear();
    createObjectManager();
}

void FakeBluez::runBluezNoAdaptersTest()
{
    clear();
    createObjectManager();
    createAgentManager();
    createProfileManager();
}

void FakeBluez::runBluezStandardTest()
{
    clear();
    createObjectManager();
    createDeviceManager();
    createAgentManager();
    createProfileManager();
}

void FakeBluez::runObexNotExportingInterfacesTest()
{
    clear();
}

void FakeBluez::runObexEmptyManagedObjectsTest()
{
    clear();
    createObexObjectManager();
}

void FakeBluez::runObexNoClientTest()
{
    clear();
    createObexObjectManager();
    createObexAgentManager();
}

void FakeBluez::runObexNoAgentManagerTest()
{
    clear();
    createObexObjectManager();
    createObexClient();
}

void FakeBluez::runObexStandardTest()
{
    clear();
    createObexObjectManager();
    createObexClient();
    createObexAgentManager();
}

#include "moc_fakebluez.cpp"
