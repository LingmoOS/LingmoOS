/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "obexagentmanager.h"

#include <QDBusConnection>
#include <QDBusMessage>

ObexAgentManager::ObexAgentManager(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setName(QStringLiteral("org.bluez.obex.AgentManager1"));
    setPath(QDBusObjectPath(QStringLiteral("/org/bluez/obex")));
}

void ObexAgentManager::runAction(const QString &actionName, const QVariantMap &properties)
{
    if (actionName == QLatin1String("authorize-push")) {
        runAuthorizePushAction(properties);
    }
}

void ObexAgentManager::RegisterAgent(const QDBusObjectPath &path, const QDBusMessage &msg)
{
    m_agent = path;
    m_service = msg.service();
}

void ObexAgentManager::UnregisterAgent(const QDBusObjectPath &path, const QDBusMessage &msg)
{
    if (m_agent == path && m_service == msg.service()) {
        m_agent = QDBusObjectPath();
        m_service.clear();
    }
}

void ObexAgentManager::runAuthorizePushAction(const QVariantMap &properties)
{
    Q_UNUSED(properties)

    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_agent.path(), QStringLiteral("org.bluez.obex.Agent1"), QStringLiteral("AuthorizePush"));

    call << QVariant::fromValue(QDBusObjectPath(QStringLiteral("/org/bluez/session1/transfer1")));
    const QString &fileName = QDBusConnection::sessionBus().call(call).arguments().first().toString();
    Q_UNUSED(fileName)
}

void ObexAgentManager::runCancelAction()
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_agent.path(), QStringLiteral("org.bluez.obex.Agent1"), QStringLiteral("Cancel"));
    QDBusConnection::sessionBus().call(call);
}

void ObexAgentManager::runReleaseAction()
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_agent.path(), QStringLiteral("org.bluez.obex.Agent1"), QStringLiteral("Release"));
    QDBusConnection::sessionBus().call(call);
}

#include "moc_obexagentmanager.cpp"
