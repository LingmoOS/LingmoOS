// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TIMEDATASTRUCT_H
#define TIMEDATASTRUCT_H

#include <QDateTime>
#include <QVector>
/**
 * @brief The ToDateTimeInfo struct 修改时间信息
 * m_Date   日期信息
 * m_Time   时间信息
 * hasDate  是否存在日期信息
 * hasTime  是否存在时间信息
 */
struct DateTimeInfo {
    QDate m_Date{};
    QTime m_Time{};
    bool  hasDate{false};
    bool  hasTime{false};
    QString strDateTime; //保存原始数据
};
/**
 * @brief The SuggestDatetimeInfo struct  建议时间信息
 * datetime         时间信息
 * hasTime          是否存在具体时间点
 */
struct SuggestDatetimeInfo {
    QDateTime datetime {};
    bool hasTime {false};
    QString strDateTime; //保存原始数据
};

/**
 * @brief The ScheduleDateTime struct   语言时间信息
 * m_DateTime       时间信息，获取时间点的时候需要用到
 * m_sugTime        建议使用的时间信息
 */
struct SemanticsDateTime {
    QVector<DateTimeInfo>  dateTime{};
    QVector<SuggestDatetimeInfo>  suggestDatetime{};
    /**
     * @brief clear 清空数据
     */
    void clear()
    {
        this->dateTime.clear();
        this->suggestDatetime.clear();
    }
};



#endif // TIMEDATASTRUCT_H
