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
#ifndef LINGMOBLUETOOTHCONNECTITEM_H
#define LINGMOBLUETOOTHCONNECTITEM_H

#include <QObject>
#include <NetworkManagerQt/BluetoothDevice>
#include <NetworkManagerQt/BluetoothSetting>
#include <NetworkManagerQt/BluetoothDevice>

class KyBluetoothConnectItem : public QObject
{
    Q_OBJECT
public:
    explicit KyBluetoothConnectItem(QObject *parent = nullptr);
    ~KyBluetoothConnectItem();

public:
    void dumpInfo();

public:
    QString m_connectName;
    QString m_connectUuid;

    QString m_deviceAddress;

    QString m_ipv4Address;
    QString m_ipv6Address;

    NetworkManager::ActiveConnection::State m_state;       //deactive、activing and actived
    //QString m_itemType;  //activeconnect or connect

};

#endif // LINGMOBLUETOOTHCONNECTITEM_H
