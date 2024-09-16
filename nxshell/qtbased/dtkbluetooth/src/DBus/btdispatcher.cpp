// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "btdispatcher.h"
#include "dbluetoothutils.h"

DBLUETOOTH_BEGIN_NAMESPACE

BluetoothDispatcher::BluetoothDispatcher(const QString &Service, QObject *parent)
    : DBusSignalDispatcher(Service, parent)
{
    connect(this->source, &DObjectManagerInterface::InterfacesAdded, this, &BluetoothDispatcher::dispatchAdded);
    connect(this->source, &DObjectManagerInterface::InterfacesRemoved, this, &BluetoothDispatcher::dispatchRemoved);
}

void BluetoothDispatcher::dispatchAdded(const QDBusObjectPath &path, const MapVariantMap &interfaces)
{
    if (interfaces.find(QString(BlueZAdapterInterface)) != interfaces.cend())
        Q_EMIT adapterAdded(path);
    if (interfaces.find(QString(BlueZDeviceInterface)) != interfaces.cend())
        Q_EMIT deviceAdded(path);
    return;
}

void BluetoothDispatcher::dispatchRemoved(const QDBusObjectPath &path, const QStringList &args)
{
    if (args.contains(QString(BlueZAdapterInterface)))
        Q_EMIT adapterRemoved(path);
    if (args.contains(QString(BlueZDeviceInterface)))
        Q_EMIT deviceRemoved(path);
    return;
}

BluetoothDispatcher &BluetoothDispatcher::instance()
{
    static BluetoothDispatcher dispatcher{BlueZService};
    return dispatcher;
}

DBLUETOOTH_END_NAMESPACE
