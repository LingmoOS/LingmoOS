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
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

#include "kappwidgetprovider.h"
#include <kappwidgetcommon.h>

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

namespace AppWidget
{
KAppWidgetProvider::KAppWidgetProvider(QString providername, QObject *parent): QObject(parent)
{
    if(providername.isEmpty()) {
        qWarning() << "providername isEmpty!";
        return;
    }

    QString servicename = KAppWidgetCommon::getProviderDbusObject(providername);
    QString objectpath = KAppWidgetCommon::getProviderDbusPath(providername);
    QString objectinterface = KAppWidgetCommon::getProviderDbusInterface(providername);
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (connection.registerService(servicename)) {
        connection.registerObject(objectpath, objectinterface, this, QDBusConnection::ExportAllSlots);
        qDebug() << "Regist Dbus Service success";

    }else {
        qCritical() << "Register DBus Service Error:" << connection.lastError().message();
    }
}
void KAppWidgetProvider::appWidgetRecevie(const QString &eventname, const QString &widgetname, const QDBusVariant &value)
{
    Q_UNUSED(eventname);
    Q_UNUSED(widgetname);
    Q_UNUSED(value);
    qDebug() << "KAppWidgetProvider::appWidgetRecevie";
}
void KAppWidgetProvider::appWidgetUpdate()
{
    qDebug() << "KAppWidgetProvider::appWidgetUpdate";
}
void KAppWidgetProvider::appWidgetDisable()
{
    qDebug() << "KAppWidgetProvider::appWidgetDisable";
}
void KAppWidgetProvider::appWidgetDeleted()
{
    qDebug() << "KAppWidgetProvider::appWidgetDeleted";
}
void KAppWidgetProvider::appWidgetEnable()
{
    qDebug() << "KAppWidgetProvider::appWidgetEnable";
}
void KAppWidgetProvider::appWidgetClicked(const QString &widgetname, const QDBusVariant &value)
{
    Q_UNUSED(widgetname);
    Q_UNUSED(value);
    qDebug() << "KAppWidgetProvider::appWidgetClicked";
}
}// namespace AppWidget
