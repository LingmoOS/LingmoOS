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
#include "kcolorbutton.h"
#include <QHBoxLayout>
#include <QDebug>
#include <QButtonGroup>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* vlayout = new QVBoxLayout(this);

    QHBoxLayout* hlayout = new QHBoxLayout();
    QHBoxLayout* hlayout1 = new QHBoxLayout();

    KColorButton* btn1 = new KColorButton();
    KColorButton* btn2 = new KColorButton();
    btn2->setBackgroundColor(Qt::cyan);
    KColorButton* btn3 = new KColorButton();
    btn3->setBackgroundColor(Qt::darkYellow);
    KColorButton* btn4 = new KColorButton();
    btn4->setBackgroundColor(Qt::red);
    KColorButton* btn5 = new KColorButton();
    btn5->setBackgroundColor(Qt::gray);
    btn5->setButtonType(KColorButton::CheckedRect);
    KColorButton* btn6 = new KColorButton();
    btn6->setBackgroundColor(Qt::green);
    btn6->setButtonType(KColorButton::CheckedRect);
    KColorButton* btn7 = new KColorButton();
    btn7->setBackgroundColor(Qt::darkCyan);
    btn7->setButtonType(KColorButton::CheckedRect);
    KColorButton* btn8 = new KColorButton();
    btn8->setBackgroundColor(Qt::darkRed);
    btn8->setButtonType(KColorButton::CheckedRect);

    QButtonGroup* group = new QButtonGroup(this);
    group->addButton(btn1);
    group->addButton(btn2);
    group->addButton(btn3);
    group->addButton(btn4);
    group->addButton(btn5);
    group->addButton(btn6);
    group->addButton(btn7);
    group->addButton(btn8);
    group->setExclusive(false); //方形按钮是否可以多选  false为可以多选

    //方形第一行
    hlayout->addStretch();
    hlayout->setSpacing(10);
    hlayout->addWidget(btn1);
    hlayout->addWidget(btn2);
    hlayout->addWidget(btn3);
    hlayout->addWidget(btn4);
    hlayout->addStretch();

    //方形第二行
    hlayout1->addStretch();
    hlayout1->setSpacing(10);
    hlayout1->addWidget(btn5);
    hlayout1->addWidget(btn6);
    hlayout1->addWidget(btn7);
    hlayout1->addWidget(btn8);
    hlayout1->addStretch();

    //圆形
    KColorButton* btn9 = new KColorButton();
    btn9->setBackgroundColor(Qt::gray);
    btn9->setButtonType(KColorButton::Circle);
    KColorButton* btn10 = new KColorButton();
    btn10->setBackgroundColor(Qt::green);
    btn10->setButtonType(KColorButton::Circle);
    KColorButton* btn11 = new KColorButton();
    btn11->setBackgroundColor(Qt::darkCyan);
    btn11->setButtonType(KColorButton::Circle);
    KColorButton* btn12 = new KColorButton();
    btn12->setBackgroundColor(Qt::darkRed);
    btn12->setButtonType(KColorButton::Circle);

    QButtonGroup* group1 = new QButtonGroup(this);
    group1->addButton(btn9);
    group1->addButton(btn10);
    group1->addButton(btn11);
    group1->addButton(btn12);
    group1->setExclusive(false); //圆形形按钮是否可以多选  false为可以多选

    QHBoxLayout* hlayout2 = new QHBoxLayout();
    hlayout2->addStretch();
    hlayout2->addWidget(btn9);
    hlayout2->addWidget(btn10);
    hlayout2->addWidget(btn11);
    hlayout2->addWidget(btn12);
    hlayout2->addStretch();

    vlayout->addLayout(hlayout);
    vlayout->addLayout(hlayout1);
    vlayout->addLayout(hlayout2);
//    vlayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    this->setFixedSize(300,240);

}

Widget::~Widget()
{
}

