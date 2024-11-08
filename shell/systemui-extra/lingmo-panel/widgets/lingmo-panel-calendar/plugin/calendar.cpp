/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "calendar.h"

#include <QDebug>
#include <QVariant>
#include <QDateTime>
#include <QGSettings>
#include <QStringLiteral>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QLocale>
#include <QTranslator>

#include "lingmosdk/lingmosdk-system/libkydate.h"

#define HOUR_SYSTEM_CONTROL "org.lingmo.control-center.panel.plugins"
#define DATE_FORMAT         "date"          // 日期格式：yyyy/MM/dd、yyyy-MM-dd
#define TIME_FORMAT         "hoursystem"    // 时间格式：12小时制、24小时制
#define EN_FORMAT           "en_US"

#define LINGMOSDK_TIMER_SERVER    "com.lingmo.lingmosdk.TimeServer"
#define LINGMOSDK_TIMER_PATH      "/com/lingmo/lingmosdk/Timer"
#define LINGMOSDK_TIMER_INTERFACE "com.lingmo.lingmosdk.TimeInterface"

#define LINGMOSDK_DATESERVER "com.lingmo.lingmosdk.DateServer"
#define LINGMOSDK_DATEPATH "/com/lingmo/lingmosdk/Date"
#define LINGMOSDK_DATEINTERFACE "com.lingmo.lingmosdk.DateInterface"

#define LINGMO_SIDEBAR_SERVER "org.lingmo.Sidebar"
#define LINGMO_SIDEBAR_PATH "/org/lingmo/Sidebar"
#define LINGMO_SIDEBAR_INTERFACE "org.lingmo.Sidebar"


#define SDK2CPP(func) []()->QString { char* p = func(); QString str(p); free(p); return str; }

Calendar::Calendar(QObject *parent) : QObject(parent)
{
    initGSetting();

    //使用系统提供的sdk刷新时间显示
    QDBusConnection::systemBus().connect(LINGMOSDK_TIMER_SERVER, LINGMOSDK_TIMER_PATH, LINGMOSDK_TIMER_INTERFACE,
                                         "TimeSignal",this, SLOT(timeSignalSlot()));
    QDBusConnection::systemBus().connect(LINGMOSDK_TIMER_SERVER, LINGMOSDK_TIMER_PATH, LINGMOSDK_TIMER_INTERFACE,
                                         "TimeChangeSignal",this, SLOT(timeSignalSlot()));
    QDBusConnection::sessionBus().connect(LINGMOSDK_DATESERVER, LINGMOSDK_DATEPATH, LINGMOSDK_DATEINTERFACE,
                                         "ShortDateSignal",this, SLOT(onShortDateSignal()));

    //get notification state
    QDBusConnection::sessionBus().connect(LINGMO_SIDEBAR_SERVER, LINGMO_SIDEBAR_PATH, LINGMO_SIDEBAR_INTERFACE,
                                          "stateChange", this, SLOT(onStateChanged(QString,QVariantMap)));
    //init shortformat date
    updateShortformatDate();
}

void Calendar::initGSetting()
{
    const QByteArray id(HOUR_SYSTEM_CONTROL);
    if (QGSettings::isSchemaInstalled(id)) {
        auto gSettings = new QGSettings(id, QByteArray(), this);

        QStringList keys = gSettings->keys();
        // "cn" ...
        if(keys.contains(DATE_FORMAT)) {
            m_dateFormat = gSettings->get(DATE_FORMAT).toString();
        }

        // 12, 24
        if(keys.contains(TIME_FORMAT)) {
            m_hourSystem = gSettings->get(TIME_FORMAT).toString();
        }

        connect(gSettings, &QGSettings::changed, this, [gSettings, this] (const QString &key) {
            if (key == DATE_FORMAT) {
                m_dateFormat = gSettings->get(DATE_FORMAT).toString();
                Q_EMIT timeUpdated();

            } else if(key == TIME_FORMAT) {
                m_hourSystem = gSettings->get(TIME_FORMAT).toString();
                Q_EMIT timeUpdated();
            }
        });
    }
}

