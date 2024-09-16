// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "changescheduletask.h"
#include "../data/changejsondata.h"
#include "../globaldef.h"
#include "../widget/schedulelistwidget.h"
#include "../widget/repeatschedulewidget.h"
#include "queryscheduleproxy.h"
#include "../state/selectandquerystate.h"
#include "../state/confirwfeedbackstate.h"
#include "../state/queryschedulestate.h"
#include "../state/repeatfeedbackstate.h"
#include "../state/getchangedatastate.h"
#include "dscheduledatamanager.h"

changeScheduleTask::changeScheduleTask()
    : scheduleBaseTask(new queryScheduleState(this))
{
}

Reply changeScheduleTask::getFeedbackByQuerySchedule(const DSchedule::List &infoVector)
{
    Reply m_reply;
    scheduleState *nextState = nullptr;
    scheduleState *currentState = getCurrentState();
    if (infoVector.size() == 0) {
        QString m_TTSMessage;
        QString m_DisplyMessage;
        m_TTSMessage = NO_SCHEDULE_TTS;
        m_DisplyMessage = NO_SCHEDULE_TTS;
        REPLY_ONLY_TTS(m_reply, m_TTSMessage, m_DisplyMessage, true);
        currentState->setNextState(nextState);
    } else if (infoVector.size() == 1) {
        currentState->getLocalData()->setSelectInfo(infoVector.at(0));
        m_reply = getReplyBySelectSchedule(infoVector.at(0));
    } else {
        nextState = new SelectAndQueryState(this);
        CLocalData::Ptr m_Data = currentState->getLocalData();
        m_Data->setScheduleInfoVector(infoVector);
        nextState->setLocalData(m_Data);
        m_reply = getListScheduleReply(infoVector);
        currentState->setNextState(nextState);
    }
    return m_reply;
}

void changeScheduleTask::slotSelectScheduleIndex(int index)
{
    scheduleState *currentState = getCurrentState();
    CLocalData::Ptr localData = currentState->getLocalData();
    if (!(localData->scheduleInfoVector().size() < index)) {
        localData->setSelectInfo(localData->scheduleInfoVector().at(index - 1));
        Reply reply = getReplyBySelectSchedule(localData->scheduleInfoVector().at(index - 1));
        updateState();
        emit signaleSendMessage(reply);
    }
}

void changeScheduleTask::slotButtonCheckNum(int index, const QString &text, const int buttonCount)
{
    Q_UNUSED(text);
    Reply reply;
    scheduleState *currentState = getCurrentState();
    if (buttonCount == 2) {
        if (index == 1) {
            reply = confirwScheduleHandle(currentState->getLocalData()->SelectInfo());
        }
    }
    if (buttonCount == 3) {
        if (index == 1) {
            reply = repeatScheduleHandle(currentState->getLocalData()->SelectInfo(), false);
        }
        if (index == 2) {
            reply = repeatScheduleHandle(currentState->getLocalData()->SelectInfo(), true);
        }
    }
    if (index == 0) {
        reply = InitState(nullptr, true);
    } else {
        InitState(nullptr, true);
    }
    emit signaleSendMessage(reply);
}

scheduleState *changeScheduleTask::getCurrentState()
{
    scheduleState *currentState = m_State;
    while (currentState->getNextState() != nullptr) {
        currentState = currentState->getNextState();
    }
    return currentState;
}

Reply changeScheduleTask::getReplyBySelectSchedule(const DSchedule::Ptr &info)
{
    Reply m_reply;
    scheduleState *nextState = nullptr;
    scheduleState *currentState = getCurrentState();
    CLocalData::Ptr m_Data = currentState->getLocalData();
    m_Data->setSelectInfo(info);
    if (m_Data->getOffet() < 0) {
        m_Data->setOffset(1);
    }
    if (m_Data->getToTime().suggestDatetime.size() == 0 && m_Data->getToTitleName() == "") {
        QWidget *infoWidget = createInquiryWidget(info);
        REPLY_WIDGET_TTS(m_reply, infoWidget, CHANGE_TO_TTS, CHANGE_TO_TTS, false);
        //添加获取修改信息状态
        nextState = new getChangeDataState(this);
        nextState->setLocalData(m_Data);
    } else {
        //获取下一个状态
        nextState = getNextStateBySelectScheduleInfo(info, m_Data, m_reply);
    }
    currentState->setNextState(nextState);
    return m_reply;
}

