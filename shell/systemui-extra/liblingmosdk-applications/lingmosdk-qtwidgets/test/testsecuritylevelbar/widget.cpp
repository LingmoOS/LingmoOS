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
    QVBoxLayout* vLayout = new QVBoxLayout();
    QHBoxLayout*hLayout = new QHBoxLayout();
    m_pInfoLabel = new QLabel("请设置密码：",this);
    m_pLineEdit = new QLineEdit(this);
    m_pLineEdit->setEchoMode(QLineEdit::Password);
    hLayout->addWidget(m_pInfoLabel);
    hLayout->addWidget(m_pLineEdit);
    vLayout->addStretch();
    vLayout->addLayout(hLayout);
    hLayout = new QHBoxLayout();
    m_pBar = new KSecurityLevelBar(this);       //构建一个安全等级提示条
    m_pBar->hide();       //隐藏安全等级提示条
    hLayout->addWidget(m_pBar);
    vLayout->addLayout(hLayout);
    vLayout->addStretch();
    baseBar()->setLayout(vLayout);

    //连接槽函数，根据输入密码的长度来设置安全等级进度条的等级并显示。
    connect(m_pLineEdit,&QLineEdit::textChanged,this,[=](){
        int length = m_pLineEdit->text().length();
        if(length<8)
            m_pBar->setSecurityLevel(SecurityLevel::Low);   //设置安全等级
        else if(length >=8 && length<12)
            m_pBar->setSecurityLevel(SecurityLevel::Medium);
        else
            m_pBar->setSecurityLevel(SecurityLevel::High);
        m_pBar->show();
    });
}

Widget::~Widget()
{
}

