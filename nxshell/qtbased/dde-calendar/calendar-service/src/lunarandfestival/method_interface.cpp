// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "method_interface.h"

#include <QtMath>
#include <QDebug>

/**
 * @brief GetLunarMonthName 获取当天的农历月名称
 * @param lunarmonthname 阴历月份
 * @param isleap 是否为闰月
 * @return 阴历月份
 */
QString GetLunarMonthName(int lunarmonthname, bool isleap)
{
    QString monthname = lunarMonthNames[lunarmonthname - 1];
    if (isleap) {
        return "闰" + monthname + "月";
    }
    return monthname + "月";
}

/**
 * @brief GetLunarDayName 获取当天的农历日名
 * @param lundayname 阴历哪一天
 * @return  阴历日
 */
QString GetLunarDayName(int lundayname)
{
    return lunarDayNames[lundayname - 1];
}

/**
 * @brief GetLunarDayFestival 获取当天的农历节日名,没有则返回空字符串
 * @param monthname 公历月份
 * @param lunarday 阴历日
 * @param lunarmonthdays 阴历月份
 * @param solarterm 第几个节气
 * @return 农历节日名称
 */
QString GetLunarDayFestival(int monthname, int lunarday, int lunarmonthdays, int solarterm)
{
    int key = monthname * 100 + lunarday;
    if (lunarFestival.contains(key)) {
        return lunarFestival[key];
    }

    // 农历腊月（十二月）的最后个一天
    if (monthname == 12 && lunarday == lunarmonthdays) {
        return "除夕";
    }
    if (solarterm == QingMing) {
        return "清明节";
    }
    return "";
}
/**
 * @brief  GetSolarTermsName 获取二十四节气名,如果没有则返回空
 * @param order　节气枚举值
 * @return 节气名称
 */
QString GetSolarTermName(int order)
{
    if (0 <= order && order <= 23) {
        return SolarTermNames[order];
    }
    return "";
}
/**
 * @brief  GetTianGanDiZhi 获取天干地支，将数字转换为天干地支
 * @param num　需要转换的数字
 * @return 天干地支
 */
QString GetTianGanDiZhi(int num)
{
    return TianGan[num % 10] + DiZhi[num % 12];
}

/**
 * @brief GetGanZhiMonth 获取当天的月干支
 * @param year 公历年
 * @param monthzhi 月份的地支
 * @return 天干地支
 */
QString GetGanZhiMonth(int year, int monthzhi)
{
    return GetTianGanDiZhi((year - 1900) * 12 + monthzhi + 12);
}

/**
 * @brief GetGanZhiYear 获取当天的年干支
 * @param lunaryear 阴历年份
 * @return 天干地支
 */
QString GetGanZhiYear(int lunaryear)
{
    return GetTianGanDiZhi(lunaryear - 1864);
}

/**
 * @brief GetGanZhiDay 获取当天的日干支
 * @param year 公历年
 * @param month 公历月
 * @param day 公历日
 * @return 天干地支
 */
QString GetGanZhiDay(int year, int month, int day)
{
    QDateTime unixDateTime = QDateTime(QDate(year, month, day), QTime(0, 0, 0, 0), Qt::TimeSpec::UTC);
    qint64 unixTime = unixDateTime.toMSecsSinceEpoch() / 1000;
    int dayCyclical = int(unixTime / 86400) + 29219 + 18;
    return GetTianGanDiZhi(dayCyclical);
}

/**
 * @brief GetYearZodiac 获取当天的生肖，即年份的生肖
 * @param lunaryear 阴历年份
 * @return 年份生肖
 */
QString GetYearZodiac(int lunaryear)
{
    return Animals[(lunaryear - 4) % 12];
}

