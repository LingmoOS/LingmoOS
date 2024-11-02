/*
 * liblingmosdk-system's Library
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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "customerevent.h"


QEvent::Type CustomerEvent::m_EventType = QEvent::None;

CustomerEvent::CustomerEvent(QString urlString,QString ipString,QString typeString,QString sendString) : QEvent(eventType())
{
    m_url = urlString;
    m_ip = ipString;
    m_type = typeString;
    m_send = sendString;
}

CustomerEvent::~CustomerEvent()
{

}

QEvent::Type CustomerEvent::eventType()
{
    // 创建事件Type
    if (m_EventType == QEvent::None)
        m_EventType = (QEvent::Type)QEvent::registerEventType();

    return m_EventType;
}

QString CustomerEvent::geturlValueString(void)
{
    return m_url;
}

QString CustomerEvent::getipValueString(void)
{
    return m_ip;
}

QString CustomerEvent::getTypeValueString(void)
{
    return m_type;
}
QString CustomerEvent::getSendValueString(void)
{
    return m_send;
}
