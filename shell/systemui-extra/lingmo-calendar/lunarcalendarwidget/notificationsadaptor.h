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

#ifndef NOTIFICATIONSADAPTOR_H
#define NOTIFICATIONSADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "frmlunarcalendarwidget.h"
#include "lunarcalendarwidget.h"
#include <QDebug>
QT_BEGIN_NAMESPACE
class QByteArray;
template <class T> class QList;
template <class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE
/*
 * Adaptor class for interface org.freedesktop.Notifications
 */
class NotificationsAdaptor : public QDBusAbstractAdaptor
{

    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.calendar.Notifications")
    Q_CLASSINFO(
        "D-Bus Introspection",
        "<interface name=\"org.lingmo.calendar.Notifications\">\n"
        "  <method name=\"ScheduleNotification\">\n"
        "    <arg direction=\"in\" type=\"i\" name=\"hour\"/>\n"
        "    <arg direction=\"in\" type=\"i\" name=\"minute\"/>\n"
        "    <arg direction=\"in\" type=\"i\" name=\"day\"/>\n"
        "    <arg direction=\"in\" type=\"i\" name=\"month\"/>\n"
        "    <arg direction=\"in\" type=\"i\" name=\"year\"/>\n"
        "    <arg direction=\"in\" type=\"i\" name=\"end_minute\"/>\n"
        "    <arg direction=\"in\" type=\"i\" name=\"end_hour\"/>\n"
        "    <arg direction=\"in\" type=\"s\" name=\"m_descript\"/>\n"
        "  </method>\n"
        "  <method name=\"OpenSchedule\">\n"
        "    <arg direction=\"in\" type=\"i\" name=\"day\"/>\n"
        "    <arg direction=\"in\" type=\"i\" name=\"month\"/>\n"
        "    <arg direction=\"in\" type=\"i\" name=\"year\"/>\n"
        "  </method>\n"
        "  <method name=\"DeleteSchedule\">\n"
        "    <arg direction=\"in\" type=\"s\" name=\"markid\"/>\n"
        "  </method>\n"
        "</interface>\n"
    )
public:
    NotificationsAdaptor(QObject *parent);
    virtual ~NotificationsAdaptor();

    frmLunarCalendarWidget *m_frmLunarWidget;
    LunarCalendarWidget *m_widget;
public:
    void changeWidowpos(bool restore = false);// PROPERTIES
public Q_SLOTS: // METHODS
    void ScheduleNotification(int hour, int minute, int day, int month, int year, int end_minute, int end_hour, const QString m_descript);
    void OpenSchedule(int day, int month, int year);
    void DeleteSchedule(QString markid);
Q_SIGNALS:
    void isShow(bool isV);
    void dayWantToChange(int day, int month, int year);
    // SIGNALS
};
#endif // NOTIFICATIONSADAPTOR_H
