// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULEBASETASK_H
#define SCHEDULEBASETASK_H

#include "semanticanalysistask.h"
#include "../interface/reply.h"
#include "dschedule.h"

#include <QObject>

class scheduleState;
class scheduleBaseTask : public QObject
{
    Q_OBJECT
public:
    scheduleBaseTask(scheduleState *state = nullptr);
    virtual ~scheduleBaseTask();
    virtual Reply SchedulePress(semanticAnalysisTask &semanticTask);
    virtual Reply getFeedbackByQuerySchedule(const DSchedule::List &info);
    virtual Reply getReplyBySelectSchedule(const DSchedule::Ptr &info);
    virtual Reply InitState(const JsonData *jsonData, bool isUpdateState = false);
    virtual Reply repeatScheduleHandle(const DSchedule::Ptr &info, bool isOnlyOne);
    virtual Reply confirwScheduleHandle(const DSchedule::Ptr &info);
    virtual Reply confirmInfo(bool isOK);
    scheduleState *getState() const;
    Reply overdueScheduleProcess();
signals:
    void signaleSendMessage(Reply reply);

protected:
    void updateState();
    /**
     * @brief errorMessage   用户输入无法匹配返回错误消息提示
     * @return 关于无法处理该关键字的答复
     */
    Reply errorMessage();

protected:
    scheduleState *m_State {nullptr};
};

#endif // SCHEDULEBASETASK_H
