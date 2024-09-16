// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "accountmanager.h"

AccountManager *AccountManager::m_accountManager = nullptr;
AccountManager::AccountManager(QObject *parent)
    : QObject(parent)
    , m_dbusRequest(new DbusAccountManagerRequest(this))
{
    initConnect();
    m_dbusRequest->clientIsShow(true);
    m_isSupportUid  = m_dbusRequest->getIsSupportUid();
}

void AccountManager::initConnect()
{
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalGetAccountListFinish, this, &AccountManager::slotGetAccountListFinish);
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalGetGeneralSettingsFinish, this, &AccountManager::slotGetGeneralSettingsFinish);
}

bool AccountManager::getIsSupportUid() const
{
    return m_isSupportUid;
}

AccountManager::~AccountManager()
{
    m_dbusRequest->clientIsShow(false);
}

AccountManager *AccountManager::getInstance()
{
    static AccountManager m_accountManager;
    return &m_accountManager;
}

/**
 * @brief AccountManager::getAccountList
 * 获取帐户列表
 * @return 帐户列表
 */
QList<AccountItem::Ptr> AccountManager::getAccountList()
{
    QList<QSharedPointer<AccountItem>> accountList;
    if (nullptr != m_localAccountItem.data()) {
        accountList.append(m_localAccountItem);
    }

    if (nullptr != m_unionAccountItem.data()) {
        accountList.append(m_unionAccountItem);
    }
    return accountList;
}

/**
 * @brief AccountManager::getLocalAccountItem
 * 获取本地帐户
 * @return
 */
AccountItem::Ptr AccountManager::getLocalAccountItem()
{
    return m_localAccountItem;
}

/**
 * @brief AccountManager::getUnionAccountItem
 * 获取unionID帐户
 * @return
 */
AccountItem::Ptr AccountManager::getUnionAccountItem()
{
    return m_unionAccountItem;
}

DScheduleType::Ptr AccountManager::getScheduleTypeByScheduleTypeId(const QString &schduleTypeId)
{
    DScheduleType::Ptr type = nullptr;
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        type = p->getScheduleTypeByID(schduleTypeId);
        if (nullptr != type) {
            break;
        }
    }
    return type;
}

AccountItem::Ptr AccountManager::getAccountItemByScheduleTypeId(const QString &schduleTypeId)
{
    DScheduleType::Ptr type = getScheduleTypeByScheduleTypeId(schduleTypeId);
    if (nullptr == type) {
        return nullptr;
    }
    return getAccountItemByAccountId(type->accountID());
}

AccountItem::Ptr AccountManager::getAccountItemByAccountId(const QString &accountId)
{
    AccountItem::Ptr account = nullptr;
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        if (p->getAccount()->accountID() == accountId) {
            account = p;
            break;
        }
    }
    return account;
}

AccountItem::Ptr AccountManager::getAccountItemByAccountName(const QString &accountName)
{
    AccountItem::Ptr account = nullptr;
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        if (p->getAccount()->accountName() == accountName) {
            account = p;
            break;
        }
    }
    return account;
}

DCalendarGeneralSettings::Ptr AccountManager::getGeneralSettings()
{
    return m_settings;
}

/**
 * @brief AccountManager::resetAccount
 * 重置帐户信息
 */
void AccountManager::resetAccount()
{
    m_localAccountItem.clear();
    m_unionAccountItem.clear();
    m_dbusRequest->getAccountList();
    m_dbusRequest->getCalendarGeneralSettings();
}

/**
 * @brief AccountManager::downloadByAccountID
 * 根据帐户ID下拉数据
 * @param accountID 帐户id
 * @param callback 回调函数
 */
void AccountManager::downloadByAccountID(const QString &accountID, CallbackFunc callback)
{
    emit signalSyncNum();
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->downloadByAccountID(accountID);
}

/**
 * @brief AccountManager::uploadNetWorkAccountData
 * 更新网络帐户数据
 * @param callback 回调函数
 */
void AccountManager::uploadNetWorkAccountData(CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->uploadNetWorkAccountData();
}

