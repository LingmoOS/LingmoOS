/*
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_GRE_DEVICE_H
#define NETWORKMANAGERQT_GRE_DEVICE_H

#include "device.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class GreDevicePrivate;

/**
 * A gre device interface
 * @deprecated renamed to IpTunnelDevice
 */
class NETWORKMANAGERQT_EXPORT GreDevice : public Device
{
    Q_OBJECT
    Q_PROPERTY(ushort inputFlags READ inputFlags NOTIFY inputFlagsChanged)
    Q_PROPERTY(ushort outputFlags READ outputFlags NOTIFY outputFlagsChanged)
    Q_PROPERTY(uint inputKey READ inputKey NOTIFY inputKeyChanged)
    Q_PROPERTY(uint outputKey READ outputKey NOTIFY outputKeyChanged)
    Q_PROPERTY(QString localEnd READ localEnd NOTIFY localEndChanged)
    Q_PROPERTY(QString remoteEnd READ remoteEnd NOTIFY remoteEndChanged)
    Q_PROPERTY(QString parent READ parent NOTIFY parentChanged)
    Q_PROPERTY(bool pathMtuDiscovery READ pathMtuDiscovery NOTIFY pathMtuDiscoveryChanged)
    Q_PROPERTY(uchar tos READ tos NOTIFY tosChanged)
    Q_PROPERTY(uchar ttl READ ttl NOTIFY ttlChanged)

public:
    typedef QSharedPointer<GreDevice> Ptr;
    typedef QList<Ptr> List;
    explicit GreDevice(const QString &path, QObject *parent = nullptr);
    ~GreDevice() override;

    Type type() const override;

    ushort inputFlags() const;
    ushort outputFlags() const;
    uint inputKey() const;
    uint outputKey() const;
    QString localEnd() const;
    QString remoteEnd() const;
    QString parent() const;
    bool pathMtuDiscovery() const;
    uchar tos() const;
    uchar ttl() const;

Q_SIGNALS:
    void inputFlagsChanged(ushort inputflags);
    void outputFlagsChanged(ushort outputFlags);
    void inputKeyChanged(uint inputKey);
    void outputKeyChanged(uint outputKey);
    void localEndChanged(const QString &localEnd);
    void remoteEndChanged(const QString &remoteEnd);
    void parentChanged(const QString &parent);
    void pathMtuDiscoveryChanged(bool pathMtuDiscovery);
    void tosChanged(uchar tos);
    void ttlChanged(uchar ttl);

private:
    Q_DECLARE_PRIVATE(GreDevice)
};

}

#endif
