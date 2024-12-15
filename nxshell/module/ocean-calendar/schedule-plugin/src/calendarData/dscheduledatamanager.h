// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSCHEDULEDATAMANAGER_H
#define DSCHEDULEDATAMANAGER_H

#include "dschedule.h"
#include "dschedulequerypar.h"

#include <QMap>

//日程数据管理模块
class DScheduleDataManager
{
public:
    static DScheduleDataManager *getInstance();

    QString createSchedule(const DSchedule::Ptr &schedule);

    DSchedule::Ptr queryScheduleByScheduleID(const QString &scheduleID);

    bool deleteScheduleByScheduleID(const QString &scheduleID);

    bool updateSchedule(const DSchedule::Ptr &schedule);

    //根据类型ID判断是否为节假日日程
    bool isFestivalSchedule(const QString &scheduleTypeID);

    DSchedule::Map querySchedulesWithParameter(const DScheduleQueryPar::Ptr &params);
    DSchedule::Map queryScheduleByRRule(const QDateTime &dtStart, const QDateTime &dtEnd, const DScheduleQueryPar::RRuleType &rrultTyep);
    DSchedule::Map queryScheduleByLimit(const QDateTime &dtStart, const QDateTime &dtEnd, int topNum);
    DSchedule::Map queryScheduleBySummary(const QDateTime &dtStart, const QDateTime &dtEnd, const QString &summary);

private:
    DScheduleDataManager();
};

#endif // DSCHEDULEDATAMANAGER_H
