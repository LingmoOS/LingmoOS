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

#ifndef CUSTOMEREVENT_H
#define CUSTOMEREVENT_H
#include <QString>
#include <QEvent>

class CustomerEvent:public QEvent
{
public:
    CustomerEvent(QString urlString ="",QString ipString ="",QString typeString ="",QString sendString="");
    ~CustomerEvent();

    static Type eventType();
    QString geturlValueString(void);
    QString getipValueString(void);
    QString getTypeValueString(void);
    QString getSendValueString(void);
private:
    static Type m_EventType;
    QString m_url;
    QString m_ip;
    QString m_type;
    QString m_send;
};

#endif // CUSTOMEREVENT_H
