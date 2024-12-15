// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lunardateinfo.h"

#include "lunarcalendar.h"

#include <QDebug>

#define RECURENCELIMIT 3650 //递归次数限制
//农历一年最少有363天
const int LunarYearMiniDays = 353;

LunarDateInfo::LunarDateInfo(KCalendarCore::RecurrenceRule *rruleStr, const qint64 interval)
    : m_recurenceRule(rruleStr)
    , m_dateInterval(interval)
{
    m_rruleType = ParseRRule(m_recurenceRule->rrule());
}

QMap<int, QDate> LunarDateInfo::getRRuleStartDate(const QDate &beginDate, const QDate &endDate, const QDate &solarDate)
{
    QMap<int, QDate> solar;
    //不在范围内直接返回,开始时间小于结束时间或者需要计算的起始时间晚于结束时间
    if (endDate < beginDate || solarDate > endDate) {
        return solar;
    }

    m_queryStartDate = beginDate;
    m_queryEndDate = endDate;
    //如果日程开始时间在查询起始时间之前
    if (solarDate > m_queryStartDate) {
        m_queryStartDate = solarDate;
    }
    //如果是农历日程
    //TODO: 重复类型
    switch (m_rruleType) {
    case RRule_Month:
        //每月
        solar = getAllNextMonthLunarDayBySolar(solarDate);
        break;
    case RRule_Year:
        //每年
        solar = getAllNextYearLunarDayBySolar(solarDate);
        break;
    default:
        //默认不重复
        break;
    }

    return solar;
}

//通过公历时间获取范围内该时间的农历天的具体公历日期
QMap<int, QDate> LunarDateInfo::getAllNextMonthLunarDayBySolar(const QDate &solarDate)
{
    QMap<int, QDate> solar;
    //如果需要通过公历信息获取下一个对应农历信息对应的天

    LunarCalendar *lunc = LunarCalendar::GetLunarCalendar(solarDate.year());
    lunarInfo info = lunc->SolarDayToLunarDay(solarDate.month(), solarDate.day());
    //计算时间为日程开始时间
    QDate nextSolar = solarDate;
    int count = 0;
    while (true) {
        info = getNextMonthLunarDay(nextSolar, info);

        //如果超过范围则退出
        if (addSolarMap(solar, nextSolar, count, info.LunarMonthDays)) {
            break;
        }
    }
    return solar;
}

QMap<int, QDate> LunarDateInfo::getAllNextYearLunarDayBySolar(const QDate &solarDate)
{
    QMap<int, QDate> solar;

    //TODO: 需要优化
    //日程的农历日期
    LunarCalendar *solarDateLunar = LunarCalendar::GetLunarCalendar(solarDate.year());
    lunarInfo info = solarDateLunar->SolarDayToLunarDay(solarDate.month(), solarDate.day());
    //计算时间为日程开始时间
    QDate bDate = solarDate;
    QDate beforeDate;

    int count = 0;

    //beforeDate == bDate时，两次执行结果相同，会进入死循环
    while (bDate <= m_queryEndDate && beforeDate != bDate) {
        beforeDate = bDate;
        //开始时间农历日期
        LunarCalendar *lunc = LunarCalendar::GetLunarCalendar(bDate.year());
        lunarInfo startLunarInfo = lunc->SolarDayToLunarDay(bDate.month(), bDate.day());
        //判断起始时间的农历月份是否大于日程的月份，如果大于则说明起始时间的农历年份没有对应的重复日程，直接计算下一个农历年份
        if (startLunarInfo.LunarMonthName > info.LunarMonthName) {
            //更新起始时间
            //农历一个月的天数范围为28-30，则农历月份最小时间查为（间隔月份*28)天
            int miniInterval = (12 - startLunarInfo.LunarMonthName + info.LunarMonthName) * 28 - startLunarInfo.LunarDay;
            bDate = bDate.addDays(miniInterval);
            continue;
        } else if (startLunarInfo.LunarMonthName == info.LunarMonthName) {
            //如果为同一个月，判断是否都为闰月或都不是闰月
            if (startLunarInfo.IsLeap == info.IsLeap) {
                //如果起始农历天日程农历天之前或为同一农历天
                if (startLunarInfo.LunarDay <= info.LunarDay) {
                    int intervalDay = info.LunarDay - startLunarInfo.LunarDay;
                    //如果该农历月没有这一天，那就计算下一年
                    if (startLunarInfo.LunarDay + intervalDay > startLunarInfo.LunarMonthDays) {
                        //农历一年最少有363天
                        bDate = bDate.addDays(LunarYearMiniDays);
                        continue;
                    }
                    //如果有这一天则添加对应的日期
                    bDate = bDate.addDays(intervalDay);
                    //如果超过范围则退出
                    if (addSolarMap(solar, bDate, count, LunarYearMiniDays)) {
                        break;
                    }
                    continue;
                } else {
                    //如果起始农历天在日程农历天之后，则更新到下一年
                    int offsetMiniDay = LunarYearMiniDays - (startLunarInfo.LunarDay - info.LunarDay);
                    bDate = bDate.addDays(offsetMiniDay);
                    continue;
                }
            } else {
                //如果起始时间为闰月
                if (startLunarInfo.IsLeap) {
                    //如果起始农历天在日程农历天之后，则更新到下一年
                    int offsetMiniDay = LunarYearMiniDays - (startLunarInfo.LunarDay - info.LunarDay);
                    bDate = bDate.addDays(offsetMiniDay);
                    continue;
                }
                //如果日程开始时间为闰月
                if (info.IsLeap) {
                    //更新起始时间的日期，更新为农历日期的下个月的初一
                    bDate = bDate.addDays(startLunarInfo.LunarMonthDays - startLunarInfo.LunarDay + 1);
                    continue;
                }
            }

        } else {
            //如果起始时间的农历月份早于日期的月份
            //农历一个月的天数范围为28-30
            //相差的月份
            int offsetMonth = info.LunarMonthName - startLunarInfo.LunarMonthName;
            int offsetMiniDay = 0;
            if (offsetMonth > 1) {
                //如果间隔大于一个月
                offsetMiniDay = (info.LunarMonthName - startLunarInfo.LunarMonthName) * 28 - startLunarInfo.LunarDay + info.LunarDay;
            } else {
                //如果间隔等于一个月，因为前面已经判断了是否为同一个月，所以不存在间隔为0的情况
                offsetMiniDay = startLunarInfo.LunarMonthDays - startLunarInfo.LunarDay + info.LunarDay;
            }

            bDate = bDate.addDays(offsetMiniDay);
            continue;
        }
    }
    return solar;
}

