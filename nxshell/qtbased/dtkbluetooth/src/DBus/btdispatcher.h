// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BTDISPATCHER_H
#define BTDISPATCHER_H

#include "dbussignaldispatcher.h"

DBLUETOOTH_BEGIN_NAMESPACE

class BluetoothDispatcher : public DBusSignalDispatcher
{
    Q_OBJECT
private:
    explicit BluetoothDispatcher(const QString &Service, QObject *parent = nullptr);
    ~BluetoothDispatcher() override = default;

public:
    static BluetoothDispatcher &instance();
    BluetoothDispatcher(const BluetoothDispatcher &) = delete;
    BluetoothDispatcher(BluetoothDispatcher &&) = delete;

public Q_SLOTS:
    void dispatchAdded(const QDBusObjectPath &path, const MapVariantMap &interfaces) override;
    void dispatchRemoved(const QDBusObjectPath &path, const QStringList &args) override;

Q_SIGNALS:
    void adapterAdded(const QDBusObjectPath &adapter);
    void adapterRemoved(const QDBusObjectPath &adapter);
    void deviceAdded(const QDBusObjectPath &devices);
    void deviceRemoved(const QDBusObjectPath &devices);
};

DBLUETOOTH_END_NAMESPACE

#endif
