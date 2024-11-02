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
    m_pSwitchBtn1 = new KSwitchButton(this);    //构造一个kswitchbutton
    m_pSwitchBtn1->setFixedSize(50,24);         //设置button的大小
    m_pSwitchBtn1->setCheckable(true);          //设置是否可选中
    m_pSwitchBtn1->setChecked(true);            //设置是否为选中状态
    m_pLabel1 = new QLabel(this);
    m_pLabel1->setText(m_pSwitchBtn1->isChecked()?"ON":"OFF");      //获取button是否是选中状态
    //监听状态改变信号stateChanged
    connect(m_pSwitchBtn1,&KSwitchButton::stateChanged,this,[=](){m_pLabel1->setText(m_pSwitchBtn1->isChecked()?"ON":"OFF");});
    m_pBtn1 = new KBorderlessButton(this);
    m_pBtn1->setChecked(true);
    m_pBtn1->setText(m_pSwitchBtn1->isEnabled()?"Enable":"Disable");
    connect(m_pBtn1,&KBorderlessButton::clicked,this,[=](){
        m_pSwitchBtn1->setEnabled(!m_pSwitchBtn1->isEnabled());
        m_pBtn1->setText(m_pSwitchBtn1->isEnabled()?"Enable":"Disable");});
    hLayout->addWidget(m_pSwitchBtn1);
    hLayout->addWidget(m_pLabel1);
    hLayout->addWidget(m_pBtn1);
    hLayout->addStretch();
    mainLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout;
    m_pSwitchBtn2 = new KSwitchButton(this);
    m_pSwitchBtn2->setFixedSize(100,48);
    m_pSwitchBtn2->setEnabled(false);   //设置button状态为不可点击状态
    m_pLabel2 = new QLabel(this);
    m_pLabel2->setText(m_pSwitchBtn2->isChecked()?"ON":"OFF");
    connect(m_pSwitchBtn2,&KSwitchButton::stateChanged,this,[=](){m_pLabel2->setText(m_pSwitchBtn2->isChecked()?"ON":"OFF");});
    m_pBtn2 = new KBorderlessButton(this);
    m_pBtn2->setText(m_pSwitchBtn2->isEnabled()?"Enable":"Disable");
    connect(m_pBtn2,&KBorderlessButton::clicked,this,[=](){
        m_pSwitchBtn2->setEnabled(!m_pSwitchBtn2->isEnabled());
        m_pBtn2->setText(m_pSwitchBtn2->isEnabled()?"Enable":"Disable");});
    hLayout->addWidget(m_pSwitchBtn2);
    hLayout->addWidget(m_pLabel2);
    hLayout->addWidget(m_pBtn2);
    hLayout->addStretch();
    mainLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout;
    m_pSwitchBtn3 = new KSwitchButton(this);
    m_pSwitchBtn3->setFixedSize(200,96);
    m_pLabel3 = new QLabel(this);
    m_pLabel3->setText(m_pSwitchBtn3->isChecked()?"ON":"OFF");
    connect(m_pSwitchBtn3,&KSwitchButton::stateChanged,this,[=](){m_pLabel3->setText(m_pSwitchBtn3->isChecked()?"ON":"OFF");});
    m_pBtn3 = new KBorderlessButton(this);
    m_pBtn3->setText(m_pSwitchBtn3->isEnabled()?"Enable":"Disable");
    connect(m_pBtn3,&KBorderlessButton::clicked,this,[=](){
        m_pSwitchBtn3->setEnabled(!m_pSwitchBtn3->isEnabled());
        m_pBtn3->setText(m_pSwitchBtn3->isEnabled()?"Enable":"Disable");});
    hLayout->addWidget(m_pSwitchBtn3);
    hLayout->addWidget(m_pLabel3);
    hLayout->addWidget(m_pBtn3);
    hLayout->addStretch();
    mainLayout->addLayout(hLayout);

    baseBar()->setLayout(mainLayout);
}

Widget::~Widget()
{
}