QVector<double> get25SolarTermJDs(int year, int start)
{
    // 从某一年的某个节气开始，连续计算25个节气，返回各节气的儒略日
    // year 年份
    // start 起始的节气
    // 返回 25 个节气的 儒略日北京时间
    int stOrder = start;
    QVector<double> list;

    for (int i = 0; i < 25; i++) {
        double jd = GetSolarTermJD(year, stOrder);
        list.append(JDUTC2BeijingTime(jd));
        if (stOrder == DongZhi) {
            year++;
        }
        stOrder = (stOrder + 1) % 24;
    }
    return list;
}

double GetEarthEclipticLongitudeForSun(double jd)
{
    // 计算地球的日心黄经
    double l = GetSunEclipticLongitudeForEarth(jd);
    // 计算地球的日心黄纬
    double b = GetSunEclipticLatitudeForEarth(jd);
    // 修正章动
    l += CalcEarthLongitudeNutation(GetJulianCentury(jd));
    // 转换到fk5
    l += Vsop2Fk5LongitudeCorrection(l, b, jd);
    // 转换成太阳的地心黄经
    l = Mod2Pi(l + M_PI);
    // 计算光行差
    // 计算日地距离
    double r = GetSunRadiusForEarth(jd);
    // 太阳到地球的光行差参数
    l -= SecondsToRadians(20.4898) / r;
    return l;
}

/**
 * 按儒略日计算地球的日心黄经
 *
 * 参数： jd 儒略日
 * 返回 地球的日心黄经，单位是弧度(rad)
 */
double GetSunEclipticLongitudeForEarth(double jd)
{
    double t = GetJulianThousandYears(jd);
    double L0 = GetEarthL0(t);
    double L1 = GetEarthL1(t);
    double L2 = GetEarthL2(t);
    double L3 = GetEarthL3(t);
    double L4 = GetEarthL4(t);
    double L5 = GetEarthL5(t);
    double L = ((((L5 * t + L4) * t + L3) * t + L2) * t + L1) * t + L0;
    return Mod2Pi(L);
}

double GetJulianThousandYears(double jd)
{
    //1000年的日数
    const double  DaysOf1000Years = 365250.0;
    return (jd - J2000) / DaysOf1000Years;
}

/**
 * 按儒略日计算地球的日心黄纬
 *
 * 参数 jd  儒略日
 * 返回 地球的日心黄纬，单位是弧度(rad)
 */
double GetSunEclipticLatitudeForEarth(double jd)
{
    double t = GetJulianThousandYears(jd);
    double B0 = GetEarthB0(t);
    double B1 = GetEarthB1(t);
    double B2 = GetEarthB2(t);
    double B3 = GetEarthB3(t);
    double B4 = GetEarthB4(t);
    double B = ((((B4 * t) + B3) * t + B2) * t + B1) * t + B0;
    return B;
}

/**
 * 用于把vsop87理论算出来的经度转换成fk5目视系统的经度的修正值，参考 Jean Meeus 的 Astronomical
 * Algorithms 第二版(1998)第32章219页(32.3)式
 *
 * 参数 l
 *            vsop87经度(rad)
 * 参数 b
 *            vsop87纬度(rad)
 * 参数 jd
 *            儒略日
 * 返回 修正量(rad)
 */
double Vsop2Fk5LongitudeCorrection(double l, double b, double jd)
{
    double t = GetJulianCentury(jd);
    double lp = l - ToRadians(1.397) * t - ToRadians(0.00031) * t * t;
    return SecondsToRadians(-0.09033 + 0.03916 * (qCos(lp) + qSin(lp)) * qTan(b));
}

/**
 * 按照儒略日计算地球和太阳的距离
 *
 * 参数 jd  儒略日
 * 返回 地球和太阳的距离，单位是天文单位(au)
 */
double GetSunRadiusForEarth(double jd)
{
    double t = GetJulianThousandYears(jd);
    double R0 = GetEarthR0(t);
    double R1 = GetEarthR1(t);
    double R2 = GetEarthR2(t);
    double R3 = GetEarthR3(t);
    double R4 = GetEarthR4(t);
    double R5 = GetEarthR5(t);
    double R = ((((R5 * t + R4) * t + R3) * t + R2) * t + R1) * t + R0;
    return R;
}

