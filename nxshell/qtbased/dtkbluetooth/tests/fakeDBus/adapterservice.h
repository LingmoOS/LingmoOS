// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ADAPTERSERVICE_H
#define ADAPTERSERVICE_H

#include <QDBusObjectPath>
#include "dbluetoothutils.h"

class FakeAdapterService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Adapter1")
public:
    explicit FakeAdapterService(QObject *parent = nullptr)
        : QObject(parent)
    {
        registerService();
    }
    ~FakeAdapterService() override { unregisterService(); }

    Q_PROPERTY(QString Address READ address CONSTANT)
    Q_PROPERTY(QString AddressType READ addressType)
    Q_PROPERTY(QString Name READ name)
    Q_PROPERTY(QString Alias READ alias WRITE setAlias)
    Q_PROPERTY(bool Powered READ powered WRITE setPowered)
    Q_PROPERTY(bool Discoverable READ discoverable WRITE setDiscoverable)
    Q_PROPERTY(quint32 DiscoverableTimeout READ discoverableTimeout WRITE setDiscoverableTimeout)
    Q_PROPERTY(bool Discovering READ discovering)

    QString m_address{"FF:FF:FF:FF:FF:FF"};
    QString m_addressType{"public"};
    QString m_name{"testbt"};
    QString m_alias{"alias-testbt"};
    bool m_powered{true};
    bool m_discoverable{true};
    quint32 m_discoverableTimeout{300};
    bool m_discovering{false};

    bool m_removeDevice{false};

    QString address() { return m_address; }
    QString addressType() { return m_addressType; }
    QString name() { return m_name; }
    QString alias() { return m_alias; }
    bool powered() { return m_powered; }
    bool discoverable() { return m_discoverable; }
    quint32 discoverableTimeout() { return m_discoverableTimeout; }
    bool discovering() { return m_discovering; }

    void setAlias(const QString &alias) { m_alias = alias; }
    void setPowered(bool powered) { m_powered = powered; }
    void setDiscoverable(bool discoverable) { m_discoverable = discoverable; }
    void setDiscoverableTimeout(quint32 discoverableTimeout) { m_discoverableTimeout = discoverableTimeout; }
public Q_SLOTS:
    void RemoveDevice(const QDBusObjectPath &) { m_removeDevice = true; };
    void StartDiscovery() { m_discovering = true; };
    void StopDiscovery() { m_discovering = false; };

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
        if (!bus.registerObject("/org/bluez/hci0", this, QDBusConnection::ExportAllContents)) {
            qWarning() << QString("Can't register %1 the D-Bus object.").arg("/org/bluez/hci0");
        }
    }
    void unregisterService()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        bus.unregisterObject("/org/bluez/hci0");
        bus.unregisterService(Dtk::Bluetooth::FakeBlueZService);
    }
};

#endif
