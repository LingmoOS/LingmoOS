// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALENDARGENERALSETTINGS_H
#define DCALENDARGENERALSETTINGS_H

#include <Qt>
#include <QString>
#include <QSharedPointer>

/**
 * @brief The DCalendarGeneralSettings class
 * 日历通用设置
 */
class DCalendarGeneralSettings
{
public:
    enum TimeShowType {
        TwentyFour = 0, //24
        Twelve, //12
    };

    enum GeneralSettingSource {
        Source_Database, // 来自系统设置（即控制中心）
        Source_System,   // 来自数据库（旧版本日历配置存放在数据库）
        Source_Unknown,  // 用于确定枚举边界
    };

    typedef QSharedPointer<DCalendarGeneralSettings> Ptr;

    DCalendarGeneralSettings();
    DCalendarGeneralSettings(const DCalendarGeneralSettings &setting);

    DCalendarGeneralSettings *clone() const;

    Qt::DayOfWeek firstDayOfWeek() const;
    void setFirstDayOfWeek(const Qt::DayOfWeek &firstDayOfWeek);

    TimeShowType timeShowType() const;
    void setTimeShowType(const TimeShowType &timeShowType);

    static void toJsonString(const DCalendarGeneralSettings::Ptr &cgSet, QString &jsonStr);
    static bool fromJsonString(DCalendarGeneralSettings::Ptr &cgSet, const QString &jsonStr);

private:
    Qt::DayOfWeek m_firstDayOfWeek; //一周首日
    TimeShowType m_timeShowType;
};

#endif // DCALENDARGENERALSETTINGS_H
