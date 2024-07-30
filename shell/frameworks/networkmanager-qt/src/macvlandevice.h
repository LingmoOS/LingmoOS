/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_MACVLAN_DEVICE_H
#define NETWORKMANAGERQT_MACVLAN_DEVICE_H

#include "device.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class MacVlanDevicePrivate;

/**
 * A macvlan device interface
 */
class NETWORKMANAGERQT_EXPORT MacVlanDevice : public Device
{
    Q_OBJECT
    Q_PROPERTY(QString mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(bool noPromisc READ noPromisc NOTIFY noPromiscChanged)
    Q_PROPERTY(QString parent READ parent NOTIFY parentChanged)

public:
    typedef QSharedPointer<MacVlanDevice> Ptr;
    typedef QList<Ptr> List;
    explicit MacVlanDevice(const QString &path, QObject *parent = nullptr);
    ~MacVlanDevice() override;

    Type type() const override;

    QString mode() const;
    bool noPromisc() const;
    QString parent() const;

Q_SIGNALS:
    void modeChanged(const QString &mode);
    void noPromiscChanged(bool noPromisc);
    void parentChanged(const QString &parent);

private:
    Q_DECLARE_PRIVATE(MacVlanDevice)
};

}

#endif
