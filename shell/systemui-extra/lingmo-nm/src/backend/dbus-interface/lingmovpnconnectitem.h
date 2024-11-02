/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef LINGMOVPNCONNECTITEM_H
#define LINGMOVPNCONNECTITEM_H

#include <QObject>
#include <NetworkManagerQt/VpnConnection>
#include <NetworkManagerQt/VpnSetting>

class KyVpnConnectItem : public QObject
{
    Q_OBJECT
public:
    explicit KyVpnConnectItem(QObject *parent = nullptr);
    ~KyVpnConnectItem();
    
public:
    QString m_vpnName;
    QString m_vpnUuid;
    
    QString m_vpnUser;
    QString m_vpnGateWay;
    
    QString m_vpnIpv4Address;
    QString m_vpnIpv6Address;
    
    bool m_vpnMppe;
    NetworkManager::VpnConnection::State m_vpnState;
};


#endif // LINGMOVPNCONNECTITEM_H
