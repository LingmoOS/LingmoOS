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
#ifndef KYWIRELESSNETITEM_H
#define KYWIRELESSNETITEM_H

#include <QString>

#include "lingmonetworkresourcemanager.h"
#include "kywirelessconnectoperation.h"

class KyWirelessNetItem
{
public:
    KyWirelessNetItem(NetworkManager::WirelessNetwork::Ptr net);
    KyWirelessNetItem() {;}
    ~KyWirelessNetItem();

    QString getDevice() {
        return m_device;
    }

private:
    void init(NetworkManager::WirelessNetwork::Ptr net);

public:
    QString                                 m_NetSsid;
    QString                                 m_connectUuid;
    QString                                 m_bssid;
    int                                     m_signalStrength;
    uint                                    m_frequency;
    QString                                 m_secuType;
    KySecuType                              m_kySecuType;
    QString                                 m_uni;
    bool                                    m_isMix;

    //only for m_isConfiged = true
    bool                                    m_isConfigured;
    QString                                 m_connName;
    QString                                 m_connDbusPath;
    uint                                    m_channel;

    int getCategory(QString uni);
    void setKySecuType(QString strSecuType);

private:
    KyNetworkResourceManager *m_networkResourceInstance = nullptr;
    QString                                 m_device;

};

void updatewirelessItemConnectInfo(KyWirelessNetItem& item);

#endif // KYWIRELESSNETITEM_H
