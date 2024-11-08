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
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QTabBar>
#include "ktag.h"
#include "kbreadcrumb.h"



Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    KTag* tag1 = new KTag(this);
    tag1->setText("tag1");      //设置tag文本
    //tag1->setTagStyle(TagStyle::HighlightTag);
    KTag* tag2 = new KTag(this);
    tag2->setText("tag2");
    tag2->setTagStyle(TagStyle::BoderTag);      //设置tag的style，有HighlightTag,BoderTag,BaseBoderTag,GrayTag四种
    KTag* tag3 = new KTag(this);
    tag3->setText("tag3");
    tag3->setTagStyle(TagStyle::BaseBoderTag);
    KTag* tag4 = new KTag(this);
    tag4->setText("tag4");
    tag4->setTagStyle(TagStyle::GrayTag);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(tag1);
    hLayout->addWidget(tag2);
    hLayout->addWidget(tag3);
    hLayout->addWidget(tag4);
    vLayout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    KTag* tag5 = new KTag(this);
    tag5->setText("tag5");
    tag5->setClosable(true);    //设置tag是否可以关闭
    KTag* tag6 = new KTag(this);
    tag6->setClosable(true);
    tag6->setText("tag6");
    tag6->setTagStyle(TagStyle::BoderTag);
    KTag* tag7 = new KTag(this);
    tag7->setClosable(true);
    tag7->setText("tag7");
    tag7->setTagStyle(TagStyle::BaseBoderTag);
    KTag* tag8 = new KTag(this);
    tag8->setClosable(true);
    tag8->setText("tag8");
    tag8->setTagStyle(TagStyle::GrayTag);
    hLayout->addWidget(tag5);
    hLayout->addWidget(tag6);
    hLayout->addWidget(tag7);
    hLayout->addWidget(tag8);
    vLayout->addLayout(hLayout);
    baseBar()->setLayout(vLayout);
    sideBar()->hide();
}

Widget::~Widget()
{
}

