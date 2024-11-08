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

    KToolButton* toolbtn1 = new KToolButton(this);      //构建一个toolbutton，默认类型为Flat
    toolbtn1->setEnabled(false);    //设置是否可用
    KToolButton* toolbtn2 = new KToolButton(this);
    //默认图标为"open-menu-symbolic"
    //toolbtn2->setIcon(QIcon::fromTheme("camera-switch-symbolic"));    //设置toolbutton的图标
    KToolButton* toolbtn3 = new KToolButton(this);
    toolbtn3->setLoading(true);         //设置正在加载状态,仅不带箭头的toolbuttuon支持该状态
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(toolbtn1);
    hLayout->addWidget(toolbtn2);
    hLayout->addWidget(toolbtn3);
    mainLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout();
    KToolButton* toolbtn4 = new KToolButton(this);
    toolbtn4->setEnabled(false);
    toolbtn4->setArrow(true);       //设置是否显示向下箭头，默认不显示
    KToolButton* toolbtn5 = new KToolButton(this);
    toolbtn5->setArrow(true);
    KToolButton* toolbtn6 = new KToolButton(this);
    toolbtn6->setLoading(true);
    toolbtn6->setArrow(true);
    hLayout->addWidget(toolbtn4);
    hLayout->addWidget(toolbtn5);
    hLayout->addWidget(toolbtn6);
    mainLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout();
    KToolButton* toolbtn7 = new KToolButton(this);
    toolbtn7->setEnabled(false);
    toolbtn7->setType(KToolButtonType::SemiFlat);       //设置toolbutton类型为SemiFlat
    KToolButton* toolbtn8 = new KToolButton(this);
    toolbtn8->setType(KToolButtonType::SemiFlat);
    KToolButton* toolbtn9 = new KToolButton(this);
    toolbtn9->setLoading(true);
    toolbtn9->setType(KToolButtonType::SemiFlat);
    hLayout->addWidget(toolbtn7);
    hLayout->addWidget(toolbtn8);
    hLayout->addWidget(toolbtn9);
    mainLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout();
    KToolButton* toolbtn10 = new KToolButton(this);
    toolbtn10->setEnabled(false);
    toolbtn10->setType(KToolButtonType::Background);    //设置toolbutton类型为Background
    KToolButton* toolbtn11 = new KToolButton(this);
    toolbtn11->setType(KToolButtonType::Background);
    KToolButton* toolbtn12 = new KToolButton(this);
    toolbtn12->setLoading(true);
    toolbtn12->setType(KToolButtonType::Background);
    hLayout->addWidget(toolbtn10);
    hLayout->addWidget(toolbtn11);
    hLayout->addWidget(toolbtn12);
    mainLayout->addLayout(hLayout);

    baseBar()->setLayout(mainLayout);
}

Widget::~Widget()
{
}

