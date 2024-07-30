/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_FAKE_NETWORK_WIRELESS_DEVICE_H
#define NETWORKMANAGERQT_FAKE_NETWORK_WIRELESS_DEVICE_H

#include <QObject>

#include <QDBusObjectPath>

#include "../device.h"
#include "../generictypes.h"

#include "accesspoint.h"
#include "device.h"

class WirelessDevice : public Device
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakenetwork.Device.Wireless")
public:
    explicit WirelessDevice(QObject *parent = nullptr);
    ~WirelessDevice() override;

    Q_PROPERTY(QList<QDBusObjectPath> AccessPoints READ accessPoints)
    Q_PROPERTY(QDBusObjectPath ActiveAccessPoint READ activeAccessPoint)
    Q_PROPERTY(uint Bitrate READ bitrate)
    Q_PROPERTY(QString HwAddress READ hwAddress)
    Q_PROPERTY(uint Mode READ mode)
    Q_PROPERTY(QString PermHwAddress READ permHwAddress)
    Q_PROPERTY(uint WirelessCapabilities READ wirelessCapabilities)

    QList<QDBusObjectPath> accessPoints() const;
    QDBusObjectPath activeAccessPoint() const;
    uint bitrate() const;
    QString hwAddress() const;
    uint mode() const;
    QString permHwAddress() const;
    uint wirelessCapabilities() const;

    /* Not part of DBus interface */
    void addAccessPoint(AccessPoint *accessPoint);
    void removeAccessPoint(AccessPoint *accessPoint);
    void setActiveAccessPoint(const QString &activeAccessPoint);
    void setBitrate(uint bitrate);
    void setHwAddress(const QString &hwAddress);
    void setMode(uint mode);
    void setPermHwAddress(const QString &permHwAddress);
    void setState(uint state) override;
    void setWirelessCapabilities(uint capabilities);

public Q_SLOTS:
    Q_SCRIPTABLE QList<QDBusObjectPath> GetAccessPoints();
    Q_SCRIPTABLE QList<QDBusObjectPath> GetAllAccessPoints();
    Q_SCRIPTABLE void RequestScan(const QVariantMap &options);

Q_SIGNALS:
    Q_SCRIPTABLE void AccessPointAdded(const QDBusObjectPath &access_point);
    Q_SCRIPTABLE void AccessPointRemoved(const QDBusObjectPath &access_point);
    Q_SCRIPTABLE void PropertiesChanged(const QVariantMap &properties);

private:
    QMap<QDBusObjectPath, AccessPoint *> m_accessPoints;
    QDBusObjectPath m_activeAccessPoint;
    uint m_bitrate;
    QString m_hwAddress;
    uint m_mode;
    QString m_permHwAddress;
    uint m_wirelessCapabilities;

    /* Not part of DBus interface */
    int m_accessPointCounter;
};

#endif
