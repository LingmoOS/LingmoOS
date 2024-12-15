// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MANAGERSERVICE_H
#define MANAGERSERVICE_H

#include <QDBusObjectPath>
#include "dbluetoothutils.h"

class FakeAgentManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.AgentManager1")
public:
    explicit FakeAgentManagerService(QObject *parent = nullptr)
        : QObject(parent)
    {
        registerService();
    }

    ~FakeAgentManagerService() { unregisterService(); }

    bool m_registerAgent{false};
    bool m_requestDefaultAgent{false};
    bool m_unregisterAgent{false};

public Q_SLOTS:
    void RegisterAgent(const QDBusObjectPath &, const QString &) { m_registerAgent = true; }
    void RequestDefaultAgent(const QDBusObjectPath &) { m_requestDefaultAgent = true; }
    void UnregisterAgent(const QDBusObjectPath &) { m_unregisterAgent = true; }

private:
    void registerService()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.registerService(Dtk::Bluetooth::FakeBlueZService)) {
            QString errorMsg = bus.lastError().message();
            if (errorMsg.isEmpty())
                errorMsg = "maybe it's running";
            qWarning() << QString("Can't register the %1 service, %2.").arg(Dtk::Bluetooth::FakeBlueZService).arg(errorMsg);
        }
        if (!bus.registerObject("/org/bluez", this, QDBusConnection::ExportAllContents)) {
            qWarning() << QString("Can't register %1 the D-Bus object.").arg("/org/bluez");
        }
    }
    void unregisterService()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        bus.unregisterObject("/org/bluez");
        bus.unregisterService(Dtk::Bluetooth::FakeBlueZService);
    }
};

#endif
