// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LUNARDATEINFO_H
#define LUNARDATEINFO_H

#include "lunarandfestival.h"
#include "method_interface.h"
#include "recurrencerule.h"

#include <QDate>
#include <QMap>

/**
 * @brief The LunarDateInfo class
 * 农历时间，用于计算农历重复日程
 */
class LunarDateInfo
{
public:
    //农历日程只支持每年和每月
    enum LunnarRRule {
        RRule_None, //不重复
        RRule_Month, //每月
        RRule_Year, //每年
    };

    explicit LunarDateInfo(KCalendarCore::RecurrenceRule *rruleStr, const qint64 interval);
    /**
     * @brief getRRuleStartDate         获取重复农历日程开始时间(公历)集
     * @param beginDate                 查询的起始时间
     * @param endDate                   查询的截止时间
     * @param solarDate                 日程开始时间
     * @return                          日程的重复日程id和对应的开始时间
     */
    QMap<int, QDate> getRRuleStartDate(const QDate &beginDate, const QDate &endDate, const QDate &solarDate);

private:
    /**
     * @brief getAllNextMonthLunarDayBySolar        获取重复类型为每月的农历日程开始时间集
     * @param solarDate                             农历日程的开始时间
     * @return                                      重复农历日程开始时间集
     */
    QMap<int, QDate> getAllNextMonthLunarDayBySolar(const QDate &solarDate);

    /**
     * @brief getAllNextYearLunarDayBySolar        获取重复类型为每年的农历日程开始时间集
     * @param solarDate                             农历日程的开始时间
     * @return                                      重复农历日程开始时间集
     */
    QMap<int, QDate> getAllNextYearLunarDayBySolar(const QDate &solarDate);

    /**
     * @brief getNextMonthLunarDay                  获取下个月对应的农历信息
     * @param nextDate                              需要计算的公历时间
     * @param info                                  重复的农历信息
     * @return                                      计算公历时间对应的农历信息
     */
    lunarInfo getNextMonthLunarDay(QDate &nextDate, const lunarInfo &info);

    /**
     * @brief ParseRRule            解析重复规则
     * @param rule                  重复规则
     * @return
     */
    LunnarRRule ParseRRule(const QString &rule);

    /**
     * @brief isWithinTimeFrame     是否在查询时间范围内
     * @return
     */
    bool isWithinTimeFrame(const QDate &solarDate);

    /**
     * @brief addSolarMap       将计算出来的时间添加到Map中，并更新时间和重复次数
     * @param solarMap          符合要求的时间集合
     * @param nextDate          重复农历日程对应的公历时间
     * @param count             重复次数
     * @param addDays           需要更新的天数
     * @return                  如果返回true表示计算出的次数或者时间超过范围，否则返回false
     */
    bool addSolarMap(QMap<int, QDate> &solarMap, QDate &nextDate, int &count, const int addDays);

private:
    KCalendarCore::RecurrenceRule *m_recurenceRule;
    LunnarRRule m_rruleType;

    //    stRRuleOptions m_options;
    QDate m_queryStartDate;
    QDate m_queryEndDate;
    qint64 m_dateInterval; //日程开始结束间隔天数
};

#endif // LUNARDATEINFO_H
