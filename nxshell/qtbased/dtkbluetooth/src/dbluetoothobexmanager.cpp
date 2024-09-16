// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbluetoothobexmanager_p.h"
#include "dobexagentmanagerinterface.h"
#include "dbluetoothutils.h"
#include "dbluetoothobexagent.h"
#include "btobexdispatcher.h"
#include "dbluetoothobexsession.h"
#include <QFileInfo>

DBLUETOOTH_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DTK_CORE_NAMESPACE::emplace_tag;

DObexManagerPrivate::DObexManagerPrivate(DObexManager *parent)
    : DObjectPrivate(parent)
    , m_obexAgentManager(new DObexAgentManagerInterface())
    , m_client(new DObexClientInterface())
{
}

DObexManagerPrivate::~DObexManagerPrivate()
{
    delete m_obexAgentManager;
    delete m_client;
}

DObexManager::DObexManager(QObject *parent)
    : QObject(parent)
    , DObject(*new DObexManagerPrivate(this))
{
    connect(&BluetoothObexDispatcher::instance(),
            &BluetoothObexDispatcher::sessionAdded,
            this,
            [this](const QDBusObjectPath &session) { Q_EMIT this->sessionAdded(dBusPathToSessionInfo(session)); });

    connect(&BluetoothObexDispatcher::instance(),
            &BluetoothObexDispatcher::sessionRemoved,
            this,
            [this](const QDBusObjectPath &session) { Q_EMIT this->sessionRemoved(dBusPathToSessionInfo(session)); });
}

bool DObexManager::available() const
{
    QFileInfo bluetoothDir("/sys/class/bluetooth");
    if (bluetoothDir.exists())
        return true;
    else
        return false;
}

DExpected<ObexSessionInfo> DObexManager::createSession(const QString &destination, const QVariantMap &args) const
{
    D_DC(DObexManager);
    auto reply = d->m_client->createSession(destination, args);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return dBusPathToSessionInfo(reply.value());
}

DExpected<QList<ObexSessionInfo>> DObexManager::sessions() const
{
    auto reply = BluetoothObexDispatcher::instance().getManagedObjects();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    }
    const auto &sessionList = getSpecificObject(reply.value(), {QString(BlueZObexSessionInterface)});
    QList<ObexSessionInfo> ret;
    for (const auto &session : sessionList)
        ret.append(dBusPathToSessionInfo(session));
    return ret;
}

DExpected<QSharedPointer<DObexSession>> DObexManager::sessionFromInfo(const ObexSessionInfo &info)
{
    const auto &reply = sessions();
    if (!reply)
        return DUnexpected{reply.error()};
    const auto &sessionList = reply.value();
    if (!sessionList.contains(info))
        return DUnexpected{emplace_tag::USE_EMPLACE, -1, "this session doesn't exists"};
    return QSharedPointer<DObexSession>(new DObexSession(info));
}

DExpected<void> DObexManager::removeSession(const QSharedPointer<DObexSession> session) const
{
    D_DC(DObexManager);
    auto reply = d->m_client->removeSession(QDBusObjectPath{sessionInfoToDBusPath(session->currentSession())});
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<void> DObexManager::registerAgent(const QSharedPointer<DObexAgent> &agent) const
{
    D_DC(DObexManager);
    auto reply = d->m_obexAgentManager->registerAgent(agent->agentPath());
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<void> DObexManager::unregisterAgent(const QSharedPointer<DObexAgent> &agent) const
{
    D_DC(DObexManager);
    auto reply = d->m_obexAgentManager->unregisterAgent(agent->agentPath());
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DBLUETOOTH_END_NAMESPACE
