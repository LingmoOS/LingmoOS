/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2011-2013 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_ACTIVECONNECTION_H
#define NETWORKMANAGERQT_ACTIVECONNECTION_H

#include <QObject>
#include <QSharedPointer>

#include "connection.h"
#include "dhcp4config.h"
#include "dhcp6config.h"
#include "ipconfig.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class ActiveConnectionPrivate;

/**
 * An active connection
 */
class NETWORKMANAGERQT_EXPORT ActiveConnection : public QObject
{
    Q_OBJECT

public:
    typedef QSharedPointer<ActiveConnection> Ptr;
    typedef QList<Ptr> List;
    /**
     * Enum describing possible active connection states
     */
    enum State {
        Unknown = 0, /**< The active connection is in an unknown state */
        Activating, /**< The connection is activating */
        Activated, /**< The connection is activated */
        Deactivating, /**< The connection is being torn down and cleaned up */
        Deactivated, /**< The connection is no longer active */
    };

    enum Reason {
        UknownReason = 0, /**< The reason for the active connection state change is unknown */
        None, /**< No reason was given for the active connection state change */
        UserDisconnected, /**< The active connection changed state because the user disconnected it */
        DeviceDisconnected, /**< The active connection changed state because the device it was using was disconnected */
        ServiceStopped, /**< The service providing the VPN connection was stopped */
        IpConfigInvalid, /**< The IP config of the active connection was invalid */
        ConnectTimeout, /**< The connection attempt to the VPN service timed out */
        ServiceStartTimeout, /**< A timeout occurred while starting the service providing the VPN connection */
        ServiceStartFailed, /**< Starting the service providing the VPN connection failed */
        NoSecrets, /**< Necessary secrets for the connection were not provided */
        LoginFailed, /**< Authentication to the server failed */
        ConnectionRemoved, /**< The connection was deleted from settings */
        DependencyFailed, /**< Master connection of this connection failed to activate */
        DeviceRealizeFailed, /**< Could not create the software device link */
        DeviceRemoved, /**< The device this connection depended on disappeared */
    };

    /**
     * Creates a new ActiveConnection object.
     *
     * @param path the DBus path of the device
     */
    explicit ActiveConnection(const QString &path, QObject *parent = nullptr);

    /**
     * Destroys an ActiveConnection object.
     */
    ~ActiveConnection() override;

    /**
     * Returns true is this object holds a valid connection
     */
    bool isValid() const;
    /**
     * Return path of the connection object
     */
    QString path() const;
    /**
     * Returns a valid NetworkManager::Connection object
     */
    Connection::Ptr connection() const;
    /**
     * Whether this connection has the default IPv4 route
     */
    bool default4() const;
    /**
     * Whether this connection has the default IPv6 route
     */
    bool default6() const;
    /**
     * The Ip4Config object describing the configuration of the
     * connection. Only valid when the connection is in the
     * NM_ACTIVE_CONNECTION_STATE_ACTIVATED state
     */
    IpConfig ipV4Config() const;
    /**
     * The Ip6Config object describing the configuration of the
     * connection. Only valid when the connection is in the
     * NM_ACTIVE_CONNECTION_STATE_ACTIVATED state
     */
    IpConfig ipV6Config() const;
    /**
     * The Dhcp4Config object describing the DHCP options
     * returned by the DHCP server (assuming the connection used DHCP). Only
     * valid when the connection is in the NM_ACTIVE_CONNECTION_STATE_ACTIVATED
     * state
     */
    Dhcp4Config::Ptr dhcp4Config() const;
    /**
     * The Dhcp6Config object describing the DHCP options
     * returned by the DHCP server (assuming the connection used DHCP). Only
     * valid when the connection is in the NM_ACTIVE_CONNECTION_STATE_ACTIVATED
     * state
     */
    Dhcp6Config::Ptr dhcp6Config() const;
    /**
     * The Id of the connection
     */
    QString id() const;
    /**
     * The type of the connection
     */
    NetworkManager::ConnectionSettings::ConnectionType type() const;
    /**
     * Returns the uni of master device if the connection is a slave.
     */
    QString master() const;
    /**
     * The path of the specific object associated with the connection.
     */
    QString specificObject() const;
    /**
     * The current state of the connection
     */
    NetworkManager::ActiveConnection::State state() const;
    /**
     * Whether this is a VPN connection
     */
    bool vpn() const;
    /**
     * The UUID of the connection.
     */
    QString uuid() const;
    /**
     * List of devices UNIs which are part of this connection.
     */
    QStringList devices() const;

Q_SIGNALS:
    /**
     * This signal is emitted when the connection path has changed
     */
    void connectionChanged(const NetworkManager::Connection::Ptr &connection);
    /**
     * The state of the default IPv4 route changed
     */
    void default4Changed(bool isDefault);
    /**
     * The state of the default IPv6 route changed
     */
    void default6Changed(bool isDefault);
    /**
     * Emitted when the DHCP configuration for IPv4 of this network has changed.
     */
    void dhcp4ConfigChanged();
    /**
     * Emitted when the DHCP configuration for IPv6 of this network has changed.
     */
    void dhcp6ConfigChanged();
    /**
     * Emitted when the IPv4 configuration of this network has changed.
     */
    void ipV4ConfigChanged();
    /**
     * Emitted when the IPv6 configuration of this network has changed.
     */
    void ipV6ConfigChanged();
    /**
     * The @p id changed
     */
    void idChanged(const QString &id);
    /**
     * The @p type changed
     */
    void typeChanged(NetworkManager::ConnectionSettings::ConnectionType type);
    /**
     * The master device changed.
     */
    void masterChanged(const QString &uni);
    /**
     * The @p path to the specific object changed
     */
    void specificObjectChanged(const QString &path);
    /**
     * The @p state changed
     */
    void stateChanged(NetworkManager::ActiveConnection::State state);
    /**
     * The @p state changed because of reason @p reason
     * (never emitted in runtime NM < 1.8.0)
     */
    void stateChangedReason(NetworkManager::ActiveConnection::State state, NetworkManager::ActiveConnection::Reason reason);
    /**
     * The VPN property changed.
     */
    void vpnChanged(bool isVpn);
    /**
     * The @p uuid changed.
     */
    void uuidChanged(const QString &uuid);
    /**
     * The list of devices changed.
     */
    void devicesChanged();

protected:
    NETWORKMANAGERQT_NO_EXPORT explicit ActiveConnection(ActiveConnectionPrivate &dd, QObject *parent = nullptr);

    ActiveConnectionPrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE(ActiveConnection)
};

} // namespace NetworkManager
#endif // NETWORKMANAGERQT_ACTIVECONNECTION_H
