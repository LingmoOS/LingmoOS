/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "bluetoothdevicewindowitem.h"
class bluetoothdeviceitem;

bluetoothdevicewindowitem::bluetoothdevicewindowitem(QString dev_address , bool show_line , QWidget *parent):
    m_dev_address(dev_address),
    m_show_line(show_line),//line显示标志
    QFrame(parent)
{
    KyDebug();
    this->setFocusPolicy(Qt::NoFocus);
    this->setObjectName(m_dev_address);
    this->setMinimumSize(580, 56);
    Init();

}

bluetoothdevicewindowitem::~bluetoothdevicewindowitem()
{
    KyDebug();
    if (m_devBtn)
    {
        m_devBtn->disconnect();
        m_devBtn->deleteLater();
    }

    if (line_frame)
        line_frame->deleteLater();

}

void bluetoothdevicewindowitem::Init()
{
    QVBoxLayout * m_VBoxLout = new  QVBoxLayout(this);
    m_VBoxLout->setSpacing(0);
    m_VBoxLout->setContentsMargins(0,0,0,0);
    m_VBoxLout->setAlignment(Qt::AlignTop);

    m_devBtn = new bluetoothdeviceitem(m_dev_address,this);
    connect(m_devBtn,&bluetoothdeviceitem::devFuncOptSignals,this,&bluetoothdevicewindowitem::devFuncOptSignals);
    connect(m_devBtn,&bluetoothdeviceitem::devPairedSuccess,this,&bluetoothdevicewindowitem::devPairedSuccess);
    connect(m_devBtn,&bluetoothdeviceitem::devConnectedChanged,this,&bluetoothdevicewindowitem::devConnectedChanged);
    connect(m_devBtn,&bluetoothdeviceitem::devRssiChanged,this,&bluetoothdevicewindowitem::devRssiChanged);
    connect(m_devBtn,&bluetoothdeviceitem::bluetoothDeviceItemRemove,this,&bluetoothdevicewindowitem::bluetoothDeviceItemRemove);

    m_VBoxLout->addWidget(m_devBtn,1,Qt::AlignTop);

    line_frame = new QFrame(this);
    line_frame->setFixedHeight(1);
    line_frame->setMinimumWidth(582);
    line_frame->setFrameStyle(QFrame::HLine);
    m_VBoxLout->addWidget(line_frame,1,Qt::AlignTop);

    setLineFrameHidden(!m_show_line);
}



void bluetoothdevicewindowitem::setLineFrameHidden(bool hidden)
{
    KyDebug() << hidden ;
    line_frame->setHidden(hidden);
}
