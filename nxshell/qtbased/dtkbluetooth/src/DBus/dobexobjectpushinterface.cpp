// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dobexobjectpushinterface.h"
#include "btobexdispatcher.h"
#include "dbluetoothutils.h"
#include <QMetaType>
#include <qdbusextratypes.h>
#include <qdbusmetatype.h>

DBLUETOOTH_BEGIN_NAMESPACE

DObexObjectPushInterface::DObexObjectPushInterface(const QString &path, QObject *parent)
    : QObject(parent)
{
#ifndef USE_FAKE_INTERFACE
    const auto &Service = QLatin1String(BlueZObexService);
    const auto &Connection = QDBusConnection::systemBus();
#else
    const auto &Service = QLatin1String(FakeBlueZObexService);
    const auto &Connection = QDBusConnection::sessionBus();
#endif
    const auto &Interface = QLatin1String(BlueZObexObjectPushInterface);
    m_inter = new DDBusInterface(Service, path, Interface, Connection, this);
#ifndef USE_FAKE_INTERFACE
    connect(&BluetoothObexDispatcher::instance(),
            &BluetoothObexDispatcher::transferAdded,
            this,
            [this](const QDBusObjectPath &transfer) { Q_EMIT transferAdded(dBusPathToTransferId(transfer.path())); });
    connect(&BluetoothObexDispatcher::instance(),
            &BluetoothObexDispatcher::transferRemoved,
            this,
            [this](const QDBusObjectPath &transfer) { Q_EMIT transferRemoved(dBusPathToTransferId(transfer.path())); });
#endif
}

QDBusPendingReply<QDBusObjectPath, QVariantMap> DObexObjectPushInterface::sendFile(const QFileInfo &filePath)
{
    return m_inter->asyncCallWithArgumentList("SendFile", {QVariant::fromValue(filePath.absoluteFilePath())});
}

DBLUETOOTH_END_NAMESPACE
