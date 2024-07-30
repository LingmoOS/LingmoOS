/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIMAXDEVICE_P_H
#define NETWORKMANAGERQT_WIMAXDEVICE_P_H

#include "device_p.h"
#include "wimaxdeviceinterface.h"

namespace NetworkManager
{
class WimaxDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    explicit WimaxDevicePrivate(const QString &path, WimaxDevice *q);
    OrgFreedesktopNetworkManagerDeviceWiMaxInterface wimaxIface;
    QString hardwareAddress;
    mutable QMap<QString, WimaxNsp::Ptr> nspMap;
    QString activeNsp;
    uint centerFrequency;
    int cinr;
    QString bsid;
    int rssi;
    int txPower;

    Q_DECLARE_PUBLIC(WimaxDevice)
protected Q_SLOTS:
    void nspAdded(const QDBusObjectPath &);
    void nspRemoved(const QDBusObjectPath &);

protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

}

#endif
