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

#include "lingmoconnectitem.h"

KyConnectItem::KyConnectItem(QObject *parent) : QObject(parent)
{
    m_connectName = "";
    m_connectUuid = "";
    m_connectPath = "";

    m_ifaceName = "";

    m_connectState = NetworkManager::ActiveConnection::State::Unknown;       //deactive、activing and actived
    m_itemType = NetworkManager::ConnectionSettings::ConnectionType::Unknown;
}

KyConnectItem::~KyConnectItem()
{

}

void KyConnectItem::setConnectUuid(QString uuid)
{
    m_connectUuid = uuid;
}

void KyConnectItem::dumpInfo()
{
    qDebug()<<"wired connection item info:";
    qDebug()<<"connect name:"<<m_connectName;
    qDebug()<<"connect uuid:"<<m_connectUuid;
    qDebug()<<"iface name:"<<m_ifaceName;
    qDebug()<<"connect path"<<m_connectPath;

    qDebug()<<"state:"<<m_connectState;
}
