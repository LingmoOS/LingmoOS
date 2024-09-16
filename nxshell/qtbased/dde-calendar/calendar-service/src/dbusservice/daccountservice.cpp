// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountservice.h"
#include "calendarprogramexitcontrol.h"

DAccountService::DAccountService(const QString &path, const QString &interface, const DAccountModule::Ptr &accountModule, QObject *parent)
    : DServiceBase(path, interface, parent)
    , m_accountModel(accountModule)
{
    connect(m_accountModel.data(), &DAccountModule::signalScheduleUpdate, this, &DAccountService::scheduleUpdate);
    connect(m_accountModel.data(), &DAccountModule::signalScheduleTypeUpdate, this, &DAccountService::scheduleTypeUpdate);

    connect(m_accountModel.data(), &DAccountModule::signalAccountState, this, [&]() {
        notifyPropertyChanged(getInterface(), "accountState");
    });
    connect(m_accountModel.data(), &DAccountModule::signalSyncState, this, [&]() {
        notifyPropertyChanged(getInterface(), "syncState");
    });
    connect(m_accountModel.data(), &DAccountModule::signalDtLastUpdate, this, [&]() {
        notifyPropertyChanged(getInterface(), "dtLastUpdate");
    });
}

QString DAccountService::getAccountInfo()
{
    //如果不在白名单内直接放回无效值
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return QString();
    }
    return m_accountModel->getAccountInfo();
}

QString DAccountService::getScheduleTypeList()
{
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return QString();
    }
    return m_accountModel->getScheduleTypeList();
}

QString DAccountService::getScheduleTypeByID(const QString &typeID)
{
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return QString();
    }
    return m_accountModel->getScheduleTypeByID(typeID);
}

QString DAccountService::createScheduleType(const QString &typeInfo)
{
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return QString();
    }
    return m_accountModel->createScheduleType(typeInfo);
}

bool DAccountService::updateScheduleType(const QString &typeInfo)
{
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return false;
    }
    return m_accountModel->updateScheduleType(typeInfo);
}

bool DAccountService::deleteScheduleTypeByID(const QString &typeID)
{
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return false;
    }
    return m_accountModel->deleteScheduleTypeByID(typeID);
}

bool DAccountService::scheduleTypeByUsed(const QString &typeID)
{
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return false;
    }
    return m_accountModel->scheduleTypeByUsed(typeID);
}

QString DAccountService::createSchedule(const QString &scheduleInfo)
{
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return QString();
    }
    return m_accountModel->createSchedule(scheduleInfo);
}

bool DAccountService::updateSchedule(const QString &scheduleInfo)
{
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return false;
    }
    return m_accountModel->updateSchedule(scheduleInfo);
}

QString DAccountService::getScheduleByScheduleID(const QString &scheduleID)
{
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return QString();
    }
    return m_accountModel->getScheduleByScheduleID(scheduleID);
}

bool DAccountService::deleteScheduleByScheduleID(const QString &scheduleID)
{
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return false;
    }
    return m_accountModel->deleteScheduleByScheduleID(scheduleID);
}

QString DAccountService::querySchedulesWithParameter(const QString &params)
{
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return QString();
    }
    return m_accountModel->querySchedulesWithParameter(params);
}

QString DAccountService::getSysColors()
{
    DServiceExitControl exitControl;
    if (!clientWhite(m_accountModel->account()->accountType())) {
        return QString();
    }
    return m_accountModel->getSysColors();
}

bool DAccountService::getExpand()
{
    return m_accountModel->getExpand();
}

void DAccountService::setExpand(const bool &isExpand)
{
    m_accountModel->setExpand(isExpand);
}

int DAccountService::getAccountState()
{
    return m_accountModel->getAccountState();
}

void DAccountService::setAccountState(const int accountState)
{
    m_accountModel->setAccountState(accountState);
}

int DAccountService::getSyncState()
{
    return m_accountModel->getSyncState();
}

QString DAccountService::getSyncFreq()
{
    return m_accountModel->getSyncFreq();
}

void DAccountService::setSyncFreq(const QString &freq)
{
    m_accountModel->setSyncFreq(freq);
}

QString DAccountService::getDtLastUpdate()
{
    return m_accountModel->getDtLastUpdate();
}

bool DAccountService::importSchedule(const QString &icsFilePath, const QString &typeID, const bool cleanExists)
{
    return m_accountModel->importSchedule(icsFilePath, typeID, cleanExists);
}

bool DAccountService::exportSchedule(const QString &icsFilePath, const QString &typeID)
{
    return m_accountModel->exportSchedule(icsFilePath, typeID);
}
