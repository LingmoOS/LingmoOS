// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "accountitem.h"
#include "doanetworkdbus.h"

AccountItem::AccountItem(const DAccount::Ptr &account, QObject *parent)
    : QObject(parent)
    , m_account(account)
    , m_dbusRequest(new DbusAccountRequest(account->dbusPath(), account->dbusInterface(), this))
{
    initConnect();
}

void AccountItem::initConnect()
{
    connect(m_dbusRequest, &DbusAccountRequest::signalGetAccountInfoFinish, this, &AccountItem::slotGetAccountInfoFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalGetScheduleTypeListFinish, this, &AccountItem::slotGetScheduleTypeListFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalGetScheduleListFinish, this, &AccountItem::slotGetScheduleListFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalGetSysColorsFinish, this, &AccountItem::slotGetSysColorsFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalSearchScheduleListFinish, this, &AccountItem::slotSearchScheduleListFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalDtLastUpdate, this, &AccountItem::signalDtLastUpdate);
    connect(m_dbusRequest, &DbusAccountRequest::signalAccountStateChange, this, &AccountItem::signalAccountStateChange);
    connect(m_dbusRequest, &DbusAccountRequest::signalSyncStateChange, this, &AccountItem::slotSyncStateChange);
    connect(m_dbusRequest, &DbusAccountRequest::signalAccountStateChange, this, &AccountItem::slotAccountStateChange);
    connect(m_dbusRequest, &DbusAccountRequest::signalSearchUpdate, this, &AccountItem::slotSearchUpdata);
}

/**
 * @brief AccountItem::getSyncMsg
 * 同步状态码转状态说明
 * @param code 状态码
 * @return
 */
QString AccountItem::getSyncMsg(DAccount::AccountSyncState code)
{
    QString msg = "";
    switch (code) {
    case DAccount::Sync_Normal: msg = tr("Sync successful"); break;
    case DAccount::Sync_NetworkAnomaly: msg = tr("Network error"); break;
    case DAccount::Sync_ServerException: msg = tr("Server exception"); break;
    case DAccount::Sync_StorageFull: msg = tr("Storage full"); break;
    }
    return msg;
}

/**
 * @brief AccountItem::resetAccount
 * 重新获取帐户相关数据
 */
void AccountItem::resetAccount()
{
    querySchedulesWithParameter(QDate().currentDate().year());
    m_dbusRequest->getScheduleTypeList();
    m_dbusRequest->getSysColors();
}

/**
 * @brief AccountItem::getAccount
 * 获取帐户数据
 * @return
 */
DAccount::Ptr AccountItem::getAccount()
{
    return m_account;
}

//获取日程
QMap<QDate, DSchedule::List> AccountItem::getScheduleMap()
{
    return m_scheduleMap;
}

QMap<QDate, DSchedule::List> AccountItem::getSearchScheduleMap()
{
    return m_searchedScheduleMap;
}

/**
 * @brief getScheduleTypeList      获取日程类型信息集
 * @return
 */
DScheduleType::List AccountItem::getScheduleTypeList()
{
    DScheduleType::List list;
    for (DScheduleType::Ptr p : m_scheduleTypeList) {
        if (p->privilege() != DScheduleType::None) {
            list.push_back(p);
        }
    }
    return list;
}

//根据类型ID获取日程类型
DScheduleType::Ptr AccountItem::getScheduleTypeByID(const QString &typeID)
{
    for (DScheduleType::Ptr p : m_scheduleTypeList) {
        if (p->typeID() == typeID) {
            return p;
        }
    }
    return nullptr;
}

DScheduleType::Ptr AccountItem::getScheduleTypeByName(const QString &typeName)
{
    for (DScheduleType::Ptr p : m_scheduleTypeList) {
        if (p->typeName() == typeName || p->displayName() == typeName) {
            return p;
        }
    }
    return nullptr;
}

DTypeColor::List AccountItem::getColorTypeList()
{
    return m_typeColorList;
}

/**
 * @brief AccountItem::isCanSyncShedule
 * 获取日程是否可以同步
 * @return
 */
bool AccountItem::isCanSyncShedule()
{
    if (getAccount()->accountType() != DAccount::Account_UnionID) {
        return true;
    }
    DOANetWorkDBus netManger;
    return getAccount()->accountState().testFlag(DAccount::Account_Calendar)
           && getAccount()->accountState().testFlag(DAccount::Account_Open) && netManger.getNetWorkState() == DOANetWorkDBus::Active;
}

/**
 * @brief AccountItem::isCanSyncSetting
 * 获取通用设置是否可以同步
 * @return
 */
bool AccountItem::isCanSyncSetting()
{
    if (!getAccount().isNull() && getAccount()->accountType() != DAccount::Account_UnionID) {
        return true;
    }
    DOANetWorkDBus netManger;
    return getAccount()->accountState().testFlag(DAccount::Account_Setting)
           && getAccount()->accountState().testFlag(DAccount::Account_Open) && netManger.getNetWorkState() == DOANetWorkDBus::Active;
}

