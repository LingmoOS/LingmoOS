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
#include "lanlistitem.h"
#include "backend/dbus-interface/lingmoconnectitem.h"

#include <QDebug>

#define LOG_FLAG "[LanListItem]"
#define NAMELABLE_MAX_WIDTH_HOVER 220
#define NAMELABLE_MAX_WIDTH_ACTIVATED 190
#define NAMELABLE_MAX_WIDTH_DEACTIVATED 326

LanListItem::LanListItem(const KyConnectItem *lanConnectItem,
                         const QString &deviceName, QWidget *parent):ListItem(parent)
{
    m_connectOperation = new KyWiredConnectOperation(this);
    m_deviceResource = new KyNetworkDeviceResourse(this);

    connectItemCopy(lanConnectItem);
    m_deviceName = deviceName;
    m_nameLabel->setLabelText(m_lanConnectItem.m_connectName);
    m_netButton->setButtonIcon(QIcon::fromTheme("network-wired-connected-symbolic"));

    qDebug() << "LanListItem init:" << m_lanConnectItem.m_connectName << m_lanConnectItem.m_connectState << m_lanConnectItem.m_ifaceName;

    if (Deactivated == m_lanConnectItem.m_connectState || Activated == m_lanConnectItem.m_connectState) {
        m_netButton->stopLoading();
        if (m_lanConnectItem.m_connectState == Activated) {
            setIcon(true);
            m_nameLabel->setLabelMaximumWidth(NAMELABLE_MAX_WIDTH_ACTIVATED);
        } else {
            setIcon(false);
            m_nameLabel->setLabelMaximumWidth(NAMELABLE_MAX_WIDTH_DEACTIVATED);
        }
    } else {
        m_netButton->startLoading();
    }

    m_itemFrame->installEventFilter(this);
//    connect(this->m_infoButton, &InfoButton::clicked, this, &LanListItem::onInfoButtonClicked);
    connect(m_hoverButton, &FixPushButton::clicked, this, &LanListItem::onNetButtonClicked);
}


LanListItem::LanListItem(QWidget *parent) : ListItem(parent)
{
    m_isActive = false;
    m_netButton->setButtonIcon(QIcon::fromTheme("network-wired-disconnected-symbolic"));
    setIcon(false);
    const QString str=tr("Not connected");
    m_nameLabel->setLabelText(str);
//    this->m_infoButton->hide();
}

LanListItem::~LanListItem()
{
    qDebug()<<"[LanPage] lan list item is deleted." << m_lanConnectItem.m_connectName;
}

void LanListItem::setIcon(bool isOn)
{
    if (isOn) {
        m_netButton->setActive(true);               //设置图标显示不同颜色
    } else {
        m_netButton->setActive(false);
    }
}

void LanListItem::connectItemCopy(const KyConnectItem *lanConnectItem)
{
    if (lanConnectItem) {
        m_lanConnectItem.m_connectName = lanConnectItem->m_connectName;
        m_lanConnectItem.m_connectPath = lanConnectItem->m_connectPath;
        m_lanConnectItem.m_connectState = lanConnectItem->m_connectState;
        m_lanConnectItem.m_connectUuid = lanConnectItem->m_connectUuid;
        m_lanConnectItem.m_ifaceName = lanConnectItem->m_ifaceName;
        m_lanConnectItem.m_itemType = lanConnectItem->m_itemType;
    } else {
        qDebug() << LOG_FLAG <<"the connect item is nullptr";
        m_lanConnectItem.m_connectName = "";
        m_lanConnectItem.m_connectPath = "";
        m_lanConnectItem.m_connectState = NetworkManager::ActiveConnection::State::Unknown;
        m_lanConnectItem.m_connectUuid = "";
        m_lanConnectItem.m_ifaceName = "";
        m_lanConnectItem.m_itemType = NetworkManager::ConnectionSettings::ConnectionType::Unknown;
    }

    return;
}

void LanListItem::onNetButtonClicked()
{
    if (m_lanConnectItem.m_connectUuid.isEmpty()) {
        qDebug() << LOG_FLAG << "connect is empty, so can not connect or disconnect.";
        return;
    }

    if (Deactivated == m_lanConnectItem.m_connectState) {
        //断开的连接，点击激活连接
        if (m_deviceResource->wiredDeviceIsCarriered(m_deviceName)) {
            m_connectOperation->activateWiredConnection(m_lanConnectItem.m_connectUuid, m_deviceName);
            qDebug() << LOG_FLAG << "it will activate connection" << m_lanConnectItem.m_connectName
                     << ". it's device is" << m_deviceName;
            m_netButton->startLoading();
        } else {
            qDebug() << LOG_FLAG << m_deviceName << "is not carried, so can not activate connection";
            this->showDesktopNotify(tr("Wired Device not carried"), "networkwrong");
        }
    } else {
        m_connectOperation->deactivateWiredConnection(m_lanConnectItem.m_connectName, m_lanConnectItem.m_connectUuid);
    }

    return;

}