lunarInfo LunarDateInfo::getNextMonthLunarDay(QDate &nextDate, const lunarInfo &info)
{
    LunarCalendar *lunc = LunarCalendar::GetLunarCalendar(nextDate.year());
    lunarInfo nextinfo = lunc->SolarDayToLunarDay(nextDate.month(), nextDate.day());
    //判断农历的天是否为重复的天，比如一月初一，加上一月份的天数应该为二月初一
    //如果不一样，则说明这个月没有这一天，比如正月三十，加上正月的月份天数，到了二月份是没有三十的，
    if (nextinfo.LunarDay != info.LunarDay) {
        nextDate = nextDate.addDays(info.LunarDay - nextinfo.LunarDay);
        return getNextMonthLunarDay(nextDate, info);
    }
    return nextinfo;
}

/**
 * @brief  ParseRRule 解析重复规则
 * @param rule 规则字符串
 * @return stRRuleOptions 包含重复规则相关字段的结构体
 */
LunarDateInfo::LunnarRRule LunarDateInfo::ParseRRule(const QString &rule)
{
    //无规则的不走这里判断所以此处默认rule不为空
    //局部变量初始化
    LunnarRRule options = RRule_None;
    QStringList rruleslist = rule.split(";", Qt::SkipEmptyParts);
    //rpeat重复规则 0 无  1 每天 2 每个工作日 3 每周 4每月 5每年
    //type结束重复类型 0 永不 1  多少次结束  2 结束日期
    if (rruleslist.contains("FREQ=MONTHLY")) {
        options = RRule_Month;
    } else if (rruleslist.contains("FREQ=YEARLY")) {
        options = RRule_Year;
    }
    return options;
}

bool LunarDateInfo::isWithinTimeFrame(const QDate &solarDate)
{
    QDate endDate = solarDate.addDays(m_dateInterval);
    //如果日程结束时间在查询起始时间之前，或者日程开始时间在查询截止时间之后，说明不在获取范围内
    return !(endDate < m_queryStartDate || solarDate > m_queryEndDate);
}

bool LunarDateInfo::addSolarMap(QMap<int, QDate> &solarMap, QDate &nextDate, int &count, const int addDays)
{
    //如果获取到的时间在查询范围内
    if (isWithinTimeFrame(nextDate)) {
        solarMap[count] = nextDate;
    }
    count++;
    //    当结束重复为按多少次结束判断时，检查重复次数是否达到，达到则退出
    //    当重复次数达到最大限制直接返回
    //    duration > 0 表示结束与次数
    if ((m_recurenceRule->duration() > 0 && (m_recurenceRule->duration() - 1) < count) || count > RECURENCELIMIT) {
        return true;
    }

    //更新查询开始时间
    nextDate = nextDate.addDays(addDays);

    //判断next是否有效,时间大于RRule的until
    //判断next是否大于查询的截止时间,这里应该比较date，而不是datetime，如果是非全天的日程，这个设计具体时间的问题，会导致返回的job个数出现问题
    if ((m_recurenceRule->duration() == 0 && nextDate > m_recurenceRule->endDt().date())
            || nextDate > m_queryEndDate) {
        return true;
    }
    return false;
}
