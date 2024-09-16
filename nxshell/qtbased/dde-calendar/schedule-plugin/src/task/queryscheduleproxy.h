// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUERYSCHEDULEPROXY_H
#define QUERYSCHEDULEPROXY_H

#include "dschedule.h"
#include "../data/jsondata.h"
#include "accountitem.h"

class queryScheduleProxy
{
public:
    queryScheduleProxy(JsonData *jsonData);
    DSchedule::Map querySchedule();
    DSchedule::List scheduleMapToList(const DSchedule::Map &scheduleMap);
    bool getTimeIsExpired() const;

private:
    struct DateTimeLimit {
        QDateTime beginTime;
        QDateTime endTime;
    };
    struct TimeLimit {
        QTime beginTime;
        QTime endTime;
        bool isInvalid {false};
    };
    DSchedule::Map queryWeeklySchedule(QDateTime &beginTime, QDateTime &endTime, int beginW = 0, int endW = 0);
    DSchedule::Map queryMonthlySchedule(QDateTime &beginTime, QDateTime &endTime, int beginM = 0, int endM = 0);
    DSchedule::Map queryEveryDaySchedule(QDateTime &beginTime, QDateTime &endTime);
    DSchedule::Map queryEveryYearSchedule(QDateTime &beginTime, QDateTime &endTime);
    DSchedule::Map queryWorkingDaySchedule(QDateTime &beginTime, QDateTime &endTime);
    DSchedule::Map queryNonRepeatingSchedule();
    DSchedule::Map queryNextNumSchedule(QDateTime &beginTime, QDateTime &endTime, int NextNum);
    DSchedule::Map queryAllSchedule(QString key, QDateTime &beginTime, QDateTime &endTime);

    DSchedule::Map sortAndFilterSchedule(DSchedule::Map &out);
    DSchedule::Map WeeklyScheduleFileter(DSchedule::Map &out, QSet<int> &weeklySet);
    DSchedule::Map MonthlyScheduleFileter(DSchedule::Map &out, int beginM = 0, int endM = 0);
    bool monthlyIsIntersections(const QDateTime &beginTime, const QDateTime &endTime, int beginM = 0, int endM = 0);

    bool checkedTimeIsIntersection(QTime &beginTime, QTime &endTime, QTime &fixbeginTime, QTime &fixendTime);

    DSchedule::Map scheduleFileterByTime(DSchedule::Map &scheduleInfo, QTime &fileterBeginTime, QTime &fileterEndTime);
    DSchedule::Map scheduleFileterByDate(DSchedule::Map &scheduleInfo, QDate &fileterBeginDate, QDate &fileterEndDate);
    DSchedule::Map scheduleFileterByTitleName(DSchedule::Map &scheduleInfo, const QString &strName);
    bool weeklyIsIntersections(const QDateTime &beginTime, const QDateTime &endTime, QSet<int> &weeklySet);

    SemanticsDateTime getQueryDateTime(JsonData *jsonData);

    DateTimeLimit getTimeLimitByTimeInfo(const SemanticsDateTime &timeInfoVect);
    TimeLimit getTimeFileterByTimeInfo(const SemanticsDateTime &timeInfoVect);
    void setTimeIsExpired(const bool timeisExp);
    /**
     * @brief timeFrameIsValid    时间范围是否有效，开始时间不能大于结束时间
     * @param timeInfoVect  时间范围
     * @return      返回false表示开始时间大于结束时间
     */
    bool timeFrameIsValid(const SemanticsDateTime &timeInfoVect);
private:
    JsonData *m_queryJsonData;
    bool m_TimeIsExpired {false};
};

#endif // QUERYSCHEDULEPROXY_H
