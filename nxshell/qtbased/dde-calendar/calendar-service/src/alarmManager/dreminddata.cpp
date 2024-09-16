// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dreminddata.h"

DRemindData::DRemindData()
    : m_alarmID("")
    , m_accountID("")
    , m_scheduleID("")
    , m_remindCount(0)
    , m_notifyid(-1)
{
}

QString DRemindData::accountID() const
{
    return m_accountID;
}

void DRemindData::setAccountID(const QString &accountID)
{
    m_accountID = accountID;
}

QString DRemindData::scheduleID() const
{
    return m_scheduleID;
}

void DRemindData::setScheduleID(const QString &scheduleID)
{
    m_scheduleID = scheduleID;
}

QDateTime DRemindData::recurrenceId() const
{
    return m_recurrenceId;
}

void DRemindData::setRecurrenceId(const QDateTime &recurrenceId)
{
    m_recurrenceId = recurrenceId;
}

int DRemindData::remindCount() const
{
    return m_remindCount;
}

void DRemindData::setRemindCount(int remindCount)
{
    m_remindCount = remindCount;
}

int DRemindData::notifyid() const
{
    return m_notifyid;
}

void DRemindData::setNotifyid(int notifyid)
{
    m_notifyid = notifyid;
}

QDateTime DRemindData::dtRemind() const
{
    return m_dtRemind;
}

void DRemindData::setDtRemind(const QDateTime &dtRemind)
{
    m_dtRemind = dtRemind;
}

QDateTime DRemindData::dtStart() const
{
    return m_dtStart;
}

void DRemindData::setDtStart(const QDateTime &dtStart)
{
    m_dtStart = dtStart;
}

QDateTime DRemindData::dtEnd() const
{
    return m_dtEnd;
}

void DRemindData::setDtEnd(const QDateTime &dtEnd)
{
    m_dtEnd = dtEnd;
}

QString DRemindData::alarmID() const
{
    return m_alarmID;
}

void DRemindData::setAlarmID(const QString &alarmID)
{
    m_alarmID = alarmID;
}

void DRemindData::updateRemindTimeByCount()
{
    qint64 Minute = 60 * 1000;
    qint64 Hour = Minute * 60;
    qint64 duration = (10 + ((m_remindCount - 1) * 5)) * Minute; //下一次提醒距离现在的时间间隔，单位毫秒
    if (duration >= Hour) {
        duration = Hour;
    }
    setDtRemind(getRemindTimeByMesc(duration));
}

void DRemindData::updateRemindTimeByMesc(qint64 duration)
{
    setDtRemind(getRemindTimeByMesc(duration));
}

QDateTime DRemindData::getRemindTimeByMesc(qint64 duration)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    currentTime = currentTime.addMSecs(duration);
    return currentTime;
}