Reply changeScheduleTask::InitState(const JsonData *jsonData, bool isUpdateState)
{
    Reply m_reply;
    scheduleState *nextState = new queryScheduleState(this);
    scheduleState *currentState = getCurrentState();
    currentState->setNextState(nextState);
    if (jsonData != nullptr) {
        if (currentState->getLocalData() != nullptr) {
            currentState->setLocalData(nullptr);
        }
        m_reply = nextState->process(jsonData);
    } else {
        REPLY_ONLY_TTS(m_reply, CANCEL_CHANGE_TTS, CANCEL_CHANGE_TTS, true);
    }
    if (isUpdateState) {
        updateState();
    }
    return m_reply;
}

Reply changeScheduleTask::repeatScheduleHandle(const DSchedule::Ptr &info, bool isOnlyOne)
{
    changeRepeatSchedule(info, isOnlyOne);
    Reply reply;
    REPLY_ONLY_TTS(reply, CONFIRM_CHANGE_TTS, CONFIRM_CHANGE_TTS, true);
    scheduleState *nextState = new queryScheduleState(this);
    scheduleState *currentState = getCurrentState();
    currentState->setNextState(nextState);
    return reply;
}

Reply changeScheduleTask::confirwScheduleHandle(const DSchedule::Ptr &info)
{
    Q_UNUSED(info);
    scheduleState *currentState = getCurrentState();
    changeOrdinarySchedule(currentState->getLocalData()->getNewInfo());
    Reply reply;
    REPLY_ONLY_TTS(reply, CONFIRM_CHANGE_TTS, CONFIRM_CHANGE_TTS, true);
    scheduleState *nextState = new queryScheduleState(this);
    currentState->setNextState(nextState);
    return reply;
}

Reply changeScheduleTask::confirmInfo(bool isOK)
{
    scheduleState *currentState = getCurrentState();
    if (isOK) {
        Q_UNUSED(isOK)
        changeOrdinarySchedule(currentState->getLocalData()->getNewInfo());
        Reply reply;
        REPLY_ONLY_TTS(reply, CONFIRM_CHANGE_TTS, CONFIRM_CHANGE_TTS, true)
        return reply;
    } else {
        return InitState(nullptr);
    }
}

QWidget *changeScheduleTask::createRepeatWidget(const DSchedule::Ptr &info)
{
    repeatScheduleWidget *repeatWidget = new repeatScheduleWidget(repeatScheduleWidget::Operation_Change, repeatScheduleWidget::Widget_Repeat);
    repeatWidget->setSchedule(info);
    connect(repeatWidget, &repeatScheduleWidget::signalButtonCheckNum, this, &changeScheduleTask::slotButtonCheckNum);
    return repeatWidget;
}

QWidget *changeScheduleTask::createConfirmWidget(const DSchedule::Ptr &info)
{
    repeatScheduleWidget *cwidget = new repeatScheduleWidget(repeatScheduleWidget::Operation_Change, repeatScheduleWidget::Widget_Confirm);
    cwidget->setSchedule(info);
    connect(cwidget, &repeatScheduleWidget::signalButtonCheckNum, this, &changeScheduleTask::slotButtonCheckNum);
    return cwidget;
}

QWidget *changeScheduleTask::createInquiryWidget(const DSchedule::Ptr &info)
{
    repeatScheduleWidget *infoWidget =
        new repeatScheduleWidget(repeatScheduleWidget::Operation_Change, repeatScheduleWidget::Widget_Confirm, false);
    infoWidget->setSchedule(info);
    return infoWidget;
}

Reply changeScheduleTask::getListScheduleReply(const DSchedule::List &infoVector)
{
    scheduleListWidget *m_viewWidget = new scheduleListWidget();
    connect(m_viewWidget, &scheduleListWidget::signalSelectScheduleIndex, this, &changeScheduleTask::slotSelectScheduleIndex);
    m_viewWidget->setScheduleInfoVector(infoVector);
    QString m_TTSMessage;
    QString m_DisplyMessage;
    m_TTSMessage = SELECT_CHANGE_TTS;
    m_DisplyMessage = SELECT_CHANGE_TTS;
    Reply reply;
    REPLY_WIDGET_TTS(reply, m_viewWidget, m_TTSMessage, m_DisplyMessage, false);
    return reply;
}

