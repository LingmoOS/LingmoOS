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
#include <QApplication>
#include <QLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include "kbubblewidget.h"
#include "kwidget.h"
#include <QHBoxLayout>
using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QPushButton* btn1 = new QPushButton(this);
    btn1->setText("常规");

    QPushButton* btn2 = new QPushButton(this);
    btn2->setText("毛玻璃");

    btn2->move(100,0);

    KBubbleWidget*  w =new KBubbleWidget();
    w->setTailPosition(TailDirection::TopDirection);
    w->setTailSize(QSize(16,8));
    w->setBorderRadius(12);
    w->setOpacity(0.4);
    w->setEnableBlur(false);
    w->setFixedSize(320,320);
    QHBoxLayout* layout = new QHBoxLayout(w);
    QPushButton* button = new QPushButton(w);
    button->setText("关闭");
    layout->addWidget(button);
    connect(button,&QPushButton::clicked,this,[=](){
        w->hide();
    });

    KBubbleWidget*  w2 =new KBubbleWidget();
    w2->setTailPosition(TailDirection::TopDirection);
    w2->setTailSize(QSize(16,8));
    w2->setBorderRadius(12);
    w2->setOpacity(0.4);
    w2->setEnableBlur(true);
    w2->setFixedSize(320,320);
    QPushButton* button2 = new QPushButton(w2);
    button2->setText("关闭");
    QHBoxLayout* layout2 = new QHBoxLayout(w2);
    layout2->addWidget(button2);

    connect(button2,&QPushButton::clicked,this,[=](){
        w2->hide();
    });

    connect(btn1,&QPushButton::clicked,this,[=](){
        w->show();
    });
    connect(btn2,&QPushButton::clicked,this,[=](){
        w2->show();
    });

}

Widget::~Widget()
{
}

