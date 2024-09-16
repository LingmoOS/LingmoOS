// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountmanagemodule.h"
#include "commondef.h"
#include "units.h"
#include "calendarprogramexitcontrol.h"
#include <qstandardpaths.h>
#include <DSysInfo>

const QString firstDayOfWeek_key = "firstDayOfWeek";
const QString shortTimeFormat_key = "shortTimeFormat";
const QString firstDayOfWeekSource_key = "firstDayOfWeekSource";
const QString shortTimeFormatSource_key = "shortTimeFormatSource";

DAccountManageModule::DAccountManageModule(QObject *parent)
    : QObject(parent)
    , m_syncFileManage(new SyncFileManage())
    , m_accountManagerDB(new DAccountManagerDataBase)
    , m_reginFormatConfig(DTK_CORE_NAMESPACE::DConfig::createGeneric("org.deepin.region-format", QString(), this))
    , m_settings( getAppConfigDir().filePath( "config.ini"), QSettings::IniFormat)
{
    if (m_reginFormatConfig->isValid()) {
        connect(m_reginFormatConfig,
                &DTK_CORE_NAMESPACE::DConfig::valueChanged,
                this,
                &DAccountManageModule::slotSettingChange);
    } else {
        connect(&m_timeDateDbus,
                &DBusTimedate::ShortTimeFormatChanged,
                this,
                &DAccountManageModule::slotSettingChange);
        connect(&m_timeDateDbus,
                &DBusTimedate::WeekBeginsChanged,
                this,
                &DAccountManageModule::slotSettingChange);
    }
    m_isSupportUid = m_syncFileManage->getSyncoperation()->hasAvailable();
    //新文件路径
    QString newDbPath = getDBPath();
    QString newDB(newDbPath + "/" + "accountmanager.db");
    m_accountManagerDB->setDBPath(newDB);
    m_accountManagerDB->dbOpen();

    QDBusConnection::RegisterOptions options = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    //将云端帐户信息基本数据与本地数据合并
    unionIDDataMerging();

    //根据获取到的帐户信息创建对应的帐户服务
    foreach (auto account, m_accountList) {
        //如果不支持云同步且帐户类型为UID则过滤
        if (!m_isSupportUid && account->accountType() == DAccount::Account_UnionID) {
            continue;
        }
        DAccountModule::Ptr accountModule = DAccountModule::Ptr(new DAccountModule(account));
        QObject::connect(accountModule.data(), &DAccountModule::signalSettingChange, this, &DAccountManageModule::slotSettingChange);
        m_accountModuleMap[account->accountID()] = accountModule;
        DAccountService::Ptr accountService = DAccountService::Ptr(new DAccountService(account->dbusPath(), account->dbusInterface(), accountModule, this));
        if (!sessionBus.registerObject(accountService->getPath(), accountService->getInterface(), accountService.data(), options)) {
            qCWarning(ServiceLogger) << "registerObject accountService failed:" << sessionBus.lastError();
        } else {
            m_AccountServiceMap[account->accountType()].insert(account->accountID(), accountService);
            //如果是网络帐户则开启定时下载任务
            if (account->isNetWorkAccount() && account->accountState().testFlag(DAccount::Account_Open)) {
                accountModule->downloadTaskhanding(0);
            }
        }
    }
    m_generalSetting = getGeneralSettings();

    connect(&m_timer, &QTimer::timeout, this, &DAccountManageModule::slotClientIsOpen);
    m_timer.start(2000);

    if (m_isSupportUid) {
        QObject::connect(m_syncFileManage->getSyncoperation(), &Syncoperation::signalLoginStatusChange, this, &DAccountManageModule::slotUidLoginStatueChange);
        QObject::connect(m_syncFileManage->getSyncoperation(), &Syncoperation::SwitcherChange, this, &DAccountManageModule::slotSwitcherChange);
    }

    //第一次启动加载完成后发送帐户改变信号
    // emit signalLoginStatusChange();
}

QString DAccountManageModule::getAccountList()
{
    QString accountStr;
    DAccount::toJsonListString(m_accountList, accountStr);
    return accountStr;
}

QString DAccountManageModule::getCalendarGeneralSettings()
{
    QString cgSetStr;
    m_generalSetting = getGeneralSettings();
    DCalendarGeneralSettings::toJsonString(m_generalSetting, cgSetStr);
    return cgSetStr;
}

