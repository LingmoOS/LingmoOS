// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULELISTWIDGET_H
#define SCHEDULELISTWIDGET_H

#include <DFrame>

#include "dschedule.h"
#include "icondframe.h"

DWIDGET_USE_NAMESPACE
class scheduleListWidget : public IconDFrame
{
    Q_OBJECT
public:
    explicit scheduleListWidget(QWidget *parent = nullptr);

    void setScheduleInfoVector(const DSchedule::List &ScheduleInfoVector);

    void updateUI();
signals:
    void signalSelectScheduleIndex(int index);

private:
    DSchedule::List m_ScheduleInfoVector;
};

#endif // SCHEDULELISTWIDGET_H
