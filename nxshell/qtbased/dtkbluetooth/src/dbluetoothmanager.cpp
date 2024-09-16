// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbluetoothmanager_p.h"
#include "dagentmanagerinterface.h"
#include "btdispatcher.h"
#include "dbluetoothutils.h"
#include <DObject>
#include <QFileInfo>

DBLUETOOTH_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DTK_CORE_NAMESPACE::emplace_tag;

DManagerPrivate::DManagerPrivate(DManager *parent)
    : DObjectPrivate(parent)
    , m_agentManager(new DAgentManagerInterface(parent))
{
}

DManagerPrivate::~DManagerPrivate()
{
    delete m_agentManager;
}

DManager::DManager(QObject *parent)
    : QObject(parent)
    , DObject(*new DManagerPrivate(this))
{
    connect(&BluetoothDispatcher::instance(), &BluetoothDispatcher::adapterAdded, this, [this](const QDBusObjectPath &adapter) {
        Q_EMIT this->adapterAdded(dBusPathToAdapterId(adapter));
    });

    connect(&BluetoothDispatcher::instance(), &BluetoothDispatcher::adapterRemoved, this, [this](const QDBusObjectPath &adapter) {
        Q_EMIT this->adapterRemoved(dBusPathToAdapterId(adapter));
    });
}

DExpected<QList<quint64>> DManager::adapters() const
{
    auto reply = BluetoothDispatcher::instance().getManagedObjects();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    }
    const auto &adapterList = getSpecificObject(reply.value(), {QString(BlueZAdapterInterface)});
    QList<quint64> ret;
    for (const auto &adapter : adapterList)
        ret.append(dBusPathToAdapterId(adapter));
    return ret;
};

bool DManager::available() const
{
    QFileInfo bluetoothDir("/sys/class/bluetooth");
    if (bluetoothDir.exists())
        return true;
    else
        return false;
};

DExpected<QSharedPointer<DAdapter>> DManager::adapterFromId(quint64 adapterId) const
{
    const auto &reply = adapters();
    if (!reply)
        return DUnexpected{reply.error()};
    const auto &adapterList = reply.value();
    if (!adapterList.contains(adapterId))
        return DUnexpected{emplace_tag::USE_EMPLACE, -1, "this adapter doesn't exists"};
    return QSharedPointer<DAdapter>(new DAdapter(adapterId));
};

DExpected<void> DManager::registerAgent(const QSharedPointer<DAgent> &agent)
{
    D_DC(DManager);
    auto reply = d->m_agentManager->registerAgent(agent->agentPath(), agentCapToString(agent->capability()));
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
};

DExpected<void> DManager::unregisterAgent(const QSharedPointer<DAgent> &agent)
{
    D_DC(DManager);
    auto reply = d->m_agentManager->unregisterAgent(agent->agentPath());
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
};

DExpected<void> DManager::requestDefaultAgent(const QSharedPointer<DAgent> &agent)
{
    D_DC(DManager);
    auto reply = d->m_agentManager->requestDefaultAgent(agent->agentPath());
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
};

DBLUETOOTH_END_NAMESPACE