void DAccountManageModule::setCalendarGeneralSettings(const QString &cgSet)
{
    DCalendarGeneralSettings::Ptr cgSetPtr = DCalendarGeneralSettings::Ptr(new DCalendarGeneralSettings);
    DCalendarGeneralSettings::fromJsonString(cgSetPtr, cgSet);
    if (m_generalSetting != cgSetPtr) {
        setGeneralSettings(cgSetPtr);
        DCalendarGeneralSettings::Ptr tmpSetting = DCalendarGeneralSettings::Ptr(m_generalSetting->clone());
        m_generalSetting = cgSetPtr;
        if (tmpSetting->firstDayOfWeek() != m_generalSetting->firstDayOfWeek()) {
            emit firstDayOfWeekChange();
        }
        if (tmpSetting->timeShowType() != m_generalSetting->timeShowType()) {
            emit timeFormatTypeChange();
        }
    }
}

int DAccountManageModule::getfirstDayOfWeek()
{
    return static_cast<int>(m_generalSetting->firstDayOfWeek());
}

void DAccountManageModule::setFirstDayOfWeek(const int firstday)
{
    if (m_generalSetting->firstDayOfWeek() != firstday) {
        m_generalSetting->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(firstday));
        setGeneralSettings(m_generalSetting);
        foreach (auto account, m_accountList) {
            if (account->accountType() == DAccount::Account_UnionID) {
                m_accountModuleMap[account->accountID()]->accountDownload();
            }
        }
    }
}

int DAccountManageModule::getTimeFormatType()
{
    return static_cast<int>(m_generalSetting->timeShowType());
}

void DAccountManageModule::setTimeFormatType(const int timeType)
{
    if (m_generalSetting->timeShowType() != timeType) {
        m_generalSetting->setTimeShowType(static_cast<DCalendarGeneralSettings::TimeShowType>(timeType));
        setGeneralSettings(m_generalSetting);
        foreach (auto account, m_accountList) {
            if (account->accountType() == DAccount::Account_UnionID) {
                m_accountModuleMap[account->accountID()]->accountDownload();
            }
        }
    }
}

void DAccountManageModule::remindJob(const QString &accountID, const QString &alarmID)
{
    if (m_accountModuleMap.contains(accountID)) {
        m_accountModuleMap[accountID]->remindJob(alarmID);
    }
}

void DAccountManageModule::updateRemindSchedules(bool isClear)
{
    QMap<QString, DAccountModule::Ptr>::const_iterator iter = m_accountModuleMap.constBegin();
    for (; iter != m_accountModuleMap.constEnd(); ++iter) {
        iter.value()->updateRemindSchedules(isClear);
    }
}

void DAccountManageModule::notifyMsgHanding(const QString &accountID, const QString &alarmID, const qint32 operationNum)
{
    if (m_accountModuleMap.contains(accountID)) {
        m_accountModuleMap[accountID]->notifyMsgHanding(alarmID, operationNum);
    }
}

void DAccountManageModule::downloadByAccountID(const QString &accountID)
{
    if (m_accountModuleMap.contains(accountID)) {
        m_accountModuleMap[accountID]->accountDownload();
    }
}

void DAccountManageModule::uploadNetWorkAccountData()
{
    QMap<QString, DAccountModule::Ptr>::const_iterator iter = m_accountModuleMap.constBegin();
    for (; iter != m_accountModuleMap.constEnd(); ++iter) {
        iter.value()->uploadNetWorkAccountData();
    }
}

//账户登录
void DAccountManageModule::login()
{
    m_syncFileManage->getSyncoperation()->optlogin();
}
//账户登出
void DAccountManageModule::logout()
{
    m_syncFileManage->getSyncoperation()->optlogout();
}

bool DAccountManageModule::isSupportUid()
{
    return m_isSupportUid;
}

void DAccountManageModule::calendarOpen(bool isOpen)
{
    //每次开启日历时需要同步数据
    if (isOpen) {
        QMap<QString, DAccountModule::Ptr>::iterator iter = m_accountModuleMap.begin();
        for (; iter != m_accountModuleMap.end(); ++iter) {
            iter.value()->accountDownload();
        }
    }
}

