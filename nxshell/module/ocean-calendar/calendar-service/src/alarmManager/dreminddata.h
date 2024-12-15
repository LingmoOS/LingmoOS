// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DREMINDDATA_H
#define DREMINDDATA_H

#include <QString>
#include <QDateTime>
#include <QSharedPointer>

class DRemindData
{
public:
    typedef QSharedPointer<DRemindData> Ptr;
    typedef QVector<Ptr> List;

    DRemindData();

    QString accountID() const;
    void setAccountID(const QString &accountID);

    QString scheduleID() const;
    void setScheduleID(const QString &scheduleID);

    QDateTime recurrenceId() const;
    void setRecurrenceId(const QDateTime &recurrenceId);

    int remindCount() const;
    void setRemindCount(int remindCount);

    int notifyid() const;
    void setNotifyid(int notifyid);

    QDateTime dtRemind() const;
    void setDtRemind(const QDateTime &dtRemind);

    QDateTime dtStart() const;
    void setDtStart(const QDateTime &dtStart);

    QDateTime dtEnd() const;
    void setDtEnd(const QDateTime &dtEnd);

    QString alarmID() const;
    void setAlarmID(const QString &alarmID);

    void updateRemindTimeByCount();

    void updateRemindTimeByMesc(qint64 duration);

private:
    QDateTime getRemindTimeByMesc(qint64 duration);

private:
    QString m_alarmID;
    QString m_accountID;
    QString m_scheduleID;
    QDateTime m_recurrenceId;
    int m_remindCount;
    int m_notifyid;
    QDateTime m_dtRemind;
    QDateTime m_dtStart;
    QDateTime m_dtEnd;
};

#endif // DREMINDDATA_H
