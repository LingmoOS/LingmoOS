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
#include <QLabel>

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QVBoxLayout*mainLayout = new QVBoxLayout;
    mainLayout->addStretch();


    QHBoxLayout*hLayout = new QHBoxLayout;
    m_pSearchLineEdit1 = new KSearchLineEdit(this);     //构造一个搜索框
    //m_pSearchLineEdit1->setPlaceholderAlignment(Qt::AlignCenter);     //设置placeholder的对齐方式
    m_pSearchLineEdit1->setClearButtonEnabled(true);    //是否启用清除键（true为启用）
    m_pSearchLineEdit1->setPlaceholderText("此处需要输入很长非常长的需要查找的内容");   //设置placeholder的文本内容
    m_pSearchLineEdit1->setAlignment(Qt::AlignLeft);    //设置输入内容的对齐方式
    m_pLabel1 = new QLabel(this);
    hLayout->addWidget(m_pSearchLineEdit1);

    QPushButton* btn1 = new QPushButton(this);
    btn1->setText("隐藏");
    QPushButton* btn2 = new QPushButton(this);
    btn2->setText("显示");
    btn2->move(110,0);
    QPushButton* btn3 = new QPushButton(this);
    btn3->setText("清除");
    btn3->move(220,0);

    connect(btn1,&QPushButton::clicked,this,[=]{
        m_pSearchLineEdit1->hide();
    });
    connect(btn2,&QPushButton::clicked,this,[=]{
        m_pSearchLineEdit1->show();
    });
    connect(btn3,&QPushButton::clicked,this,[=]{
        m_pSearchLineEdit1->clear();
    });

    connect(m_pSearchLineEdit1,&KSearchLineEdit::returnPressed,this,[=](){
        m_pLabel1->setText(QString("Search Content is:%1").arg(m_pSearchLineEdit1->text()));
    });
    mainLayout->addLayout(hLayout);
    mainLayout->addWidget(m_pLabel1);


    hLayout = new QHBoxLayout;
    m_pSearchLineEdit2 = new KSearchLineEdit(this);
    m_pLabel2 = new QLabel(this);
    m_pSearchLineEdit2->setEnabled(false);     //设置搜索框是否可用
    hLayout->addWidget(m_pSearchLineEdit2);
    connect(m_pSearchLineEdit2,&KSearchLineEdit::returnPressed,this,[=](){
        m_pLabel2->setText(QString("Search Content is:%1").arg(m_pSearchLineEdit2->text()));
    });
    mainLayout->addLayout(hLayout);
    mainLayout->addWidget(m_pLabel2);
    mainLayout->addStretch();
    baseBar()->setLayout(mainLayout);
}

Widget::~Widget()
{
}