void DAccountManageModule::unionIDDataMerging()
{
    m_accountList = m_accountManagerDB->getAccountList();

    //如果不支持云同步
    if (!m_isSupportUid) {
        DAccount::Ptr unionidDB;
        auto hasUnionid = [ =, &unionidDB](const DAccount::Ptr & account) {
            if (account->accountType() == DAccount::Account_UnionID) {
                unionidDB = account;
                return true;
            }
            return false;
        };
        //如果数据库中有unionid帐户
        if (std::any_of(m_accountList.begin(), m_accountList.end(), hasUnionid)) {
            //如果包含则移除
            removeUIdAccount(unionidDB);
        }
        return;
    }

    DAccount::Ptr accountUnionid = m_syncFileManage->getuserInfo();

    DAccount::Ptr unionidDB;
    auto hasUnionid = [ =, &unionidDB](const DAccount::Ptr & account) {
        if (account->accountType() == DAccount::Account_UnionID) {
            unionidDB = account;
            return true;
        }
        return false;
    };
    //如果unionid帐户不存在，则判断数据库中是否有登陆前的信息
    //若有则移除
    if (accountUnionid.isNull() || accountUnionid->accountID().isEmpty()) {
        //如果数据库中有unionid帐户
        if (std::any_of(m_accountList.begin(), m_accountList.end(), hasUnionid)) {
            removeUIdAccount(unionidDB);
        }
    } else {
        //如果unionID登陆了

        //如果数据库中有unionid帐户
        if (std::any_of(m_accountList.begin(), m_accountList.end(), hasUnionid)) {
            //如果是一个帐户则判断信息是否一致，不一致需更新
            if (unionidDB->accountName() == accountUnionid->accountName()) {
                updateUIdAccount(unionidDB, accountUnionid);
            } else {
                removeUIdAccount(unionidDB);
                addUIdAccount(accountUnionid);
            }
        } else {
            addUIdAccount(accountUnionid);
        }
    }
}

void DAccountManageModule::initAccountDBusInfo(const DAccount::Ptr &account)
{
    QString typeStr = "";
    switch (account->accountType()) {
    case DAccount::Type::Account_UnionID:
        typeStr = "uid";
        break;
    case DAccount::Type::Account_CalDav:
        typeStr = "caldav";
        break;
    default:
        typeStr = "default";
        break;
    }
    QString sortID = DDataBase::createUuid().mid(0, 5);
    account->setAccountState(DAccount::AccountState::Account_Setting | DAccount::Account_Calendar);
    setUidSwitchStatus(account);
    //设置DBus路径和数据库名
    //account
    account->setAccountType(DAccount::Account_UnionID);
    account->setDtCreate(QDateTime::currentDateTime());
    account->setDbName(QString("account_%1_%2.db").arg(typeStr).arg(sortID));
    account->setDbusPath(QString("%1/account_%2_%3").arg(serviceBasePath).arg(typeStr).arg(sortID));
    account->setDbusInterface(accountServiceInterface);
}

void DAccountManageModule::removeUIdAccount(const DAccount::Ptr &uidAccount)
{
    //帐户列表移除uid帐户
    m_accountList.removeOne(uidAccount);
    //移除对应的数据库 ，停止对应的定时器
    DAccountModule::Ptr accountModule(new DAccountModule(uidAccount));
    accountModule->removeDB();
    accountModule->downloadTaskhanding(2);
    //帐户管理数据库中删除相关数据
    m_accountManagerDB->deleteAccountInfo(uidAccount->accountID());
}

void DAccountManageModule::addUIdAccount(const DAccount::Ptr &uidAccount)
{
    //帐户管理数据库中添加uid帐户
    initAccountDBusInfo(uidAccount);
    m_accountManagerDB->addAccountInfo(uidAccount);
    m_accountList.append(uidAccount);
}

void DAccountManageModule::updateUIdAccount(const DAccount::Ptr &oldAccount, const DAccount::Ptr &uidAccount)
{
    oldAccount->avatar() = uidAccount->avatar();
    oldAccount->displayName() = uidAccount->displayName();
    setUidSwitchStatus(oldAccount);
    m_accountManagerDB->updateAccountInfo(oldAccount);
}

void DAccountManageModule::setUidSwitchStatus(const DAccount::Ptr &account)
{
    //获取控制中心开关状态
    bool calendarSwitch = m_syncFileManage->getSyncoperation()->optGetCalendarSwitcher().switch_state;
    //获取帐户信息状态
    DAccount::AccountStates accountState = account->accountState();
    accountState.setFlag(DAccount::Account_Open, calendarSwitch);
    account->setAccountState(accountState);
}

