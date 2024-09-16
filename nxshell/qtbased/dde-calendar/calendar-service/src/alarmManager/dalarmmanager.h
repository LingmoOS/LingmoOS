// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DALARMMANAGER_H
#define DALARMMANAGER_H

#include "dreminddata.h"
#include "dschedule.h"

#include <QObject>
#include <QSharedPointer>

class DBusNotify;
//日程提醒管理类
class DAlarmManager : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<DAlarmManager> Ptr;

    explicit DAlarmManager(QObject *parent = nullptr);
    //更新提醒任务
    void updateRemind(const DRemindData::List &remindList);
    void notifyJobsChanged(const DRemindData::List &remindList);

    /**
     * @brief notifyMsgHanding      通知提示框交互处理
     * @param remindData            提醒日程相关信息
     * @param operationNum          操作编号
     *                              1：打开日历，
     *                              2：稍后提醒 21：15min后提醒 22：一个小时后提醒 23：四个小时后提醒
     *                              3：明天提醒 4： 提前1天提醒
     */
    void notifyMsgHanding(const DRemindData::Ptr &remindData, const int operationNum);

    void remindLater(const DRemindData::Ptr &remindData, const int operationNum);

    int remindJob(const DRemindData::Ptr &remindData, const DSchedule::Ptr &schedule);

    DBusNotify *getdbusnotify();
private:
    int getRemindLaterDuration(int count, qint64 &duration);

    //获取日程提醒内容
    QString getRemindBody(const DSchedule::Ptr &schedule);

    QString getBodyTimePart(const QDateTime &nowtime, const QDateTime &jobtime, bool allday, bool isstart);
signals:
    //发送打开日历信号
    void signalCallOpenCalendarUI(const QString &alarmID);

private:
    DBusNotify *m_dbusnotify; //日程提醒dbus操作相关
};

#endif // DALARMMANAGER_H
