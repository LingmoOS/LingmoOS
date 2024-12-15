// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lunarmanager.h"
#include "lunarcalendar.h"
#include "commondef.h"
#include "pinyin/pinyinsearch.h"

#include <QDebug>

stLunarDayInfo SolarToLunar(qint32 year, qint32 month, qint32 day)
{
    stLunarDayInfo info;
    LunarCalendar *pcalendar = LunarCalendar::GetLunarCalendar(year);
    lunarInfo lday = pcalendar->SolarDayToLunarDay(month, day);
    info.GanZhiYear = GetGanZhiYear(lday.LunarYear);
    info.GanZhiMonth = GetGanZhiMonth(year, lday.MonthZhi);
    info.GanZhiDay = GetGanZhiDay(year, month, day);
    info.LunarMonthName = GetLunarMonthName(lday.LunarMonthName, lday.IsLeap);
    info.LunarDayName = GetLunarDayName(lday.LunarDay);
    info.Term = GetSolarTermName(lday.SolarTerm);
    info.SolarFestival = GetSolarDayFestival(year, month, day);
    info.LunarFestival = GetLunarDayFestival(lday.LunarMonthName, lday.LunarDay, lday.LunarMonthDays, lday.SolarTerm);
    info.Zodiac = GetYearZodiac(lday.LunarYear);
    return info;
}

/**
 * 获取指定公历月份的农历数据
 * year,month 公历年，月
 * fill 是否用上下月数据补齐首尾空缺，首例数据从周日开始
 */
LunarMonthInfo GetLunarMonthCalendar(qint32 year, qint32 month, bool fill)
{
    SolarMonthInfo solarMonth = GetSolarMonthCalendar(year, month, fill);
    return GetLunarMonthCalendar(solarMonth);
}

LunarMonthInfo GetLunarMonthCalendar(const SolarMonthInfo &solarMonth)
{
    LunarMonthInfo lunarmonth;
    lunarmonth.FirstDayWeek = solarMonth.FirstDayWeek;
    lunarmonth.Days = solarMonth.Days;
    foreach (stDay d, solarMonth.Datas) {
        stLunarDayInfo info = SolarToLunar(d.Year, d.Month, d.Day);
        lunarmonth.Datas.append(info);
    }
    return lunarmonth;
}

/**
 * 公历某月日历
 * year,month 公历年，月
 * fill 是否用上下月数据补齐首尾空缺，首例数据从周日开始(7*6阵列)
 */
SolarMonthInfo GetSolarMonthCalendar(qint32 year, qint32 month, bool fill)
{
    SolarMonthInfo solarMonth;
    int weekday = GetWeekday(year, month);
    int daycount = GetSolarMonthDays(year, month);
    // 本月的数据
    QList<stDay> daysData = GetMonthDays(year, month, 1, daycount);
    QList<int> YearMonth;
    if (fill) {
        if (weekday > 0) {
            //获取前一个月所在年份及月份（如果当前月份为1月则为前一年的十二月）
            YearMonth = GetPreMonth(year, month);
            int premonthyear = YearMonth.at(0);
            int premonth = YearMonth.at(1);
            GetSolarMonthDays(premonthyear, premonth);
            // 前一个月的天数
            int preMonthDays = GetSolarMonthDays(premonthyear, premonth);
            // 要补充上去的前一个月的数据
            QList<stDay> preMonthCompleteDaysData = GetMonthDays(premonthyear, premonth, preMonthDays - weekday + 1, preMonthDays);
            preMonthCompleteDaysData.append(daysData);
            daysData = preMonthCompleteDaysData;
        }
        YearMonth = GetNextMonth(year, month);
        int nextmonthyear = YearMonth.at(0);
        int nextmonth = YearMonth.at(1);
        int count = 6 * 7 - (weekday + daycount);
        // 要补充上去的下一个月的数据
        QList<stDay> nextMonthCompleteDaysData = GetMonthDays(nextmonthyear, nextmonth, 1, count);
        daysData.append(nextMonthCompleteDaysData);
    }
    solarMonth.FirstDayWeek = weekday;
    solarMonth.Days = daycount;
    solarMonth.Datas = daysData;
    return solarMonth;
}