scheduleState *changeScheduleTask::getNextStateBySelectScheduleInfo(const DSchedule::Ptr &info, const CLocalData::Ptr &localData, Reply &reply)
{
    QString m_TTSMessage;
    QString m_DisplyMessage;
    //获取当前状态
    scheduleState *currentState = getCurrentState();
    //下一个状态
    scheduleState *nextState{nullptr};
    //如果修改的新日程时间在范围内则正常提醒
    if (getNewInfo()) {
        //需要显示的窗口
        QWidget *_showWidget;
        if (info->getRRuleType() == DSchedule::RRule_None) {
            m_TTSMessage = CONFIRM_SCHEDULE_CHANGE_TTS;
            m_DisplyMessage = CONFIRM_SCHEDULE_CHANGE_TTS;
            _showWidget = createConfirmWidget(currentState->getLocalData()->getNewInfo());
            //设置下一个状态为普通日程确认状态
            nextState = new confirwFeedbackState(this);
        } else {
            m_TTSMessage = REPEST_SCHEDULE_CHANGE_TTS;
            m_DisplyMessage = REPEST_SCHEDULE_CHANGE_TTS;
            _showWidget = createRepeatWidget(currentState->getLocalData()->getNewInfo());
            //设置下一个状态为重复日程确认状态
            nextState = new repeatfeedbackstate(this);
        }
        //设置修改的日程信息
        localData->setNewInfo(currentState->getLocalData()->getNewInfo());
        //设置存储数据
        nextState->setLocalData(localData);
        REPLY_WIDGET_TTS(reply, _showWidget, m_TTSMessage, m_DisplyMessage, false);
    } else {
        //如果修改的日程不在正常范围内则回复错误，并设置下一个状态为询问查询状态
        m_TTSMessage = CHANGE_TIME_OUT_TTS;
        m_DisplyMessage = CHANGE_TIME_OUT_TTS;
        REPLY_ONLY_TTS(reply, m_TTSMessage, m_DisplyMessage, true);
        nextState = new queryScheduleState(this);
    };
    return nextState;
}

bool changeScheduleTask::getNewInfo()
{
    scheduleState *currentState = getCurrentState();
    DSchedule::Ptr m_NewInfo = currentState->getLocalData()->SelectInfo();
    if (!currentState->getLocalData()->getToTitleName().isEmpty())
        m_NewInfo->setSummary(currentState->getLocalData()->getToTitleName());
    QVector<DateTimeInfo> m_ToTime = currentState->getLocalData()->getToTime().dateTime;
    //获取建议时间
    QVector<SuggestDatetimeInfo> m_suggestDatetime = currentState->getLocalData()->getToTime().suggestDatetime;
    if (m_ToTime.size() > 0) {
        if (m_ToTime.size() == 1) {
            //如果存在日期信息
            if (m_ToTime.at(0).hasDate) {
                //获取日程的开始结束时间差
                qint64 interval = m_NewInfo->dtStart().secsTo(m_NewInfo->dtEnd());
                //设置修改的开始日期
                m_NewInfo->setDtStart(QDateTime(m_ToTime.at(0).m_Date, m_NewInfo->dtStart().time()));
                //设置修改的结束日期
                m_NewInfo->setDtEnd(m_NewInfo->dtStart().addSecs(interval));
            }
            //如果修改的DateTime带时间则设置该时间，否则保持原来的时间点
            if (m_ToTime.at(0).hasTime) {
                //如果修改的日期为当天则取suggestTime时间
                if (m_NewInfo->dtStart().date() == QDate::currentDate()) {
                    m_NewInfo->setDtStart(m_suggestDatetime.at(0).datetime);
                } else {
                    m_NewInfo->setDtStart(QDateTime(m_NewInfo->dtStart().date(), m_ToTime.at(0).m_Time));
                }
                m_NewInfo->setDtEnd(m_NewInfo->dtStart().addSecs(3600));
                //如果存在时间点则将全天的日程修改为非全天并修改提醒规则
                if (m_NewInfo->allDay()) {
                    m_NewInfo->setAllDay(false);
                    //非全天设置为立即提醒
                    m_NewInfo->setAlarmType(DSchedule::Alarm_Begin);
                }
            }
        }
        if (m_ToTime.size() == 2) {
            //如果存在日期信息
            if (m_ToTime.at(0).hasDate) {
                //设置修改的开始日期
                m_NewInfo->setDtStart(QDateTime(m_ToTime.at(0).m_Date));
            }
            //如果修改的DateTime带时间则设置该时间，否则保持原来的时间点
            if (m_ToTime.at(0).hasTime) {
                m_NewInfo->setDtStart(QDateTime(m_NewInfo->dtStart().date(), m_ToTime.at(0).m_Time));
            }
            //如果存在日期信息
            if (m_ToTime.at(1).hasDate) {
                //设置修改的结束日期
                m_NewInfo->setDtEnd(QDateTime(m_ToTime.at(1).m_Date));
            }
            //如果修改的DateTime带时间则设置该时间，否则保持原来的时间点
            if (m_ToTime.at(1).hasTime)
                m_NewInfo->setDtEnd(QDateTime(m_NewInfo->dtEnd().date(), m_ToTime.at(1).m_Time));
            //如果开始时间大于结束时间则设置结束时间为开始时间往后一小时
            if (m_NewInfo->dtEnd() < m_NewInfo->dtStart()) {
                m_NewInfo->setDtEnd(m_NewInfo->dtStart().addSecs(3600));
            }
            //TODO 对于多个时间点还未支持,全天非全天的修改待做
        }
    }
    currentState->getLocalData()->setNewInfo(m_NewInfo);
    return changeDateTimeIsInNormalRange(m_NewInfo);
}

