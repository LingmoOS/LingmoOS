/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "widget.h"
#include <QDebug>
#include <QLabel>

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QVBoxLayout* vLayout = new QVBoxLayout();
    m_pBar = new KNavigationBar(this);

    QStandardItem * item1 = new QStandardItem(QIcon::fromTheme("system-computer-symbolic"),tr("一级导航"));
    QStandardItem * item2 = new QStandardItem(QIcon::fromTheme("stock-people-symbolic"),tr("二级导航"));
    QList<QStandardItem *> list;
    QStandardItem * item3 = new QStandardItem(QIcon::fromTheme("camera-switch-symbolic"),tr("一组一级导航1"));
    QStandardItem * item4 = new QStandardItem(QIcon::fromTheme("media-eq-symbolic"),tr("一组一级导航2"));
    list<<item3<<item4;
    m_pBar->addItem(item1);   //增加常规Item
    m_pBar->addSubItem(item2);   //增加次级Item
    m_pBar->addGroupItems(list,"测试一组");  //组增加Item,在导航栏中会显示tag
    QStandardItem * item5 = new QStandardItem(QIcon::fromTheme("camera-switch-symbolic"),tr("二组一级导航1"));
    QStandardItem * item6 = new QStandardItem(QIcon::fromTheme("media-eq-symbolic"),tr("二组一级导航2"));
    QStandardItem * item7 = new QStandardItem(QIcon::fromTheme("media-eq-symbolic"),tr("二组二级导航"));
    QList<QStandardItem *> list2;
    list2<<item5<<item6;
    m_pBar->addGroupItems(list2,"测试二组");
    m_pBar->addSubItem(item7);
    m_pBar->addTag("测试三组");
    vLayout->addWidget(m_pBar);
    sideBar()->setLayout(vLayout);

    vLayout = new QVBoxLayout;
    QLabel* pLabel = new QLabel(this);
    pLabel->setAlignment(Qt::AlignCenter);
    vLayout->addWidget(pLabel);
    baseBar()->setLayout(vLayout);

    m_pBar->model()->item(1)->setEnabled(false);
    m_pBar->model()->item(2)->setEnabled(false);
    m_pBar->model()->item(3)->setEnabled(false);
    m_pBar->model()->item(4)->setEnabled(false);

//    QListView* ba= m_pBar->listview(); //获取listview
//    ba->setDisabled(true);  //整体禁用

//    QStandardItemModel* m_mode = m_pBar->model();  //获取mode

    connect(m_pBar->listview(),&QListView::clicked,this,[=](const QModelIndex &index){pLabel->setText(index.data().toString());});
    this->setLayoutType(HorizontalType);
}

Widget::~Widget()
{
}