double NewtonIteration(double angle, double x0, bool IsGetSolarTermJD)
{
    //此函数原是传入的匿名函数，这里用bool用作区分，后续可以优化
    const double Epsilon = 1e-7;
    const double Delta = 5e-6;
    double x;
    auto func = [angle](double x) -> double {
        return ModPi(GetEarthEclipticLongitudeForSun(x) - angle);
    };

    auto func1 = [](double x) -> double {
        double earth = GetEarthEclipticLongitudeForSun(x);
        double moon = GetMoonEclipticLongitudeEC(x);
        double res = ModPi(earth - moon);
        return res;
        // return ModPi(GetEarthEclipticLongitudeForSun(x) - GetMoonEclipticLongitudeEC(x));
    };

    while (1) {
        x = x0;
        double fx, fpx;
        if (IsGetSolarTermJD) {
            fx = func(x);
            // 导数
            fpx = (func(x + Delta) - func(x - Delta)) / Delta / 2;
        } else {
            //            fx = ModPi(GetEarthEclipticLongitudeForSun(x) - GetMoonEclipticLongitudeEC(x));
            //            // 导数
            //            fpx = (ModPi(GetEarthEclipticLongitudeForSun(x + Delta) - GetMoonEclipticLongitudeEC(x))
            //                   - GetEarthEclipticLongitudeForSun(x - Delta) - GetMoonEclipticLongitudeEC(x)) / Delta / 2;
            fx = func1(x);
            // 导数
            fpx = (func1(x + Delta) - func1(x - Delta)) / Delta / 2;
        }
        x0 = x - fx / fpx;
        if (qAbs(x0 - x) <= Epsilon) {
            break;
        }
    }
    return x;
}

double ModPi(double r)
{
    while (r < -M_PI) {
        r += M_PI * 2;
    }
    while (r > M_PI) {
        r -= M_PI * 2;
    }
    return r;
}

double DmsToDegrees(int degrees, int mintues, double seconds)
{
    return double(degrees) + double(mintues) / 60 + seconds / 3600;
}

double DmsToSeconds(int d, int m, double s)
{
    return double(d) * 3600 + double(m) * 60 + s;
}

// DmsToRadians 把度分秒表示的角度换算成弧度(rad)
double DmsToRadians(int d, int m, int s)
{
    return ToRadians(DmsToDegrees(d, m, s));
}

// GetSolarTermJD 使用牛顿迭代法计算24节气的时间
// f(x) = Vsop87dEarthUtil.getEarthEclipticLongitudeForSun(x) - angle = 0
// year 年
// order 节气序号
// 返回 节气的儒略日力学时间 TD
double GetSolarTermJD(int year, int order)
{
    const double RADIANS_PER_TERM = M_PI / 12.0;
    double angle = double(order) * RADIANS_PER_TERM;
    int month = ((order + 1) / 2 + 2) % 12 + 1;
    // 春分 order 0
    // 3 月 20 号
    int day = 6;
    if (order % 2 == 0) {
        day = 20;
    }
    double jd0 = ToJulianDateHMS(year, month, day, 12, 0, 0.0);
    double jd = NewtonIteration(angle, jd0, true);
    return jd;
}

// IsLeapYear 公历闰年判断
bool IsLeapYear(int year)
{
    return ((year & 3) == 0 && year % 100 != 0) || year % 400 == 0;
}

// GetSolarMonthDays 获取公历月份的天数
int GetSolarMonthDays(int year, int month)
{
    if (month == 2 && IsLeapYear(year)) {
        return 29;
    } else {
        return monthDays[month - 1];
    }
}