void LanListItem::onRightButtonClicked()
{
    //右键点击事件
    qDebug()<< LOG_FLAG <<"onRightButtonClicked";
    if (!m_menu) {
        return;
    }

    m_menu->clear();
    if (Activated == m_lanConnectItem.m_connectState || Activating == m_lanConnectItem.m_connectState) {
        m_menu->addAction(new QAction(tr("Disconnect"), this));
    } else if (Deactivated == m_lanConnectItem.m_connectState) {
        m_menu->addAction(new QAction(tr("Connect"), this));
    } else {
        return;
    }
    m_menu->addAction(new QAction(tr("Property"), this));
    m_menu->addAction(new QAction(tr("Delete"), this));

    m_menu->popup(cursor().pos());
    return;
}

void LanListItem::onMenuTriggered(QAction *action)
{
    if (action->text() == tr("Connect")) {
        this->onNetButtonClicked();
    } else if (action->text() == tr("Disconnect")) {
        m_connectOperation->deactivateWiredConnection(m_lanConnectItem.m_connectName, m_lanConnectItem.m_connectUuid);
        qDebug() << LOG_FLAG << "it will disconnect connection" << m_lanConnectItem.m_connectName
                 << ". it's device is" << m_deviceName;
        m_netButton->startLoading();
    } else if (action->text() == tr("Property")) {
        onInfoButtonClicked();
    } else if (action->text() == tr("Delete")) {
        m_connectOperation->deleteConnect(m_lanConnectItem.m_connectUuid);
    }
    return;
}


void LanListItem::onInfoButtonClicked()
{
    if (m_lanConnectItem.m_connectUuid.isEmpty()) {
        qDebug() << LOG_FLAG << "connect is empty, so can not show detail info.";
        return;
    }

//    if(netDetail != nullptr){
//        netDetail->activateWindow();
//        return;
//    }

    qDebug()<< LOG_FLAG << "the info button of lan is clicked! uuid = "
            << m_lanConnectItem.m_connectUuid << "; name = " << m_lanConnectItem.m_connectName
            << "." <<Q_FUNC_INFO << __LINE__;
#if 0
   bool isActivated = false;
    if (Activated == m_lanConnectItem.m_connectState) {
        isActivated = true;
    }

    netDetail = new NetDetail(m_deviceName, m_lanConnectItem.m_connectName,
                                         m_lanConnectItem.m_connectUuid, isActivated,false, false);

    connect(netDetail, &NetDetail::destroyed, [&](){
        if (netDetail != nullptr) {
            netDetail = nullptr;
        }
    });

    netDetail->show();
#endif
    Q_EMIT this->detailShow(m_deviceName, m_lanConnectItem.m_connectUuid);

    return;
}

void LanListItem::updateConnectionState(ConnectState state)
{
    m_lanConnectItem.m_connectState = (NetworkManager::ActiveConnection::State)state;

    if (Deactivated == state || Activated == state) {
        m_netButton->stopLoading();
        if (state == Activated) {
            setIcon(true);
        } else {
            setIcon(false);
        }
    } else {
        m_netButton->startLoading();
    }

    return;
}

QString LanListItem::getConnectionName()
{
    return m_lanConnectItem.m_connectName;
}

void LanListItem::updateConnectionName(QString connectionName)
{
    m_lanConnectItem.m_connectName = connectionName;
    m_nameLabel->setLabelText(m_lanConnectItem.m_connectName);
    return;
}

QString LanListItem::getConnectionPath()
{
    return m_lanConnectItem.m_connectPath;
}

void LanListItem::updateConnectionPath(QString connectionPath)
{
    m_lanConnectItem.m_connectPath = connectionPath;
}

void LanListItem::enterEvent(QEvent *event)
{
    m_nameLabel->setLabelMaximumWidth(NAMELABLE_MAX_WIDTH_HOVER);
    if (m_lanConnectItem.m_connectState != UnknownState) {
        if (Deactivated != m_lanConnectItem.m_connectState) {
            m_hoverButton->setProperty("useButtonPalette", true);
            m_hoverButton->setProperty("isImportant", false);
            m_hoverButton->setButtonText(tr("Disconnect"));
        } else {
            m_hoverButton->setProperty("isImportant", true);
            m_hoverButton->setProperty("useButtonPalette", false);
            m_hoverButton->setButtonText(tr("Connect"));
        }
        m_hoverButton->show();
        m_lbLoadUp->hide();
        m_lbLoadDown->hide();
        m_lbLoadDownImg->hide();
        m_lbLoadUpImg->hide();
    }
    return ListItem::enterEvent(event);
}

void LanListItem::leaveEvent(QEvent *event)
{
    m_hoverButton->hide();
    if (m_lanConnectItem.m_connectState == Activated || m_connectState == Deactivating) {
        m_nameLabel->setLabelMaximumWidth(NAMELABLE_MAX_WIDTH_ACTIVATED);
        m_lbLoadUp->show();
        m_lbLoadDown->show();
        m_lbLoadDownImg->show();
        m_lbLoadUpImg->show();
    } else {
        m_nameLabel->setLabelMaximumWidth(NAMELABLE_MAX_WIDTH_DEACTIVATED);
    }
    return ListItem::leaveEvent(event);
}
