// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACCOUNTMANAGERDATABASE_H
#define DACCOUNTMANAGERDATABASE_H

#include "daccount.h"
#include "dschedule.h"
#include "ddatabase.h"
#include "dcalendargeneralsettings.h"

#include <QObject>
#include <QSharedPointer>

class DAccountManagerDataBase : public DDataBase
{
    Q_OBJECT
public:
    typedef QSharedPointer<DAccountManagerDataBase> Ptr;

    explicit DAccountManagerDataBase(QObject *parent = nullptr);

    //初始化数据库数据，只有在创建表的时候才需要
    void initDBData() override;

    ///////////////帐户
    //获取所有帐户信息
    DAccount::List getAccountList();

    //根据帐户id获取帐户信息
    DAccount::Ptr getAccountByID(const QString &accountID);
    //添加帐户信息
    QString addAccountInfo(const DAccount::Ptr &accountInfo);
    //更新帐户信息
    bool updateAccountInfo(const DAccount::Ptr &accountInfo);
    //根据帐户id删除对应帐户
    bool deleteAccountInfo(const QString &accountID);

    ///////////////通用设置

    DCalendarGeneralSettings::Ptr getCalendarGeneralSettings();
    void setCalendarGeneralSettings(const DCalendarGeneralSettings::Ptr &cgSet);


    void setLoaclDB(const QString &loaclDB);

protected:
    //创建数据库
    void createDB() override;
    void initAccountManagerDB();

private:
    QString m_loaclDB;
};

#endif // DACCOUNTMANAGERDATABASE_H
