// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calendarmanager.h"
#include "constants.h"
#include "commondef.h"

#include <QDBusConnection>

CalendarManager* CalendarManager::m_scheduleManager = nullptr;
CalendarManager* CalendarManager::instance()
{
    CaHuangLiDayInfo::registerMetaType();
    if (!m_scheduleManager) {
        m_scheduleManager = new CalendarManager;
        qRegisterMetaType<QMap<QDate, bool>>("QMap<QDate,bool>");
        qRegisterMetaType<QMap<QDate, CaHuangLiDayInfo>>("QMap<QDate, CaHuangLiDayInfo>");
        qRegisterMetaType<QMap<QDate, int>>("QMap<QDate, int>");
    }
    return m_scheduleManager;
}

CalendarManager::WeekDayFormat CalendarManager::weekDayFormat() const
{
    return m_weekDayFormat;
}

//设置周显示格式
void CalendarManager::setWeekDayFormat(WeekDayFormat format)
{
    if (m_weekDayFormat == format)
        return;

    m_weekDayFormat = format;
    Q_EMIT weekDayFormatChanged(format);
}

// 根据日期获取当前周第一天的日期
QDate CalendarManager::firstDayOfWeek(const QDate& date)
{
    //根据选择时间周工作日和每周第一天的周工作日得到偏移量
    int offset = date.dayOfWeek() - firstDayOfWeek();
    //根据偏移量获取需要添加还有减去的偏移天数
    const int offsetDay = (offset + DAYS_INWEEK) % DAYS_INWEEK;
    //返回这周第一天的日期
    return date.addDays(-offsetDay);
}

void CalendarManager::setDateFormat(DateFormat format)
{
    if (m_dateFormat == format)
        return;

    m_dateFormat = format;
    Q_EMIT dateFormatChanged(m_dateFormat);
}

CalendarManager::DateFormat CalendarManager::dateFormat() const
{
    return m_dateFormat;
}

/**
 * @brief getFirstDayOfWeek
 * @return 一周首日
 */
Qt::DayOfWeek CalendarManager::firstDayOfWeek()
{
    return Qt::DayOfWeek(m_firstDayOfWeek);
}

void CalendarManager::setFirstDayOfWeek(int day, bool update)
{
    if (m_firstDayOfWeek == day)
        return;

    m_firstDayOfWeek = day;
    //更新显示界面
    if (update) {
        emit sidebarFirstDayChanged(day);
    }
}

CalendarManager::CalendarManager(QObject* parent)
    : QObject(parent)
    , m_timeDateDbus(new DaemonTimeDate(DateTimeServiceName, DateTimePath, QDBusConnection::sessionBus(), this))
    , m_firstDayOfWeek(Qt::Sunday)
    , m_currentDateTime(QDateTime::currentDateTime())
    , m_selectDate(m_currentDateTime.date())
    , m_weekDayFormat(WeekDayFormat_1)
    , m_dateFormat(DateFormat_1)
{
    initConnection();
    initData();
}

CalendarManager::~CalendarManager()
{
}

/**
 * @brief CalendarManager::initData 初始化数据
 */
void CalendarManager::initData()
{
    //获取时间日期格式
    const int dateFormat = m_timeDateDbus->shortDateFormat();
    const int weekdayFormat = m_timeDateDbus->weekdayFormat();
    const int weekBegins = m_timeDateDbus->weekBegins();
    //设置时间日期格式
    setDateFormat(static_cast<DateFormat>(dateFormat));
    setWeekDayFormat(static_cast<WeekDayFormat>(weekdayFormat));
    onWeekBeginsChanged(weekBegins);
}

/**
 * @brief CalendarManager::initConnection   初始化关联
 */
void CalendarManager::initConnection()
{
    connect(m_timeDateDbus, &DaemonTimeDate::ShortDateFormatChanged, this, &CalendarManager::onDateFormatChanged);
    connect(m_timeDateDbus, &DaemonTimeDate::WeekBeginsChanged, this, &CalendarManager::onWeekBeginsChanged);
    connect(m_timeDateDbus, &DaemonTimeDate::WeekdayFormatChanged, this, &CalendarManager::onWeekDayFormatChanged);
}

/**
 * @brief CalendarManager::WeekBeginsChanged    关联dbus信号，每周首日改变事触发
 * @param value
 */
void CalendarManager::onWeekBeginsChanged(int value)
{
    // Qt::DayOfWeek 从1开始（1表示周一），dbus获取的首日为从0开始（0表示周一），故加一
    setFirstDayOfWeek(++value);
}

/**
 * @brief CalendarManager::slotDateFormatChanged 更新日期显示格式
 * @param value
 */
void CalendarManager::onDateFormatChanged(int value)
{
    setDateFormat(static_cast<DateFormat>(value));
}

void CalendarManager::onWeekDayFormatChanged(int value)
{
    setWeekDayFormat(static_cast<WeekDayFormat>(value));
}
