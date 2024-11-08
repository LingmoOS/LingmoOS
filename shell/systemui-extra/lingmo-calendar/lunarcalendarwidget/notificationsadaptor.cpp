/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include "schedulestruct.h"
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include "frmlunarcalendarwidget.h"
#include "lunarcalendarwidget.h"
#include "calendardatabase.h"
/*
 * Implementation of adaptor class NotificationsAdaptor
 */
const int WIDGET_GAP = 8;
#include "notificationsadaptor.h"
class LunarCalendarItem;
static const QString NotificationsDBusService = "org.lingmo.calendar.Notifications";
static const QString NotificationsDBusPath = "/org/lingmo/calendar/Notifications";
NotificationsAdaptor::NotificationsAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent)
{
    // constructor
    //    this->parent() = parent;
    setAutoRelaySignals(true);
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.interface()->registerService(
                  NotificationsDBusService,
                  QDBusConnectionInterface::ReplaceExistingService,
                  QDBusConnectionInterface::AllowReplacement);
    connection.registerObject(NotificationsDBusPath, this);
}
NotificationsAdaptor::~NotificationsAdaptor()
{
    // destructor
}
/**
 * @brief NotificationsAdaptor::ScheduleNotification
 * @param hour 开始小时
 * @param minute 开始分钟
 * @param day 开始天
 * @param month 开始月
 * @param shangwu 是上午/下午
 * @param end_hour 结束小时
 * @param end_minute 结束分钟
 * @param m_descript 内容
 * @param markId id QDateTime::currentDateTime().toString("yyyyMMddhhmmss");获取
 * @return
 */
void NotificationsAdaptor::ScheduleNotification(int hour, int minute, int day, int month, int  year, int end_minute, int end_hour, const QString m_descript)
{
    //2024.7.12日程新需求 31068 如果开始时间结束时间都是空 按照全天处理
    qDebug() << "开始通过Dbus添加到日程";
    //    display = getenv("DISPLAY");
    //    userName = getenv("USER"); //获取当前用户名
    //    xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
    MarkInfo info;
    QDate beginDate(year, month, day);
    info.m_markId = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    info.m_descript = m_descript;
    info.minute = info.Alarm_end_minute = minute;
    info.hour = info.Alarm_end_hour = hour;
    info.day = day;
    info.week = beginDate.dayOfWeek();
    info.month = month;
    info.m_markStartDate = beginDate;
    if(hour <= 12)
    {
        info.shangwu = tr("morning");
    }
    else
    {
        info.shangwu = tr("afternoon");
    }
    //    info.shangwu = shangwu;
    info.end_hour = end_hour;
    info.end_minute = end_minute;
    if(hour == 0 && minute == 0 && end_hour == 0 && end_minute == 0 || hour == 0 && minute == 0 || end_hour == 0 && end_minute == 0 )
    {
        info.m_isAllDay = true;
        CalendarDataBase::getInstance().AlarmType = 10;
        //        info.remind=tr("At 9am on the same day of the schedule");
        info.AlldayRemind = tr("At 9am on the same day of the schedule");
        info.hour = info.Alarm_end_hour = 0;
        info.minute = info.Alarm_end_minute = 0;
        info.end_hour = 23;
        info.end_minute = 59;
    }
    else
    {
        info.m_isAllDay = false;
        CalendarDataBase::getInstance().AlarmType = 1;
        info.remind = tr("Schedule Begin");
    }
    //    QDate date(QDate::currentDate().year(),month,day);//仅仅适合今年
    //    info.m_markStartDate=date;
    //    info.m_markEndDate=QDate::currentDate();

    info.end_day = 0;
    info.end_month = 0;
    info.timeLong = "";
    //    info.remind=tr("Schedule Begin");
    info.repeat = tr("No Repeat");
    info.beginrepeat = "";
    info.endrepeat = QDateTime::currentDateTime();
    info.beginDateTime = QDateTime(beginDate);
    info.endDateTime = QDateTime::currentDateTime();

    info.m_isLunar = false;
    //    info.AlldayRemind="";
    CalendarDataBase::getInstance().add(info);
    CalendarDataBase::getInstance().insertDataSlot(info);
    qDebug() << "通过Dbus添加成功了";
}

void NotificationsAdaptor::OpenSchedule(int day, int month, int year)
{
    Q_EMIT dayWantToChange(day, month, year);
}
void NotificationsAdaptor::DeleteSchedule(QString markid)
{
    CalendarDataBase::getInstance().deleteDataSlotfromMarkID(markid);
}
