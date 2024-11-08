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
#include "kinputdialog.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>

using namespace  kdk;
Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QVBoxLayout* vlayout = new QVBoxLayout();
    auto btn1 = new QPushButton("inputText",this);
    auto btn2 = new QPushButton("inputMultiText",this);
    auto btn3 = new QPushButton("inputInt",this);
    auto btn4 = new QPushButton("inputDouble",this);
    vlayout->addWidget(btn1);
    vlayout->addWidget(btn2);
    vlayout->addWidget(btn3);
    vlayout->addWidget(btn4);
    connect(btn1,&QPushButton::clicked,this,[=](){KInputDialog::getText(this,tr("please input:"));}); //单行文本输入框
    connect(btn2,&QPushButton::clicked,this,[=](){KInputDialog::getMultiLineText(this,tr("please input:"));}); //多行文本输入框
    connect(btn3,&QPushButton::clicked,this,[=](){KInputDialog::getInt(this,tr("please input:"));});  //整型数字输入框
    connect(btn4,&QPushButton::clicked,this,[=](){KInputDialog::getDouble(this,tr("please input:"));});  //浮点型数字输入框
    baseBar()->setLayout(vlayout);
}

Widget::~Widget()
{
}