// 获取通用配置
DCalendarGeneralSettings::Ptr DAccountManageModule::getGeneralSettings()
{
    auto cg = m_accountManagerDB->getCalendarGeneralSettings();
    if (getFirstDayOfWeekSource() == DCalendarGeneralSettings::Source_System) {
        // deepin23使用dconfig存储系统配置
        if (m_reginFormatConfig->isValid()) {
            bool ok;
            auto dayofWeek =
                Qt::DayOfWeek(m_reginFormatConfig->value(firstDayOfWeek_key).toInt(&ok));
            if (ok) {
                cg->setFirstDayOfWeek(dayofWeek);
            } else {
                qWarning() << "Unable to get first day of week from control center config file";
            }
        } else {
            // 在deepin20.9使用dbus获取系统配置
            cg->setFirstDayOfWeek(m_timeDateDbus.weekBegins());
        }
    }
    if (getTimeFormatTypeSource() == DCalendarGeneralSettings::Source_System) {
        // 在deepin23使用dconfig获取系统配置
        if (m_reginFormatConfig->isValid()) {
            auto shortTimeFormat = m_reginFormatConfig->value(shortTimeFormat_key).toString();
            if (shortTimeFormat.isEmpty()) {
                qWarning() << "Unable to short time format from control center config file";
            } else if (shortTimeFormat.contains("ap")) {
                cg->setTimeShowType(DCalendarGeneralSettings::Twelve);
            } else {
                cg->setTimeShowType(DCalendarGeneralSettings::TwentyFour);
            }
        } else {
            // 在deepin20.9使用dbus获取系统配置
            if (m_timeDateDbus.shortTimeFormat() == 0) {
                cg->setTimeShowType(DCalendarGeneralSettings::Twelve);
            } else {
                cg->setTimeShowType(DCalendarGeneralSettings::TwentyFour);
            }
        }
    }
    return cg;
}

// 更改通用配置
void DAccountManageModule::setGeneralSettings(const DCalendarGeneralSettings::Ptr &cgSet)
{
    m_accountManagerDB->setCalendarGeneralSettings(cgSet);
};

void DAccountManageModule::slotFirstDayOfWeek(const int firstDay)
{
    if (getfirstDayOfWeek() != firstDay) {
        setFirstDayOfWeek(firstDay);
        emit firstDayOfWeekChange();
    }
}

void DAccountManageModule::slotTimeFormatType(const int timeType)
{
    if (getTimeFormatType() != timeType) {
        setTimeFormatType(timeType);
        emit timeFormatTypeChange();
    }
}

DCalendarGeneralSettings::GeneralSettingSource DAccountManageModule::getFirstDayOfWeekSource()
{
    auto val = m_settings.value(firstDayOfWeekSource_key, DCalendarGeneralSettings::Source_Database);
    return static_cast<DCalendarGeneralSettings::GeneralSettingSource>(val.toInt());
}

void DAccountManageModule::setFirstDayOfWeekSource(const DCalendarGeneralSettings::GeneralSettingSource source)
{
    m_settings.setValue(firstDayOfWeekSource_key, source);
    emit firstDayOfWeekChange();
}

DCalendarGeneralSettings::GeneralSettingSource DAccountManageModule::getTimeFormatTypeSource()
{
    auto val = m_settings.value(shortTimeFormatSource_key, DCalendarGeneralSettings::GeneralSettingSource::Source_Database);
    return static_cast<DCalendarGeneralSettings::GeneralSettingSource>(val.toInt());
}

void DAccountManageModule::setTimeFormatTypeSource(const DCalendarGeneralSettings::GeneralSettingSource source)
{
    m_settings.setValue(shortTimeFormatSource_key, source);
    emit timeFormatTypeChange();
}

