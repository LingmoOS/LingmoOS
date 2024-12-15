// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cancelscheduletask.h"
#include "../data/canceljsondata.h"
#include "../globaldef.h"
#include "../widget/schedulelistwidget.h"
#include "../widget/repeatschedulewidget.h"
#include "queryscheduleproxy.h"
#include "../state/queryschedulestate.h"
#include "../state/selectinquirystate.h"
#include "../state/repeatfeedbackstate.h"
#include "../data/clocaldata.h"
#include "../state/confirwfeedbackstate.h"
#include "dscheduledatamanager.h"

cancelScheduleTask::cancelScheduleTask()
    : scheduleBaseTask(new queryScheduleState(this))
{
}

void cancelScheduleTask::slotSelectScheduleIndex(int index)
{
    scheduleState *currentState = getCurrentState();
    CLocalData::Ptr localData = currentState->getLocalData();
    if (!(localData->scheduleInfoVector().size() < index)) {
        Reply reply = getReplyBySelectSchedule(localData->scheduleInfoVector().at(index - 1));
        updateState();
        emit signaleSendMessage(reply);
    }
}

void cancelScheduleTask::slotButtonCheckNum(int index, const QString &text, const int buttonCount)
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

scheduleState *cancelScheduleTask::getCurrentState()
{
    scheduleState *currentState = m_State;
    while (currentState->getNextState() != nullptr) {
        currentState = currentState->getNextState();
    }
    return currentState;
}

Reply cancelScheduleTask::getFeedbackByQuerySchedule(const DSchedule::List &infoVector)
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
        m_reply = getReplyBySelectSchedule(infoVector.at(0));
    } else {
        nextState = new selectInquiryState(this);
        CLocalData::Ptr m_Data(new CLocalData());
        m_Data->setScheduleInfoVector(infoVector);
        nextState->setLocalData(m_Data);
        m_reply = getListScheduleReply(infoVector);
        currentState->setNextState(nextState);
    }
    return m_reply;
}

Reply cancelScheduleTask::getReplyBySelectSchedule(const DSchedule::Ptr &info)
{
    Reply m_reply;
    CLocalData::Ptr m_Data(new CLocalData());
    scheduleState *nextState = nullptr;
    scheduleState *currentState = getCurrentState();
    m_Data->setSelectInfo(info);
    if (info->getRRuleType() == DSchedule::RRuleType::RRule_None) {
        nextState = new confirwFeedbackState(this);
        m_reply = getConfirwScheduleReply(info);
    } else {
        nextState = new repeatfeedbackstate(this);
        m_reply = getRepeatReply(info);
    }
    nextState->setLocalData(m_Data);
    currentState->setNextState(nextState);
    return m_reply;
}

Reply cancelScheduleTask::InitState(const JsonData *jsonData, bool isUpdateState)
{
    Reply m_reply;
    scheduleState *nextState = new queryScheduleState(this);
    scheduleState *currentState = getCurrentState();
    currentState->setNextState(nextState);
    if (jsonData != nullptr) {
        m_reply = nextState->process(jsonData);
    } else {
        REPLY_ONLY_TTS(m_reply, CANCEL_DELETION_TTS, CANCEL_DELETION_TTS, true);
    }
    if (isUpdateState) {
        updateState();
    }
    return m_reply;
}

Reply cancelScheduleTask::repeatScheduleHandle(const DSchedule::Ptr &info, bool isOnlyOne)
{
    deleteRepeatSchedule(info, isOnlyOne);
    Reply reply;
    REPLY_ONLY_TTS(reply, CONFIRM_DELETION_TTS, CONFIRM_DELETION_TTS, true);
    scheduleState *nextState = new queryScheduleState(this);
    scheduleState *currentState = getCurrentState();
    currentState->setNextState(nextState);
    return reply;
}

Reply cancelScheduleTask::confirwScheduleHandle(const DSchedule::Ptr &info)
{
    deleteOrdinarySchedule(info);
    Reply reply;
    REPLY_ONLY_TTS(reply, CONFIRM_DELETION_TTS, CONFIRM_DELETION_TTS, true);
    scheduleState *nextState = new queryScheduleState(this);
    scheduleState *currentState = getCurrentState();
    currentState->setNextState(nextState);
    return reply;
}

