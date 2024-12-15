// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lunarcalendar.h"

#include <QTime>
#include <QDateTime>
#include <QDate>

QMap<int, LunarCalendar *> LunarCalendar::glYearCache;

LunarCalendar *LunarCalendar::GetLunarCalendar(qint32 year)
{
    auto it = glYearCache.find(year);
    LunarCalendar *plcal = nullptr;
    if (it != glYearCache.end()) {
        plcal = it.value();
    } else {
        plcal = new LunarCalendar(year);
        glYearCache.insert(year, plcal);
    }
    return plcal;
}

/**
 * @brief LunarCalendar::LogOffEmptyData    //清空数据
 */
void LunarCalendar::LogOffEmptyData()
{
    QMap<int, LunarCalendar *>::iterator it = glYearCache.begin();
    for (; it != glYearCache.end(); ++it) {
        delete it.value();
        it.value() = nullptr;
    }
    glYearCache.clear();
}

//指定年份内公历日期转换为农历日
lunarInfo LunarCalendar::SolarDayToLunarDay(qint32 month, qint32 day)
{
    lunarInfo dayinfo;
    QDateTime dt(QDate(Year, month, day), QTime(0, 0, 0, 0), Qt::TimeSpec::UTC);
    int yd = dt.date().dayOfYear();

    // 求月地支
    int monthZhi = 0;
    while (monthZhi < solarTermYearDays.size()) {
        if (yd >= solarTermYearDays[monthZhi]) {
            monthZhi++;
        } else {
            break;
        }
    }
    dayinfo.MonthZhi = monthZhi;
    // 求农历年、月、日
    foreach (lunarInfo lm, Months) {
        int dd = static_cast<int>(deltaDays(lm.ShuoTime, dt)) + 1;
        if (1 <= dd && dd <= lm.LunarMonthDays) {
            dayinfo.LunarYear = lm.LunarYear;
            dayinfo.LunarMonthName = lm.LunarMonthName;
            dayinfo.LunarMonthDays = lm.LunarMonthDays;
            dayinfo.ShuoJD = lm.ShuoJD;
            dayinfo.ShuoTime = lm.ShuoTime;
            dayinfo.IsLeap = lm.IsLeap;
            dayinfo.LunarDay = dd;
            break;
        }
    }
    // 求二十四节气
    dayinfo.SolarTerm = getSolarTermInfo(month, day);
    return dayinfo;
}

LunarCalendar::LunarCalendar(qint32 year)
{
    Year = year;
    Months.reserve(13);
    calcProcData();
    fillMonths();
    calcLeapMonth();
}

void LunarCalendar::calcProcData()
{
    SolarTermJDs = get25SolarTermJDs(Year - 1, DongZhi);
    for (int i = 0; i < 25; i++) {
        SolarTermTimes.append(GetDateTimeFromJulianDay(SolarTermJDs[i]));
    }
    for (int i = 1; i < 25; i += 2) {
        int yd = SolarTermTimes[i].date().dayOfYear();
        solarTermYearDays.append(yd);
    }
    double beijin2utc = JDBeijingTime2UTC(SolarTermJDs[0]);
    double tmpNewMoonJD = getNewMoonJD(beijin2utc);
    if (tmpNewMoonJD > SolarTermJDs[0]) {
        tmpNewMoonJD -= 29.53;
    }
    NewMoonJDs = get15NewMoonJDs(tmpNewMoonJD);
}

void LunarCalendar::fillMonths()
{
    //采用夏历建寅，冬至所在月份为农历11月(冬月)
    int yuejian = 11;
    for (int i = 0; i < 14; i++) {
        lunarInfo info;
        if (yuejian <= 12) {
            info.LunarMonthName = yuejian;
            info.LunarYear = Year - 1;
        } else {
            info.LunarMonthName = yuejian - 12;
            info.LunarYear = Year;
        }

        info.ShuoJD = NewMoonJDs[i];
        info.ShuoTime = GetDateTimeFromJulianDay(info.ShuoJD);
        double nextShuoJD = NewMoonJDs[i + 1];
        QDateTime nextShuoTime = GetDateTimeFromJulianDay(nextShuoJD);
        info.LunarMonthDays = static_cast<int>(deltaDays(info.ShuoTime, nextShuoTime));
        Months.append(info);
        yuejian++;
    }
}

void LunarCalendar::calcLeapMonth()
{
    // 根据节气计算是否有闰月，如果有闰月，根据农历月命名规则，调整月名称
    if (int(NewMoonJDs[13] + 0.5) <= int(SolarTermJDs[24] + 0.5)) {
        // 第13月的月末没有超过冬至，说明今年需要闰一个月
        int i = 1;
        while (i < 14) {
            if (int(NewMoonJDs[i + 1] + 0.5) <= int(SolarTermJDs[2 * i] + 0.5)) {
                //NewMoonJDs[i + 1] 是第i个农历月的下一个月的月首
                //本该属于第i个月的中气如果比下一个月的月首还晚，或者与下个月的月首是同一天（民间历法），则说明第 i 个月没有中气, 是闰月
                break;
            }
            i++;
        }
        if (i < 14) {
            // 找到闰月
            //qCDebug(ServiceLogger)<<QString("找到闰月 %1").arg(i);
            Months[i].IsLeap = true;
            // 对后面的农历月调整月名
            while (i < 14) {
                Months[i].LunarMonthName--;
                i++;
            }
        }
    }
}

qint32 LunarCalendar::getSolarTermInfo(qint32 month, qint32 day) const
{
    int index = 2 * month - 1;
    qint32 SolarTerm = -1;
    QDateTime dt1 = SolarTermTimes[index];
    QDateTime dt2 = SolarTermTimes[index + 1];
    if (dt1.date().day() == day) {
        SolarTerm = (index + DongZhi) % 24;
    } else if (dt2.date().day() == day) {
        SolarTerm = (index + 1 + DongZhi) % 24;
    }
    return SolarTerm;
}
