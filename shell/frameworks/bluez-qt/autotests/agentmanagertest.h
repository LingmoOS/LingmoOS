/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef AGENTMANAGERTEST_H
#define AGENTMANAGERTEST_H

#include <QDBusObjectPath>
#include <QObject>

#include "agent.h"
#include "device.h"

class TestAgent : public BluezQt::Agent
{
    Q_OBJECT

public:
    explicit TestAgent(QObject *parent = nullptr);

    QDBusObjectPath objectPath() const override;

    void requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request) override;
    void displayPinCode(BluezQt::DevicePtr device, const QString &pinCode) override;
    void requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request) override;
    void displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered) override;
    void requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request) override;
    void requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request) override;
    void authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request) override;

    void cancel() override;
    void release() override;

    BluezQt::DevicePtr m_device;

    // requestPinCode
    bool m_pinRequested;

    // displayPinCode
    QString m_displayedPinCode;

    // requestPasskey
    bool m_passkeyRequested;

    // displayPasskey
    QString m_displayedPasskey;
    QString m_enteredPasskey;

    // requestConfirmation
    QString m_requestedPasskey;

    // requestAuthorization
    bool m_authorizationRequested;

    // authorizeService
    QString m_authorizedUuid;

    // cancel
    bool m_cancelCalled;

    // release
    bool m_releaseCalled;
};

class AgentManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void requestPinCodeTest();
    void displayPinCodeTest();
    void requestPasskeyTest();
    void displayPasskeyTest();
    void requestConfirmationTest();
    void requestAuthorizationTest();
    void authorizeServiceTest();
    void cancelTest();
    void releaseTest();

private:
    TestAgent *m_agent;
    QDBusObjectPath m_device;
};

#endif // AGENTMANAGERTEST_H
