// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSCHEDULE_H
#define DSCHEDULE_H

#include "event.h"
#include "dschedulequerypar.h"

#include <QString>
#include <QDebug>
#include <QSharedPointer>

//日程信息
class DSchedule : public KCalendarCore::Event
{
public:
    //提醒规则
    enum AlarmType {
        Alarm_None, //从不
        Alarm_Begin, //日程开始时
        Alarm_15Min_Front, //提前15分钟
        Alarm_30Min_Front, //提前30分钟
        Alarm_1Hour_Front, //提前1小时
        Alarm_1Day_Front, //提前1天
        Alarm_2Day_Front, //提前2天
        Alarm_1Week_Front, //提前1周
        Alarm_AllDay_None, //全天从不
        Alarm_9Hour_After, //日程当天9点(全天)
        Alarm_15Hour_Front, //一天前（全天），为日程开始前15小时
        Alarm_39Hour_Front, //2天前（全天），为日程开始前39小时
        Alarm_159Hour_Front, //一周前(全天)，为日程开始前159小时
    };

    //重复规则
    enum RRuleType {
        RRule_None, //从不
        RRule_Day, //每天
        RRule_Work, //每工作日
        RRule_Week, //每周
        RRule_Month, //每月
        RRule_Year, //每年
    };

    typedef QSharedPointer<DSchedule> Ptr;
    typedef QVector<DSchedule::Ptr> List;
    typedef QMap<QDate, List> Map;
    typedef QMap<QDate, List>::Iterator Iter;

    DSchedule();
    DSchedule(const DSchedule &schedule);
    DSchedule(const KCalendarCore::Event &event);

    DSchedule *clone() const override;

    QString scheduleTypeID() const;
    void setScheduleTypeID(const QString &typeID);

    //为了与旧数据兼容处理（与联系人交互中使用的是自增ID作为日程的ID）
    int dbID() const;
    void setDbID(int id);

    bool isMoved();
    void setMoved(bool moved);

    bool isValid() const;

    //是否是跨天的
    bool isMultiDay() const;

    bool operator==(const DSchedule &schedule) const;

    bool operator<(const DSchedule &schedule) const;

    void setAlarmType(const AlarmType &alarmType);
    AlarmType getAlarmType();

    //设置重复规则，若有重复规则，规则的截止次数或日期通过RecurrenceRule::duration 判断为永不，结束与次数还是结束与日期
    void setRRuleType(const RRuleType &rtype);
    RRuleType getRRuleType();

    //重复日程相对于原始日程重复的次数，若为第一个日程则返回0
    static int numberOfRepetitions(const DSchedule::Ptr &scheudle, const QDateTime &datetime);

    static bool fromJsonString(DSchedule::Ptr &schedule, const QString &json);
    static bool toJsonString(const DSchedule::Ptr &schedule, QString &json);

    static bool fromIcsString(DSchedule::Ptr &schedule, const QString &string);
    static QString toIcsString(const DSchedule::Ptr &schedule);

    //
    static QMap<QDate, DSchedule::List> fromMapString(const QString &json);
    static QString toMapString(const QMap<QDate, DSchedule::List> &scheduleMap);

    static QPair<QString, DSchedule::List> fromListString(const QString &json);
    static QString toListString(const QString &query, const DSchedule::List &scheduleList);

    static void expendRecurrence(DSchedule::Map &scheduleMap, const DSchedule::Ptr &schedule, const QDateTime &dtStart, const QDateTime &dtEnd);
    static QMap<QDate, DSchedule::List> convertSchedules(const DScheduleQueryPar::Ptr &queryPar, const DSchedule::List &scheduleList);
    static QMap<QDate, DSchedule::List> fromQueryResult(const QString &query);

private:
    QMap<int, AlarmType> getAlarmMap();

public:
    //日程信息调试打印
    friend QDebug operator<<(QDebug debug, const DSchedule &scheduleJsonData);
    friend bool operator==(const DSchedule::Ptr &s1, const DSchedule::Ptr &s2);
    friend bool operator<(const DSchedule::Ptr &s1, const DSchedule::Ptr &s2);

    QString fileName() const;
    void setFileName(const QString &fileName);

    //为了与旧数据兼容处理（与联系人交互中使用的是自增ID作为日程的ID）
    int compatibleID() const;
    void setcompatibleID(int compatibleID);

private:
    QString m_fileName; //日程对应文件名称
    //日程类型
    QString m_scheduleTypeID;
    bool m_moved = false;
    int m_compatibleID; //为了与旧数据兼容处理（与联系人交互中使用的是自增ID作为日程的ID）
};

#endif // DSCHEDULE_H