bool AccountItem::isEnableForUosAccount()
{
    if (getAccount()->accountType() != DAccount::Account_UnionID) {
        return false;
    }

    DOANetWorkDBus netManger;
    return getAccount()->accountState().testFlag(DAccount::Account_Open) && netManger.getNetWorkState() == DOANetWorkDBus::Active;
}

/**
 * @brief AccountItem::setAccountExpandStatus
 * 更新帐户列表展开状态
 * @param expandStatus 展开状态
 */
void AccountItem::setAccountExpandStatus(bool expandStatus)
{
    m_account->setIsExpandDisplay(expandStatus);
    m_dbusRequest->setAccountExpandStatus(expandStatus);
}

void AccountItem::setAccountState(DAccount::AccountStates state)
{
    m_account->setAccountState(state);
    m_dbusRequest->setAccountState(state);
    emit signalAccountStateChange();
}

void AccountItem::setSyncFreq(DAccount::SyncFreqType freq)
{
    m_account->setSyncFreq(freq);
    QString syncFreq = DAccount::syncFreqToJsonString(m_account);
    m_dbusRequest->setSyncFreq(syncFreq);
}

DAccount::AccountStates AccountItem::getAccountState()
{
    return m_dbusRequest->getAccountState();
}

bool AccountItem::getSyncState()
{
    return m_dbusRequest->getSyncState();
}

DAccount::SyncFreqType AccountItem::getSyncFreq()
{
    QString syncFreq = m_dbusRequest->getSyncFreq();
    DAccount::syncFreqFromJsonString(m_account, syncFreq);
    return m_account->syncFreq();
}

/**
 * @brief AccountItem::createJobType
 * 创建日程类型
 * @param typeInfo 日程类型数据
 * @param callback
 */
void AccountItem::createJobType(const DScheduleType::Ptr &typeInfo, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    typeInfo->setPrivilege(DScheduleType::User);
    m_dbusRequest->createScheduleType(typeInfo);
}

/**
 * @brief AccountItem::updateScheduleType
 * 更新类型信息
 * @param typeInfo 新的日程类型数据
 * @param callback
 */
void AccountItem::updateScheduleType(const DScheduleType::Ptr &typeInfo, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->updateScheduleType(typeInfo);
}

/**
 * @brief AccountItem::updateScheduleTypeShowState
 * 更新类型显示状态
 * @param scheduleInfo
 * @param callback
 */
void AccountItem::updateScheduleTypeShowState(const DScheduleType::Ptr &scheduleInfo, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->updateScheduleTypeShowState(scheduleInfo);
}

/**
 * @brief AccountItem::deleteScheduleTypeByID
 * 根据类型ID删除日程类型
 * @param typeID    日程类型id
 * @param callback 回调函数
 */
void AccountItem::deleteScheduleTypeByID(const QString &typeID, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->deleteScheduleTypeByID(typeID);
}

/**
 * @brief AccountItem::scheduleTypeIsUsed
 * 类型是否被日程使用
 * @param typeID 日程类型id
 * @param callback 回调函数
 */
bool AccountItem::scheduleTypeIsUsed(const QString &typeID)
{
    return m_dbusRequest->scheduleTypeByUsed(typeID);
}

/**
 * @brief AccountItem::createSchedule
 * 创建日程
 * @param scheduleInfo  日程数据
 * @param callback 回调函数
 */
void AccountItem::createSchedule(const DSchedule::Ptr &scheduleInfo, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->createSchedule(scheduleInfo);
}

/**
 * @brief AccountItem::updateSchedule
 * 更新日程
 * @param scheduleInfo 新的日程数据
 * @param callback 回调函数
 */
void AccountItem::updateSchedule(const DSchedule::Ptr &scheduleInfo, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->updateSchedule(scheduleInfo);
}

DSchedule::Ptr AccountItem::getScheduleByScheduleID(const QString &scheduleID)
{
    return m_dbusRequest->getScheduleByScheduleID(scheduleID);
}

/**
 * @brief AccountItem::deleteScheduleByID
 * 根据日程ID删除日程
 * @param schedule ID日程id
 * @param callback 回调函数
 */
void AccountItem::deleteScheduleByID(const QString &scheduleID, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->deleteScheduleByScheduleID(scheduleID);
}

/**
 * @brief AccountItem::deleteSchedulesByTypeID
 * 根据类型ID删除日程
 * @param typeID 日程类型id
 * @param callback 回调函数
 */
void AccountItem::deleteSchedulesByTypeID(const QString &typeID, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->deleteSchedulesByScheduleTypeID(typeID);
}