// GetWeekday 计算Gregorian日历的星期几
// 算法摘自 http://en.wikipedia.org/wiki/Zeller%27s_congruence
// 返回星期几的数字表示，1-6表示星期一到星期六，0表示星期日
int GetWeekday(int y, int m, int d)
{
    if (m <= 2) {
        y -= 1;
        m += 12;
    }
    int c = int(y / 100);
    y = y % 100;
    int w = (d + 13 * (m + 1) / 5 + y + (y / 4) + (c / 4) - 2 * c - 1) % 7;
    if (w < 0) {
        w += 7;
    }
    return w;
}

// GetDeltaT 计算地球时和UTC的时差，算法摘自
// http://eclipse.gsfc.nasa.gov/SEhelp/deltatpoly2004.html NASA网站
// ∆T = TT - UT 此算法在-1999年到3000年有效
double GetDeltaT(int year, int month)
{
    double y = double(year) + (double(month) - 0.5) / 12;

    if (year < -500) {
        double u = (double(year) - 1820) / 100;
        return -20 + 32 * u * u;
    } else if (year < 500) {
        double u = y / 100;
        double u2 = u * u;
        double u3 = u2 * u;
        double u4 = u3 * u;
        double u5 = u4 * u;
        double u6 = u5 * u;
        return 10583.6 - 1014.41 * u + 33.78311 * u2 - 5.952053 * u3 - 0.1798452 * u4 + 0.022174192 * u5 + 0.0090316521 * u6;
    } else if (year < 1600) {
        double u = (y - 1000) / 100;
        double u2 = u * u;
        double u3 = u2 * u;
        double u4 = u3 * u;
        double u5 = u4 * u;
        double u6 = u5 * u;
        return 1574.2 - 556.01 * u + 71.23472 * u2 + 0.319781 * u3 - 0.8503463 * u4 - 0.005050998 * u5 + 0.0083572073 * u6;
    } else if (year < 1700) {
        double t = y - 1600;
        double t2 = t * t;
        double t3 = t2 * t;
        return 120 - 0.9808 * t - 0.01532 * t2 + t3 / 7129;
    } else if (year < 1800) {
        double t = y - 1700;
        double t2 = t * t;
        double t3 = t2 * t;
        double t4 = t3 * t;
        return 8.83 + 0.1603 * t - 0.0059285 * t2 + 0.00013336 * t3 - t4 / 1174000;
    } else if (year < 1860) {
        double t = y - 1800;
        double t2 = t * t;
        double t3 = t2 * t;
        double t4 = t3 * t;
        double t5 = t4 * t;
        double t6 = t5 * t;
        double t7 = t6 * t;
        return 13.72 - 0.332447 * t + 0.0068612 * t2 + 0.0041116 * t3 - 0.00037436 * t4 + 0.0000121272 * t5 - 0.0000001699 * t6 + 0.000000000875 * t7;
    } else if (year < 1900) {
        double t = y - 1860;
        double t2 = t * t;
        double t3 = t2 * t;
        double t4 = t3 * t;
        double t5 = t4 * t;
        return 7.62 + 0.5737 * t - 0.251754 * t2 + 0.01680668 * t3 - 0.0004473624 * t4 + t5 / 233174;
    } else if (year < 1920) {
        double t = y - 1900;
        double t2 = t * t;
        double t3 = t2 * t;
        double t4 = t3 * t;
        return -2.79 + 1.494119 * t - 0.0598939 * t2 + 0.0061966 * t3 - 0.000197 * t4;
    } else if (year < 1941) {
        double t = y - 1920;
        double t2 = t * t;
        double t3 = t2 * t;
        return 21.20 + 0.84493 * t - 0.076100 * t2 + 0.0020936 * t3;
    } else if (year < 1961) {
        double t = y - 1950;
        double t2 = t * t;
        double t3 = t2 * t;
        return 29.07 + 0.407 * t - t2 / 233 + t3 / 2547;
    } else if (year < 1986) {
        double t = y - 1975;
        double t2 = t * t;
        double t3 = t2 * t;
        return 45.45 + 1.067 * t - t2 / 260 - t3 / 718;
    } else if (year < 2005) {
        double t = y - 2000;
        double t2 = t * t;
        double t3 = t2 * t;
        double t4 = t3 * t;
        double t5 = t4 * t;
        return 63.86 + 0.3345 * t - 0.060374 * t2 + 0.0017275 * t3 + 0.000651814 * t4 + 0.00002373599 * t5;
    } else if (year < 2050) {
        double t = y - 2000;
        double t2 = t * t;
        return 62.92 + 0.32217 * t + 0.005589 * t2;
    } else if (year < 2150) {
        double u = (y - 1820) / 100;
        double u2 = u * u;
        return -20 + 32 * u2 - 0.5628 * (2150 - y);
    } else {
        double u = (y - 1820) / 100;
        double u2 = u * u;
        return -20 + 32 * u2;
    }
}

