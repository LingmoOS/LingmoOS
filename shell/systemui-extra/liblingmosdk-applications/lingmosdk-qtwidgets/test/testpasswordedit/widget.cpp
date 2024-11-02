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
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

static const QString password = "lingmo";
Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    m_pPasswordEdit = new KPasswordEdit(this);
    hLayout->addWidget(m_pPasswordEdit);
    m_pPasswordEdit1 = new KPasswordEdit(this);
    m_pPasswordEdit2 = new KPasswordEdit(this);


    QPushButton *confirmBtn = new QPushButton("confirm",this);
    hLayout->addWidget(confirmBtn);

    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_pPasswordEdit1);
    vLayout->addWidget(m_pPasswordEdit2);
    hLayout = new QHBoxLayout(this);
    QPushButton *enableBtn = new QPushButton("enable",this);
    hLayout->addWidget(enableBtn);
    QPushButton *loadingBtn = new QPushButton("isLoading:false",this);
    hLayout->addWidget(loadingBtn);
    vLayout->addLayout(hLayout);
    this->baseBar()->setLayout(vLayout);

    QString str("dwadwa");
   m_pPasswordEdit1->setPlaceholderText(str);//设置背景文字
   m_pPasswordEdit1->setClearButtonEnabled(false); //禁用ClearBtn按钮
   m_pPasswordEdit2->setEchoModeBtnVisible(false); //隐藏EchoMode按钮
   bool flag = m_pPasswordEdit2->echoModeBtnVisible();//返回EchoMode按钮是否隐藏
   m_pPasswordEdit2->setEchoMode(QLineEdit::EchoMode::Normal);//明文输入
   m_pPasswordEdit->setClearBtnVisible(false);  //隐藏ClearBtn按钮（启用但隐藏）
   bool flag1 =m_pPasswordEdit->clearBtnVisible();//返回ClearBtn按钮是否隐藏


    connect(confirmBtn,&QPushButton::clicked,this,[=](){
        if(m_pPasswordEdit->text() == password)
            m_pPasswordEdit->setState(LoginState::LoginSuccess);  //设置登录状态

        else
            m_pPasswordEdit->setState(LoginState::LoginFailed);
        m_pPasswordEdit->setFocus();
    });
    connect(m_pPasswordEdit,&KPasswordEdit::returnPressed,this,[=](){
        if(m_pPasswordEdit->text() == password)
            m_pPasswordEdit->setState(LoginState::LoginSuccess);

        else
            m_pPasswordEdit->setState(LoginState::LoginFailed);
        m_pPasswordEdit->setFocus();
    });
    connect(enableBtn,&QPushButton::clicked,this,[=](){
        if(m_pPasswordEdit->isEnabled())
        {
            m_pPasswordEdit->setEnabled(false);  //设置KLineEdit不可用
            enableBtn->setText("disable");
        }
        else
        {
            m_pPasswordEdit->setEnabled(true);
            enableBtn->setText("enable");
        }
    });
    connect(loadingBtn,&QPushButton::clicked,this,[=](){
        if(!m_pPasswordEdit->isLoading())
        {
            qDebug()<<"is loading";
            m_pPasswordEdit->setLoading(true);  //设置启用加载状态
            loadingBtn->setText("isLoading:true");
        }
        else
        {
            qDebug()<<"is not loading";
            m_pPasswordEdit->setLoading(false);
            loadingBtn->setText("isLoading:false");
        }
    });
}

Widget::~Widget()
{
}

