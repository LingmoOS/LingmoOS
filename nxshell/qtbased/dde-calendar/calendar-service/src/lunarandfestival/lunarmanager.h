// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LUNARMANAGER_H
#define LUNARMANAGER_H
#include "lunarandfestival.h"
#include "method_interface.h"

stLunarDayInfo SolarToLunar(qint32 year, qint32 month, qint32 day);
SolarMonthInfo GetSolarMonthCalendar(qint32 year, qint32 month, bool fill);
LunarMonthInfo GetLunarMonthCalendar(qint32 year, qint32 month, bool fill);
LunarMonthInfo GetLunarMonthCalendar(const SolarMonthInfo &solarMonth);
QList<stDay> GetMonthDays(qint32 year, qint32 month, qint32 start, qint32 days);
QList<int> GetPreMonth(qint32 year, qint32 month);
QList<int> GetNextMonth(qint32 year, qint32 month);
QList<stDayFestival> GetFestivalsInRange(const QDateTime &start, const QDateTime &end);
QList<stDayFestival> FilterDayFestival(QList<stDayFestival> &festivaldays, const QString &querykey);
//清空黄历数据
void logOffEmptyData();

#endif // LUNARMANAGER_H