// GetDateFromJulianDay 从儒略日中获取公历的日期
void GetDateFromJulianDay(double jd, int &yy, int &mm, int &dd)
{
    /*
     * This algorithm is taken from
     * "Numerical Recipes in c, 2nd Ed." (1992), pp. 14-15
     * and converted to integer math.
     * The electronic version of the book is freely available
     * at http://www.nr.com/ , under "Obsolete Versions - Older
     * book and code versions.
     */
    const int JD_GREG_CAL = 2299161;
    const int JB_MAX_WITHOUT_OVERFLOW = 107374182;
    int64_t julian = int64_t(qFloor(jd + 0.5));

    int64_t ta, jalpha, tb, tc, td, te;

    if (julian >= JD_GREG_CAL) {
        jalpha = (4 * (julian - 1867216) - 1) / 146097;
        ta = int64_t(julian) + 1 + jalpha - jalpha / 4;
    } else if (julian < 0) {
        ta = julian + 36525 * (1 - julian / 36525);
    } else {
        ta = julian;
    }

    tb = ta + 1524;
    if (tb <= JB_MAX_WITHOUT_OVERFLOW) {
        tc = (tb * 20 - 2442) / 7305;
    } else {
        tc = int64_t((uint64_t(tb) * 20 - 2442) / 7305);
    }

    td = 365 * tc + tc / 4;
    te = ((tb - td) * 10000) / 306001;
    dd = int(tb - td - (306001 * te) / 10000);
    mm = int(te - 1);

    if (mm > 12) {
        mm -= 12;
    }
    yy = int(tc - 4715);
    if (mm > 2) {
        yy--;
    }
    if (julian < 0) {
        yy -= int(100 * (1 - julian / 36525));
    }
}

// GetTimeFromJulianDay 从儒略日中获取时间 时分秒
void GetTimeFromJulianDay(double jd, int &hour, int &minute, int &second)
{
    double frac = jd - qFloor(jd);
    int s = int(qFloor(frac * 24.0 * 60.0 * 60.0));

    hour = ((s / (60 * 60)) + 12) % 24;
    minute = (s / (60)) % 60;
    second = s % 60;
}

// GetDateTimeFromJulianDay 将儒略日转换为 time.Time
// 其中包含了 TT 到 UTC 的转换
QDateTime GetDateTimeFromJulianDay(double jd)
{
    int year, month, day;
    GetDateFromJulianDay(jd, year, month, day);
    //  TT -> UTC
    jd -= GetDeltaT(year, month) / 86400;
    GetDateFromJulianDay(jd, year, month, day);
    int hour, minute, second;
    GetTimeFromJulianDay(jd, hour, minute, second);

    return QDateTime(QDate(year, month, day), QTime(hour, minute, second, 0), Qt::TimeSpec::UTC);
}

// JDUTC2BeijingTime 儒略日 UTC 时间转换到北京时间
double JDUTC2BeijingTime(double utcJD)
{
    return utcJD + 8.0 / 24.0;
}

