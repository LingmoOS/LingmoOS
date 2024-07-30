/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_BEARER_H
#define MODEMMANAGERQT_BEARER_H

#include <modemmanagerqt_export.h>

#include <QObject>
#include <QSharedPointer>

#include <QDBusPendingReply>

#include "generictypes.h"

namespace ModemManager
{
class BearerPrivate;

/**
 * This class represents IP configuration
 */
class MODEMMANAGERQT_EXPORT IpConfig
{
public:
    /**
     * Constructs an empty IP config object
     */
    IpConfig();

    /**
     * Destroys this IpConfig object.
     */
    ~IpConfig();

    /**
     * Constructs an IpConfig object that is a copy of the object @p other.
     */
    IpConfig(const IpConfig &other);

    /**
     * Returns the MMBearerIpMethod
     */
    MMBearerIpMethod method() const;

    /**
     * Sets the MMBearerIpMethod
     */
    void setMethod(MMBearerIpMethod method);

    /**
     * Returns the IP address
     */
    QString address() const;

    /**
     * Sets the IP address
     */
    void setAddress(const QString &address);

    /**
     * Returns the numeric CIDR network prefix (ie, 24, 32, etc)
     */
    uint prefix() const;

    /**
     * Sets the numeric CIDR network prefix
     */
    void setPrefix(uint prefix);

    /**
     * Returns the IP address of the first DNS server
     */
    QString dns1() const;

    /**
     * Sets the IP address of the first DNS server
     */
    void setDns1(const QString &dns1);

    /**
     * Returns the IP address of the second DNS server
     */
    QString dns2() const;

    /**
     * Sets the IP address of the second DNS server
     */
    void setDns2(const QString &dns2);

    /**
     * Returns the IP address of the third DNS server
     */
    QString dns3() const;

    /**
     * Sets the IP address of the third DNS server
     */
    void setDns3(const QString &dns3);

    /**
     * Returns the IP address of the default gateway
     */
    QString gateway() const;

    /**
     * Sets the IP address of the default gateway
     */
    void setGateway(const QString &gateway);

    /**
     * Makes a copy of the IpConfig object @p other.
     */
    IpConfig &operator=(const IpConfig &other);

private:
    class Private;
    Private *const d;
};

/**
 * @brief The Bearer class
 *
 * This class provides access to specific actions that may be performed on available bearers.
 */
class MODEMMANAGERQT_EXPORT Bearer : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<Bearer> Ptr;
    typedef QList<Ptr> List;

    explicit Bearer(const QString &path, QObject *parent = nullptr);
    ~Bearer() override;

    /**
     * @return the operating system name for the network data interface that
     * provides packet data using this bearer.
     *
     * Connection managers must configure this interface depending on the IP
     * "method" given by the ip4Config() or ip6Config() properties set by bearer
     * activation.
     *
     * If MM_BEARER_IP_METHOD_STATIC or MM_BEARER_IP_METHOD_DHCP methods are
     * given, the interface will be an ethernet-style interface suitable for DHCP
     * or setting static IP configuration on, while if the
     * MM_BEARER_IP_METHOD_PPP method is given, the interface will be a serial
     * TTY which must then have PPP run over it.
     *
     */
    QString interface() const;

    /**
     * @return whether or not the bearer is connected and thus whether packet
     * data communication using this bearer is possible.
     */
    bool isConnected() const;

    /**
     * In some devices, packet data service will be suspended while the device
     * is handling other communication, like a voice call. If packet data
     * service is suspended (but not deactivated) this property will be @p true
     */
    bool isSuspended() const;

    /**
     * If the bearer was configured for IPv4 addressing, upon activation
     * this property contains the addressing details for assignment to the data
     * interface.
     */
    IpConfig ip4Config() const;

    /**
     * If the bearer was configured for IPv6 addressing, upon activation this
     * property contains the addressing details for assignment to the data
     * interface.
     */
    IpConfig ip6Config() const;

    /**
     * @return maximum time to wait for a successful IP establishment, when PPP is used.
     */
    uint ipTimeout() const;

    /**
     * @return map of properties used when creating the bearer
     * @see IpConfig
     */
    QVariantMap properties() const;

    /**
     * Requests activation of a packet data connection with the network using
     * this bearer's properties. Upon successful activation, the modem can send
     * and receive packet data and, depending on the addressing capability of
     * the modem, a connection manager may need to start PPP, perform DHCP, or
     * assign the IP address returned by the modem to the data interface. Upon
     * successful return, the ip4Config() and/or ip6Config() properties become
     * valid and may contain IP configuration information for the data interface
     * associated with this bearer.
     */
    QDBusPendingReply<void> connectBearer();

    /**
     * Disconnect and deactivate this packet data connection.
     *
     * Any ongoing data session will be terminated and IP addresses become invalid when this method is called.
     */
    QDBusPendingReply<void> disconnectBearer();

    /**
     * Sets the timeout in milliseconds for all async method DBus calls.
     * -1 means the default DBus timeout (usually 25 seconds).
     */
    void setTimeout(int timeout);

    /**
     * Returns the current value of the DBus timeout in milliseconds.
     * -1 means the default DBus timeout (usually 25 seconds).
     */
    int timeout() const;

    /**
     * @return the DBUS path (uni) to the object
     */
    QString uni() const;

Q_SIGNALS:
    void interfaceChanged(const QString &iface);
    void connectedChanged(bool connected);
    void suspendedChanged(bool suspended);
    void ip4ConfigChanged(const ModemManager::IpConfig &ipv4Config);
    void ip6ConfigChanged(const ModemManager::IpConfig &ipv6Config);
    void ipTimeoutChanged(uint ipTimeout);
    void propertiesChanged(const QVariantMap &properties);

private:
    Q_DECLARE_PRIVATE(Bearer)
    BearerPrivate *const d_ptr;
};

} // namespace ModemManager

Q_DECLARE_METATYPE(ModemManager::IpConfig)

#endif
