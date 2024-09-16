// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSHUANGLIREQUEST_H
#define DBUSHUANGLIREQUEST_H

#include "huangliData/lunardatastruct.h"
#include "huangliData/dbusdatastruct.h"

//黄历数据请求类
class LunarDBusInterface : public QObject
{
    Q_OBJECT
public:
    explicit LunarDBusInterface(QObject *parent = nullptr);

    //按天获取黄历信息
    bool huangLiDay(quint32 year, quint32 month, quint32 day, CaHuangLiDayInfo &);

private:
    QDBusInterface *m_huangLiInter;
};

#endif // DBUSHUANGLIREQUEST_H