// JDBeijingTime2UTC 儒略日 北京时间到 UTC 时间
double JDBeijingTime2UTC(double bjtJD)
{
    return bjtJD - 8.0 / 24.0;
}

double getNewMoonJD(double jd0)
{
    return NewtonIteration(0, jd0, false);
}

QVector<double> get15NewMoonJDs(double jd)
{
    // 计算从某个时间之后的连续15个朔日
    // 参数: jd 开始时间的 儒略日
    // 返回 15个朔日时间 数组指针 儒略日北京时间
    QVector<double> list;
    for (int i = 0; i < 15; i++) {
        double newMoonJD = getNewMoonJD(jd);
        list.append(JDUTC2BeijingTime(newMoonJD));
        // 转到下一个最接近朔日的时间
        jd = newMoonJD + 29.53;
    }
    return list;
}

qint64 deltaDays(QDateTime t1, QDateTime t2)
{
    // 计算两个时间相差的天数
    // t2 > t1 结果为正数
    return int(t1.date().daysTo(t2.date()));
    // return qint64((t2.toSecsSinceEpoch() - t1.toSecsSinceEpoch()) / 86400);
}

QString festivalForFatherAndMother(int year, int month, int day)
{
    int disparityMotherDay, disparityFatherDay, fatherDay, motherDay;
    int leapYear = 0;
    for (int i = 1900; i <= year; i++) {
        if ((i % 400 == 0) || ((i % 100 != 0) && (i % 4 == 0))) {
            leapYear += 1;
        }
    }
    if (month == 5) {
        disparityMotherDay = (((year - 1899) * 365 + leapYear) - (31 + 30 + 31 + 31 + 30 + 31 + 30 + 31)) % 7;
        motherDay = 14 - disparityMotherDay;
        if (day == motherDay) {
            return "母亲节";
        } else {
            return "";
        }
    }
    if (month == 6) {
        disparityFatherDay = (((year - 1899) * 365 + leapYear) - (30 + 31 + 31 + 30 + 31 + 30 + 31)) % 7;
        fatherDay =  21 - disparityFatherDay;
        if (day == fatherDay) {
            return "父亲节";
        } else {
            return "";
        }
    }
    return "";
}
/**
 * @brief  GetMoonEclipticParameter 弧度计算
 * @param T 儒略世纪数
 * @return 弧度
 */
void GetMoonEclipticParameter(MoonEclipticParameter &moonEclipticParameter, double T)
{
    double T2 = T * T;
    double T3 = T2 * T;
    double T4 = T3 * T;

    /*月球平黄经*/
    moonEclipticParameter.Lp = Mod2Pi(ToRadians(218.3164591 + 481267.88134236 * T - 0.0013268 * T2 + T3 / 538841.0 - T4 / 65194000.0));
    /*月日距角*/
    moonEclipticParameter.D = Mod2Pi(ToRadians(297.8502042 + 445267.1115168 * T - 0.0016300 * T2 + T3 / 545868.0 - T4 / 113065000.0));
    /*太阳平近点角*/
    moonEclipticParameter.M = Mod2Pi(ToRadians(357.5291092 + 35999.0502909 * T - 0.0001536 * T2 + T3 / 24490000.0));
    /*月亮平近点角*/
    moonEclipticParameter.Mp = Mod2Pi(ToRadians(134.9634114 + 477198.8676313 * T + 0.0089970 * T2 + T3 / 69699.0 - T4 / 14712000.0));
    /*月球经度参数(到升交点的平角距离)*/
    moonEclipticParameter.F = Mod2Pi(ToRadians(93.2720993 + 483202.0175273 * T - 0.0034029 * T2 - T3 / 3526000.0 + T4 / 863310000.0));
    /* 反映地球轨道偏心率变化的辅助参量 */
    moonEclipticParameter.E = 1 - 0.002516 * T - 0.0000074 * T2;
}
/**
 * @brief  ToRadians 角度转换为弧度
 * @param degrees 角度
 * @return 弧度
 */
