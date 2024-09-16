// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CSCHEDULEMANAGE_H
#define CSCHEDULEMANAGE_H

#include "lunarmanager.h"
#include <com_deepin_daemon_timedate.h>

#include <QObject>

struct ShowDateRange {
    int showYear{0};
    QDate startDate;
    QDate stopDate;
};

using DaemonTimeDate = com::deepin::daemon::Timedate ;

class CScheduleBaseWidget;
/**
 * @brief The CalendarManage class
 *  日历数据管理类
 */
class CalendarManager : public QObject
{
public:
    enum WeekDayFormat {        // 周显示格式
        WeekDayFormat_1 = 0,    // 类似星期一、星期二这种显示方式
        WeekDayFormat_2         // 类似周一、周二
    };
    enum DateFormat {           // 日期显示格式，如yyyy/M/d、yyyy-M-d
        DateFormat_1 = 0,       // yyyy/M/d
        DateFormat_2,
        DateFormat_3,
        DateFormat_4,
        DateFormat_5,
        DateFormat_6,
        DateFormat_7,
        DateFormat_8,
        DateFormat_9,
        DateFormat_10,
        DateFormat_11
    };

    Q_OBJECT
public:
    static CalendarManager *instance();

    //周显示格式
    WeekDayFormat weekDayFormat() const;
    void setWeekDayFormat(WeekDayFormat format);
    //当前周第一天的日期
    QDate firstDayOfWeek(const QDate &date);
    void setFirstDayOfWeek(int, bool update = true);
    //一周首日
    Qt::DayOfWeek firstDayOfWeek();

    DateFormat dateFormat() const;
    void setDateFormat(DateFormat format);

signals:
    void dateFormatChanged(DateFormat format);
    void sidebarFirstDayChanged(int value);
    void weekDayFormatChanged(WeekDayFormat value);

private slots:
    void onWeekBeginsChanged(int value);
    void onDateFormatChanged(int value);
    void onWeekDayFormatChanged(int value);

private:
    explicit CalendarManager(QObject *parent = nullptr);
    ~CalendarManager();

private:
    //初始化数据
    void initData();
    //初始化关联
    void initConnection();

private:
    static CalendarManager      *m_scheduleManager;
    DaemonTimeDate              *m_timeDateDbus;                     //控制中心dbus
    QList<CScheduleBaseWidget*> m_showWidget;                        //日历主视图
    int                         m_firstDayOfWeek;                    //一周首日
    QDateTime                   m_currentDateTime;                   //当前时间
    QDate                       m_selectDate;                        //当前选择的时间
    ShowDateRange               m_showDateRange;                     //时间范围
    WeekDayFormat               m_weekDayFormat;                     //周显示格式， ddd: 周一， dddd: 星期一
    DateFormat                  m_dateFormat;                        //日期显示格式
};
#define gCalendarManager CalendarManager::instance()
#endif // CSCHEDULEMANAGE_H
