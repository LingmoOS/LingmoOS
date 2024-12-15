// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSACCOUNTMANAGERREQUEST_H
#define DBUSACCOUNTMANAGERREQUEST_H

#include "dbusrequestbase.h"
#include "daccount.h"
#include "dcalendargeneralsettings.h"

//所有帐户信息管理类
class DbusAccountManagerRequest : public DbusRequestBase
{
    Q_OBJECT
public:
    explicit DbusAccountManagerRequest(QObject *parent = nullptr);

    //设置一周首日
    void setFirstDayofWeek(int);
    //设置时间显示格式
    void setTimeFormatType(int);
    // 设置一周首日来源
    void setFirstDayofWeekSource(DCalendarGeneralSettings::GeneralSettingSource);
    // 设置时间显示格式来源
    void setTimeFormatTypeSource(DCalendarGeneralSettings::GeneralSettingSource);
    // 获取一周首日来源
    DCalendarGeneralSettings::GeneralSettingSource getFirstDayofWeekSource();
    // 获取时间显示格式来源
    DCalendarGeneralSettings::GeneralSettingSource getTimeFormatTypeSource();

    //获取帐户列表
    void getAccountList();
    //根据帐户id下拉数据
    void downloadByAccountID(const QString &accountID);
    //更新网络帐户数据
    void uploadNetWorkAccountData();
    //获取通用设置
    void getCalendarGeneralSettings();
    //
    void clientIsShow(bool isShow);
    //获取是否支持云同步
    bool getIsSupportUid();

    //帐户登录
    void login();
    //帐户登出
    void logout();

signals:
    //获取帐户列表数据完成信号
    void signalGetAccountListFinish(DAccount::List accountList);
    //获取通用设置完成信号
    void signalGetGeneralSettingsFinish(DCalendarGeneralSettings::Ptr ptr);

public slots:
    //dbus调用完成事件
    void slotCallFinished(CDBusPendingCallWatcher *) override;
    //后端发送信号事件
    void slotDbusCall(const QDBusMessage &msg) override;

private:
    void onPropertiesChanged(const QString &interfaceName,
                             const QVariantMap &changedProperties,
                             const QStringList &invalidatedProperties);
};

#endif // DBUSACCOUNTMANAGERREQUEST_H
