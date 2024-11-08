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
#include "ktabbar.h"
#include <QVBoxLayout>
#include <QLabel>

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    /* 测试说明
     * 1.tabbar1-tabbar3测试三种样式的显示以及添加tab的功能（在tabbar的开始、中间和末尾位置添加图标、文字以及二者都存在的tab）
     * 2.tabbar4测试设置tabbar样式、修改tabbar圆角(0-20)以及设置背景色。
     */
    QWidget *w = new QWidget();
    KTabBar *tabBar1 = new KTabBar(KTabBarStyle::SegmentLight,w);
    tabBar1->addTab(QIcon::fromTheme("lingmo-music"),"Segment1");
    tabBar1->addTab("Segment2");
    tabBar1->addTab(QIcon::fromTheme("lingmo-music"),"");
    tabBar1->addTab("Segment4");
    tabBar1->addTab("Segment5");
    tabBar1->addTab("Segment6");
    KTabBar *tabBar2 = new KTabBar(KTabBarStyle::SegmentDark,w);
    tabBar2->addTab("SegmentDark1");
    tabBar2->addTab("SegmentDark2");
    tabBar2->addTab(QIcon::fromTheme("lingmo-music"),"SegmentDark3");
    tabBar2->addTab("SegmentDark4");
    tabBar2->addTab("SegmentDark5");
    tabBar2->addTab(QIcon::fromTheme("lingmo-music"),"");
    KTabBar *tabBar3 = new KTabBar(KTabBarStyle::Sliding,w);
    tabBar3->addTab(QIcon::fromTheme("lingmo-music"),"");
    tabBar3->addTab("Sliding2");
    tabBar3->addTab("Sliding3");
    tabBar3->addTab("Sliding4");
    tabBar3->addTab("Sliding5");
    tabBar3->addTab(QIcon::fromTheme("lingmo-music"),"Sliding6");
    KTabBar *tabBar4 = new KTabBar(KTabBarStyle::Sliding,w);
    tabBar4->addTab("Segment1");
    tabBar4->addTab("Segment2");
    tabBar4->addTab("Segment3");
    tabBar4->addTab("Segment4");
    tabBar4->setTabBarStyle(KTabBarStyle::SegmentDark);     //设置style
    tabBar4->setBorderRadius(10);       //设置圆角半径，只对SegmentDark，SegmentLight样式生效
    tabBar4->setBackgroundColor(QColor(0,255,0));   //设置背景色

    QLabel* pLabel = new QLabel(w);
    pLabel->setAlignment(Qt::AlignCenter);
    connect(tabBar1,&KTabBar::tabBarClicked,this,[=](int index){pLabel->setText(tabBar1->tabText(index));});
    connect(tabBar2,&KTabBar::tabBarClicked,this,[=](int index){pLabel->setText(tabBar2->tabText(index));});
    connect(tabBar3,&KTabBar::tabBarClicked,this,[=](int index){pLabel->setText(tabBar3->tabText(index));});
    connect(tabBar4,&KTabBar::tabBarClicked,this,[=](int index){pLabel->setText(tabBar4->tabText(index));});

    QHBoxLayout *hlayout = new QHBoxLayout();
    QLabel *label = new QLabel();
    label->setText("just a test label");
    hlayout->addWidget(label);

    QVBoxLayout*vLayout = new QVBoxLayout;
    vLayout->addStretch();
    vLayout->addWidget(tabBar1);
    vLayout->addWidget(tabBar2);
    vLayout->addWidget(tabBar3);
    vLayout->addWidget(tabBar4);
    vLayout->addStretch();
    vLayout->addWidget(pLabel);
    vLayout->addStretch();
    vLayout->addLayout(hlayout);
    vLayout->addStretch();
    baseBar()->setLayout(vLayout);

}


Widget::~Widget()
{
}

