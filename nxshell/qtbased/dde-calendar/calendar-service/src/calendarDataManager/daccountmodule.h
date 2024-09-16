// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACCOUNTMODULE_H
#define DACCOUNTMODULE_H

#include "daccount.h"
#include "daccountdatabase.h"
#include "dschedule.h"
#include "dalarmmanager.h"
#include "ddatasyncbase.h"
#include "icalformat.h"
#include "memorycalendar.h"

#include <QObject>
#include <QSharedPointer>

//帐户模块
//处理后端数据获取，提醒，上传下载等
class DAccountModule : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<DAccountModule> Ptr;
    typedef QList<Ptr> List;

    explicit DAccountModule(const DAccount::Ptr &account, QObject *parent = nullptr);
    ~DAccountModule();
    //获取帐户信息
    QString getAccountInfo();
    //设置获取帐户是否展开
    bool getExpand();
    void setExpand(const bool &isExpand);
    //帐户状态。
    int getAccountState();
    void setAccountState(const int accountState);
    //获取同步状态
    int getSyncState();
    QString getSyncFreq();
    void setSyncFreq(const QString &freq);

    //获取类型
    QString getScheduleTypeList();
    QString getScheduleTypeByID(const QString &typeID);
    QString createScheduleType(const QString &typeInfo);
    bool deleteScheduleTypeByID(const QString &typeID);
    bool scheduleTypeByUsed(const QString &typeID);
    bool updateScheduleType(const QString &typeInfo);

    //日程信息
    QString createSchedule(const QString &scheduleInfo);
    bool updateSchedule(const QString &scheduleInfo);
    QString getScheduleByScheduleID(const QString &scheduleID);
    bool deleteScheduleByScheduleID(const QString &scheduleID);
    QString querySchedulesWithParameter(const QString &params);

    bool exportSchedule(const QString &icsFilePath, const QString &typeID);                         // 导出ICS文件
    bool importSchedule(const QString &icsFilePath, const QString &typeID, const bool cleanExists); // 导入ICS文件

    //获取需要提醒的日程
    DSchedule::List getRemindScheduleList(const QDateTime &dtStart, const QDateTime &dtEnd);

    //内置颜色
    QString getSysColors();

    DAccount::Ptr account() const;

    /**
     * @brief updateRemindSchedules     更新未来10分钟的提醒任务
     * @param isClear                   是否清空提醒任务数据库
     */
    void updateRemindSchedules(bool isClear);

    /**
     * @brief notifyMsgHanding      通知提示框交互处理
     * @param alarmID               提醒任务id
     * @param operationNum          操作编号
     *                              1：打开日历，
     *                              2：稍后提醒 21：15min后提醒 22：一个小时后提醒 23：四个小时后提醒
     *                              3：明天提醒 4： 提前1天提醒
     */
    void notifyMsgHanding(const QString &alarmID, const qint32 operationNum);

    void remindJob(const QString &alarmID);

    void accountDownload();
    void uploadNetWorkAccountData();

    //删除数据库
    void removeDB();
    //index: 0:帐户登录 1：修改同步频率 2：帐户登出
    void downloadTaskhanding(int index);

    // 0:关闭 1：启动
    void uploadTaskHanding(int open);

    //获取最后一次同步时间
    QString getDtLastUpdate();

private:
    QMap<QDate, DSchedule::List> getScheduleTimesOn(const QDateTime &dtStart, const QDateTime &dtEnd, const DSchedule::List &scheduleList, bool extend = true);
    DSchedule::List getFestivalSchedule(const QDateTime &dtStart, const QDateTime &dtEnd, const QString &key);

    //根据重复规则扩展非农历重复日程
    void extendRecurrence(DSchedule::Map &scheduleMap, const DSchedule::Ptr &schedule, const QDateTime &dtStart, const QDateTime &dtEnd, bool extend = true);

    /**
     * @brief closeNotification     关闭通知弹框
     * @param scheduleId            日程id
     */
    void closeNotification(const QString &scheduleId);

    //根据提醒任务获取对应的日程信息
    DSchedule::Ptr getScheduleByRemind(const DRemindData::Ptr &remindData);

signals:
    void signalScheduleUpdate();
    void signalScheduleTypeUpdate();
    //关闭通知弹框
    void signalCloseNotification(quint64 notifyID);

    void signalAccountState();
    void signalSyncState();

    void signalDtLastUpdate();
    //通用设置发生改变
    void signalSettingChange();

    //数据同步完成
    void signalSyncFinished(int);

public slots:
    void slotOpenCalendar(const QString &alarmID);

    void slotSyncState(const int syncState);
    void slotDateUpdate(const DDataSyncBase::UpdateTypes updateType);

private:
    DAccount::Ptr m_account;
    DAccountDataBase::Ptr m_accountDB;
    DAlarmManager::Ptr m_alarm;
    DDataSyncBase *m_dataSync;
};

#endif // DACCOUNTMODULE_H
