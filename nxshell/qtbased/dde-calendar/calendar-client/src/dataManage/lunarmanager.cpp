// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lunarmanager.h"

LunarManager::LunarManager(QObject *parent) : QObject(parent)
  , m_dbusRequest(new DbusHuangLiRequest)
{

}

LunarManager* LunarManager::getInstace()
{
    static LunarManager lunarManager;
    return &lunarManager;
}

/**
 * @brief LunarManager::getFestivalMonth
 * 按月获取节假日信息
 * @param year 年
 * @param month 月
 * @param festivalInfo 数据保存位置
 * @return
 */
bool LunarManager::getFestivalMonth(quint32 year, quint32 month, FestivalInfo& festivalInfo)
{
    return m_dbusRequest->getFestivalMonth(year, month, festivalInfo);
}

/**
 * @brief LunarManager::getFestivalMonth
 * 按月获取节假日信息
 * @param date 月所在日期
 * @param festivalInfo 数据储存位置
 * @return  请求成功状态
 */
bool LunarManager::getFestivalMonth(const QDate &date, FestivalInfo& festivalInfo)
{
    return m_dbusRequest->getFestivalMonth(quint32(date.year()), quint32(date.month()), festivalInfo);
}

/**
 * @brief LunarManager::getHuangLiDay
 * 按天获取黄历信息
 * @param year 年
 * @param month 月
 * @param day 日
 * @param info 数据储存位置
 * @return  请求成功状态
 */
bool LunarManager::getHuangLiDay(quint32 year, quint32 month, quint32 day, CaHuangLiDayInfo &info)
{
    return m_dbusRequest->getHuangLiDay(year, month, day, info);
}

/**
 * @brief LunarManager::getHuangLiDay
 * 按天获取农历信息
 * @param date 请求日期
 * @param info 数据储存位置
 * @return 请求成功状态
 */
bool LunarManager::getHuangLiDay(const QDate &date, CaHuangLiDayInfo &info)
{
    return getHuangLiDay(quint32(date.year()), quint32(date.month()), quint32(date.day()), info);
}

/**
 * @brief LunarManager::getHuangLiMonth
 * 按月获取农历信息
 * @param year 年
 * @param month 月
 * @param info 日
 * @param fill
 * @return  请求成功状态
 */
bool LunarManager::getHuangLiMonth(quint32 year, quint32 month, CaHuangLiMonthInfo &info, bool fill)
{
    return m_dbusRequest->getHuangLiMonth(year, month, fill, info);
}

/**
 * @brief LunarManager::getHuangLiMonth
 * 按月获取农历信息
 * @param date 请求日期
 * @param info 数据储存位置
 * @return 请求成功状态
 */
bool LunarManager::getHuangLiMonth(const QDate &date, CaHuangLiMonthInfo &info, bool fill)
{
    return getHuangLiMonth(quint32(date.year()), quint32(date.month()), info, fill);
}

/**
 * @brief LunarManager::getHuangLiShortName
 * 获取当天的农历月日期和日日期名
 * @param date 请求日期
 * @return 农历名
 */
QString LunarManager::getHuangLiShortName(const QDate &date)
{
    CaHuangLiDayInfo info = getHuangLiDay(date);
    return info.mLunarMonthName + info.mLunarDayName;
}

/**
 * @brief LunarManager::queryLunarInfo
 * 查询农历信息
 * @param startDate 开始时间
 * @param stopDate 结束时间
 */
void LunarManager::queryLunarInfo(const QDate &startDate, const QDate &stopDate)
{
    QMap<QDate, CaHuangLiDayInfo> lunarInfoMap;
    CaHuangLiMonthInfo monthInfo;
    const int offsetMonth = (stopDate.year() - startDate.year()) * 12 + stopDate.month() - startDate.month();
    //获取开始时间至结束时间所在月的农历和节假日信息
    for (int i = 0; i <= offsetMonth; ++i) {
        monthInfo.clear();
        QDate beginDate = startDate.addMonths(i);
        getHuangLiMonth(beginDate, monthInfo);

        QDate getDate(beginDate.year(), beginDate.month(), 1);
        for (int j = 0; j < monthInfo.mDays; ++j) {
            lunarInfoMap[getDate.addDays(j)] = monthInfo.mCaLunarDayInfo.at(j);
        }
    }
    m_lunarInfoMap = lunarInfoMap;
}

/**
 * @brief LunarManager::queryFestivalInfo
 * 查询节假日信息
 * @param startDate 开始时间
 * @param stopDate 结束时间
 */
void LunarManager::queryFestivalInfo(const QDate &startDate, const QDate &stopDate)
{
    QVector<FestivalInfo> festivallist{};

    const int offsetMonth = (stopDate.year() - startDate.year()) * 12 + stopDate.month() - startDate.month();

    for (int i = 0; i <= offsetMonth; ++i) {
        FestivalInfo info;
        QDate beginDate = startDate.addMonths(i);
        if (getFestivalMonth(beginDate, info)) {
            festivallist.push_back(info);
        }
    }

    m_festivalDateMap.clear();
    for (FestivalInfo info : festivallist) {
        for (HolidayInfo h : info.listHoliday) {
            m_festivalDateMap[h.date] = h.status;
        }
    }
}

/**
 * @brief LunarManager::getHuangLiDay
 * 获取农历信息
 * @param date 获取日期
 * @return
 */
CaHuangLiDayInfo LunarManager::getHuangLiDay(const QDate &date)
{
    //首先在缓存中查找是否存在该日期的农历信息，没有则通过dbus获取
    CaHuangLiDayInfo info;
    if (m_lunarInfoMap.contains(date)) {
        info = m_lunarInfoMap[date];
    } else {
        getHuangLiDay(date, info);
    }
    return info;
}

/**
 * @brief LunarManager::getHuangLiDayMap
 * 获取一定时间范围内的农历数据
 * @param startDate 开始时间
 * @param stopDate 结束时间
 * @return 农历信息
 */
QMap<QDate, CaHuangLiDayInfo> LunarManager::getHuangLiDayMap(const QDate &startDate, const QDate &stopDate)
{
    QMap<QDate, CaHuangLiDayInfo> lunarInfoMap;
    auto iterator = m_lunarInfoMap.begin();
    while(iterator != m_lunarInfoMap.end()) {
        if (iterator.key() >= startDate || iterator.key() <= stopDate) {
            iterator.value();
            lunarInfoMap[iterator.key()] = iterator.value();
        }
        iterator++;
    }
    return lunarInfoMap;
}

/**
 * @brief LunarManager::getFestivalInfoDateMap
 * 获取节假日日期信息
 * @param startDate 开始时间
 * @param stopDate 结束时间
 * @return 节假日日期信息
 */
QMap<QDate, int> LunarManager::getFestivalInfoDateMap(const QDate &startDate, const QDate &stopDate)
{
    QMap<QDate, int> festivalDateMap;
    auto iterator = m_festivalDateMap.begin();
    while(iterator != m_festivalDateMap.end()) {
        if (iterator.key() >= startDate || iterator.key() <= stopDate) {
            iterator.value();
            festivalDateMap[iterator.key()] = iterator.value();
        }
        iterator++;
    }
    return festivalDateMap;
}
