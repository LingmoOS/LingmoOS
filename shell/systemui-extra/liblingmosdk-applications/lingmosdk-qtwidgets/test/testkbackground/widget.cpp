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
#include <QHBoxLayout>
#include "kbackgroundgroup.h"
#include <QFrame>
#include "klineframe.h"
#include "kpushbutton.h"
#include <QCheckBox>
#include <QLineEdit>
#include "kswitchbutton.h"
#include "kborderlessbutton.h"
#include "ktoolbutton.h"
#include <QLabel>

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* vmainLayout=new QVBoxLayout(this);

    QLabel *networkLabel = new QLabel();
    networkLabel->setText("有线网络");
    networkLabel->setContentsMargins(20,0,0,0);

    QWidget *widget1 = new QWidget();

    QLabel* widget1Label = new QLabel();
    widget1Label->setText("开启");

    KSwitchButton *widget1Switch = new KSwitchButton();

    QHBoxLayout* widget1Layout = new QHBoxLayout(widget1);
    widget1Layout->setContentsMargins(16,0,16,0);
    widget1Layout->addWidget(widget1Label);
    widget1Layout->addStretch();
    widget1Layout->addWidget(widget1Switch);

    KBackgroundGroup *back = new KBackgroundGroup();
    back->addWidget(widget1);

    QWidget *widget2 = new QWidget();
//    widget2->setFixedHeight(100);

    QLabel *widget2label = new QLabel();
    widget2label->setText("网卡：wwwwwww");

    KBorderlessButton *less = new KBorderlessButton();
    less->setIcon(QIcon::fromTheme("system-computer-symbolic"));
    less->setAttribute(Qt::WA_TranslucentBackground);

    KSwitchButton *widget2Switch = new KSwitchButton();
    widget2Switch->setAttribute(Qt::WA_TranslucentBackground);

    QHBoxLayout *widget2Layout = new QHBoxLayout(widget2);
    widget2Layout->setContentsMargins(16,0,16,0);
    widget2Layout->addWidget(widget2label);
    widget2Layout->addStretch();
    widget2Layout->addWidget(less);
    widget2Layout->addWidget(widget2Switch);

    QWidget *widget3 = new QWidget();
//    QPushButton *widget3 = new QPushButton();
    widget3->setProperty("isWindowButton",0x01);

    QLabel *widget3Label1 = new QLabel();
    widget3Label1->setPixmap(QIcon::fromTheme("system-computer-symbolic").pixmap(16,16));
    QLabel *widget3Label2 = new QLabel();
    widget3Label2->setText("网络名称");

    QLabel *widget3Label3 = new QLabel();
    widget3Label3->setText("连接");

    KBorderlessButton *widget3btn = new KBorderlessButton(this);
    widget3btn->setIcon(QIcon::fromTheme("stock-people-symbolic"));
    widget3btn->setAttribute(Qt::WA_TranslucentBackground);

    QHBoxLayout *widget3Layout= new QHBoxLayout(widget3);
    widget3Layout->setContentsMargins(16,0,16,0);
    widget3Layout->addWidget(widget3Label1);
    widget3Layout->addWidget(widget3Label2);
    widget3Layout->addStretch();
    widget3Layout->addWidget(widget3Label3);
    widget3Layout->addWidget(widget3btn);

    QWidget *widget4=new QWidget();

    QLabel *widget4Label1 = new QLabel();
    widget4Label1->setText("点击添加");

    QLabel *widget4Label2 = new QLabel();
    widget4Label2->setPixmap(QIcon::fromTheme("system-computer-symbolic").pixmap(16,16));

    QHBoxLayout *widget4Layout = new QHBoxLayout(widget4);
    widget4Layout->addStretch();
    widget4Layout->addWidget(widget4Label2);
    widget4Layout->addWidget(widget4Label1);
    widget4Layout->addStretch();

    QList<QWidget*> list;
    list.append(widget2);
    list.append(widget3);
    list.append(widget4);

    KBackgroundGroup *back1 = new KBackgroundGroup();
    back1->addWidgetList(list);
    back1->setStateEnable(widget3,true);
    back1->setStateEnable(widget4,true);


    QWidget *widget5 = new QWidget();
    widget5->hide();

    QWidget *widget6 =new QWidget();
    widget6->setWindowIcon(QIcon::fromTheme("lingmo-music"));
    widget6->hide();

    connect(widget1Switch,&KSwitchButton::clicked,this,[=](){
        if(back1->isVisible())
            back1->hide();
        else
            back1->show();
    });

    //插入删除
    connect(widget2Switch,&KSwitchButton::clicked,this,[=](){
        if(back1->widgetList().contains(widget3))
            back1->removeWidget(widget3);
        else
            back1->insertWidgetAt(back1->widgetList().count()-1,widget3);
        back1->setStateEnable(widget3,true);
    });

    connect(less,&KBorderlessButton::clicked,this,[=](){
        if(back1->widgetList().contains(widget3))
            back1->removeWidget(widget3);
        else
            back1->insertWidgetAt(back1->widgetList().count()-1,widget3);
        back1->setStateEnable(widget3,true);
    });

    connect(back1,&KBackgroundGroup::clicked,this,[=](QWidget* widget){
        if(widget == widget3 )
        {
            if(widget3Label3->isVisible())
                widget3Label3->hide();
            else
                widget3Label3->show();
        }
        if(widget == widget4 )
        {
            if(widget6->isVisible())
                widget6->hide();
            else
                widget6->show();
        }
    });

    connect(widget3btn,&KBorderlessButton::clicked,this,[=](){
        if(widget5->isVisible())
            widget5->hide();
        else
            widget5->show();
    });
    vmainLayout->addWidget(networkLabel);
    vmainLayout->addWidget(back);
    vmainLayout->addWidget(back1);
    vmainLayout->addStretch();
}

Widget::~Widget()
{
}

