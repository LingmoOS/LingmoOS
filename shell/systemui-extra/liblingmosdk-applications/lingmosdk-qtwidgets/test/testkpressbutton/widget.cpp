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
#include "kwidget.h"
#include "kpressbutton.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    KPressButton* button1 = new KPressButton();
    button1->setIcon(QIcon::fromTheme("stock-people-symbolic"));
    button1->setBorderRadius(20);
    button1->setTranslucent(true);
    button1->setChecked(true);

    KPressButton* button2 = new KPressButton();
    button2->setCheckable(false);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(button1);
    layout->addWidget(button2);

    KPressButton* button3 = new KPressButton(this);
    KPressButton* button4 = new KPressButton();
    button4->setFixedSize(36,36);
    button4->setButtonType(KPressButton::CircleType);
    button4->setIcon(QIcon::fromTheme("system-computer-symbolic"));

    QHBoxLayout* layout1 = new QHBoxLayout();
    layout1->addWidget(button3);
    layout1->addWidget(button4);

    KPressButton* button5 = new KPressButton();
    button5->setLoaingStatus(true);
    button5->setChecked(true);

    KPressButton* button6 = new KPressButton();

    QHBoxLayout* layout2 = new QHBoxLayout();
    layout2->addWidget(button5);
    layout2->addWidget(button6);

    QVBoxLayout* vlayout = new QVBoxLayout(this);
    vlayout->addLayout(layout);
    vlayout->addLayout(layout1);
    vlayout->addLayout(layout2);

    connect(button4,&KPressButton::clicked,this,[=](){
        if(button4->isChecked())
            button4->setLoaingStatus(true);
        else
            button4->setLoaingStatus(false);
    });
    connect(button5,&KPressButton::clicked,this,[=](){
        if(button5->isChecked())
            button5->setLoaingStatus(true);
        else
            button5->setLoaingStatus(false);
    });

    connect(button2,&KPressButton::clicked,this,[=](bool flag){
        qDebug() << flag << button2->isChecked()<<button2->isCheckable();
    });
}

Widget::~Widget()
{
}

