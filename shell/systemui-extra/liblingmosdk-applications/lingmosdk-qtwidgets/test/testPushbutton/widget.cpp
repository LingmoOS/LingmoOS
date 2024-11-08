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
#include <QGroupBox>
#include <QIcon>
#include <QPalette>

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QVBoxLayout* vLayout = new QVBoxLayout;
    QGroupBox* groupBox1 = new QGroupBox("border",this);
    QGroupBox* groupBox2 = new QGroupBox("borderless",this);
    QGroupBox* groupBox3 = new QGroupBox("borderless tooltip",this);
    QHBoxLayout* hLayout =new QHBoxLayout;
    m_pBtn1 = new KBorderButton("border",this);     //构造一个带边框按钮
    m_pBtn2 = new KBorderButton("border",this);
    m_pBtn2->setIcon(QIcon::fromTheme("browser-download-symbolic"));        //设置按钮图标
    m_pBtn3 = new KBorderButton("border",this);
    m_pBtn3->setIcon(QIcon::fromTheme("browser-download-symbolic"));
    m_pBtn3->setEnabled(false);         //设置不可点击
    hLayout->addWidget(m_pBtn1);
    hLayout->addWidget(m_pBtn2);
    hLayout->addWidget(m_pBtn3);
    groupBox1->setLayout(hLayout);
    m_pBtn4 = new KBorderlessButton("borderless",this);     //构造一个无边框按钮
    m_pBtn5 = new KBorderlessButton("borderless",this);
    m_pBtn5->setIcon(QIcon::fromTheme("document-send-symbolic"));       //设置按钮图标
    m_pBtn6 = new KBorderlessButton("borderless",this);
    m_pBtn6->setIcon(QIcon::fromTheme("browser-download-symbolic"));
    m_pBtn6->setEnabled(false);     //设置不可点击

    hLayout = new QHBoxLayout;
    hLayout->addWidget(m_pBtn4);
    hLayout->addWidget(m_pBtn5);
    hLayout->addWidget(m_pBtn6);
    groupBox2->setLayout(hLayout);

    KBorderlessButton* m_pBtn7 = new KBorderlessButton("borderless test",this);     //构造一个无边框按钮
    m_pBtn7->setFixedWidth(80);
    KBorderlessButton*  m_pBtn8 = new KBorderlessButton("borderless",this);
    m_pBtn8->setIcon(QIcon::fromTheme("document-send-symbolic"));       //设置按钮图标
    KBorderlessButton* m_pBtn9 = new KBorderlessButton("borderless",this);
    m_pBtn9->setFixedWidth(80);
    m_pBtn9->setIcon(QIcon::fromTheme("browser-download-symbolic"));

    hLayout = new QHBoxLayout;
    hLayout->addWidget(m_pBtn7);
    hLayout->addWidget(m_pBtn8);
    hLayout->addWidget(m_pBtn9);

    groupBox3->setLayout(hLayout);
    vLayout->addWidget(groupBox1);
    vLayout->addWidget(groupBox2);
    vLayout->addWidget(groupBox3);
    baseBar()->setLayout(vLayout);
}

Widget::~Widget()
{
}

