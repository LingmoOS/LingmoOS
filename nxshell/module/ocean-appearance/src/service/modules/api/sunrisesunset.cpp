// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sunrisesunset.h"

#include <QDebug>
#include <QtMath>
#include <QTime>

#define ZENITH -0.833333333333333333
enum CalcSunType:int{
    Sunrise = 6,
    Sunset = 18,
};
/*****
 * http://williams.best.vwh.net/sunrise_sunset_algorithm.htm
 * http://stackoverflow.com/questions/7064531/sunrise-sunset-times-in-c
 * 如果返回值小于0，则没有日出。如果返回值大于24，则始终是日出状态
 ***/
static float calculateSunChangedAsUTCHour(const int &dayOfYear, const float &lat, const float &lng, const float &localOffset, const CalcSunType &sunChanged)
{
    /*
    localOffset will be <0 for western hemisphere and >0 for eastern hemisphere
    daylightSavings should be 1 if it is in effect during the summer otherwise it should be 0
    */
//    // 1. first calculate the day of the year 首先，计算一年中的哪一天
//    float N1 = floor(275 * month / 9);
//    float N2 = floor((month + 9) / 12);
//    float N3 = (1 + floor((year - 4 * floor(year / 4) + 2) / 3));
//    float N = N1 - (N2 * N3) + day - 30;
    float N = dayOfYear;

    // 2. convert the longitude to hour value and calculate an approximate time 将经度转换为小时值并计算近似时间
    float lngHour = lng / 15.0;
    float t = N + ((sunChanged - lngHour) / 24);

    // 3. calculate the Sun's mean anomaly 计算太阳的平均近点角
    float M = (0.9856 * t) - 3.289;

    // 4. calculate the Sun's true longitude 计算太阳的真实经度
    float L = fmod(M + (1.916 * sin((M_PI / 180.0) * M)) + (0.020 * sin(2 * (M_PI / 180.0) * M)) + 282.634, 360.0);

    // 5a. calculate the Sun's right ascension 计算太阳的赤经
    float RA = fmod(180.0 / M_PI * atan(0.91764 * tan((M_PI / 180.0) * L)), 360.0);

    // 5b. right ascension value needs to be in the same quadrant as L 赤经值需要与L位于同一象限
    float Lquadrant = floor(L / 90.0) * 90.0;
    float RAquadrant = floor(RA / 90.0) * 90.0;
    RA = RA + (Lquadrant - RAquadrant);

    // 5c. right ascension value needs to be converted into hours 赤经值需要转换为小时
    RA = RA / 15.0;

    // 6. calculate the Sun's declination 计算太阳的赤纬
    float sinDec = 0.39782 * sin((M_PI / 180.0) * L);
    float cosDec = cos(asin(sinDec));

    // 7a. calculate the Sun's local hour angle 计算太阳的本地时角
    float cosH = (sin((M_PI / 180.0) * ZENITH) - (sinDec * sin((M_PI / 180.0) * lat))) / (cosDec * cos((M_PI / 180.0) * lat));

    if (cosH > 1) // the sun never rises on this location (on the specified date) 太阳永远不会在这个位置升起（在指定的日期）长夜
        return 100;

    if (cosH < -1) // the sun never sets on this location (on the specified date) 太阳永远不会落在这个位置（在指定的日期）长昼
        return -100;

    // 7b. finish calculating H and convert into hours 完成计算H并转换为小时
    float H = (180.0 / M_PI) * acos(cosH);
    if (sunChanged == Sunrise) //   if if rising time is desired:
        H = 360.0 - H;
    H = H / 15.0;

    // 8. calculate local mean time of rising/setting 计算本地平均上升/下降时间
    float T = H + RA - (0.06571 * t) - 6.622;

    // 9. adjust back to UTC 调整回UTC
    float UT = fmod(T - lngHour, 24.0) + localOffset;
    if (UT < 0)
        UT += 24;

    if (UT >= 24)
        UT -= 24;

    // 10. convert UT value to local time zone of latitude/longitude 将UT值转换为纬度/经度的本地时区
    // return UT + localOffset + daylightSavings;
    return UT;
}

bool SunriseSunset::getSunriseSunset(double latitude, double longitude, double utcOffset, const QDate &date, QDateTime &sunrise, QDateTime &sunset)
{
    int dayOfYear = date.dayOfYear();
    float sunriseUT = calculateSunChangedAsUTCHour(dayOfYear, latitude, longitude, utcOffset, CalcSunType::Sunrise);

    float sunsetUT = calculateSunChangedAsUTCHour(dayOfYear, latitude, longitude, utcOffset, CalcSunType::Sunset);
    if (sunsetUT <= -100) // 长昼 返回的sunrise-sunset区间用于判断当前是否为白天
        sunsetUT = 100;

    sunrise = date.startOfDay().addMSecs(static_cast<int>(sunriseUT * 3600 * 1000));
    sunset = date.startOfDay().addMSecs(static_cast<int>(sunsetUT * 3600 * 1000));
    qInfo() << __FUNCTION__ << date << latitude << longitude << sunriseUT << sunrise << sunsetUT << sunset;
    return true;
}
