// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BTOBEXDISPATCHER_H
#define BTOBEXDISPATCHER_H

#include "dbussignaldispatcher.h"

DBLUETOOTH_BEGIN_NAMESPACE

class BluetoothObexDispatcher : public DBusSignalDispatcher
{
    Q_OBJECT
private:
    explicit BluetoothObexDispatcher(const QString &Service, QObject *parent = nullptr);
    ~BluetoothObexDispatcher() override = default;

public:
    static BluetoothObexDispatcher &instance();
    BluetoothObexDispatcher(const BluetoothObexDispatcher &) = delete;
    BluetoothObexDispatcher(BluetoothObexDispatcher &&) = delete;

public Q_SLOTS:
    void dispatchAdded(const QDBusObjectPath &path, const MapVariantMap &interfaces) override;
    void dispatchRemoved(const QDBusObjectPath &path, const QStringList &args) override;

Q_SIGNALS:
    void sessionAdded(const QDBusObjectPath &sessions);
    void sessionRemoved(const QDBusObjectPath &sessions);
    void transferAdded(const QDBusObjectPath &transfers);
    void transferRemoved(const QDBusObjectPath &transfers);
};

DBLUETOOTH_END_NAMESPACE

#endif
