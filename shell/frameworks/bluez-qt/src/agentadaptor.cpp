/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "agentadaptor.h"
#include "adapter.h"
#include "agent.h"
#include "device.h"
#include "manager.h"
#include "request.h"

#include <QDBusMessage>
#include <QDBusObjectPath>

namespace BluezQt
{
AgentAdaptor::AgentAdaptor(Agent *parent, Manager *manager)
    : QDBusAbstractAdaptor(parent)
    , m_agent(parent)
    , m_manager(manager)
{
}

QString AgentAdaptor::RequestPinCode(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    Request<QString> req(OrgBluezAgent, msg);

    DevicePtr dev = m_manager->deviceForUbi(device.path());
    if (!dev) {
        req.cancel();
        return QString();
    }

    m_agent->requestPinCode(dev, req);
    return QString();
}

void AgentAdaptor::DisplayPinCode(const QDBusObjectPath &device, const QString &pincode)
{
    DevicePtr dev = m_manager->deviceForUbi(device.path());
    if (!dev) {
        return;
    }

    m_agent->displayPinCode(dev, pincode);
}

quint32 AgentAdaptor::RequestPasskey(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    Request<quint32> req(OrgBluezAgent, msg);

    DevicePtr dev = m_manager->deviceForUbi(device.path());
    if (!dev) {
        req.cancel();
        return 0;
    }

    m_agent->requestPasskey(dev, req);
    return 0;
}

void AgentAdaptor::DisplayPasskey(const QDBusObjectPath &device, quint32 passkey, quint16 entered)
{
    DevicePtr dev = m_manager->deviceForUbi(device.path());
    if (!dev) {
        return;
    }

    m_agent->displayPasskey(dev, passkeyToString(passkey), QString::number(entered));
}

void AgentAdaptor::RequestConfirmation(const QDBusObjectPath &device, quint32 passkey, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    Request<> req(OrgBluezAgent, msg);

    DevicePtr dev = m_manager->deviceForUbi(device.path());
    if (!dev) {
        req.cancel();
        return;
    }

    m_agent->requestConfirmation(dev, passkeyToString(passkey), req);
}

void AgentAdaptor::RequestAuthorization(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    Request<> req(OrgBluezAgent, msg);

    DevicePtr dev = m_manager->deviceForUbi(device.path());
    if (!dev) {
        req.cancel();
        return;
    }

    m_agent->requestAuthorization(dev, req);
}

void AgentAdaptor::AuthorizeService(const QDBusObjectPath &device, const QString &uuid, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    Request<> req(OrgBluezAgent, msg);

    DevicePtr dev = m_manager->deviceForUbi(device.path());
    if (!dev) {
        req.cancel();
        return;
    }

    m_agent->authorizeService(dev, uuid.toUpper(), req);
}

void AgentAdaptor::Cancel()
{
    m_agent->cancel();
}

void AgentAdaptor::Release()
{
    m_agent->release();
}

QString AgentAdaptor::passkeyToString(quint32 passkey) const
{
    // Passkey will always be a 6-digit number, zero-pad it at the start
    return QStringLiteral("%1").arg(passkey, 6, 10, QLatin1Char('0'));
}

} // namespace BluezQt

#include "moc_agentadaptor.cpp"
