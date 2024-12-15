// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OBEXMANAGERSERVICE_H
#define OBEXMANAGERSERVICE_H

#include <QDBusObjectPath>
#include "dbluetoothutils.h"

class FakeObexAgentManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.AgentManager1")
public:
    explicit FakeObexAgentManagerService(QObject *parent = nullptr)
        : QObject(parent)
    {
        registerService();
    }

    ~FakeObexAgentManagerService() { unregisterService(); }

    bool m_registerAgent{false};
    bool m_unregisterAgent{false};
public Q_SLOTS:
    void RegisterAgent(const QDBusObjectPath &) { m_registerAgent = true; }
    void UnregisterAgent(const QDBusObjectPath &) { m_unregisterAgent = true; }

private:
    void registerService()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.registerService(Dtk::Bluetooth::FakeBlueZObexService)) {
            QString errorMsg = bus.lastError().message();
            if (errorMsg.isEmpty())
                errorMsg = "maybe it's running";
            qWarning() << QString("Can't register the %1 service, %2.").arg(Dtk::Bluetooth::FakeBlueZObexService).arg(errorMsg);
        }
        if (!bus.registerObject("/org/bluez/obex", this, QDBusConnection::ExportAllContents)) {
            qWarning() << QString("Can't register %1 the D-Bus object.").arg("/org/bluez/obex");
        }
    }
    void unregisterService()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        bus.unregisterObject("/org/bluez/obex");
        bus.unregisterService(Dtk::Bluetooth::FakeBlueZObexService);
    }
};

class FakeObexClientService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.Client1")
public:
    explicit FakeObexClientService(QObject *parent = nullptr)
        : QObject(parent)
    {
        registerService();
    }

    ~FakeObexClientService() { unregisterService(); };
    bool m_createSession{false};
    bool m_removeSession{false};

public Q_SLOTS:
    QDBusObjectPath CreateSession(const QString &, const QVariantMap &)
    {
        m_createSession = true;
        return QDBusObjectPath{"/org/bluez/obex/session0"};
    };

    void RemoveSession(const QDBusObjectPath &) { m_removeSession = true; };

private:
    void registerService()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.registerService(Dtk::Bluetooth::FakeBlueZObexService)) {
            QString errorMsg = bus.lastError().message();
            if (errorMsg.isEmpty())
                errorMsg = "maybe it's running";
            qWarning() << QString("Can't register the %1 service, %2.").arg(Dtk::Bluetooth::FakeBlueZObexService).arg(errorMsg);
        }
        if (!bus.registerObject("/org/bluez/obex", this, QDBusConnection::ExportAllContents)) {
            qWarning() << QString("Can't register %1 the D-Bus object.").arg("/org/bluez/obex");
        }
    }
    void unregisterService()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        bus.unregisterObject("/org/bluez/obex");
        bus.unregisterService(Dtk::Bluetooth::FakeBlueZObexService);
    }
};

#endif
