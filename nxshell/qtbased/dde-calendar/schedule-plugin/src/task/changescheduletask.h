// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CHANGESCHEDULETASK_H
#define CHANGESCHEDULETASK_H

#include "schedulebasetask.h"
#include "dschedule.h"
#include "clocaldata.h"

class changeScheduleTask : public scheduleBaseTask
{
    Q_OBJECT
public:
    explicit changeScheduleTask();
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
    QWidget *createInquiryWidget(const DSchedule::Ptr &info);

    Reply getListScheduleReply(const DSchedule::List &infoVector);
    /**
     * @brief getNextStateBySelectScheduleInfo      根据选择的日程获取下一个修改状态
     * @param info                                  选择的日程信息
     * @param localData                             当前状态的存储数据
     * @param reply                                 修改的回复
     * @return                                      下一个状态
     */
    scheduleState *getNextStateBySelectScheduleInfo(const DSchedule::Ptr &info, const CLocalData::Ptr &localData, Reply &reply);
    /**
     * @brief getNewInfo        根据修改信息获取新的日程信息
     * @return                  在时间范围内返回true
     */
    bool getNewInfo();

    void changeRepeatSchedule(const DSchedule::Ptr &info, bool isOnlyOne);
    void changeOnlyInfo(const DSchedule::Ptr &info);
    void changeAllInfo(const DSchedule::Ptr &info);
    void changeOrdinarySchedule(const DSchedule::Ptr &info);
    /**
     * @brief changeDateTimeIsInRange   判断修改的日期在正确的时间范围内
     * @param info          修改过的日程
     * @return              在正常范围内则返回true
     */
    bool changeDateTimeIsInNormalRange(const DSchedule::Ptr &info);

private:
    DSchedule::List m_scheduleInfo;
};

#endif // CHANGESCHEDULETASK_H
