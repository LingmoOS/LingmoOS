// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSHUANGLIREQUEST_H
#define DBUSHUANGLIREQUEST_H

#include "dbusrequestbase.h"
#include "huangliData/lunardatastruct.h"
#include "huangliData/dbusdatastruct.h"

//黄历数据请求类
class DbusHuangLiRequest : public DbusRequestBase
{
    Q_OBJECT
public:
    explicit DbusHuangLiRequest(QObject *parent = nullptr);

    //按月获取节假日信息
    bool getFestivalMonth(quint32 year, quint32 month, FestivalInfo&);
    //按天获取黄历信息
    bool getHuangLiDay(quint32 year, quint32 month, quint32 day, CaHuangLiDayInfo &);
    //按月获取黄历信息
    bool getHuangLiMonth(quint32 year, quint32 month, bool fill, CaHuangLiMonthInfo &);
    //获取农历信息
    void getLunarInfoBySolar(quint32 year, quint32 month, quint32 day);
    //获取农历月日程
    void getLunarMonthCalendar(quint32 year, quint32 month, bool fill);

signals:

public slots:
    void slotCallFinished(CDBusPendingCallWatcher*) override;

};

#endif // DBUSHUANGLIREQUEST_H
