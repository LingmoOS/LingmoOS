// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dslcontrollerinter.h"
#include "networkdevicebase.h"

#include <networkmanagerqt/manager.h>

using namespace dde::network;
using namespace NetworkManager;

DSLController_Inter::DSLController_Inter(NetworkInter *networkInter, QObject *parent)
    : DSLController(parent)
    , m_networkInter(networkInter)
{
    Q_ASSERT(m_networkInter);
}

DSLController_Inter::~DSLController_Inter()
{
}

void DSLController_Inter::connectItem(DSLItem *item)
{
    // 查找该连接对应的设备的path
    QString devicePath = "/";
    if (m_deviceInfo.contains(item->connection()->hwAddress()))
        devicePath = m_deviceInfo[item->connection()->hwAddress()];

    m_networkInter->ActivateConnection(item->connection()->uuid(), QDBusObjectPath(devicePath));
}

void DSLController_Inter::connectItem(const QString &uuid)
{
    // 根据UUID查找对应的DSL项，然后连接
    for (DSLItem *item : m_items) {
        if (item->connection()->uuid() == uuid) {
            connectItem(item);
            break;
        }
    }
}

void DSLController_Inter::disconnectItem(WiredDevice *device)
{
    Q_UNUSED(device);
    if (!m_activePath.isEmpty())
        deactivateConnection(m_activePath);
}

void DSLController_Inter::updateDevice(const QList<NetworkDeviceBase *> &devices)
{
    m_deviceInfo.clear();
    for (NetworkDeviceBase *device : devices)
        m_deviceInfo[device->realHwAdr()] = device->path();
}

void DSLController_Inter::updateDSLItems(const QJsonArray &dsljson)
{
    // 更新DSL具体项的数据，先从列表中查找是否存在当前连接的path，如果找到，就直接更新，
    // 没有找到的情况下就新建一个连接，并且将新连接添加到新列表中
    QStringList paths;
    QList<DSLItem *> newItems, changeItems;
    for (QJsonValue value : dsljson) {
        QJsonObject dsl = value.toObject();
        QString path = dsl.value("Path").toString();
        DSLItem *item = findItem(path);
        if (!item) {
            item = new DSLItem;
            m_items << item;
            newItems << item;
        } else {
            if (item->connection()->id() != dsl.value("Id").toString())
                changeItems << item;
        }

        item->setConnection(dsl);
        paths << path;
    }

    qCDebug(DNC) << "change DSL size: " << changeItems.size() << " new DSL size: " << newItems.size();

    // 如果ID发生变化的数量大于0，则向外抛出连接变化的信号
    if (changeItems.size())
        Q_EMIT itemChanged(changeItems);

    // 如果新连接列表的数量大于0,则向外抛出连接新增的信号
    if (newItems.size() > 0)
        Q_EMIT itemAdded(newItems);

    QList<DSLItem *> rmItems;
    for (DSLItem *item : m_items) {
        if (!paths.contains(item->connection()->path()))
            rmItems << item;
    }

    for (DSLItem *item : rmItems)
        m_items.removeOne(item);

    // 如果有连接移除，则向外抛出连接移除的信号
    if (rmItems.size() > 0)
        Q_EMIT itemRemoved(rmItems);

    for (DSLItem *item : rmItems)
        delete item;
}

void DSLController_Inter::updateActiveConnections(const QJsonObject &connectionJson)
{
    // 先保存所有的连接状态
    QMap<QString, ConnectionStatus> allConnectionStatus;
    // 恢复所有的状态为未连接
    for (DSLItem *item : m_items) {
        allConnectionStatus[item->connection()->uuid()] = item->status();
        item->setConnectionStatus(ConnectionStatus::Deactivated);
    }

    m_activePath.clear();
    bool statusChanged = false;
    // 找到对应的连接，将其修改成连接成功
    QStringList keys = connectionJson.keys();
    for (int i = 0; i < keys.size(); i++) {
        QString key = keys[i];
        QJsonObject json = connectionJson.value(key).toObject();
        QString uuid = json.value("Uuid").toString();
        DSLItem *dslItem = findDSLItemByUuid(uuid);
        if (dslItem) {
            ConnectionStatus state = convertConnectionStatus(json.value("State").toInt());
            dslItem->setConnectionStatus(state);
            if (state == ConnectionStatus::Activated)
                m_activePath = key;
            if (allConnectionStatus.contains(uuid)) {
                ConnectionStatus oldStatus = allConnectionStatus[uuid];
                if (!statusChanged && oldStatus != state)
                    statusChanged = true;
            }
        }
    }

    if (statusChanged)
        Q_EMIT activeConnectionChanged();
}

DSLItem *DSLController_Inter::findItem(const QString &path)
{
    for (DSLItem *item : m_items) {
        if (item->connection()->path() == path)
            return item;
    }

    return Q_NULLPTR;
}

DSLItem *DSLController_Inter::findDSLItemByUuid(const QString &uuid)
{
    // 根据uuid在列表中查询指定的dsl连接
    for (DSLItem *item : m_items) {
        if (item->connection()->uuid() == uuid)
            return item;
    }

    return Q_NULLPTR;
}
