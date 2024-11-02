/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef LUNARCALENDARINFO_H
#define LUNARCALENDARINFO_H

#include <QObject>

class LunarCalendarInfo : public QObject
{
    Q_OBJECT
public:
    static LunarCalendarInfo *Instance();
    explicit LunarCalendarInfo(QObject *parent = 0);

    //计算是否闰年
    bool isLoopYear(int year);

    //计算指定年月该月共多少天
    int getMonthDays(int year, int month);

    //计算指定年月对应到该月共多少天
    int getTotalMonthDays(int year, int month);

    //计算指定年月对应星期几
    int getFirstDayOfWeek(int year, int month, bool FirstDayisSun);

    //计算国际节日
    QString getHoliday(int month, int day);

    //计算二十四节气
    QString getSolarTerms(int year, int month, int day);

    //计算农历节日(必须传入农历年份月份)
    QString getLunarFestival(int month, int day);

    //计算农历年 天干+地支+生肖
    QString getLunarYear(int year);

    //计算指定年月日农历信息,包括公历节日和农历节日及二十四节气
    void getLunarCalendarInfo(int year, int month, int day,
                              QString &strHoliday,
                              QString &strSolarTerms,
                              QString &strLunarFestival,
                              QString &strLunarYear,
                              QString &strLunarMonth,
                              QString &strLunarDay);

    //获取指定年月日农历信息
    QString getLunarInfo(int year, int month, int day, bool yearInfo, bool monthInfo, bool dayInfo);
    QString getLunarYearMonthDay(int year, int month, int day);
    QString getLunarMonthDay(int year, int month, int day);
    QString getLunarDay(int year, int month, int day);

private:
    static QScopedPointer<LunarCalendarInfo> self;

    QList<int> lunarCalendarTable;      //农历年表
    QList<int> springFestival;          //春节公历日期
    QList<int> lunarData;               //农历每月数据
    QList<int> chineseTwentyFourData;   //农历二十四节气数据
    QList<int> monthAdd;                //公历每月前面的天数

    QList<QString> listDayName;         //农历日期名称集合
    QList<QString> listMonthName;       //农历月份名称集合
    QList<QString> listSolarTerm;       //二十四节气名称集合

    QList<QString> listTianGan;         //天干名称集合
    QList<QString> listDiZhi;           //地支名称集合
    QList<QString> listShuXiang;        //属相名称集合
};

#endif // LUNARCALENDARINFO_H
