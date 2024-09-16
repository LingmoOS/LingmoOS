// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbluetoothagentadaptor.h"
#include "dbluetoothagent.h"
#include "dbluetoothdevice.h"
#include "dbluetoothutils.h"
#include "dbluetoothrequest.h"
#include <QStringView>

DBLUETOOTH_BEGIN_NAMESPACE

DAgentAdaptor::DAgentAdaptor(DAgent *parent)
    : QDBusAbstractAdaptor(parent)
    , m_agent(parent)
{
}

QSharedPointer<DDevice> DAgentAdaptor::deviceFromUbi(const QDBusObjectPath &device) const
{
    const auto &str = device.path();
    int index = str.lastIndexOf("/");
    const auto &adapterPath = str.left(index);
    const auto &deviceAddress = dBusPathToDeviceAddr(device);
    return QSharedPointer<DDevice>(new DDevice(adapterPath, deviceAddress));
}

QString DAgentAdaptor::passkeyToString(quint32 passkey) const
{
    return QStringLiteral("%1").arg(passkey, 6, 10, QLatin1Char('0'));
}

QString DAgentAdaptor::RequestPinCode(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    DRequest<QString> request(RequestDest::OrgBluezAgent, msg);

    const auto ptr = deviceFromUbi(device);
    if (!ptr->isValid()) {
        request.cancel();
        return QString{};
    }
    m_agent->requestPinCode(ptr, request);
    return QString{};
}

void DAgentAdaptor::DisplayPinCode(const QDBusObjectPath &device, const QString &pinCode)
{
    const auto ptr = deviceFromUbi(device);
    if (!ptr->isValid())
        return;
    m_agent->displayPinCode(ptr, pinCode);
}

quint32 DAgentAdaptor::RequestPasskey(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    DRequest<quint32> request(RequestDest::OrgBluezAgent, msg);
    const auto ptr = deviceFromUbi(device);
    if (!ptr->isValid()) {
        request.cancel();
        return 0;
    }

    m_agent->requestPasskey(ptr, request);
    return 0;
}

void DAgentAdaptor::DisplayPasskey(const QDBusObjectPath &device, quint32 passkey, quint16 entered)
{
    const auto ptr = deviceFromUbi(device);
    if (!ptr->isValid())
        return;
    m_agent->displayPasskey(ptr, passkeyToString(passkey), QString::number(entered));
}

void DAgentAdaptor::RequestConfirmation(const QDBusObjectPath &device, quint32 passkey, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    DRequest<> request(RequestDest::OrgBluezAgent, msg);
    const auto ptr = deviceFromUbi(device);
    if (!ptr->isValid()) {
        request.cancel();
        return;
    }

    m_agent->requestConfirmation(ptr, passkeyToString(passkey), request);
}

void DAgentAdaptor::RequestAuthorization(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    DRequest<> request(RequestDest::OrgBluezAgent, msg);
    const auto ptr = deviceFromUbi(device);
    if (!ptr->isValid()) {
        request.cancel();
        return;
    }

    m_agent->requestAuthorization(ptr, request);
}

void DAgentAdaptor::AuthorizeService(const QDBusObjectPath &device, const QString &uuid, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    DRequest<> req(RequestDest::OrgBluezAgent, msg);

    const auto ptr = deviceFromUbi(device);
    if (!ptr->isValid()) {
        req.cancel();
        return;
    }

    m_agent->authorizeService(ptr, uuid.toUpper(), req);
}

void DAgentAdaptor::Cancel()
{
    m_agent->cancel();
}

void DAgentAdaptor::Release()
{
    m_agent->release();
}

DBLUETOOTH_END_NAMESPACE
