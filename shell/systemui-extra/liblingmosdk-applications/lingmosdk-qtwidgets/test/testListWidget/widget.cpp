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
#include <QVBoxLayout>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* vlayout = new QVBoxLayout(this);

    m_listwidget = new KListWidget(this);
    this->setWindowTitle("test");
    this->setWindowIcon(QIcon::fromTheme("lingmo-music"));
    this->resize(800,500);
    //KItemWidget 构造函数  KItemWidget(const QIcon &Myicon,QString MmainText,QString MsecText,QWidget *parent);
    //Myicon 需要显示的图片   MmainText 需要写入的miantext   MsecText 需要写入的sectext
    m_item = new KItemWidget(QIcon::fromTheme("lingmo-music"),QString("IMG202202121544.JPG"),QString("1.6MB"),m_listwidget);

    m_item1 = new KItemWidget(QIcon::fromTheme("lingmo-music"),QString("IMG202202121544.JPG"),QString("1.5MB"),m_listwidget);
    KItemWidget* m_item2 = new KItemWidget(QIcon::fromTheme("lingmo-music"),QString("IMG202202121544.JPG"),QString("1.5MB"),m_listwidget);

    m_listwidget->AddItemWidget(m_item);
    m_listwidget->AddItemWidget(m_item1);
    m_listwidget->AddItemWidget(m_item2);

    vlayout->addWidget(m_listwidget);
}

Widget::~Widget()
{
}

