// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "block_dev_stat_view_widget.h"
#include "block_dev_item_widget.h"
#include "system/system_monitor.h"
#include "system/block_device_info_db.h"
#include "system/device_db.h"
#include <QThread>
#include <QGridLayout>
#include <QTimer>

using namespace core::system;
const int itemSpace = 6;
BlockStatViewWidget::BlockStatViewWidget(QWidget *parent) : QScrollArea(parent)
{
    m_centralWidget = new QWidget(this);
    this->setWidget(m_centralWidget);
    this->setFrameShape(QFrame::NoFrame);

    onUpdateData();
    connect(SystemMonitor::instance(), &SystemMonitor::statInfoUpdated, this, &BlockStatViewWidget::onUpdateData);
}

void BlockStatViewWidget::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    updateWidgetGeometry();
}

void BlockStatViewWidget::fontChanged(const QFont &font)
{
    for (int i = 0; i < m_listBlockItemWidget.size(); ++i) {
        m_listBlockItemWidget[i]->fontChanged(font);
    }
}

void BlockStatViewWidget::updateWidgetGeometry()
{
    int deviceCount  = m_listDevice.size();
    if (deviceCount <= 0) {
        return ;
    }

    if (deviceCount == 1) {
        showItem1();
    } else if (deviceCount == 2) {
        showItem2();
    } else if (deviceCount > 2) {
        showItemLg2(deviceCount);
    }
}

void BlockStatViewWidget::onSetItemStatus(const QString &deviceName)
{
    for (auto it = m_mapDeviceItemWidget.begin(); it != m_mapDeviceItemWidget.end(); ++it) {
        it.value()->activeItemWidget(it.key() == deviceName);
    }
    emit changeInfo(deviceName);
}

void BlockStatViewWidget::showItem1()
{
    BlockDevItemWidget *item = m_listBlockItemWidget.at(0);
    item->updateData(m_listDevice[0]);
    item->setMode(BlockDevItemWidget::TITLE_HORIZONTAL);
    item->show();

    m_mapDeviceItemWidget.insert(m_listDevice[0].deviceName(), item);

    item->setGeometry(0, 0, this->width(), this->height());
    item->activeItemWidget(false);

    m_centralWidget->setFixedSize(this->width(), this->height());

    for (int i = 1 ; i < m_listBlockItemWidget.size(); i++) {
        m_listBlockItemWidget.at(i)->hide();
        m_listBlockItemWidget.at(i)->setMode(BlockDevItemWidget::TITLE_HORIZONTAL);
    }
    emit changeInfo(m_listDevice[0].deviceName());
}
void BlockStatViewWidget::showItem2()
{
    int avgWidth = (this->width() - itemSpace) / 2;
    int avgheight = this->height();

    BlockDevItemWidget *item1 = m_listBlockItemWidget.at(0);
    BlockDevItemWidget *item2 = m_listBlockItemWidget.at(1);
    item1->show();
    item2->show();

    item1->updateData(m_listDevice[0]);
    item2->updateData(m_listDevice[1]);

    item1->setMode(BlockDevItemWidget::TITLE_VERTICAL);
    item2->setMode(BlockDevItemWidget::TITLE_VERTICAL);

    m_mapDeviceItemWidget.insert(m_listDevice[0].deviceName(), item1);
    m_mapDeviceItemWidget.insert(m_listDevice[1].deviceName(), item2);

    item1->setGeometry(0, 0, avgWidth, avgheight);
    item2->setGeometry(item1->geometry().right() + itemSpace, 0, avgWidth, avgheight);

    if (!item1->isActiveItem() && !item2->isActiveItem()) {
        item1->activeItemWidget(true);
    }

    m_centralWidget->setFixedSize(this->width(), this->height());

    bool  haveSelect = false;
    for (int i = 2 ; i < m_listBlockItemWidget.size(); i++) {
        m_listBlockItemWidget.at(i)->hide();
        if (m_listBlockItemWidget.at(i)->isActiveItem()) {
            haveSelect = true;
        }
        m_listBlockItemWidget.at(i)->setMode(BlockDevItemWidget::TITLE_HORIZONTAL);
    }

    if (haveSelect) {
        m_listBlockItemWidget.at(0)->activeItemWidget(true);
        emit changeInfo(m_listDevice[0].deviceName());
    }
}
void BlockStatViewWidget::showItemLg2(int count)
{
    int totalPage = qMax(0, (count - 1) / 4);
    int itemWidth = (this->width() - itemSpace) / 2;;
    int itemHeight = this->height() / 2;

    bool noSelect = true;
    for (int i = 0 ; i < count; i++) {
        int page = i / 4;
        BlockDevItemWidget *itemWidget = m_listBlockItemWidget.at(i);
        itemWidget->setMode(BlockDevItemWidget::TITLE_VERTICAL);
        itemWidget->show();

        m_mapDeviceItemWidget.insert(m_listDevice[i].deviceName(), itemWidget);
        if (i % 4 == 0) {
            itemWidget->setGeometry(page * this->width(), 0, itemWidth, itemHeight);
        } else if (i % 4 == 1) {
            itemWidget->setGeometry(page * this->width() + itemWidth + itemSpace, 0, itemWidth, itemHeight);
        } else if (i % 4 == 2) {
            itemWidget->setGeometry(page * this->width(), itemHeight, itemWidth, itemHeight);
        } else if (i % 4 == 3) {
            itemWidget->setGeometry(page * this->width() + itemWidth + itemSpace, itemHeight, itemWidth, itemHeight);
        }

        if (itemWidget->isActiveItem()) {
            noSelect = false;
        }
        itemWidget->updateData(m_listDevice[i]);
    }

    if (noSelect) {
        m_listBlockItemWidget.at(0)->activeItemWidget(true);
        emit changeInfo(m_listDevice[0].deviceName());
    }

    m_centralWidget->setFixedSize(this->width() + this->width() * totalPage, this->height());

    bool  haveSelect = false;
    for (int i = count ; i < m_listBlockItemWidget.size(); i++) {
        m_listBlockItemWidget.at(i)->hide();
        if (m_listBlockItemWidget.at(i)->isActiveItem()) {
            haveSelect = true;
        }
        m_listBlockItemWidget.at(i)->setMode(BlockDevItemWidget::TITLE_HORIZONTAL);
    }

    if (haveSelect) {
        m_listBlockItemWidget.at(0)->activeItemWidget(true);
        emit changeInfo(m_listDevice[0].deviceName());
    }
}

void BlockStatViewWidget::onUpdateData()
{
    m_listDevice = DeviceDB::instance()->blockDeviceInfoDB()->deviceList();
    m_mapDeviceItemWidget.clear();

    int deviceCount = m_listDevice.size();
    int curItemSize = m_listBlockItemWidget.size();
    for (int i = 0 ; i < deviceCount - curItemSize; i++) {
        BlockDevItemWidget *item = new BlockDevItemWidget(m_centralWidget);
        m_listBlockItemWidget << item;
        connect(item, &BlockDevItemWidget::clicked, this, &BlockStatViewWidget::onSetItemStatus);
    }
    updateWidgetGeometry();
}
