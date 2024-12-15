// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netif_stat_view_widget.h"
#include "netif_item_view_widget.h"

#include "system/device_db.h"
#include "system/netif_info_db.h"

#include <QDebug>

const int itemSpace = 6;
using namespace core::system;
NetifStatViewWidget::NetifStatViewWidget(QWidget *parent) : DScrollArea(parent)
{
    m_centralWidget = new QWidget(this);
    this->setWidget(m_centralWidget);
    this->setFrameShape(QFrame::NoFrame);

    m_info = DeviceDB::instance()->netifInfoDB();
}

void NetifStatViewWidget::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    updateWidgetGeometry();
}

void NetifStatViewWidget::fontChanged(const QFont &font)
{
    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        NetifItemViewWidget *itemView = iter.value();
        itemView->fontChanged(font);
    }
}

void NetifStatViewWidget::onModelUpdate()
{
    const QMap<QByteArray, NetifInfoPtr> &netifInfoDB = m_info->infoDB();
    for (auto iter = netifInfoDB.begin(); iter != netifInfoDB.end(); iter++) {
        const QByteArray &mac = iter.key();
        if (!m_mapItemView.contains(mac)) {
            NetifItemViewWidget *itemWidget = new NetifItemViewWidget(m_centralWidget, mac);
            connect(itemWidget, &NetifItemViewWidget::clicked, this, &NetifStatViewWidget::onSetItemActiveStatus);
            itemWidget->updateData(iter.value());

            m_mapItemView.insert(mac, itemWidget);
        } else {
            NetifItemViewWidget *itemWidget = m_mapItemView.value(mac);
            itemWidget->updateData(iter.value());
        }
    }
    updateWidgetGeometry();

    int netifCnt = netifInfoDB.size();
    if (!m_initStatus && netifCnt > 0) {
        m_initStatus = true;
        if (netifCnt > 1) {
            onSetItemActiveStatus(netifInfoDB.begin().key());
        } else {
            emit netifItemClicked(netifInfoDB.begin().key());
        }
        m_currentMac = netifInfoDB.begin().key();
    }

    if (netifCnt > 0 && !netifInfoDB.contains(m_currentMac)) {
        if (netifInfoDB.size()  == 1) {
            m_mapItemView.value(netifInfoDB.begin().key())->updateActiveStatus(false);
        } else {
            m_mapItemView.value(netifInfoDB.begin().key())->updateActiveStatus(true);
        }
        m_currentMac = netifInfoDB.begin().key();
        emit netifItemClicked(netifInfoDB.begin().key());
        return ;
    }
}

void NetifStatViewWidget::onSetItemActiveStatus(const QString &mac)
{
    const QMap<QByteArray, NetifInfoPtr> &netifInfoDB = m_info->infoDB();
    int netCount  = netifInfoDB.size();
    m_currentMac = mac.toUtf8();

    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        NetifItemViewWidget *itemView = iter.value();
        if (netCount > 1 && iter.key() == mac) {
            itemView->updateActiveStatus(true);
            emit netifItemClicked(mac);
        } else {
            itemView->updateActiveStatus(false);
        }
    }
}

void NetifStatViewWidget::updateWidgetGeometry()
{
    const QMap<QByteArray, NetifInfoPtr> &netifInfoDB = m_info->infoDB();
    int netCount  = netifInfoDB.size();

    if (netCount == 1)
        showItemOnlyeOne();
    else if (netCount == 2)
        showItemDouble();
    else if (netCount > 2)
        showItemLgDouble();
}

void NetifStatViewWidget::showItemOnlyeOne()
{
    const QMap<QByteArray, NetifInfoPtr> &netifInfoDB = m_info->infoDB();
    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        NetifItemViewWidget *itemView = iter.value();
        if (netifInfoDB.contains(iter.key())) {
            itemView->show();
            itemView->setMode(NetifItemViewWidget::TITLE_HORIZONTAL);
            itemView->setGeometry(0, 0, this->width(), this->height());
            itemView->updateActiveStatus(false);
        } else {
            itemView->hide();
            itemView->updateActiveStatus(false);
        }
    }

    m_centralWidget->setFixedSize(this->width(), this->height());
}

void NetifStatViewWidget::showItemDouble()
{
    int itemOffsetX = 0;
    int itemHeight  = this->height();
    int itemWidth   = (this->width() - itemSpace) / 2;

    const QMap<QByteArray, NetifInfoPtr> &netifInfoDB = m_info->infoDB();
    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        NetifItemViewWidget *itemView = iter.value();
        if (netifInfoDB.contains(iter.key())) {
            itemView->show();
            itemView->setMode(NetifItemViewWidget::TITLE_VERTICAL);
            itemView->setGeometry(itemOffsetX, 0, itemWidth, itemHeight);
            itemOffsetX = itemOffsetX + itemWidth + itemSpace;
            if (iter.key() == m_currentMac) {
                itemView->updateActiveStatus(true);
            }
        } else {
            itemView->hide();
            itemView->updateActiveStatus(false);
        }
    }

    m_centralWidget->setFixedSize(this->width(), this->height());
}

void NetifStatViewWidget::showItemLgDouble()
{
    const QMap<QByteArray, NetifInfoPtr> &netifInfoDB = m_info->infoDB();

    int itemHeight  = this->height() / 2;
    int itemWidth   = (this->width() - itemSpace) / 2;
    int itemCurrentCnt = -1;
    int page = 0;
    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        NetifItemViewWidget *itemView = iter.value();
        if (netifInfoDB.contains(iter.key())) {
            itemCurrentCnt++;
            itemView->setMode(NetifItemViewWidget::TITLE_VERTICAL);
            itemView->show();
            page = itemCurrentCnt / 4;
            if (itemCurrentCnt % 4 == 0) {
                itemView->setGeometry(page * this->width(), 0, itemWidth, itemHeight);
            } else if (itemCurrentCnt % 4 == 1) {
                itemView->setGeometry(page * this->width() + itemWidth + itemSpace, 0, itemWidth, itemHeight);
            } else if (itemCurrentCnt % 4 == 2) {
                itemView->setGeometry(page * this->width(), itemHeight, itemWidth, itemHeight);
            } else if (itemCurrentCnt % 4 == 3) {
                itemView->setGeometry(page * this->width() + itemWidth + itemSpace, itemHeight, itemWidth, itemHeight);
            }
            if (iter.key() == m_currentMac) {
                itemView->updateActiveStatus(true);
            }
        } else {
            itemView->hide();
            itemView->updateActiveStatus(false);
        }
    }
    m_centralWidget->setFixedSize(this->width() + this->width() * page, this->height());
}

