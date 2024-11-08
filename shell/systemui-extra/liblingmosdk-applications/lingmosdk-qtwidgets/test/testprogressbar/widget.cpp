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
#include <QDebug>
#include <QProgressBar>
Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    m_pTimer = new QTimer(this);
    m_pTimer->setInterval(50);
    QVBoxLayout* vLayout = new QVBoxLayout(this);
    QHBoxLayout* tmpLayout = new QHBoxLayout();
    m_pVerticalBar = new KProgressBar(this);
    m_pVerticalBar->setOrientation(Qt::Vertical);  //设置KProgressBar方向
    m_pVerticalBar->setTextVisible(false);
    m_pVerticalBar->setRange(0,100);
    m_pVerticalBar->setValue(50);
    tmpLayout->addWidget(m_pVerticalBar);
    vLayout->addLayout(tmpLayout);
    QHBoxLayout* hLayout = new QHBoxLayout(this);
    m_pHorizontalBar = new KProgressBar(this);
    m_pHorizontalBar->setRange(0,100);
    m_pHorizontalBar->setValue(50);
    hLayout->addWidget(m_pHorizontalBar);
    vLayout->addLayout(hLayout);
    hLayout = new QHBoxLayout(this);
    m_pStartBtn = new QPushButton(this);
    m_pStartBtn->setText("开始");
    m_pStopBtn = new QPushButton(this);
    m_pStopBtn->setText("停止");
    m_pRestoreBtn = new QPushButton(this);
    m_pRestoreBtn->setText("恢复");
    hLayout->addStretch();
    hLayout->addWidget(m_pStartBtn);
    hLayout->addWidget(m_pStopBtn);
    hLayout->addWidget(m_pRestoreBtn);
    hLayout->addStretch();
    vLayout->addLayout(hLayout);
    baseBar()->setLayout(vLayout);
    connect(m_pTimer,&QTimer::timeout,this,[=](){
        if(m_pVerticalBar->value() != m_pVerticalBar->maximum())
            m_pVerticalBar->setValue(m_pVerticalBar->value()+1);
        else
        {
            m_pVerticalBar->setValue(m_pVerticalBar->minimum());
            m_pVerticalBar->setState(ProgressBarState::NormalProgress);  //设置KprogressBar状态
        }
        if(m_pHorizontalBar->value() != m_pHorizontalBar->maximum())
            m_pHorizontalBar->setValue(m_pHorizontalBar->value()+1);
        else
        {
            m_pHorizontalBar->setValue(m_pHorizontalBar->minimum());
            m_pHorizontalBar->setState(ProgressBarState::NormalProgress);
        }
    });
    connect(m_pStartBtn,&QPushButton::clicked,this,[=](){
        if(!m_pTimer->isActive())
            m_pTimer->start();
        //qDebug() << m_pVerticalBar->text(); //获取文本值
    });
    connect(m_pStopBtn,&QPushButton::clicked,this,[=]()
    {
        m_pTimer->stop();
        m_pVerticalBar->setState(ProgressBarState::FailedProgress);
        m_pHorizontalBar->setState(ProgressBarState::FailedProgress);
    });
    connect(m_pRestoreBtn,&QPushButton::clicked,this,[=]()
    {
        m_pTimer->start();
        m_pVerticalBar->setValue(0);
        m_pVerticalBar->setState(ProgressBarState::NormalProgress);
        m_pHorizontalBar->setValue(0);
        m_pHorizontalBar->setState(ProgressBarState::NormalProgress);
    });
}

Widget::~Widget()
{

}

