// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUERYSCHEDULETASK_H
#define QUERYSCHEDULETASK_H

#include "schedulebasetask.h"
#include "../data/queryjsondata.h"
#include "../widget/viewschedulewidget.h"

class queryScheduleTask : public scheduleBaseTask
{
    Q_OBJECT
public:
    queryScheduleTask();
    Reply SchedulePress(semanticAnalysisTask &semanticTask) override;

private:
    /**
     * @brief setDateTime 设置查询的开始结束时间
     * @param queryJsonData jsondata句柄
     */
    void setDateTime(QueryJsonData *queryJsonData);

    /**
     * @brief queryOverDueDate 是否查询的是过去的日程
     * @param queryJsonData jsondata句柄
     * @return bool
     */
    bool queryOverDueDate(QueryJsonData *queryJsonData);

private:
    QDateTime       m_BeginDateTime;
    QDateTime       m_EndDateTime;
    viewschedulewidget      *viewWidget = nullptr;

protected:
};

#endif // QUERYSCHEDULETASK_H