QList<stDay> GetMonthDays(qint32 year, qint32 month, qint32 start, qint32 days)
{
    QList<stDay> DayInfo;
    for (int day = start; day <= days; day++) {
        stDay stday = {year, month, day};
        DayInfo.append(stday);
    }
    return DayInfo;
}

QList<int> GetPreMonth(qint32 year, qint32 month)
{
    QList<int> datas;
    int preYear, preMonth;
    if (month == 1) {
        preYear = year - 1;
        preMonth = 12;
    } else {
        preYear = year;
        preMonth = month - 1;
    }
    datas.append(preYear);
    datas.append(preMonth);
    return datas;
}

QList<int> GetNextMonth(qint32 year, qint32 month)
{
    QList<int> datas;
    int nextYear, nextMonth;
    if (month == 12) {
        nextYear = year + 1;
        nextMonth = 1;
    } else {
        nextYear = year;
        nextMonth = month + 1;
    }
    datas.append(nextYear);
    datas.append(nextMonth);
    return datas;
}

/**
 * @brief  GetFestivalsInRange 获取指定范围内的节日信息集合
 * @param start 起始时间
 * @param end 结束时间
 */
QList<stDayFestival> GetFestivalsInRange(const QDateTime &start, const QDateTime &end)
{
    QList<stDayFestival> festivaldays;
    if (start <= end) {
        //days为需要查询的天数,而不是两个时间的差值
        int days = static_cast<int>(start.daysTo(end) + 1);
        for (int i = 0; i < days; ++i) {
            stDayFestival stdayfestival;
            QDateTime tem = start.addDays(i);
            stdayfestival.date = tem;
            int year = tem.date().year();
            int month = tem.date().month();
            int day = tem.date().day();
            LunarCalendar *pcalendar = LunarCalendar::GetLunarCalendar(year);
            lunarInfo lday = pcalendar->SolarDayToLunarDay(month, day);
            QString festival = GetSolarDayFestival(year, month, day);
            QStringList strfestivallist = festival.split(",");
            strfestivallist << GetLunarDayFestival(lday.LunarMonthName, lday.LunarDay, lday.LunarMonthDays, lday.SolarTerm);
            stdayfestival.Festivals = strfestivallist;
            festivaldays.append(stdayfestival);
        }
    } else {
        qCDebug(ServiceLogger) << __FUNCTION__ << "start day later than  end day";
    }

    return festivaldays;
}
/**
 * @brief FilterDayFestival 过滤节日信息
 * @param festivaldays 节日信息
 * @param querykey 拼音
 * @return 节日信息
 */
QList<stDayFestival> FilterDayFestival(QList<stDayFestival> &festivaldays, const QString &querykey)
{
    QList<stDayFestival> m_festivaldays;
    pinyinsearch *search = pinyinsearch::getPinPinSearch();

    for (int i = 0; i < festivaldays.size(); i++) {
        stDayFestival m_festivals{};
        QStringList festivals = festivaldays.at(i).Festivals;
        QDateTime festivalsdate = festivaldays.at(i).date;

        for (int j = 0; j < festivals.size(); j++) {
            if (festivals.at(j).contains(querykey))
                m_festivals.Festivals.append(festivals.at(j));
            if (search->CanQueryByPinyin(querykey) && search->PinyinMatch(festivals.at(j), querykey))
                m_festivals.Festivals.append(festivals.at(j));
        }

        if (m_festivals.Festivals.isEmpty())
            continue;

        m_festivals.date = festivalsdate;
        m_festivaldays.append(m_festivals);
    }

    return m_festivaldays;
}

void logOffEmptyData()
{
    LunarCalendar::LogOffEmptyData();
}
