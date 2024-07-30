/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_ACTIVECONNECTION_P_H
#define NETWORKMANAGERQT_ACTIVECONNECTION_P_H

#include "activeconnection.h"
#include "activeconnectioninterface.h"

namespace NetworkManager
{
class ActiveConnectionPrivate : public QObject
{
    Q_OBJECT
public:
    explicit ActiveConnectionPrivate(const QString &, ActiveConnection *q);
    ~ActiveConnectionPrivate() override;
    static NetworkManager::ActiveConnection::State convertActiveConnectionState(uint);
    static NetworkManager::ActiveConnection::Reason convertActiveConnectionReason(uint);
    NetworkManager::Connection::Ptr connection;
    QString path;
    bool default4;
    bool default6;
    QStringList devices;
    OrgFreedesktopNetworkManagerConnectionActiveInterface iface;
    mutable Dhcp4Config::Ptr dhcp4Config;
    QString dhcp4ConfigPath;
    mutable Dhcp6Config::Ptr dhcp6Config;
    QString dhcp6ConfigPath;
    mutable IpConfig ipV4Config;
    QString ipV4ConfigPath;
    mutable IpConfig ipV6Config;
    QString ipV6ConfigPath;
    QString id;
    QString type;
    QString specificObject;
    ActiveConnection::State state;
    bool vpn;
    QString uuid;
    QString master;

    Q_DECLARE_PUBLIC(ActiveConnection)
    ActiveConnection *q_ptr;

public:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    virtual void propertyChanged(const QString &property, const QVariant &value);

public Q_SLOTS:
    void propertiesChanged(const QVariantMap &properties);
    void stateChanged(uint state, uint reason);

private Q_SLOTS:
    void dbusPropertiesChanged(const QString &interfaceName, const QVariantMap &properties, const QStringList &invalidatedProperties);
};

}
#endif
