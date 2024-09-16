// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_method_interface.h"
#include <QVector>
#include <QDebug>
#include <QtGlobal>

test_method_interface::test_method_interface()
{

}

//QString GetLunarMonthName(int lunarmonthname, bool isleap)
TEST_F(test_method_interface, GetLunarMonthName)
{
    // 腊月
    int lunarmonthname = 12;
    bool isleap = false;
    QString lunarMName = GetLunarMonthName(lunarmonthname, isleap);
    assert("腊月" == lunarMName);

    // 闰腊月
    isleap = true;
    lunarMName = GetLunarMonthName(lunarmonthname, isleap);
    assert("闰腊月" == lunarMName);
}

//QString GetLunarDayName(int lundayname)
TEST_F(test_method_interface, GetLunarDayName)
{
    // "初八"
    int lundayname = 8;
    QString lunarDayName = GetLunarDayName(lundayname);
    //qCInfo(CommonLogger) << lunarDayName;
    assert("初八" == lunarDayName);

    // "廿五"
    lundayname = 25;
    lunarDayName = GetLunarDayName(lundayname);
    //qCInfo(CommonLogger) << lunarDayName;
    assert("廿五" == lunarDayName);
}

//QString GetLunarDayFestival(int monthname, int lunarday, int lunarmonthdays, int solarterm)
TEST_F(test_method_interface, GetLunarDayFestival)
{
    // "除夕"
    int monthname = 12;
    int lunarday = 30;
    int lunarmonthdays = 30;
    int solarterm = 1;
    QString dayFestival = GetLunarDayFestival(monthname, lunarday, lunarmonthdays, solarterm);
    //qCInfo(CommonLogger) << dayFestival;
    assert("除夕" == dayFestival);

    // "清明节"
    monthname = 4;
    lunarday = 2;
    lunarmonthdays = 23;
    solarterm = 1;
    dayFestival = GetLunarDayFestival(monthname, lunarday, lunarmonthdays, solarterm);
    //qCInfo(CommonLogger) << dayFestival;
    assert("清明节" == dayFestival);

    // ""
    monthname = 5;
    lunarday = 1;
    lunarmonthdays = 13;
    solarterm = 0;
    dayFestival = GetLunarDayFestival(monthname, lunarday, lunarmonthdays, solarterm);
    //qCInfo(CommonLogger) << dayFestival;
    assert("" == dayFestival);
}

//QString GetSolarTermName(int order)
TEST_F(test_method_interface, GetSolarTermName)
{
    // "小满"
    QString stName = GetSolarTermName(4);
    assert("小满" == stName);

    // ""
    stName = GetSolarTermName(24);
    assert("" == stName);
}

//QString GetGanZhiMonth(int year, int monthzhi)
TEST_F(test_method_interface, GetGanZhiMonth)
{
    QString ganzhi = GetGanZhiMonth(2020, 12);
    assert("戊子" == ganzhi);
}

//QString GetGanZhiYear(int lunaryear)
TEST_F(test_method_interface, GetGanZhiYear)
{
    // "庚子"
    QString ganzhi = GetGanZhiYear(2020);
    assert("庚子" == ganzhi);
}

//QString GetGanZhiDay(int year, int month, int day)
TEST_F(test_method_interface, GetGanZhiDay)
{
    // "己丑"
    QString ganzhi = GetGanZhiDay(2020, 12, 12);
    assert("己丑" == ganzhi);
}

//QString GetYearZodiac(int lunaryear)
TEST_F(test_method_interface, GetYearZodiac)
{
    // "鼠"
    QString Zodiac = GetYearZodiac(2020);
    assert("鼠" == Zodiac);

    // "猪"
    Zodiac = GetYearZodiac(2019);
    assert("猪" == Zodiac);
}

//QVector<double> get25SolarTermJDs(int year, int start)
TEST_F(test_method_interface, get25SolarTermJDs)
{
    QVector<double> jds = get25SolarTermJDs(0, 0);
}

//double GetSolarTermJD(int year, int order)
TEST_F(test_method_interface, GetSolarTermJD)
{
    //春分 3月20日
    assert(2.45893e+06 > GetSolarTermJD(2020, 0));
}

//bool IsLeapYear(int year)
TEST_F(test_method_interface, IsLeapYear)
{
    assert(true == IsLeapYear(2020) && false == IsLeapYear(2019));
}

// int GetSolarMonthDays(int year, int month)
TEST_F(test_method_interface, GetSolarMonthDays)
{
    assert(29 == GetSolarMonthDays(2020, 2));
}

//int GetWeekday(int y, int m, int d)
TEST_F(test_method_interface, GetWeekday)
{
    assert(6 == GetWeekday(2020, 2, 29));
}

//double DmsToDegrees(int degrees, int mintues, double seconds)
TEST_F(test_method_interface, DmsToDegrees)
{
    assert(qAbs(6.05 - DmsToDegrees(4, 3, 7200)) < 0.001);
}

//double DmsToSeconds(int d, int m, double s)
TEST_F(test_method_interface, DmsToSeconds)
{
    assert(qAbs(21780 - DmsToSeconds(4, 3, 7200)) < 0.001);
}

//double DmsToRadians(int d, int m, int s)
TEST_F(test_method_interface, DmsToRadians)
{
    assert(0.105592 <= DmsToRadians(4, 3, 7200));
}

//QDateTime GetDateTimeFromJulianDay(double jd)
TEST_F(test_method_interface, GetDateTimeFromJulianDay)
{
    QString strJulianDay = "周五 3月 20 03:49:33 2020 GMT";
    //qCInfo(CommonLogger) << julianDay.toString();
    assert(strJulianDay.contains(GetDateTimeFromJulianDay(GetSolarTermJD(2020, 0)).toString()));
}

//double GetDeltaT(int year, int month)
TEST_F(test_method_interface, GetDeltaT)
{
    const int count = 15;
    const int year[count] = {-501, 0, 1599, 1699, 1799,
                             1859, 1899, 1919, 1940, 1960,
                             1985, 2004, 2049, 2149, 2150
                            };
    for (int i = 0; i < count; ++i) {
        GetDeltaT(year[i], 6);
    }
}

//double JDBeijingTime2UTC(double bjtJD)
TEST_F(test_method_interface, JDBeijingTime2UTC)
{
    assert(0.67667 > JDBeijingTime2UTC(1.01));
}

//QString GetSolarDayFestival(int year, int month, int day)
TEST_F(test_method_interface, GetSolarDayFestival)
{
    QString getFesStr = GetSolarDayFestival(2020, 8, 1);
    assert("建军节" == getFesStr);

    getFesStr = GetSolarDayFestival(2020, 6, 1);
    assert("儿童节" == getFesStr);
}

//double CalcEarthObliquityNutation(double dt)
TEST_F(test_method_interface, CalcEarthObliquityNutation)
{
    CalcEarthObliquityNutation(GetJulianCentury(1));
}

//double lightAberration()
TEST_F(test_method_interface, lightAberration)
{
    lightAberration();
    //qCInfo(CommonLogger) << lightAb;
}
