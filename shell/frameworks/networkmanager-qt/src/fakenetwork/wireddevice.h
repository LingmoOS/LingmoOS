/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_FAKE_NETWORK_WIRED_DEVICE_H
#define NETWORKMANAGERQT_FAKE_NETWORK_WIRED_DEVICE_H

#include <QObject>

#include "../device.h"
#include "../generictypes.h"

#include "device.h"

class WiredDevice : public Device
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakenetwork.Device.Wired")
public:
    explicit WiredDevice(QObject *parent = nullptr);
    ~WiredDevice() override;

    Q_PROPERTY(bool Carrier READ carrier)
    Q_PROPERTY(QString HwAddress READ hwAddress)
    Q_PROPERTY(QString PermHwAddress READ permHwAddress)
    Q_PROPERTY(uint Speed READ speed)

    bool carrier() const;
    QString hwAddress() const;
    QString permHwAddress() const;
    uint speed() const;

    /* Not part of DBus interface */
    void setCarrier(bool carrier);
    void setHwAddress(const QString &hwAddress);
    void setPermanentHwAddress(const QString &permanentHwAddress);
    void setSpeed(uint speed);
    void setState(uint state) override;

Q_SIGNALS:
    Q_SCRIPTABLE void PropertiesChanged(const QVariantMap &properties);

private:
    bool m_carrier;
    QString m_hwAddress;
    QString m_permHwAddress;
    uint m_speed;
};

#endif
