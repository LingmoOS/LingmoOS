// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEVICESERVICE_H
#define DEVICESERVICE_H

#include <QDBusObjectPath>
#include "dbluetoothutils.h"

class FakeDeviceService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Device1")
public:
    explicit FakeDeviceService(QObject *parent = nullptr)
        : QObject(parent)
    {
        registerService();
    }
    ~FakeDeviceService() { unregisterService(); }

    Q_PROPERTY(bool Blocked READ blocked WRITE setBlocked)
    Q_PROPERTY(bool Connected READ connected)
    Q_PROPERTY(bool LegacyPairing READ legacyPairing)
    Q_PROPERTY(bool Paired READ paired)
    Q_PROPERTY(bool ServicesResolved READ servicesResolved)
    Q_PROPERTY(bool Trusted READ trusted WRITE setTrusted)
    Q_PROPERTY(QString Adapter READ adapter)
    Q_PROPERTY(QString Address READ address)
    Q_PROPERTY(QString AddressType READ addressType)
    Q_PROPERTY(QString Alias READ alias)
    Q_PROPERTY(quint32 Class READ Class)
    Q_PROPERTY(quint16 Appearance READ appearance)
    Q_PROPERTY(QStringList UUIDs READ UUIDs)
    Q_PROPERTY(QString Icon READ icon)
    Q_PROPERTY(QString Name READ name)
    Q_PROPERTY(qint16 RSSI READ RSSI)

    bool m_blocked{false};
    bool m_connected{false};
    bool m_legacyPairing{false};
    bool m_paired{false};
    bool m_servicesResolved{false};
    bool m_trusted{false};
    quint32 m_class{1835276};
    quint16 m_appearance{0x40};
    QString m_adapter{"/org/bluez/hci0"};
    QString m_address{"FF:FF:FF:FF:FF:FF"};
    QString m_addressType{"public"};
    QString m_alias{"testbt"};
    QStringList m_uuids{{"00001104-0000-1000-8000-00805f9b34fb"}};
    QString m_icon{"computer"};
    QString m_name{"testbt"};
    qint16 m_rssi{10};

    bool blocked() const { return m_blocked; };
    void setBlocked(bool blocked) { m_blocked = blocked; };
    bool connected() const { return m_connected; };
    bool legacyPairing() const { return m_legacyPairing; };
    bool paired() const { return m_paired; };
    bool servicesResolved() const { return m_servicesResolved; };
    bool trusted() const { return m_trusted; };
    void setTrusted(bool trusted) { m_trusted = trusted; };
    quint32 Class() const { return m_class; };
    quint16 appearance() const { return m_appearance; };
    QString adapter() const { return m_adapter; };
    QString address() const { return m_address; };
    QString addressType() const { return m_addressType; };
    QString alias() const { return m_alias; };
    QStringList UUIDs() const { return m_uuids; };
    QString icon() const { return m_icon; };
    QString name() const { return m_name; };
    qint16 RSSI() const { return m_rssi; };

    bool m_disconnect{false};
    bool m_connect{false};
    bool m_cancelPairing{false};
    bool m_pair{false};

public Q_SLOTS:
    void Disconnect() { m_disconnect = true; }
    void Connect() { m_connect = true; }
    void CancelPairing() { m_cancelPairing = true; }
    void Pair() { m_pair = true; };

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
        if (!bus.registerObject("/org/bluez/hci0/dev_FF_FF_FF_FF_FF_FF", this, QDBusConnection::ExportAllContents)) {
            qWarning() << QString("Can't register %1 the D-Bus object.").arg("/org/bluez/hci0/dev_FF_FF_FF_FF_FF_FF");
        }
    }
    void unregisterService()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        bus.unregisterObject("/org/bluez/hci0/dev_FF_FF_FF_FF_FF_FF");
        bus.unregisterService(Dtk::Bluetooth::FakeBlueZService);
    }
};

#endif
