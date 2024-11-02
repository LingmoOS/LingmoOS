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
#include <QGraphicsDropShadowEffect>

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QVBoxLayout* vLayout = new QVBoxLayout();

    QPushButton* button  = new QPushButton(this);
    button->setText("NothingTip");
    QPushButton* button1  = new QPushButton(this);
    button1->setText("NormalTip");
    QPushButton* button2  = new QPushButton(this);
    button2->setText("InfoTip");
    QPushButton* button3  = new QPushButton(this);
    button3->setText("WarningTip");
    QPushButton* button4  = new QPushButton(this);
    button4->setText("ErrorTip");
    vLayout->addWidget(button);
    vLayout->addWidget(button1);
    vLayout->addWidget(button2);
    vLayout->addWidget(button3);
    vLayout->addWidget(button4);

    // 提供了两种构造函数 m_pNothingTip 和 m_pInfoTip 分别为两种构造函数
    m_pNothingTip = new KBallonTip();
    m_pNothingTip->setWindowFlag(Qt::FramelessWindowHint);
    m_pNothingTip->setAttribute(Qt::WA_TranslucentBackground);
    m_pNothingTip->setTipType(TipType::Nothing);  //设置KBallonTip类型
    m_pNothingTip->setText("这是一段操作信息描述的文字,这是一段操作信息描述的文字,这是一段操作信息描述的文字,这是一段操作信息描述的文字");  // 设置文本内容

    m_pNormalTip = new KBallonTip();
    m_pNormalTip->setWindowFlag(Qt::FramelessWindowHint);
    m_pNormalTip->setAttribute(Qt::WA_TranslucentBackground);
    m_pNormalTip->setTipType(TipType::Normal);
    m_pNormalTip->setText("setContentsMargins调整边距");
    //两种setmargins都支持
    //m_pWarningTip->setContentsMargins(QMargins(0,0,0,0));
    m_pNormalTip->setContentsMargins(20,20,20,20);  //设置内容边距

    m_pInfoTip = new KBallonTip("提供两种构造函数",TipType::Info);
    m_pInfoTip->setWindowFlag(Qt::FramelessWindowHint);
    m_pInfoTip->setAttribute(Qt::WA_TranslucentBackground);

    m_pWarningTip = new KBallonTip();
    m_pWarningTip->setWindowFlag(Qt::FramelessWindowHint);
    m_pWarningTip->setAttribute(Qt::WA_TranslucentBackground);
    m_pWarningTip->setTipType(TipType::Warning);
    m_pWarningTip->setText("这是一段操作信息描述的文字,这是一段操作信息描述的文字,这是一段操作信息描述的文字,这是一段操作信息描述的文字");

    QMargins marge(20,20,20,20);
    m_pErrorTip = new KBallonTip();
    m_pErrorTip->setWindowFlag(Qt::FramelessWindowHint);
    m_pErrorTip->setAttribute(Qt::WA_TranslucentBackground);
    m_pErrorTip->setTipType(TipType::Error);
    m_pErrorTip->setContentsMargins(marge);  //通过 QMargins 设置内容边距
    m_pErrorTip->setText("这是一段操作信息描述的文字,这是一段操作信息描述的文字,这是一段操作信息描述的文字,这是一段操作信息描述的文字");

    connect(button,&QPushButton::clicked,this,[=](){
        m_pNothingTip->setTipTime(4000);  //设置显示时间
        m_pNothingTip->showInfo();  //定时显示KBallonTip，随后隐退   默认为1s
    });

    connect(button1,&QPushButton::clicked,this,[=](){
        m_pNormalTip->setTipTime(3000);
        m_pNormalTip->showInfo();
    });

    connect(button2,&QPushButton::clicked,this,[=](){
        m_pInfoTip->setTipTime(2000);
        m_pInfoTip->showInfo();
    });

    connect(button3,&QPushButton::clicked,this,[=](){
        m_pWarningTip->setTipTime(1000);
        m_pWarningTip->showInfo();
    });
    connect(button4,&QPushButton::clicked,this,[=](){
        m_pErrorTip->showInfo();
    });
//    //vLayout->addWidget(m_pNothingTip);
//    vLayout->addWidget(m_pNormalTip);
//    vLayout->addWidget(m_pInfoTip);
//    vLayout->addWidget(m_pWarningTip);
//    vLayout->addWidget(m_pErrorTip);
    this->baseBar()->setLayout(vLayout);
}

Widget::~Widget()
{
}

