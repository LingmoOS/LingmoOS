// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OBEXSESSIONSERVICE_H
#define OBEXSESSIONSERVICE_H

#include <QDBusObjectPath>
#include "dbluetoothutils.h"

class FakeObexSessionService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.Session1")
public:
    explicit FakeObexSessionService(QObject *parent = nullptr)
        : QObject(parent)
    {
        registerService();
    }

    ~FakeObexSessionService() { unregisterService(); }

    Q_PROPERTY(QString Source READ source)
    Q_PROPERTY(QString Destination READ destination)
    Q_PROPERTY(QString Target READ target)
    Q_PROPERTY(QString Root READ root)

    QString m_source{"AA:AA:AA:AA:AA:AA"};
    QString m_destination{"BB:BB:BB:BB:BB:BB"};
    QString m_target{"XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"};
    QString m_root{"/home/test/.cache/obex"};

    bool m_getCap{false};
    QString source() const { return m_source; };
    QString destination() const { return m_destination; };
    QString target() const { return m_target; };
    QString root() const { return m_root; };

public Q_SLOTS:
    QString GetCapabilities()
    {
        m_getCap = true;
        return {};
    }

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
        if (!bus.registerObject("/org/bluez/obex/client/session0", this, QDBusConnection::ExportAllContents)) {
            qWarning() << QString("Can't register %1 the D-Bus object.").arg("/org/bluez/obex/client/session0");
        }
    }
    void unregisterService()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        bus.unregisterObject("/org/bluez/obex/client/session0");
        bus.unregisterService(Dtk::Bluetooth::FakeBlueZObexService);
    }
};

#endif