void changeScheduleTask::changeRepeatSchedule(const DSchedule::Ptr &info, bool isOnlyOne)
{
    if (isOnlyOne) {
        changeOnlyInfo(info);
    } else {
        changeAllInfo(info);
    }
}

void changeScheduleTask::changeOnlyInfo(const DSchedule::Ptr &info)
{
    Q_UNUSED(info)
    scheduleState *currentState = getCurrentState();
    DSchedule::Ptr newschedule = currentState->getLocalData()->getNewInfo();
    //原始信息
    DSchedule::Ptr updatescheduleData = DScheduleDataManager::getInstance()->queryScheduleByScheduleID(newschedule->uid());
    updatescheduleData->recurrence()->addExDateTime(newschedule->dtStart());
    newschedule->setRRuleType(DSchedule::RRule_None);
    newschedule->setUid(DScheduleDataManager::getInstance()->createSchedule(newschedule));

    DScheduleDataManager::getInstance()->updateSchedule(updatescheduleData);
}

void changeScheduleTask::changeAllInfo(const DSchedule::Ptr &info)
{
    scheduleState *currentState = getCurrentState();
    DSchedule::Ptr newinfo = currentState->getLocalData()->getNewInfo();
    if (info->getRRuleType() == DSchedule::RRule_None) {
        DSchedule::Ptr schedule = newinfo;
        DScheduleDataManager::getInstance()->updateSchedule(schedule);
    } else {
        //获取原始日程信息
        DSchedule::Ptr updatescheduleData = DScheduleDataManager::getInstance()->queryScheduleByScheduleID(info->uid());
        //第几次重复日程
        int repetNum = DSchedule::numberOfRepetitions(updatescheduleData, newinfo->dtStart());
        if (repetNum == 1) {
            //如果为第一个
            DScheduleDataManager::getInstance()->updateSchedule(newinfo);

        } else {
            //如果不是第一个
            if (newinfo->recurrence()->duration() > 1) {
                int duration = newinfo->recurrence()->duration() - repetNum + 1;
                //结束于次数
                if (duration < 2) {
                    newinfo->setRRuleType(DSchedule::RRule_None);
                } else {
                    newinfo->recurrence()->setDuration(duration);
                }
                //修改原始日程，如果原始日程剩余的重复次数为1，则修改为普通日程
                updatescheduleData->recurrence()->setDuration(repetNum - 1);
                if (updatescheduleData->recurrence()->duration() == 1) {
                    updatescheduleData->setRRuleType(DSchedule::RRule_None);
                }
            } else if (newinfo->recurrence()->duration() == 0) {
                //结束于时间
                if (newinfo->dtStart().date() == newinfo->recurrence()->endDateTime().date()) {
                    newinfo->setRRuleType(DSchedule::RRule_None);
                }
                updatescheduleData->recurrence()->setEndDate(newinfo->dtStart().date().addDays(-1));
                if (updatescheduleData->dtStart().date() == updatescheduleData->recurrence()->endDate()) {
                    updatescheduleData->setRRuleType(DSchedule::RRule_None);
                }
            } else {
                //永不
                updatescheduleData->recurrence()->setEndDate(newinfo->dtStart().date().addDays(-1));
                if (updatescheduleData->dtStart().date() == updatescheduleData->recurrence()->endDate()) {
                    updatescheduleData->setRRuleType(DSchedule::RRule_None);
                }
            }
            DScheduleDataManager::getInstance()->createSchedule(newinfo);
            //修改原始日程
            DScheduleDataManager::getInstance()->updateSchedule(updatescheduleData);
        }
    }
}

void changeScheduleTask::changeOrdinarySchedule(const DSchedule::Ptr &info)
{
    DScheduleDataManager::getInstance()->updateSchedule(info);
}

bool changeScheduleTask::changeDateTimeIsInNormalRange(const DSchedule::Ptr &info)
{
    bool result {true};
    //当前时间
    QDateTime currentDateTime {QDateTime::currentDateTime()};
    //最大时间
    QDateTime maxDateTime = currentDateTime.addMonths(6);
    //如果开始时间为过期时间则为false
    if (info->dtStart() < currentDateTime) {
        result = false;
    };
    //如果开始时间或结束时间大于最大时间则为false
    if (info->dtStart() > maxDateTime
        || info->dtEnd() > maxDateTime) {
        result = false;
    }
    return  result;
}
