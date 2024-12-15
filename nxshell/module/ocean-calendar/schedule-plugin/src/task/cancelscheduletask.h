// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CANCELSCHEDULETASK_H
#define CANCELSCHEDULETASK_H

#include "schedulebasetask.h"

class scheduleState;

class cancelScheduleTask : public scheduleBaseTask
{
    Q_OBJECT
public:
    cancelScheduleTask();
    Reply getFeedbackByQuerySchedule(const DSchedule::List &infoVector) override;
    Reply getReplyBySelectSchedule(const DSchedule::Ptr &info) override;
    Reply InitState(const JsonData *jsonData, bool isUpdateState = false) override;
    Reply repeatScheduleHandle(const DSchedule::Ptr &info, bool isOnlyOne) override;
    Reply confirwScheduleHandle(const DSchedule::Ptr &info) override;
    Reply confirmInfo(bool isOK) override;
public slots:
    void slotSelectScheduleIndex(int index);
    void slotButtonCheckNum(int index, const QString &text, const int buttonCount);

private:
    scheduleState *getCurrentState();
    QWidget *createRepeatWidget(const DSchedule::Ptr &info);
    QWidget *createConfirmWidget(const DSchedule::Ptr &info);

    Reply getListScheduleReply(const DSchedule::List &infoVector);

    Reply getConfirwScheduleReply(const DSchedule::Ptr &info);
    Reply getRepeatReply(const DSchedule::Ptr &info);

    void deleteRepeatSchedule(const DSchedule::Ptr &info, bool isOnlyOne = true);
    void deleteOrdinarySchedule(const DSchedule::Ptr &info);

private:
};

#endif // CANCELSCHEDULETASK_H
