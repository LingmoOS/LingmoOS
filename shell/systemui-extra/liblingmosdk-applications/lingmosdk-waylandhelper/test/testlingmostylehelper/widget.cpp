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
#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    m_btn = new QPushButton("switch",this);
    m_btn->setCheckable(true);
    m_btn->move(100,100);
    m_testWidget = new QWidget();

    connect(m_btn,&QPushButton::clicked,this,[=](bool checked)
    {
        if(checked)
        {
            kdk::LingmoUIStyleHelper::self()->removeHeader(m_testWidget);
            m_testWidget->show();
            kdk::WindowManager::setGeometry(m_testWidget->windowHandle(),QRect(0,0,600,400));
            kdk::WindowManager::setWindowRadius(m_testWidget->windowHandle(),0);
        }
        else
            m_testWidget->hide();

    });
}

Widget::~Widget()
{
}

