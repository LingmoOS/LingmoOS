/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "agentmanager.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDebug>

AgentManager::AgentManager(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setName(QStringLiteral("org.bluez.AgentManager1"));
    setPath(QDBusObjectPath(QStringLiteral("/org/bluez")));
}

void AgentManager::runAction(const QString &actionName, const QVariantMap &properties)
{
    if (actionName == QLatin1String("request-pincode")) {
        runRequestPinCodeAction(properties);
    } else if (actionName == QLatin1String("display-pincode")) {
        runDisplayPinCodeAction(properties);
    } else if (actionName == QLatin1String("request-passkey")) {
        runRequestPasskeyAction(properties);
    } else if (actionName == QLatin1String("display-passkey")) {
        runDisplayPasskeyAction(properties);
    } else if (actionName == QLatin1String("request-confirmation")) {
        runRequestConfirmationAction(properties);
    } else if (actionName == QLatin1String("request-authorization")) {
        runRequestAuthorizationAction(properties);
    } else if (actionName == QLatin1String("authorize-service")) {
        runAuthorizeServiceAction(properties);
    } else if (actionName == QLatin1String("cancel")) {
        runCancelAction();
    } else if (actionName == QLatin1String("release")) {
        runReleaseAction();
    }
}

void AgentManager::RegisterAgent(const QDBusObjectPath &path, const QString &capability, const QDBusMessage &msg)
{
    m_agent = path;
    m_service = msg.service();
    m_capability = capability;
}

void AgentManager::UnregisterAgent(const QDBusObjectPath &path, const QDBusMessage &msg)
{
    if (m_agent == path && m_service == msg.service()) {
        m_agent = QDBusObjectPath();
        m_service.clear();
        m_capability.clear();
    }
}

void AgentManager::RequestDefaultAgent(const QDBusObjectPath &path)
{
    qDebug() << "RequestDefaultAgent" << path.path();
}

void AgentManager::runRequestPinCodeAction(const QVariantMap &properties)
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_agent.path(), QStringLiteral("org.bluez.Agent1"), QStringLiteral("RequestPinCode"));

    call << QVariant::fromValue(properties.value(QStringLiteral("Device")).value<QDBusObjectPath>());
    QDBusConnection::sessionBus().asyncCall(call);
}

void AgentManager::runDisplayPinCodeAction(const QVariantMap &properties)
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_agent.path(), QStringLiteral("org.bluez.Agent1"), QStringLiteral("DisplayPinCode"));

    call << QVariant::fromValue(properties.value(QStringLiteral("Device")).value<QDBusObjectPath>());
    call << properties.value(QStringLiteral("PinCode"));
    QDBusConnection::sessionBus().asyncCall(call);
}

void AgentManager::runRequestPasskeyAction(const QVariantMap &properties)
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_agent.path(), QStringLiteral("org.bluez.Agent1"), QStringLiteral("RequestPasskey"));

    call << QVariant::fromValue(properties.value(QStringLiteral("Device")).value<QDBusObjectPath>());
    QDBusConnection::sessionBus().asyncCall(call);
}

void AgentManager::runDisplayPasskeyAction(const QVariantMap &properties)
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_agent.path(), QStringLiteral("org.bluez.Agent1"), QStringLiteral("DisplayPasskey"));

    call << QVariant::fromValue(properties.value(QStringLiteral("Device")).value<QDBusObjectPath>());
    call << properties.value(QStringLiteral("Passkey"));
    call << properties.value(QStringLiteral("EnteredPasskey"));
    QDBusConnection::sessionBus().asyncCall(call);
}

void AgentManager::runRequestConfirmationAction(const QVariantMap &properties)
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_agent.path(), QStringLiteral("org.bluez.Agent1"), QStringLiteral("RequestConfirmation"));

    call << QVariant::fromValue(properties.value(QStringLiteral("Device")).value<QDBusObjectPath>());
    call << properties.value(QStringLiteral("Passkey"));
    QDBusConnection::sessionBus().asyncCall(call);
}

void AgentManager::runRequestAuthorizationAction(const QVariantMap &properties)
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_agent.path(), QStringLiteral("org.bluez.Agent1"), QStringLiteral("RequestAuthorization"));

    call << QVariant::fromValue(properties.value(QStringLiteral("Device")).value<QDBusObjectPath>());
    QDBusConnection::sessionBus().asyncCall(call);
}

void AgentManager::runAuthorizeServiceAction(const QVariantMap &properties)
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_agent.path(), QStringLiteral("org.bluez.Agent1"), QStringLiteral("AuthorizeService"));

    call << QVariant::fromValue(properties.value(QStringLiteral("Device")).value<QDBusObjectPath>());
    call << properties.value(QStringLiteral("UUID"));
    QDBusConnection::sessionBus().asyncCall(call);
}

void AgentManager::runCancelAction()
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_agent.path(), QStringLiteral("org.bluez.Agent1"), QStringLiteral("Cancel"));
    QDBusConnection::sessionBus().asyncCall(call);
}

void AgentManager::runReleaseAction()
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_agent.path(), QStringLiteral("org.bluez.Agent1"), QStringLiteral("Release"));
    QDBusConnection::sessionBus().asyncCall(call);
}

#include "moc_agentmanager.cpp"
