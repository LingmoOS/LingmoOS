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
#include "kslider.h"
#include "kswitchbutton.h"
#include <kwindoweffects.h>
#include <QPalette>
#include <QVBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QEvent>
#include "ksearchlineedit.h"
#include "kpressbutton.h"
#include "kpushbutton.h"

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QWidget *mainWidget = new QWidget(this);
    mainWidget->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    mainWidget->installEventFilter(this);
    QLabel* label1 = new QLabel("KSlider",this);
    label1->move(50,100);
    KSlider* slider = new KSlider(mainWidget);
    slider->setToolTip("this is a tool tip");
    slider->setRange(0,100);
    //测试接口
    slider->setTranslucent(true);
    slider->setTickInterval(20);
    slider->setFixedWidth(300);
    slider->move(200,100);

    QLabel* label2 = new QLabel("KSwitchButton",this);
    label2->setFixedWidth(150);
    label2->move(50,200);
    KSwitchButton* switchbutton = new KSwitchButton(mainWidget);
    //测试接口
    switchbutton->setTranslucent(true);
    switchbutton->move(200,200);

    QLabel* label3 = new QLabel("KSearchLineEdit",this);
    label3->move(50,300);
    KSearchLineEdit* lineEdit = new KSearchLineEdit(mainWidget);
    lineEdit->move(200,300);
    //测试接口
    lineEdit->setTranslucent(true);

    QLabel* label4 = new QLabel("KPressButton",this);
    label4->move(50,400);
    KPressButton* pressBtn1 = new KPressButton(this);
    pressBtn1->setButtonType(KPressButton::CircleType);
    pressBtn1->setFixedSize(40,40);
    pressBtn1->setIcon(QIcon::fromTheme("lingmo-alarm-symbolic"));
    //测试接口
    pressBtn1->setTranslucent(true);
    pressBtn1->move(200,400);

    KPressButton* pressBtn2 = new KPressButton(this);
    pressBtn2->setFixedSize(40,40);
    pressBtn2->setIcon(QIcon::fromTheme("lingmo-alarm-symbolic"));
    //测试接口
    pressBtn2->setTranslucent(true);
    pressBtn2->move(250,400);

    QLabel* label5 = new QLabel("KPushButton",this);
    label5->move(50,500);


    KPushButton* pushbtn1 = new KPushButton(this);
    pushbtn1->setButtonType(KPushButton::CircleType);
    pushbtn1->setFixedSize(40,40);
    pushbtn1->setIcon(QIcon::fromTheme("lingmo-alarm-symbolic"));
    pushbtn1->setTranslucent(true);
    pushbtn1->move(200,500);

    KPushButton* pushbtn2 = new KPushButton(this);
    pushbtn2->setFixedSize(40,40);
    pushbtn2->setIcon(QIcon::fromTheme("lingmo-alarm-symbolic"));
    pushbtn2->setIconColor(QColor(255,0,0));
    //测试接口
    pushbtn2->setTranslucent(true);
    pushbtn2->move(250,500);


    mainLayout->addWidget(mainWidget);
    mainLayout->setMargin(0);
    setFixedSize(800,600);
}

Widget::~Widget()
{
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Paint)
    {
        auto widget = qobject_cast<QWidget*>(watched);
        if(widget)
        {
            widget->setAutoFillBackground(true);

            QPalette palette = this->palette();
            auto color = this->palette().color(QPalette::Window);
            color.setAlphaF(0.39);
            palette.setColor(QPalette::Window,color);
            widget->setPalette(palette);
            widget->setBackgroundRole(QPalette::Window);
        }
    }
    return QObject::eventFilter(watched,event);
}

