// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LUNARCALENDAR_H
#define LUNARCALENDAR_H
#include "lunarandfestival.h"
#include "method_interface.h"

#include <QObject>
#include <QMap>

class LunarCalendar
{
public:
    static LunarCalendar *GetLunarCalendar(qint32 year);
    //程序退出时情况数据
    static void LogOffEmptyData();
    lunarInfo SolarDayToLunarDay(qint32 month, qint32 day);

private:
    explicit LunarCalendar(qint32 year);
    void calcProcData();
    void fillMonths();
    void calcLeapMonth();
    qint32 getSolarTermInfo(qint32 month, qint32 day) const;

public:
private:
    static QMap<int, LunarCalendar *> glYearCache;
    int Year; // 公历年份
    QVector<double> SolarTermJDs; // 相关的 25 节气 北京时间 儒略日
    QVector<QDateTime> SolarTermTimes; // 对应 SolarTermJDs 转换为 time.Time 的时间
    QVector<double> NewMoonJDs; // 相关的 15 个朔日 北京时间 儒略日
    QVector<lunarInfo> Months; // 月
    QVector<int> solarTermYearDays; // 十二节的 yearDay 列表
};

#endif // LUNARCALENDAR_H