double ToRadians(double degrees)
{
    return degrees * M_PI / 180;
}
/**
 * @brief  Mod2Pi 把角度限制在[0, 2π]之间
 * @param r 角度
 * @return 角度
 */
double Mod2Pi(double r)
{
    while (r < 0) {
        r += M_PI * 2;
    }
    while (r > 2 * M_PI) {
        r -= M_PI * 2;
    }
    return r;
}
/**
 * @brief  CalcMoonECLongitudePeriodic 计算月球地心黄经周期项的和
 * @param raDian 弧度
 * @return 月球地心黄经周期项的和
 */
double CalcMoonECLongitudePeriodic(MoonEclipticParameter &moonEclipticParameter)
{
    double EI = 0.0;
    for (int i = 0; i < MoonLongitude.count(); i++) {
        double theta = MoonLongitude[i].D * moonEclipticParameter.D + MoonLongitude[i].M * moonEclipticParameter.M
                       + MoonLongitude[i].Mp * moonEclipticParameter.Mp + MoonLongitude[i].F * moonEclipticParameter.F;
        EI += MoonLongitude[i].EiA * qSin(theta) * qPow(moonEclipticParameter.E, qAbs(MoonLongitude[i].M));
    }
    return EI;
}
/**
 * @brief  CalcMoonLongitudePerturbation 计算金星摄动,木星摄动以及地球扁率摄动对月球地心黄经的影响
 * @param T 儒略世纪数
 * @param raDian 弧度
 * @return
 */
double CalcMoonLongitudePerturbation(double T, MoonEclipticParameter &moonEclipticParameter)
{
    double A1 = Mod2Pi(ToRadians(119.75 + 131.849 * T));
    double A2 = Mod2Pi(ToRadians(53.09 + 479264.290 * T));

    return 3958.0 * qSin(A1) + 1962.0 * qSin(moonEclipticParameter.Lp - moonEclipticParameter.F) + 318.0 * qSin(A2);
}
/**
 * @brief  GetMoonEclipticLongitudeEC 计算月球地心黄经
 * @param julianDay 儒略日
 * @return 弧度
 */
double GetMoonEclipticLongitudeEC(double julianDay)
{
    MoonEclipticParameter m_radian;
    double T = GetJulianCentury(julianDay);
    GetMoonEclipticParameter(m_radian, T);
    // Lp 计算是正确的
    /*计算月球地心黄经周期项*/
    double EI = CalcMoonECLongitudePeriodic(m_radian);
    /*修正金星,木星以及地球扁率摄动*/
    EI += CalcMoonLongitudePerturbation(T, m_radian);
    double longitude = m_radian.Lp + ToRadians(EI / 1000000.0);
    /*计算天体章动干扰*/
    longitude += CalcEarthLongitudeNutation(T);
    return longitude;
}
/**
 * @brief  GetJulianCentury 计算儒略世纪数
 * @param julianDay 儒略日
 * @return 儒略世纪数
 */
double GetJulianCentury(double julianDay)
{
    // 100年的日数
    const double DaysOfCentury = 36525.0;
    return (julianDay - J2000) / DaysOfCentury;
}
/**
 * @brief  GetEarthNutationParameter 返回弧度
 * @param earthNutationParameter 弧度
 * @param T 儒略世纪数
 * @return 弧度
 */