Reply cancelScheduleTask::confirmInfo(bool isOK)
{
    if (isOK) {
        scheduleState *currentState = getCurrentState();
        return confirwScheduleHandle(currentState->getLocalData()->SelectInfo());
    } else {
        return InitState(nullptr);
    }
}

QWidget *cancelScheduleTask::createRepeatWidget(const DSchedule::Ptr &info)
{
    repeatScheduleWidget *repeatWidget = new repeatScheduleWidget(repeatScheduleWidget::Operation_Cancel, repeatScheduleWidget::Widget_Repeat);
    repeatWidget->setSchedule(info);
    connect(repeatWidget, &repeatScheduleWidget::signalButtonCheckNum, this, &cancelScheduleTask::slotButtonCheckNum);
    return repeatWidget;
}

QWidget *cancelScheduleTask::createConfirmWidget(const DSchedule::Ptr &info)
{
    repeatScheduleWidget *cwidget = new repeatScheduleWidget(repeatScheduleWidget::Operation_Cancel, repeatScheduleWidget::Widget_Confirm);
    cwidget->setSchedule(info);
    connect(cwidget, &repeatScheduleWidget::signalButtonCheckNum, this, &cancelScheduleTask::slotButtonCheckNum);
    return cwidget;
}

Reply cancelScheduleTask::getListScheduleReply(const DSchedule::List &infoVector)
{
    scheduleListWidget *m_viewWidget = new scheduleListWidget();
    m_viewWidget->setScheduleInfoVector(infoVector);
    connect(m_viewWidget, &scheduleListWidget::signalSelectScheduleIndex, this, &cancelScheduleTask::slotSelectScheduleIndex);
    QString m_TTSMessage;
    QString m_DisplyMessage;
    m_TTSMessage = CANCEL_SELECT_TTS;
    m_DisplyMessage = CANCEL_SELECT_TTS;
    Reply reply;
    REPLY_WIDGET_TTS(reply, m_viewWidget, m_TTSMessage, m_DisplyMessage, false);
    return reply;
}

Reply cancelScheduleTask::getConfirwScheduleReply(const DSchedule::Ptr &info)
{
    QString m_TTSMessage;
    QString m_DisplyMessage;
    m_TTSMessage = CONFIRM_SCHEDULE_CANCEL_TTS;
    m_DisplyMessage = CONFIRM_SCHEDULE_CANCEL_TTS;
    QWidget *m_confirwWidget = createConfirmWidget(info);
    Reply reply;
    REPLY_WIDGET_TTS(reply, m_confirwWidget, m_TTSMessage, m_DisplyMessage, false);
    return reply;
}

Reply cancelScheduleTask::getRepeatReply(const DSchedule::Ptr &info)
{
    QString m_TTSMessage;
    QString m_DisplyMessage;
    m_TTSMessage = REPEST_SCHEDULE_CANCEL_TTS;
    m_DisplyMessage = REPEST_SCHEDULE_CANCEL_TTS;
    QWidget *m_repeatReply = createRepeatWidget(info);
    Reply reply;
    REPLY_WIDGET_TTS(reply, m_repeatReply, m_TTSMessage, m_DisplyMessage, false);
    return reply;
}

void cancelScheduleTask::deleteRepeatSchedule(const DSchedule::Ptr &info, bool isOnlyOne)
{
    if (isOnlyOne) {
        DSchedule::Ptr newschedule = DScheduleDataManager::getInstance()->queryScheduleByScheduleID(info->uid());
        newschedule->recurrence()->addExDateTime(info->dtStart());
        DScheduleDataManager::getInstance()->updateSchedule(newschedule);
    } else {
        if (info->recurrenceId().isValid() == 0) {
            DScheduleDataManager::getInstance()->deleteScheduleByScheduleID(info->uid());
        } else {
            DSchedule::Ptr newschedule = DScheduleDataManager::getInstance()->queryScheduleByScheduleID(info->uid());
            newschedule->recurrence()->setDuration(0);
            newschedule->recurrence()->setEndDateTime(info->dtStart().addDays(-1));
            DScheduleDataManager::getInstance()->updateSchedule(newschedule);
        }
    }
}

void cancelScheduleTask::deleteOrdinarySchedule(const DSchedule::Ptr &info)
{
    DScheduleDataManager::getInstance()->deleteScheduleByScheduleID(info->uid());
}
