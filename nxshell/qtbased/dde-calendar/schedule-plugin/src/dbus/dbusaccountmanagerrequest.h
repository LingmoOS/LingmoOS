// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSACCOUNTMANAGERREQUEST_H
#define DBUSACCOUNTMANAGERREQUEST_H

#include "dbusrequestbase.h"
#include "daccount.h"
#include "dcalendargeneralsettings.h"

//所有账户信息管理类
class DbusAccountManagerRequest : public DbusRequestBase
{
    Q_OBJECT
public:
    explicit DbusAccountManagerRequest(QObject *parent = nullptr);

    //获取账户列表
    DAccount::List getAccountList();
    //根据账户id下拉数据
    void downloadByAccountID(const QString &accountID);
    //更新网络账户数据
    void uploadNetWorkAccountData();
    //获取通用设置
    void getCalendarGeneralSettings();
    //设置通用设置
    void setCalendarGeneralSettings(DCalendarGeneralSettings::Ptr ptr);
    //
    void clientIsShow(bool isShow);

signals:
    //获取账户列表数据完成信号
    void signalGetAccountListFinish(DAccount::List accountList);
    //获取通用设置完成信号
    void signalGetGeneralSettingsFinish(DCalendarGeneralSettings::Ptr ptr);

public slots:
    //dbus调用完成事件
    void slotCallFinished(CDBusPendingCallWatcher *) override;
};

#endif // DBUSACCOUNTMANAGERREQUEST_H
