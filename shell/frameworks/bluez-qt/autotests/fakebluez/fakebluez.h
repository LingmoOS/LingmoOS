/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef FAKEBLUEZ_H
#define FAKEBLUEZ_H

#include <QObject>
#include <QVariantMap>

class TestInterface;
class ObjectManager;
class AgentManager;
class ProfileManager;
class DeviceManager;
class ObexObject;
class ObexAgentManager;
class ObexClient;

class FakeBluez : public QObject
{
    Q_OBJECT

public:
    explicit FakeBluez(QObject *parent = nullptr);

    void runTest(const QString &testName);
    void runAction(const QString &object, const QString &actionName, const QVariantMap &properties);

private Q_SLOTS:
    void doRunAction();

private:
    void clear();
    void createObjectManager();
    void createAgentManager();
    void createProfileManager();
    void createDeviceManager();
    void createObexObjectManager();
    void createObexAgentManager();
    void createObexClient();

    void runBluezNotExportingInterfacesTest();
    void runBluezEmptyManagedObjectsTest();
    void runBluezNoAdaptersTest();
    void runBluezStandardTest();

    void runObexNotExportingInterfacesTest();
    void runObexEmptyManagedObjectsTest();
    void runObexNoClientTest();
    void runObexNoAgentManagerTest();
    void runObexStandardTest();

    TestInterface *m_testInterface;
    ObjectManager *m_objectManager;
    AgentManager *m_agentManager;
    ProfileManager *m_profileManager;
    DeviceManager *m_deviceManager;

    ObexObject *m_obexObject;
    ObexAgentManager *m_obexAgentManager;
    ObexClient *m_obexClient;

    QString m_actionObject;
    QString m_actionName;
    QVariantMap m_actionProperties;
};

#endif // FAKEBLUEZ_H
