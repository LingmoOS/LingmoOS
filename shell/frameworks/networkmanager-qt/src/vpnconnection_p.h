/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VPNCONNECTION_P_H
#define NETWORKMANAGERQT_VPNCONNECTION_P_H

#include "activeconnection_p.h"
#include "manager_p.h"
#include "vpnconnection.h"

#include "vpnconnectioninterface.h"

namespace NetworkManager
{
class VpnConnectionPrivate : public ActiveConnectionPrivate
{
    Q_OBJECT
public:
    VpnConnectionPrivate(const QString &path, VpnConnection *q);

    static NetworkManager::VpnConnection::State convertVpnConnectionState(uint state);
    static NetworkManager::VpnConnection::StateChangeReason convertVpnConnectionStateReason(uint reason);

    QString banner;
    NetworkManager::VpnConnection::State state;
    OrgFreedesktopNetworkManagerVPNConnectionInterface iface;

    Q_DECLARE_PUBLIC(VpnConnection)
    VpnConnection *q_ptr;

protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;

private Q_SLOTS:
    void dbusPropertiesChanged(const QString &interfaceName, const QVariantMap &properties, const QStringList &invalidatedProperties);
    void vpnStateChanged(uint new_state, uint reason);
};

}

#endif // NETWORKMANAGERQT_VPNCONNECTION_P_H
