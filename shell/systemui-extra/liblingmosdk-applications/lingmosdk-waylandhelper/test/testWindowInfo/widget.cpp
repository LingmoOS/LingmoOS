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

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->setWindowTitle("testWindowInfo");
    initUi();
    initConnections();
}

Widget::~Widget()
{
}

void Widget::initUi()
{
    QHBoxLayout *hLayout = new QHBoxLayout();
    m_btnActivate = new QPushButton("Activate",this);
    m_btnMaximum = new QPushButton("Maximum",this);
    m_btnMinimize = new QPushButton("Minimum",this);
    m_btnClose = new QPushButton("Close",this);
    m_btnStayOnTop = new QPushButton("StayOnTop",this);

    hLayout->addWidget(m_btnActivate);
    hLayout->addWidget(m_btnMaximum);
    hLayout->addWidget(m_btnMinimize);
    hLayout->addWidget(m_btnClose);
    hLayout->addWidget(m_btnStayOnTop);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addLayout(hLayout);
}

void Widget::initConnections()
{
    connect(WindowManager::self(),&WindowManager::windowAdded,this,[=](const WindowId& windowId){
        if(WindowManager::getWindowTitle(windowId) == "testWindowInfo")
            m_lastestWindowId = windowId;
    });

    connect(m_btnActivate,&QPushButton::clicked,this,[=](){
        WindowManager::activateWindow(m_lastestWindowId);
    });

    connect(m_btnClose,&QPushButton::clicked,this,[=](){
        WindowManager::closeWindow(m_lastestWindowId);
    });

    connect(m_btnMaximum,&QPushButton::clicked,this,[=](){
        WindowManager::maximizeWindow(m_lastestWindowId);
    });

    connect(m_btnMinimize,&QPushButton::clicked,this,[=](){
        WindowManager::minimizeWindow(m_lastestWindowId);
    });

    connect(m_btnStayOnTop,&QPushButton::clicked,this,[=](){
        WindowManager::keepWindowAbove(m_lastestWindowId);
    });

    connect(WindowManager::self(),&WindowManager::windowChanged,this,[=](const WindowId& windowId){
        if(m_lastestWindowId == windowId)
        {
            kdk::WindowInfo windowInfo =  WindowManager::getwindowInfo(m_lastestWindowId);
            qDebug() << "-----------window state-----------";
            qDebug() << "isActive:"<< windowInfo.isActive();
            qDebug() << "isMaximized:"<<windowInfo.isMaximized();
            qDebug() << "isMinimized"<<windowInfo.isMinimized();
            qDebug() << "isValid"<<windowInfo.isValid();
            qDebug() << "isKeepAbove"<<windowInfo.isKeepAbove();
            //qDebug() << "isWaylandWindow"<<windowInfo.isWaylandWindow();

            qDebug() << "-----------window ability-----------";
            qDebug() << "isCloseable:"<< windowInfo.isCloseable();
            qDebug() << "isFullScreenable:"<< windowInfo.isFullScreenable();
            qDebug() << "isGroupable:"<< windowInfo.isGroupable();
            qDebug() << "isMovable:"<< windowInfo.isMovable();
            qDebug() << "isMinimizable:"<< windowInfo.isMinimizable();
            qDebug() << "isMaximizable:"<< windowInfo.isMaximizable();
        }
    });

}
