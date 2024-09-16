// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lunarmanager.h"

LunarManager::LunarManager(QObject* parent)
    : QObject(parent)
    , m_dbusInter(new LunarDBusInterface(this))
{
}

LunarManager* LunarManager::instace()
{
    static LunarManager lunarManager;
    return &lunarManager;
}

bool LunarManager::huangLiDay(quint32 year, quint32 month, quint32 day, CaHuangLiDayInfo& info)
{
    return m_dbusInter->huangLiDay(year, month, day, info);
}

bool LunarManager::huangLiDay(const QDate& date, CaHuangLiDayInfo& info)
{
    return huangLiDay(quint32(date.year()), quint32(date.month()), quint32(date.day()), info);
}

CaHuangLiDayInfo LunarManager::huangLiDay(const QDate& date)
{
    // 首先在缓存中查找是否存在该日期的农历信息，没有则通过dbus获取
    CaHuangLiDayInfo info;
    if (m_lunarInfoMap.contains(date)) {
        info = m_lunarInfoMap[date];
    } else {
        huangLiDay(date, info);
    }
    return info;
}
