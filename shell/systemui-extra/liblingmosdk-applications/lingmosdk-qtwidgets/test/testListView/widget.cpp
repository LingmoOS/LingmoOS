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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "widget.h"
#include <QStandardItem>
#include <QListView>
#include <QStandardItemModel>
#include <QAbstractItemView>
#include "klistview.h"
#include "klistviewdelegate.h"
#include <QHBoxLayout>
#include <QDebug>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout* hlayout = new QHBoxLayout(this);

    QListView* view = new QListView(this);

    view->setIconSize(QSize(32,32));  //设置图片大小

    QStandardItemModel*model =new QStandardItemModel(this);
    KListViewDelegate *delegate =new KListViewDelegate(view);

    view->setModel(model);
    QStandardItem* item = new QStandardItem();
    item->setIcon(QIcon::fromTheme("lingmo-music"));
    item->setData(QVariant("Test"),Qt::DisplayRole);
    item->setData(QVariant("subTEST"),Qt::UserRole);

    QStandardItem* item1 = new QStandardItem();
    item1->setIcon(QIcon::fromTheme("lingmo-music"));
    item1->setData(QVariant("Test"),Qt::DisplayRole);
    item1->setData(QVariant("supTEST"),Qt::UserRole);

    QStandardItem* item2 = new QStandardItem();
    item2->setIcon(QIcon::fromTheme("lingmo-music"));
    item2->setData(QVariant("Test"),Qt::DisplayRole);
    item2->setData(QVariant("supTEST"),Qt::UserRole);

    model->appendRow(item);
    model->appendRow(item1);
    model->appendRow(item2);


    view->setItemDelegate(delegate);

    view->setViewMode(QListView::ListMode);
    //view->setGridSize(QSize(60,120));//IconMode下用

    view->setFixedSize(300,480);
    hlayout->addWidget(view);

    QListView* view1 = new QListView(this);

    view1->setIconSize(QSize(32,32));

    QStandardItemModel*model1 =new QStandardItemModel(this);
    KListViewDelegate *delegate1 =new KListViewDelegate(view1);

    view1->setModel(model1);

    QStandardItem* item3 = new QStandardItem();
    item3->setIcon(QIcon::fromTheme("lingmo-music"));
    item3->setData(QVariant("Test"),Qt::DisplayRole);

    QStandardItem* item4 = new QStandardItem();
    item4->setIcon(QIcon::fromTheme("lingmo-music"));
    item4->setData(QVariant("Test"),Qt::DisplayRole);

    QStandardItem* item5 = new QStandardItem();
    item5->setIcon(QIcon::fromTheme("lingmo-music"));
    item5->setData(QVariant("Test"),Qt::DisplayRole);

    model1->appendRow(item3);
    model1->appendRow(item4);
    model1->appendRow(item5);
    view1->setItemDelegate(delegate1);

    view1->setViewMode(QListView::ListMode);
    //view->setGridSize(QSize(60,120));//IconMode下用

    hlayout->addWidget(view1);

//    view->setSelectionMode(QAbstractItemView::ContiguousSelection);
//    view->setSelectionMode(QAbstractItemView::MultiSelection);
//    view->setSelectionMode(QAbstractItemView::ExtendedSelection);

    view1->setFixedSize(300,480);
//    setFixedSize(800,600);
}

Widget::~Widget()
{
}


