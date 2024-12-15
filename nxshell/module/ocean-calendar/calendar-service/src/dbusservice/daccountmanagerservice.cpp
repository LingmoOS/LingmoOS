// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountmanagerservice.h"
#include "units.h"
#include "calendarprogramexitcontrol.h"

#include <QMetaType>
#include <QDBusMetaType>
#include <QDebug>

DAccountManagerService::DAccountManagerService(QObject *parent)
    : DServiceBase(serviceBasePath + "/AccountManager", serviceBaseName + ".AccountManager", parent)
    , m_accountManager(new DAccountManageModule(this))
{
    //自动退出
    DServiceExitControl exitControl;
    connect(m_accountManager.data(), &DAccountManageModule::signalLoginStatusChange, this, &DAccountManagerService::accountUpdate);

    connect(m_accountManager.data(), &DAccountManageModule::firstDayOfWeekChange, this, [&]() {
        notifyPropertyChanged(getInterface(), "firstDayOfWeek");
    });
    connect(m_accountManager.data(), &DAccountManageModule::timeFormatTypeChange, this, [&]() {
        notifyPropertyChanged(getInterface(), "timeFormatType");
    });
}

QString DAccountManagerService::getAccountList()
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return QString();
    }
    return m_accountManager->getAccountList();
}

void DAccountManagerService::remindJob(const QString &accountID, const QString &alarmID)
{
    DServiceExitControl exitControl;
    m_accountManager->remindJob(accountID, alarmID);
}

void DAccountManagerService::updateRemindJob(bool isClear)
{
    DServiceExitControl exitControl;
    m_accountManager->updateRemindSchedules(isClear);
}

void DAccountManagerService::notifyMsgHanding(const QString &accountID, const QString &alarmID, const qint32 operationNum)
{
    DServiceExitControl exitControl;
    m_accountManager->notifyMsgHanding(accountID, alarmID, operationNum);
}

void DAccountManagerService::downloadByAccountID(const QString &accountID)
{
    //TODO:云同步获取数据
    DServiceExitControl exitControl;
    m_accountManager->downloadByAccountID(accountID);
}

void DAccountManagerService::uploadNetWorkAccountData()
{
    //TODO:云同步上传数据
    DServiceExitControl exitControl;
    m_accountManager->uploadNetWorkAccountData();
}

QString DAccountManagerService::getCalendarGeneralSettings()
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return QString();
    }
    return m_accountManager->getCalendarGeneralSettings();
}

void DAccountManagerService::setCalendarGeneralSettings(const QString &cgSet)
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return;
    }
    m_accountManager->setFirstDayOfWeekSource(DCalendarGeneralSettings::Source_Database);
    m_accountManager->setTimeFormatTypeSource(DCalendarGeneralSettings::Source_Database);
    m_accountManager->setCalendarGeneralSettings(cgSet);
}

void DAccountManagerService::calendarIsShow(const bool &isShow)
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return;
    }
    exitControl.setClientIsOpen(isShow);
    m_accountManager->calendarOpen(isShow);
}

void DAccountManagerService::login()
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return;
    }
    m_accountManager->login();
}

void DAccountManagerService::logout()
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return;
    }
    m_accountManager->logout();
}

bool DAccountManagerService::isSupportUid()
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return false;
    }
    return m_accountManager->isSupportUid();
}

int DAccountManagerService::getfirstDayOfWeek() const
{
    DServiceExitControl exitControl;
    return m_accountManager->getfirstDayOfWeek();
}

void DAccountManagerService::setFirstDayOfWeek(const int firstday)
{
    DServiceExitControl exitControl;
    m_accountManager->setFirstDayOfWeekSource(DCalendarGeneralSettings::Source_Database);
    m_accountManager->setFirstDayOfWeek(firstday);
}

int DAccountManagerService::getTimeFormatType() const
{
    DServiceExitControl exitControl;
    return m_accountManager->getTimeFormatType();
}

void DAccountManagerService::setTimeFormatType(const int timeType)
{
    DServiceExitControl exitControl;
    m_accountManager->setTimeFormatTypeSource(DCalendarGeneralSettings::Source_Database);
    m_accountManager->setTimeFormatType(timeType);
}

int DAccountManagerService::getFirstDayOfWeekSource()
{
    return static_cast<int>(m_accountManager->getFirstDayOfWeekSource());
}

void DAccountManagerService::setFirstDayOfWeekSource(const int source)
{
    if (source >= 0 && source < DCalendarGeneralSettings::GeneralSettingSource::Source_Unknown) {
        auto val = static_cast<DCalendarGeneralSettings::GeneralSettingSource>(source);
        m_accountManager->setFirstDayOfWeekSource(val);
    }
}

int DAccountManagerService::getTimeFormatTypeSource()
{
    return static_cast<int>(m_accountManager->getTimeFormatTypeSource());
}

void DAccountManagerService::setTimeFormatTypeSource(const int source)
{
    if (source >= 0 && source < DCalendarGeneralSettings::GeneralSettingSource::Source_Unknown) {
        auto val = static_cast<DCalendarGeneralSettings::GeneralSettingSource>(source);
        m_accountManager->setTimeFormatTypeSource(val);
    }
}