// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "accountitem.h"

AccountItem::AccountItem(const DAccount::Ptr &account, QObject *parent)
    : QObject(parent)
    , m_account(account)
    , m_dbusRequest(new DbusAccountRequest(account->dbusPath(), account->dbusInterface(), this))
{
    initConnect();
}

void AccountItem::initConnect()
{
    connect(m_dbusRequest, &DbusAccountRequest::signalCreateScheduleFinish, this, &AccountItem::signalCreateFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalGetScheduleFinish, this, &AccountItem::signalGetScheduleFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalGetAccountInfoFinish, this, &AccountItem::slotGetAccountInfoFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalGetScheduleTypeListFinish, this, &AccountItem::slotGetScheduleTypeListFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalGetScheduleListFinish, this, &AccountItem::slotGetScheduleListFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalGetSysColorsFinish, this, &AccountItem::slotGetSysColorsFinish);
}

/**
 * @brief AccountItem::resetAccount
 * 重新获取账户相关数据
 */
void AccountItem::resetAccount()
{
    m_scheduleTypeList = m_dbusRequest->getScheduleTypeList();
    m_typeColorList = m_dbusRequest->getSysColors();
}

/**
 * @brief AccountItem::getAccount
 * 获取账户数据
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

DSchedule::Ptr AccountItem::getScheduleByID(const QString &scheduleID)
{
    return m_dbusRequest->getScheduleByID(scheduleID);
}

DTypeColor::List AccountItem::getColorTypeList()
{
    return m_typeColorList;
}

/**
 * @brief AccountItem::updateAccountInfo
 * 更新账户信息
 * @param callback
 */
void AccountItem::updateAccountInfo(CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->updateAccountInfo(m_account);
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
void AccountItem::scheduleTypeIsUsed(const QString &typeID, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->scheduleTypeByUsed(typeID);
}

/**
 * @brief AccountItem::createSchedule
 * 创建日程
 * @param scheduleInfo  日程数据
 * @param callback 回调函数
 */
QString AccountItem::createSchedule(const DSchedule::Ptr &scheduleInfo)
{
    return m_dbusRequest->createSchedule(scheduleInfo);
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

/**
 * @brief AccountItem::querySchedulesWithParameter
 * 根据查询条件查询数据
 * @param params 查询条件
 * @param callback 回调函数
 */
DSchedule::Map AccountItem::querySchedulesWithParameter(const DScheduleQueryPar::Ptr &params)
{
    return m_dbusRequest->querySchedulesWithParameter(params);
}

/**
 * @brief AccountItem::monitorScheduleTypeData
 * 监听日程类型数据完成事件
 * @param callback 回调函数
 */
void AccountItem::monitorScheduleTypeData(Func callback)
{
    auto statusIterator = m_dataStatus.find("ScheduleType");
    if (statusIterator != m_dataStatus.end() && statusIterator.value()) {
        callback();
    } else {
        m_dataStatus.insert("ScheduleType", false);
    }
    auto iterator = m_callbackMap.find("ScheduleType");
    QList<Func> funcList;
    if (iterator == m_callbackMap.end()) {
        funcList.append(callback);
    } else {
        funcList = iterator.value();
    }
    m_callbackMap.insert("ScheduleType", funcList);
}

/**
 * @brief AccountItem::slotGetAccountInfoFinish
 * 获取账户信息完成事件
 * @param account 账户数据
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

    m_dataStatus.insert("ScheduleType", true);
    auto iterator = m_callbackMap.find("ScheduleType");
    if (iterator != m_callbackMap.end()) {
        for (Func func : iterator.value()) {
            func();
        }
    }
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
