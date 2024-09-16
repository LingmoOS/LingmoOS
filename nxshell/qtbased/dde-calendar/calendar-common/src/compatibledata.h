// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMPATIBLEDATA_H
#define COMPATIBLEDATA_H

#include "dschedule.h"

#include <QString>

namespace DDE_Calendar {
//通过旧的日程id获取新的日程id
QString getNewTypeIDByOldTypeID(int oldTypeID);
int getOldTypeIDByNewTypeID(const QString &newTypeID);

DSchedule::Ptr getScheduleByExported(const QString &scheduleStr);

//
void setAlarmByOldRemind(const DSchedule::Ptr &schedule, const QString &remind);
void setRRuleByOldRRule(const DSchedule::Ptr &schedule, const QString &rrule);
void setExDate(const DSchedule::Ptr &schedule, const QJsonArray &ignore);
QString getOldRemindByAlarm(const DSchedule::AlarmType &alarmType);

//将新的日程数据转换为旧的查询数据({"Date":"",Jobs:["",""]})
QString getExternalSchedule(const DSchedule::Map &scheduleMap);
} // namespace DDE_Calendar

#endif // COMPATIBLEDATA_H