void AccountManager::setFirstDayofWeek(int value)
{
    m_settings->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(value));
    m_dbusRequest->setFirstDayofWeek(value);
}

void AccountManager::setTimeFormatType(int value)
{
    m_settings->setTimeShowType(static_cast<DCalendarGeneralSettings::TimeShowType>(value));
    m_dbusRequest->setTimeFormatType(value);
}

// 设置一周首日来源
void AccountManager::setFirstDayofWeekSource(DCalendarGeneralSettings::GeneralSettingSource value)
{
    m_dbusRequest->setFirstDayofWeekSource(value);
};

// 设置时间显示格式来源
void AccountManager::setTimeFormatTypeSource(DCalendarGeneralSettings::GeneralSettingSource value)
{
    m_dbusRequest->setTimeFormatTypeSource(value);
};

// 获取一周首日来源
DCalendarGeneralSettings::GeneralSettingSource AccountManager::getFirstDayofWeekSource()
{
    return m_dbusRequest->getFirstDayofWeekSource();
};

// 获取时间显示格式来源
DCalendarGeneralSettings::GeneralSettingSource AccountManager::getTimeFormatTypeSource()
{
    return m_dbusRequest->getTimeFormatTypeSource();
};

/**
 * @brief login
 * 帐户登录
 */
void AccountManager::login()
{
    m_dbusRequest->login();
}

/**
 * @brief loginout
 * 帐户登出
 */
void AccountManager::loginout()
{
    m_dbusRequest->logout();
}

/**
 * @brief AccountManager::slotGetAccountListFinish
 * 获取帐户信息完成事件
 * @param accountList 帐户列表
 */
void AccountManager::slotGetAccountListFinish(DAccount::List accountList)
{
    bool hasUnionAccount = false;
    for (DAccount::Ptr account : accountList) {
        if (account->accountType() == DAccount::Account_Local) {
            QString localName = tr("Local account");
            if (!gAccountManager->getIsSupportUid()) {
                localName = tr("Event types");
            }
            account->setAccountName(localName);
            if (!m_localAccountItem) {
                m_localAccountItem.reset(new AccountItem(account, this));
            }
            m_localAccountItem->resetAccount();

        } else if (account->accountType() == DAccount::Account_UnionID) {
            hasUnionAccount = true;
            if (!m_unionAccountItem) {
                m_unionAccountItem.reset(new AccountItem(account, this));
                m_unionAccountItem->resetAccount();
            } else if (m_unionAccountItem && m_unionAccountItem->getAccount()->accountID() != account->accountID()) {
                emit m_unionAccountItem->signalLogout(m_unionAccountItem->getAccount()->accountType());
                m_unionAccountItem.reset(new AccountItem(account, this));
                m_unionAccountItem->resetAccount();
            }
        }
    }
    if (!hasUnionAccount && m_unionAccountItem) {
        emit m_unionAccountItem->signalLogout(m_unionAccountItem->getAccount()->accountType());
        m_unionAccountItem.reset(nullptr);
    }

    for (AccountItem::Ptr p : getAccountList()) {
        connect(p.data(), &AccountItem::signalScheduleUpdate, this, &AccountManager::signalScheduleUpdate);
        connect(p.data(), &AccountItem::signalSearchScheduleUpdate, this, &AccountManager::signalSearchScheduleUpdate);
        connect(p.data(), &AccountItem::signalScheduleTypeUpdate, this, &AccountManager::signalScheduleTypeUpdate);
        connect(p.data(), &AccountItem::signalLogout, this, &AccountManager::signalLogout);
        connect(p.data(), &AccountItem::signalAccountStateChange, this, &AccountManager::signalAccountStateChange);
    }

    emit signalAccountUpdate();
}

/**
 * @brief AccountManager::slotGetGeneralSettingsFinish
 * 获取通用设置完成事件
 * @param ptr 通用设置数据
 */
void AccountManager::slotGetGeneralSettingsFinish(DCalendarGeneralSettings::Ptr ptr)
{
    m_settings = ptr;
    emit signalDataInitFinished();
    emit signalGeneralSettingsUpdate();
}