void GetEarthNutationParameter(EarthNutationParameter &earthNutationParameter, double T)
{
    double T2 = T * T;
    double T3 = T2 * T;

    /*平距角（如月对地心的角距离）*/
    earthNutationParameter.D = ToRadians(297.85036 + 445267.111480 * T - 0.0019142 * T2 + T3 / 189474.0);
    /*太阳（地球）平近点角*/
    earthNutationParameter.M = ToRadians(357.52772 + 35999.050340 * T - 0.0001603 * T2 - T3 / 300000.0);
    /*月亮平近点角*/
    earthNutationParameter.Mp = ToRadians(134.96298 + 477198.867398 * T + 0.0086972 * T2 + T3 / 56250.0);
    /*月亮纬度参数*/
    earthNutationParameter.F = ToRadians(93.27191 + 483202.017538 * T - 0.0036825 * T2 + T3 / 327270.0);
    /*黄道与月亮平轨道升交点黄经*/
    earthNutationParameter.Omega = ToRadians(125.04452 - 1934.136261 * T + 0.0020708 * T2 + T3 / 450000.0);
}

double CalcEarthLongitudeNutation(double T)
{
    EarthNutationParameter m_radian;
    GetEarthNutationParameter(m_radian, T);
    double result = 0.0;
    for (int i = 0; i < nuation.count(); i++) {
        double theta = nuation[i].D * m_radian.D + nuation[i].M * m_radian.M + nuation[i].Mp * m_radian.Mp
                       + nuation[i].F * m_radian.F + nuation[i].Omega * m_radian.Omega;
        result += (nuation[i].Sine1 + nuation[i].Sine2 * T) * qSin(theta);
    }
    //乘以章动表的系数 0.0001 角秒
    return result * coefficient();
}
/**
 * @brief  SecondsToDegrees 把角秒换算成角度
 * @param seconds 角秒
 * @return 角度
 */
double SecondsToDegrees(double seconds)
{
    return seconds / 3600;
}

double SecondsToRadians(double seconds)
{
    return ToRadians(SecondsToDegrees(seconds));
}

double CalcEarthObliquityNutation(double dt)
{
    EarthNutationParameter m_radian;
    GetEarthNutationParameter(m_radian, dt);
    double result = 0.0;
    for (int i = 0; i < nuation.count(); i++) {
        double theta = nuation[i].D * m_radian.D + nuation[i].M * m_radian.M + nuation[i].Mp * m_radian.Mp
                       + nuation[i].F * m_radian.F + nuation[i].Omega * m_radian.Omega;
        result += (nuation[i].Cosine1 + nuation[i].Cosine2 * dt) * qCos(theta);
    }
    //乘以章动表的系数 0.0001 角秒
    return result * coefficient();
}

double ToJulianDateHMS(int year, int month, int day, int hour, int minute, double second)
{
    int jdn = ToJulianDate(year, month, day);
    return double(jdn) + (double(hour) - 12) / 24.0 + double(minute) / 1440.0 + second / 86400.0;
}

int ToJulianDate(int year, int month, int day)
{
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    return day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
}

double coefficient()
{
    return SecondsToRadians(0.0001);
}

double lightAberration()
{
    return SecondsToRadians(20.4898);
}

//获取阳历节日
QString GetSolarDayFestival(int year, int month, int day)
{
    QString festivals;
    if ((month == 5) || (month == 6)) {
        QString name = festivalForFatherAndMother(year, month, day);
        if (!name.isEmpty()) {
            festivals.append(name);
        }
    }
    int key = month * 100 + day;
    QString solarFestival = solarFestivals[key];
    if (!solarFestival.isEmpty()) {
        QStringList temlist = solarFestival.split(",");
        for (int i = 0; i < temlist.size(); ++i) {
            //节日名称
            QString temname = temlist.at(i);
            //带有节日映射的的迭代器
            auto it = solarFestivalStarYear.find(temname);
            //保证不越界，并且节日的开始年份在year之前(包括year)
            if (it != solarFestivalStarYear.end() && *it <= year) {
                if (!festivals.isEmpty()) {
                    festivals.append(',');
                }
                festivals.append(temname);
                festivals.append(',');
            }
        }
    }
    if (festivals.endsWith(','))
        festivals = festivals.left(festivals.length() - 1);
    return festivals;
}
