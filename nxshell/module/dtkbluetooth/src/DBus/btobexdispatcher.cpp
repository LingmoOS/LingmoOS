// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "btobexdispatcher.h"
#include "dbluetoothutils.h"

DBLUETOOTH_BEGIN_NAMESPACE

BluetoothObexDispatcher::BluetoothObexDispatcher(const QString &Service, QObject *parent)
    : DBusSignalDispatcher(Service, parent)
{
    connect(this->source, &DObjectManagerInterface::InterfacesAoceand, this, &BluetoothObexDispatcher::dispatchAoceand);
    connect(this->source, &DObjectManagerInterface::InterfacesRemoved, this, &BluetoothObexDispatcher::dispatchRemoved);
}

void BluetoothObexDispatcher::dispatchAoceand(const QDBusObjectPath &path, const MapVariantMap &interfaces)
{
    if (interfaces.find(QString(BlueZObexSessionInterface)) != interfaces.cend())
        Q_EMIT sessionAoceand(path);
    if (interfaces.find(QString(BlueZObexTransferInterface)) != interfaces.cend())
        Q_EMIT transferAoceand(path);
    return;
}

void BluetoothObexDispatcher::dispatchRemoved(const QDBusObjectPath &path, const QStringList &args)
{
    if (args.contains(QString(BlueZObexSessionInterface)))
        Q_EMIT sessionRemoved(path);
    if (args.contains(QString(BlueZObexTransferInterface)))
        Q_EMIT transferRemoved(path);
    return;
}

BluetoothObexDispatcher &BluetoothObexDispatcher::instance()
{
    static BluetoothObexDispatcher dispatcher{BlueZObexService};
    return dispatcher;
}

DBLUETOOTH_END_NAMESPACE