void DAccountManageModule::slotUidLoginStatueChange(const int status)
{
    //因为有时登录成功会触发2次
    static QList<int> oldStatus{};
    //登录成功后会触发多次，状态也不一致。比如登录后会连续触发 1 -- 4 -- 1 信号
    if (!oldStatus.contains(status)) {
        oldStatus.append(status);
    } else {
        //如果当前状态和上次状态一直，则退出
        return;
    }
    //1：登陆成功 2：登陆取消 3：登出 4：获取服务端配置的应用数据成功
    QDBusConnection::RegisterOptions options = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    switch (status) {
    case 1: {
        //移除登出状态
        if (oldStatus.contains(3)) {
            oldStatus.removeAt(oldStatus.indexOf(3));
        }

        //登陆成功
        DAccount::Ptr accountUnionid = m_syncFileManage->getuserInfo();
        if (accountUnionid.isNull() || accountUnionid->accountName().isEmpty()) {
            qCWarning(ServiceLogger) << "Error getting account information";
            oldStatus.removeAt(oldStatus.indexOf(1));
            return;
        }
        addUIdAccount(accountUnionid);

        DAccountModule::Ptr accountModule = DAccountModule::Ptr(new DAccountModule(accountUnionid));
        QObject::connect(accountModule.data(), &DAccountModule::signalSettingChange, this, &DAccountManageModule::slotSettingChange);
        m_accountModuleMap[accountUnionid->accountID()] = accountModule;
        DAccountService::Ptr accountService = DAccountService::Ptr(new DAccountService(accountUnionid->dbusPath(), accountUnionid->dbusInterface(), accountModule, this));
        if (!sessionBus.registerObject(accountService->getPath(), accountService->getInterface(), accountService.data(), options)) {
            qCWarning(ServiceLogger) << "registerObject accountService failed:" << sessionBus.lastError();
        } else {
            m_AccountServiceMap[accountUnionid->accountType()].insert(accountUnionid->accountID(), accountService);
            if (accountUnionid->accountState().testFlag(DAccount::Account_Open)) {
                accountModule->downloadTaskhanding(0);
            }
        }
    } break;
    case 3: {
        //移除登录状态
        if (oldStatus.contains(1)) {
            oldStatus.removeAt(oldStatus.indexOf(1));
        }
        //登出
        if (m_AccountServiceMap[DAccount::Type::Account_UnionID].size() > 0) {
            //如果存在UID帐户则移除相关信息
            //移除服务并注销
            QString accountID = m_AccountServiceMap[DAccount::Type::Account_UnionID].firstKey();
            DAccountService::Ptr accountService = m_AccountServiceMap[DAccount::Type::Account_UnionID].first();
            m_AccountServiceMap[DAccount::Type::Account_UnionID].clear();
            sessionBus.unregisterObject(accountService->getPath());
            //移除uid帐户信息
            //删除对应数据库
            m_accountModuleMap[accountID]->removeDB();
            m_accountModuleMap[accountID]->downloadTaskhanding(2);
            m_accountList.removeOne(m_accountModuleMap[accountID]->account());
            m_accountModuleMap.remove(accountID);
            m_accountManagerDB->deleteAccountInfo(accountID);
        }
    } break;
    default:
        //其它状态当前不做处理
        return;
    }
    emit signalLoginStatusChange();
}

void DAccountManageModule::slotSwitcherChange(const bool state)
{
    foreach (auto schedule, m_accountList) {
        if (schedule->accountType() == DAccount::Account_UnionID) {
            if (state) {
                schedule->setAccountState(schedule->accountState() | DAccount::Account_Open);
                //开启
                m_accountModuleMap[schedule->accountID()]->downloadTaskhanding(1);
            } else {
                schedule->setAccountState(schedule->accountState() & ~DAccount::Account_Open);
                //关闭
                m_accountModuleMap[schedule->accountID()]->downloadTaskhanding(2);
                m_accountModuleMap[schedule->accountID()]->uploadTaskHanding(0);
            }
            emit m_accountModuleMap[schedule->accountID()]->signalAccountState();
            return;
        }
    }
}

void DAccountManageModule::slotSettingChange()
{
    DCalendarGeneralSettings::Ptr newSetting = getGeneralSettings();
    if (newSetting->firstDayOfWeek() != m_generalSetting->firstDayOfWeek()) {
        m_generalSetting->setFirstDayOfWeek(newSetting->firstDayOfWeek());
        emit firstDayOfWeekChange();
    }

    if (newSetting->timeShowType() != m_generalSetting->timeShowType()) {
        m_generalSetting->setTimeShowType(m_generalSetting->timeShowType());
        emit timeFormatTypeChange();
    }
}

void DAccountManageModule::slotClientIsOpen()
{
    //如果日历界面不存在则退出
    QProcess process;
    process.start("/bin/bash", QStringList() << "-c"
                  << "pidof dde-calendar");
    process.waitForFinished();
    QString strResult = process.readAllStandardOutput();

    static QString preResult = "";

    if (preResult == strResult) {
        return;
    } else {
        preResult = strResult;
        DServiceExitControl exitControl;
        exitControl.setClientIsOpen(!strResult.isEmpty());
    }
}
