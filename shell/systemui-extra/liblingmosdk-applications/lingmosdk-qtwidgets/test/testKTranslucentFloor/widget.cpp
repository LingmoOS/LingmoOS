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
#include "ktranslucentfloor.h"
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    QPushButton* button1 = new QPushButton("常规效果",this);
    QPushButton* button2 = new QPushButton("毛玻璃效果",this);
    mainLayout->addWidget(button1);
    mainLayout->addWidget(button2);

    KTranslucentFloor* floor1 = new KTranslucentFloor();
    floor1->setEnableBlur(false);
    floor1->setFixedSize(400,300);
    floor1->setShadow(true);
    QHBoxLayout* layout1 = new QHBoxLayout(floor1);
    QPushButton* subBtn1 = new QPushButton("关闭",this);
    layout1->addWidget(subBtn1);

    KTranslucentFloor* floor2 = new KTranslucentFloor();
    floor2->setEnableBlur(true);
    floor2->setFixedSize(400,300);
    floor2->setShadow(true);
    QHBoxLayout* layout2 = new QHBoxLayout(floor2);
    QPushButton* subBtn2 = new QPushButton("关闭",this);
    layout2->addWidget(subBtn2);

    connect(subBtn1,&QPushButton::clicked,this,[=](){
        floor1->close();
    });
    connect(subBtn2,&QPushButton::clicked,this,[=](){
        floor2->close();
    });
    connect(button1,&QPushButton::clicked,this,[=](){
        floor1->show();
    });
    connect(button2,&QPushButton::clicked,this,[=](){
        floor2->show();
    });
}

Widget::~Widget()
{
}

