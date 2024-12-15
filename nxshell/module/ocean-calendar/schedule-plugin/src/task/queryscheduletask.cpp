// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "queryscheduletask.h"
#include "../globaldef.h"
#include "dscheduledatamanager.h"
#include "queryscheduleproxy.h"

queryScheduleTask::queryScheduleTask()
    : scheduleBaseTask()
{
}

Reply queryScheduleTask::SchedulePress(semanticAnalysisTask &semanticTask)
{
    QueryJsonData *queryJsonData = dynamic_cast<QueryJsonData *>(semanticTask.getJsonData());
    //如果转换失败则返回错误消息
    if (queryJsonData == nullptr)
        return errorMessage();
    //如果时间无效
    if (queryJsonData->getDateTimeInvalid()) {
        Reply m_reply;
        REPLY_ONLY_TTS(m_reply, DATETIME_ERR_TTS, DATETIME_ERR_TTS, true);
        return m_reply;
    }
    //查询日程
    if (queryJsonData->offset() > -1
            && queryJsonData->getPropertyStatus() == JsonData::PRO_NONE) {
        Reply m_reply;
        REPLY_ONLY_TTS(m_reply, CANCEL_ERR_TTS, CANCEL_ERR_TTS, true);
        return m_reply;
    }
    viewWidget = new viewschedulewidget();

    queryScheduleProxy m_querySchedule(queryJsonData);
    DSchedule::Map showdate = m_querySchedule.querySchedule();

    setDateTime(queryJsonData);

    viewWidget->viewScheduleInfoShow(showdate);

    Reply m_reply;

    if (queryJsonData->ShouldEndSession()) {
        //不进行多轮
        if (queryOverDueDate(queryJsonData)) {
            //过期时间
            REPLY_ONLY_TTS(m_reply, VIEW_DATE_IS_OVERDUE_TTS, VIEW_DATE_IS_OVERDUE_TTS, true);
        } else if (queryJsonData->getDateTime().suggestDatetime.size() > 0
                   && queryJsonData->getDateTime().suggestDatetime.at(0).datetime > QDateTime::currentDateTime().addMonths(6)) {
            //超过半年的时间
            REPLY_ONLY_TTS(m_reply, VIEW_DATETIME_OUT_TTS, VIEW_DATETIME_OUT_TTS, true);
        } else {
            if (viewWidget->getScheduleNum(showdate) == 0) {
                //没有查询的日程
                REPLY_ONLY_TTS(m_reply, NO_SCHEDULE_VIEWED_TTS, NO_SCHEDULE_VIEWED_TTS, true);
            } else {
                //查询到日程
                QString str = QString(VIEW_SCHEDULE_TTS).arg(viewWidget->getScheduleNum(showdate));
                REPLY_WIDGET_TTS(m_reply, viewWidget, str, str, true);
            }
        }
    } else {
        //多轮的情况
        REPLY_ONLY_TTS(m_reply, queryJsonData->SuggestMsg(), queryJsonData->SuggestMsg(), false);
    }

    return m_reply;
}

void queryScheduleTask::setDateTime(QueryJsonData *queryJsonData)
{
    switch (queryJsonData->getDateTime().suggestDatetime.size()) {
    case 1: {
        m_BeginDateTime = queryJsonData->getDateTime().suggestDatetime.at(0).datetime;
        m_EndDateTime = m_BeginDateTime;
        //时间处理
        if (!queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
            if (m_BeginDateTime.date() == QDate::currentDate()) {
                m_BeginDateTime.setTime(QTime::currentTime());
            } else {
                m_BeginDateTime.setTime(QTime(0, 0, 0));
            }
            m_EndDateTime.setTime(QTime(23, 59, 59));
        }
    }
    break;
    case 2: {
        m_BeginDateTime = queryJsonData->getDateTime().suggestDatetime.at(0).datetime;
        m_EndDateTime = queryJsonData->getDateTime().suggestDatetime.at(1).datetime;
        //查询时间为过期时间或者是超过半年的时间，则返回一个无效的时间
        if (queryJsonData->getDateTime().suggestDatetime.at(1).datetime.date() < QDateTime::currentDateTime().date()
            || queryJsonData->getDateTime().suggestDatetime.at(0).datetime.date() > QDate::currentDate().addMonths(6)) {
            //如果查询结束时间小于当前时间，设置开始结束时间为无效时间
            m_BeginDateTime.setDate(QDate(0, 0, 0));
            m_BeginDateTime.setTime(QTime(0, 0, 0));
            m_EndDateTime.setDate(QDate(0, 0, 0));
            m_EndDateTime.setTime(QTime(0, 0, 0));
            break;
        }
        //对查询的开始时间进行处理
        if (queryJsonData->getDateTime().suggestDatetime.at(0).datetime < QDateTime::currentDateTime()) {
            //开始时间小于当前时间，设置当前时间
            m_BeginDateTime = QDateTime::currentDateTime();
        } else {
            if (!queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
                //没有time
                if (queryJsonData->getDateTime().suggestDatetime.at(0).datetime.date() == QDate::currentDate())
                    //如果是今天，设置当前时间
                    m_BeginDateTime.setTime(QTime::currentTime());
                else
                    //不是今天，设置一天最初的时间
                    m_BeginDateTime.setTime(QTime(0, 0, 0));
            }
        }
        //对查询的结束时间进行处理
        if (queryJsonData->getDateTime().suggestDatetime.at(1).datetime.date() > QDate::currentDate().addMonths(6)) {
            //如果查询的结束时间超过了半年，则设置为半年以后的时间
            m_EndDateTime.setDate(QDate::currentDate().addMonths(6));
            m_EndDateTime.setTime(QTime(23, 59, 59));
        } else {
            //如果查询的结束时间没有超过半年，并且没有具体时间，则设置为一天最晚的时间
            if (!queryJsonData->getDateTime().suggestDatetime.at(1).hasTime) {
                m_EndDateTime.setTime(QTime(23, 59, 59));
            }
        }
    }
    break;
    default: {
        //如果没有时间，设置开始结束时间为无效时间
        m_BeginDateTime.setDate(QDate(0, 0, 0));
        m_BeginDateTime.setTime(QTime(0, 0, 0));
        m_EndDateTime.setDate(QDate(0, 0, 0));
        m_EndDateTime.setTime(QTime(0, 0, 0));
    }
    break;
    }
}

bool queryScheduleTask::queryOverDueDate(QueryJsonData *queryJsonData)
{
    bool overduedate = false;
    int datenum;
    if (queryJsonData->getDateTime().suggestDatetime.size() > 0) {
        if (queryJsonData->getDateTime().suggestDatetime.size() == 1)
            datenum = 0;
        else
            datenum = 1;

        if (queryJsonData->getDateTime().suggestDatetime.at(datenum).datetime.date() < QDate::currentDate()) {
            overduedate = true;
        } else if (queryJsonData->getDateTime().suggestDatetime.at(datenum).datetime.date() == QDate::currentDate()
                   && queryJsonData->getDateTime().suggestDatetime.at(datenum).hasTime
                   && queryJsonData->getDateTime().suggestDatetime.at(datenum).datetime.time() < QTime::currentTime()) {
            overduedate = true;
        } else {
            overduedate = false;
        }
    } else {
        overduedate = false;
    }
    return overduedate;
}
