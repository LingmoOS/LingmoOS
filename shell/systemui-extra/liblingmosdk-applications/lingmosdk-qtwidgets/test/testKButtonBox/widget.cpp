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
#include "kbuttonbox.h"
#include <QList>
#include "kpushbutton.h"
#include <QDebug>
#include <QBoxLayout>
#include <QPushButton>
#include <QComboBox>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBoxLayout *layout1 = new QHBoxLayout;
    KButtonBox *box1 = new KButtonBox(this);
    KPushButton *btn1 = new KPushButton;
    btn1->setIcon(QIcon::fromTheme("list-add-symbolic"));
    btn1->setFixedSize(48,48);
    btn1->setIconSize(QSize(32,32));
    KPushButton *btn2 = new KPushButton;
    btn2->setIcon(QIcon::fromTheme("list-remove-symbolic"));
    btn2->setFixedSize(48,48);
    btn2->setIconSize(QSize(32,32));
    box1->addButton(btn1);
    box1->addButton(btn2);

    KButtonBox *box2 = new KButtonBox(this);
    KPushButton *btn3 = new KPushButton;
    btn3->setIcon(QIcon::fromTheme("list-add-symbolic"));
    btn3->setFixedSize(48,48);
    btn3->setIconSize(QSize(32,32));
    KPushButton *btn4 = new KPushButton;
    btn4->setIcon(QIcon::fromTheme("list-remove-symbolic"));
    btn4->setFixedSize(48,48);
    btn4->setIconSize(QSize(32,32));
    btn4->setEnabled(false);
    box2->addButton(btn3);
    box2->addButton(btn4);

    QHBoxLayout *layout2 = new QHBoxLayout;
    KButtonBox *box3 = new KButtonBox(this);
    KPushButton *btn5 = new KPushButton(this);
    btn5->setIcon(QIcon::fromTheme("system-computer-symbolic"));
    KPushButton *btn6 = new KPushButton(this);
    btn6->setIcon(QIcon::fromTheme("format-text-italic-symbolic"));
    KPushButton *btn7 = new KPushButton(this);
    btn7->setIcon(QIcon::fromTheme("format-text-underline-symbolic"));
    KPushButton *btn8 = new KPushButton(this);
    btn8->setIcon(QIcon::fromTheme("format-text-strikethrough-symbolic"));
    QList<KPushButton *> list;
    list.insert(0,btn5);
    list.insert(1,btn6);
    list.insert(2,btn7);
    list.insert(3,btn8);

    box3->setButtonList(list);
    box3->setCheckable(true);
    box3->setExclusive(false);

    KPushButton *btn = new KPushButton;
    btn->setText("New Button");

    connect(box1,&KButtonBox::buttonClicked,this,[=](QAbstractButton *button){
        if(btn1 == button)
            box3->addButton(btn);
        if(btn2 == button)
            box3->removeButton(btn);
    });

    layout1->addWidget(box1);
    layout1->addWidget(box2);
    layout2->addWidget(box3);
    layout->addLayout(layout1);
    layout->addLayout(layout2);
    this->setLayout(layout);
    this->setFixedSize(500,400);


}

Widget::~Widget()
{
}

