// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VIEWSCHEDULEWIDGET_H
#define VIEWSCHEDULEWIDGET_H

#include "icondframe.h"
#include "dschedule.h"
#include "scheduleitemwidget.h"

#include <QWidget>

class viewschedulewidget : public IconDFrame
{
    Q_OBJECT
public:
    explicit viewschedulewidget(QWidget *parent = nullptr);
    ~viewschedulewidget();
    void setScheduleDateRangeInfo(const DSchedule::Map &scheduleDateInfo);
    void viewScheduleInfoShow(const DSchedule::Map &showSchedule);
    int getScheduleNum(DSchedule::Map scheduleList);
    void setQueryBeginDateTime(QDateTime begindatetime);
    void setQueryEndDateTime(QDateTime enddatetime);
    /**
     * @brief getNextScheduleInfo 获取下一个日程
     * @return 下一个日程信息
     */
    DSchedule::Map getNextScheduleInfo();
public slots:
    void slotItemPress(const DSchedule::Ptr &info);

private:
    DSchedule::List m_scheduleInfo;
    DSchedule::Map m_scheduleDateInfo;
    DSchedule::Map m_showdate;
    QDateTime m_beginDateTime;
    QDateTime m_endDateTime;
};

#endif // QUERYSCHEDULEWIDGET_H
