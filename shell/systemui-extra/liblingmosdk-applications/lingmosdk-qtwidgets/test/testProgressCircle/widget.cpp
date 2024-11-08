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
#include "kprogresscircle.h"
#include <QBoxLayout>
#include <QColor>

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QVBoxLayout*vLayout = new QVBoxLayout;
    QHBoxLayout* hLayout = new QHBoxLayout;
    KProgressCircle *circle1 = new KProgressCircle(this);
    circle1->setValue(40);   //设置初始值
    hLayout->addWidget(circle1);
    KProgressCircle *circle2 = new KProgressCircle(this);
    circle2->setValue(10);
    circle2->setState(ProgressBarState::FailedProgress);   //设置失败状态
    hLayout->addWidget(circle2);
    KProgressCircle *circle3 = new KProgressCircle(this);
    circle3->setValue(75);
    circle3->setState(ProgressBarState::SuccessProgress);  //设置成功状态
    hLayout->addWidget(circle3);
    vLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout;
    KProgressCircle *circle4 = new KProgressCircle(this);
    circle4->setValue(40);
    circle4->setTextVisible(false);   //设置文本不可见
    hLayout->addWidget(circle4);
    KProgressCircle *circle5 = new KProgressCircle(this);
    circle5->setValue(93);
    circle5->setTextVisible(false);
    circle5->setState(ProgressBarState::FailedProgress);
    hLayout->addWidget(circle5);
    KProgressCircle *circle6 = new KProgressCircle(this);
    circle6->setValue(75);
    circle6->setTextVisible(false);
    circle6->setState(ProgressBarState::SuccessProgress);
    hLayout->addWidget(circle6);
    vLayout->addLayout(hLayout);

    baseBar()->setLayout(vLayout);

    m_pTimer = new QTimer(this);
    m_pTimer->setInterval(20);
    connect(m_pTimer,&QTimer::timeout,circle1,[=](){
        if(circle1->value()<circle1->maximum())
            circle1->setValue(circle1->value()+1);
        else
        {
            circle1->setValue(circle1->minimum());
            circle1->setState(NormalProgress);
        }

    });
    connect(m_pTimer,&QTimer::timeout,circle4,[=](){
        if(circle4->value()<circle4->maximum())
            circle4->setValue(circle4->value()+1);
        else
        {
            circle4->setValue(circle4->minimum());
            circle4->setState(NormalProgress);
        }

    });
    m_pTimer->start();
}

Widget::~Widget()
{
}

