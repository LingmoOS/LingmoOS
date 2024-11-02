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
#include <QBoxLayout>

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* hLayout = new QHBoxLayout;
    KBreadCrumb*crumb = new KBreadCrumb(this);
    //默认是true
    crumb->setFlat(true);
    crumb->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    crumb->addTab("页面目录一");
    crumb->addTab("页面目录二");
    crumb->addTab("页面目录三");
    crumb->addTab("短的");
    crumb->addTab("长的长的长的长的长的长的");
    crumb->setIcon(QIcon::fromTheme("dialog-info"));  //设置KBreadCrumb的图标
    hLayout->addWidget(crumb);
    mainLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout;
    KBreadCrumb*crumb2 = new KBreadCrumb(this);
    //默认是true
    crumb->setFlat(false);  //设置KBreadCrumb是否为flat类型
    crumb2->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    crumb2->addTab("页面目录一");
    crumb2->addTab("页面目录二");
    crumb2->addTab("页面目录三");
    crumb2->addTab("短的");
    crumb2->addTab("长的很长的特别长的特别特别长的");
    crumb2->setIcon(QIcon::fromTheme("dialog-warning"));
    hLayout->addWidget(crumb2);
    mainLayout->addLayout(hLayout);
    baseBar()->setLayout(mainLayout);
}

Widget::~Widget()
{
}

