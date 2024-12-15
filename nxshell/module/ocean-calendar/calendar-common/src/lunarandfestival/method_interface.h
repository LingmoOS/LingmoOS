// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef METHOD_INTERFACE_H
#define METHOD_INTERFACE_H
#include "celestialbodies.h"
#include "lunarandfestival.h"

#include <QDateTime>
#include <QVector>
#include <QMap>

//获取当天的农历月名称
QString GetLunarMonthName(int lunarmonthname, bool isleap = false);
//获取当天的农历日名
QString GetLunarDayName(int lundayname);
//获取当天的农历节日名,没有则返回空字符串
QString GetLunarDayFestival(int monthname, int lunarday, int lunarmonthdays, int solarterm);
//获取公历节日,没有则返回空字符串
QString GetSolarDayFestival(int year, int month, int day);
//获取二十四节气名,如果没有则返回空
QString GetSolarTermName(int order);
//获取天干地支，将数字转换为天干地支
QString GetTianGanDiZhi(int num);
//获取当天的月干支
QString GetGanZhiMonth(int year, int monthzhi);
//获取当天的年干支
QString GetGanZhiYear(int lunaryear);
//获取当天的日干支
QString GetGanZhiDay(int year, int month, int day);
//获取当天的生肖，即年份的生肖
QString GetYearZodiac(int lunaryear);

QVector<double> get25SolarTermJDs(int year, int start);

//弧度计算
void GetMoonEclipticParameter(MoonEclipticParameter &moonEclipticParameter, double T);
//角度转换为弧度
double ToRadians(double degrees);
//把角度限制在[0, 2π]之间
double Mod2Pi(double r);
//计算月球地心黄经周期项的和
double CalcMoonECLongitudePeriodic(MoonEclipticParameter &moonEclipticParameter);
/*计算金星摄动,木星摄动以及地球扁率摄动对月球地心黄经的影响, T 是儒略世纪数，Lp和F单位是弧度*/
// A1 = 119.75 + 131.849 * T                                             （4.13式）
// A2 = 53.09 + 479264.290 * T                                           （4.14式）
// A3 = 313.45 + 481266.484 * T                                          （4.15式）
double CalcMoonLongitudePerturbation(double T, MoonEclipticParameter &moonEclipticParameter);
//计算月球地心黄经
double GetMoonEclipticLongitudeEC(double julianDay);
//计算儒略世纪数
double GetJulianCentury(double julianDay);
//返回弧度
void GetEarthNutationParameter(EarthNutationParameter &earthNutationParameter, double T);
//计算某时刻的黄经章动干扰量
double CalcEarthLongitudeNutation(double T);
//把角秒换算成角度
double SecondsToDegrees(double seconds);
//把角秒换算成弧度
double coefficient();

double SecondsToRadians(double seconds);
/*计算某时刻的黄赤交角章动干扰量，dt是儒略千年数，返回值单位是度*/
// 计算某时刻的黄赤交角章动干扰量
// dt 是儒略世纪数
// 返回弧度
double CalcEarthObliquityNutation(double dt);
// ToJulianDateHMS 计算Gregorian时间的儒略日数
// 算法摘自 http://en.wikipedia.org/wiki/Julian_day
double ToJulianDateHMS(int year, int month, int day, int hour, int minute, double second);
// ToJulianDate 计算Gregorian日期的儒略日数，以TT当天中午12点为准(结果是整数)。
// 算法摘自 http://en.wikipedia.org/wiki/Julian_day
int ToJulianDate(int year, int month, int day);
double GetEarthEclipticLongitudeForSun(double jd);
double GetSunEclipticLongitudeForEarth(double jd);
//计算儒略千年数
double GetJulianThousandYears(double jd);
double GetSunEclipticLatitudeForEarth(double jd);
double Vsop2Fk5LongitudeCorrection(double l, double b, double jd);
double GetSunRadiusForEarth(double jd);
/**
 * 计算修正后的太阳的地心视黄经
 *
 * 参数 jd
 *            儒略日
 * 返回 修正后的地心黄经(rad)
 */
// 常量
double lightAberration();

// NewtonIteration 牛顿迭代法求解方程的根
double NewtonIteration(double angle, double x0, bool IsGetSolarTermJD);
// ModPi 把角度限制在[-π, π]之间
double ModPi(double r);
// DmsToDegrees 把度分秒表示的角度换算成度
double DmsToDegrees(int degrees, int mintues, double seconds);
// DmsToSeconds 把度分秒表示的角度换算成角秒(arcsecond)
double DmsToSeconds(int d, int m, double s);
double DmsToRadians(int d, int m, int s);
double GetSolarTermJD(int year, int order);

bool IsLeapYear(int year);
int GetSolarMonthDays(int year, int month);
int GetWeekday(int y, int m, int d = 1);
double GetDeltaT(int year, int month);
void GetDateFromJulianDay(double jd, int &yy, int &mm, int &dd);
void GetTimeFromJulianDay(double jd, int &hour, int &minute, int &second);
QDateTime GetDateTimeFromJulianDay(double jd);
double JDUTC2BeijingTime(double utcJD);
double JDBeijingTime2UTC(double bjtJD);
double getNewMoonJD(double jd0);
QVector<double> get15NewMoonJDs(double jd);
qint64 deltaDays(QDateTime t1, QDateTime t2);
QString festivalForFatherAndMother(int year, int month, int day);

#endif // LUNARANDFESTIVALALGORITHM_H