void AccountItem::querySchedulesWithParameter(const int year, CallbackFunc callback)
{
    QDateTime start = QDateTime(QDate(year, 1, 1));
    QDateTime end = QDateTime(QDate(year, 12, 31));
    querySchedulesWithParameter(start, end, callback);
}

void AccountItem::querySchedulesWithParameter(const QDateTime &start, const QDateTime &end, CallbackFunc callback)
{
    querySchedulesWithParameter("", start, end, callback);
}

void AccountItem::querySchedulesWithParameter(const QString &key, const QDateTime &start, const QDateTime &end, CallbackFunc callback)
{
    DScheduleQueryPar::Ptr ptr;
    ptr.reset(new DScheduleQueryPar);
    ptr->setKey(key);
    ptr->setDtStart(start);
    ptr->setDtEnd(end);
    querySchedulesWithParameter(ptr, callback);
}

/**
 * @brief AccountItem::querySchedulesWithParameter
 * 根据查询条件查询数据
 * @param params 查询条件
 * @param callback 回调函数
 */
void AccountItem::querySchedulesWithParameter(const DScheduleQueryPar::Ptr &params, CallbackFunc callback)
{
    m_preQuery = params;
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->querySchedulesWithParameter(params);
}

QString AccountItem::querySchedulesByExternal(const QString &key, const QDateTime &start, const QDateTime &end)
{
    DScheduleQueryPar::Ptr ptr;
    ptr.reset(new DScheduleQueryPar);
    ptr->setKey(key);
    ptr->setDtStart(start);
    ptr->setDtEnd(end);
    QString json;
    m_dbusRequest->querySchedulesByExternal(ptr, json);
    return json;
}

bool AccountItem::querySchedulesByExternal(const QString &key, const QDateTime &start, const QDateTime &end, QMap<QDate, DSchedule::List> &out)
{
    DScheduleQueryPar::Ptr ptr;
    ptr.reset(new DScheduleQueryPar);
    ptr->setKey(key);
    ptr->setDtStart(start);
    ptr->setDtEnd(end);
    QString json;
    if (m_dbusRequest->querySchedulesByExternal(ptr, json)) {
        out = DSchedule::fromMapString(json);
        return true;
    }
    return false;
}

/**
 * @brief AccountItem::slotGetAccountInfoFinish
 * 获取帐户信息完成事件
 * @param account 帐户数据
 */
void AccountItem::slotGetAccountInfoFinish(DAccount::Ptr account)
{
    m_account = account;
    emit signalAccountDataUpdate();
}

/**
 * @brief AccountItem::slotGetScheduleTypeListFinish
 * 获取日程类型数据完成事件
 * @param scheduleTypeList 日程类型数据
 */
void AccountItem::slotGetScheduleTypeListFinish(DScheduleType::List scheduleTypeList)
{
    m_scheduleTypeList = scheduleTypeList;
    emit signalScheduleTypeUpdate();
}

/**
 * @brief AccountItem::slotGetScheduleListFinish
 * 获取日程数据完成事件
 * @param map 日程数据
 */
void AccountItem::slotGetScheduleListFinish(QMap<QDate, DSchedule::List> map)
{
    m_scheduleMap = map;
    emit signalScheduleUpdate();
}

/**
 * @brief AccountItem::slotSearchScheduleListFinish
 * 搜索日程数据完成事件
 * @param map 日程数据
 */
void AccountItem::slotSearchScheduleListFinish(QMap<QDate, DSchedule::List> map)
{
    m_searchedScheduleMap = map;
    emit signalSearchScheduleUpdate();
}

/**
 * @brief AccountItem::slotGetSysColorsFinish
 * 获取系统颜色完成事件
 */
void AccountItem::slotGetSysColorsFinish(DTypeColor::List typeColorList)
{
    m_typeColorList = typeColorList;
}

void AccountItem::slotAccountStateChange(DAccount::AccountStates state)
{
    getAccount()->setAccountState(state);
    emit signalAccountStateChange();
}

void AccountItem::slotSyncStateChange(DAccount::AccountSyncState state)
{
    getAccount()->setSyncState(state);
    emit signalSyncStateChange(state);
}

QString AccountItem::getDtLastUpdate()
{
    return m_dbusRequest->getDtLastUpdate();
}

void AccountItem::slotSearchUpdata()
{
    //如果存在查询则更新查询
    if (nullptr != m_preQuery) {
        querySchedulesWithParameter(m_preQuery);
    }
}

void AccountItem::importSchedule(QString icsFilePath, QString typeID, bool cleanExists, CallbackFunc func)
{
    m_dbusRequest->setCallbackFunc(func);
    m_dbusRequest->importSchedule(icsFilePath, typeID, cleanExists);
}

void AccountItem::exportSchedule(QString icsFilePath, QString typeID, CallbackFunc func)
{
    m_dbusRequest->setCallbackFunc(func);
    m_dbusRequest->exportSchedule(icsFilePath, typeID);
}