void Calendar::updateShortformatDate()
{
    m_shortDate = SDK2CPP(kdk_system_get_shortformat_date)();

    if (m_shortDate.contains(QStringLiteral("/"))) {
        m_separator = QStringLiteral("/");
    } else if (m_shortDate.contains(QStringLiteral("-"))) {
        m_separator = QStringLiteral("-");
    } else if (m_shortDate.contains(QStringLiteral("."))) {
        m_separator = QStringLiteral(".");
    } else {
        m_separator = m_dateFormat == QStringLiteral("cn") ? QStringLiteral("/") : QStringLiteral("-");
    }
}

QString Calendar::time() const
{
    return SDK2CPP(kdk_system_nowtime)();
#if 0
    if (m_hourSystem == QStringLiteral("24")) {
        return QDateTime::currentDateTime().toString(QStringLiteral("HH:mm"));
    }

    return QDateTime::currentDateTime().toString(QStringLiteral("hh:mm ap")).split(" ").first();
#endif
}

QString Calendar::ap() const
{
    if (m_hourSystem == QStringLiteral("24")) {
        return {};
    }

    return QDateTime::currentDateTime().toString("ap");
}

QString Calendar::week() const
{
    if (QLocale::system().name() == EN_FORMAT) {
        return SDK2CPP(kdk_system_shortweek)();
    } else {
        return SDK2CPP(kdk_system_longweek)();
    }
    //return QDateTime::currentDateTime().toString("ddd");
}

QString Calendar::year() const
{
    QStringList dateParts = m_shortDate.split(separator());
    if (dateParts.count() > 0 && !dateParts[0].isEmpty()) {
        return dateParts[0];
    } else {
        qWarning() << "[lingmo-panel] calendar short format date from sdk invalid, return format 'yyyy'.";
        return QDateTime::currentDateTime().toString("yyyy");
    }
}

QString Calendar::month() const
{
    QStringList dateParts = m_shortDate.split(separator());
    if (m_shortDate.split(separator()).count() > 2 && !dateParts[1].isEmpty() && !dateParts[2].isEmpty()) {
        return dateParts[1] + m_separator + dateParts[2];
    } else {
        qWarning() << "[lingmo-panel] calendar short format date from sdk invalid, return format 'MM dd'.";
        QString fmt = "MM" + separator() + "dd";
        return QDateTime::currentDateTime().toString(fmt);
    }
}

QString Calendar::separator() const
{
    return m_separator;
}

QString Calendar::tooltipText() const
{
    return SDK2CPP(kdk_system_get_longformat_date)();
}

QStringList Calendar::allData() const
{
    QStringList list;
    list << time();
    list << ap();
    list << week();
    list << year();
    list << month();

    return list;
}

int Calendar::unreadMsgCount() const
{
    return m_unreadmsgCount;
}

QString Calendar::notifyIconTooltipText()
{
    if (unreadMsgCount() == 1) {
        return QString::number(1) + QString(tr(" notification"));
    } else if (unreadMsgCount() > 1) {
        return QString::number(unreadMsgCount()) + QString(tr(" notifications"));
    }

    return QString(tr("Notification center"));
}

void Calendar::openSidebar()
{
    QDBusInterface sidebarIFace(LINGMO_SIDEBAR_SERVER, LINGMO_SIDEBAR_PATH, LINGMO_SIDEBAR_INTERFACE, QDBusConnection::sessionBus(), this);
    sidebarIFace.asyncCallWithArgumentList("active", (QVariantList() << "sidebar"));
}

void Calendar::timeSignalSlot()
{
    m_shortDate = SDK2CPP(kdk_system_get_shortformat_date)();
    Q_EMIT timeUpdated();
}

void Calendar::onStateChanged(const QString &module, const QVariantMap &data)
{
    if (module == QStringLiteral("sidebar")) {
        if (data.contains(QStringLiteral("unreadMsg"))) {
            m_unreadmsgCount = data.value(QStringLiteral("unreadMsg")).toInt();
            Q_EMIT unreadMsgChanged();
        }
    }
}

void Calendar::onShortDateSignal()
{
    updateShortformatDate();
    Q_EMIT timeUpdated();
}

void Calendar::activeCalendar()
{
    QDBusInterface calendar(QStringLiteral("org.lingmo.lingmo_calendar"),
                            QStringLiteral("/org/lingmo/lingmo_calendar"),
                            QStringLiteral("org.lingmo.lingmo_calendar"));
    calendar.asyncCall("command_Control", QVariant(""));
}
