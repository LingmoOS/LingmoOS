// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CSCHEDULEOPERATION_H
#define CSCHEDULEOPERATION_H

#include "scheduledatamanage.h"
#include "accountitem.h"

#include <QObject>
#include <QWidget>

class CScheduleDBus;
/**
 * @brief The CScheduleOperation class
 * 日程操作类,创建,修改和删除日程
 */
class CScheduleOperation : public QObject
{
    Q_OBJECT
public:
    explicit CScheduleOperation(const AccountItem::Ptr &accountItem = nullptr, QWidget *parent = nullptr);
    explicit CScheduleOperation(const QString &scheduleTypeID, QWidget *parent = nullptr);
    //创建日程
    bool createSchedule(const DSchedule::Ptr &scheduleInfo);
    //修改日程
    bool changeSchedule(const DSchedule::Ptr &newInfo, const DSchedule::Ptr &oldInfo);
    //删除日程  false :取消删除  true: 确定删除
    bool deleteSchedule(const DSchedule::Ptr &scheduleInfo);
    //删除日程 仅删除此日程 不弹框提醒
    void deleteOnlyInfo(const DSchedule::Ptr &scheduleInfo);


    //是否为节假日日程
    static bool isFestival(const DSchedule::Ptr &schedule);
    //日程是不可修改的
    static bool scheduleIsInvariant(const DSchedule::Ptr &schedule);

private:
    //修改重复日程
    bool changeRecurInfo(const DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo);
    //修改重复日程,仅修改此日程
    bool changeOnlyInfo(const DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo);
    //修改重复规则
    void changeRepetitionRule(DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo);

    //农历每年闰月提示信息
    void lunarMessageDialogShow(const DSchedule::Ptr &newinfo);
    //根据新旧日程信息判断是否提示
    void showLunarMessageDialog(const DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo);
signals:
public slots:
private:
    AccountItem::Ptr m_accountItem;
    QWidget                         *m_widget;
};

#endif // CSCHEDULEOPERATION_H
