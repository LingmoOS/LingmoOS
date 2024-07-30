/*
    SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_IPTUNNEL_DEVICE_H
#define NETWORKMANAGERQT_IPTUNNEL_DEVICE_H

#include "device.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class IpTunnelDevicePrivate;

/**
 * A Ip Tunnel device interface
 */
class NETWORKMANAGERQT_EXPORT IpTunnelDevice : public Device
{
    Q_OBJECT
    Q_PROPERTY(uchar encapsulationLimit READ encapsulationLimit NOTIFY encapsulationLimitChanged)
    Q_PROPERTY(uint flowLabel READ flowLabel NOTIFY flowLabelChanged)
    Q_PROPERTY(QString inputKey READ inputKey NOTIFY inputKeyChanged)
    Q_PROPERTY(QString local READ local NOTIFY localChanged)
    Q_PROPERTY(uint mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(QString outputKey READ outputKey NOTIFY outputKeyChanged)
    Q_PROPERTY(NetworkManager::Device::Ptr parent READ parent NOTIFY parentChanged)
    Q_PROPERTY(bool pathMtuDiscovery READ pathMtuDiscovery NOTIFY pathMtuDiscoveryChanged)
    Q_PROPERTY(QString remote READ remote NOTIFY remoteChanged)
    Q_PROPERTY(uchar tos READ tos NOTIFY tosChanged)
    Q_PROPERTY(uchar ttl READ ttl NOTIFY ttlChanged)

public:
    typedef QSharedPointer<IpTunnelDevice> Ptr;
    typedef QList<Ptr> List;
    explicit IpTunnelDevice(const QString &path, QObject *parent = nullptr);
    ~IpTunnelDevice() override;

    Type type() const override;

    /**
     * How many additional levels of encapsulation are permitted to be prepended to packets.
     * This property applies only to IPv6 tunnels.
     */
    uchar encapsulationLimit() const;
    /**
     * The flow label to assign to tunnel packets. This property applies only to IPv6 tunnels.
     */
    uint flowLabel() const;
    /**
     * The key used for incoming packets.
     */
    QString inputKey() const;
    /**
     * The local endpoint of the tunnel.
     */
    QString local() const;
    /**
     * The tunneling mode.
     */
    uint mode() const;
    /**
     * The key used for outgoing packets.
     */
    QString outputKey() const;
    /**
     * The object path of the parent device.
     */
    NetworkManager::Device::Ptr parent() const;
    /**
     * Whether path MTU discovery is enabled on this tunnel.
     */
    bool pathMtuDiscovery() const;
    /**
     * The remote endpoint of the tunnel.
     */
    QString remote() const;
    /**
     * The type of service (IPv4) or traffic class (IPv6) assigned to tunneled packets.
     */
    uchar tos() const;
    /**
     * The TTL assigned to tunneled packets. 0 is a special value meaning that packets inherit the TTL value
     */
    uchar ttl() const;

Q_SIGNALS:
    /**
     * Emitted when the encapsulation limit has changed
     */
    void encapsulationLimitChanged(uchar limit);
    /**
     * Emitted when the flow label has changed
     */
    void flowLabelChanged(uint flowLabel);
    /**
     * Emitted when the key used for incoming packets has changed
     */
    void inputKeyChanged(const QString &inputKey);
    /**
     * Emitted when the local endpoint of the tunnel has changed
     */
    void localChanged(const QString &local);
    /**
     * Emitted when the tunneling mode has changed
     */
    void modeChanged(uint mode);
    /**
     * Emitted when the key used for outgoing packets has changed
     */
    void outputKeyChanged(const QString &outputKey);
    /**
     * Emitted when the parent of this device has changed
     */
    void parentChanged(const QString &parent);
    /**
     * Emitted when the path MTU discovery enablemened has changed
     */
    void pathMtuDiscoveryChanged(bool pathMtuDiscovery);
    /**
     * Emitted when the remote endpoint of the tunnel has changed
     */
    void remoteChanged(const QString &remote);
    /**
     * Emitted when the type of service or traffic class assigned to tunneled packets has changed
     */
    void tosChanged(uchar tos);
    /**
     * Emitted when the TTL assigned to tunneled packets has changed
     */
    void ttlChanged(uchar ttl);

private:
    Q_DECLARE_PRIVATE(IpTunnelDevice)
};

}

#endif
