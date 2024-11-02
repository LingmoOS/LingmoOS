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

#include "searchprovider.h"

#include <QDBusPendingCall>
#include <QDBusInterface>
#include <QDebug>
SearchProvider::SearchProvider(QString providername ,QObject *parent)
    : KAppWidgetProvider(providername , parent)
{
    searchinterface = new QDBusInterface("com.lingmo.search.service",
                                         "/",
                                         "org.lingmo.search.service",
                                         QDBusConnection::sessionBus());
}
void SearchProvider::appWidgetRecevie(const QString &eventname, const QString &widgetname, const QDBusVariant &value)
{
    Q_UNUSED(eventname);
    Q_UNUSED(widgetname);
    Q_UNUSED(value);
    qDebug() << "TestProvider::appWidgetRecevie";
}
void SearchProvider::appWidgetUpdate()
{
    qDebug() << "TestProvider::appWidgetUpdate";
}
void SearchProvider::appWidgetDisable()
{
    qDebug() << "TestProvider::appWidgetDisable";
}
void SearchProvider::appWidgetDeleted()
{
    qDebug() << "TestProvider::appWidgetDeleted";
}
void SearchProvider::appWidgetEnable()
{
    qDebug() << "TestProvider::appWidgetEnable";
}
void SearchProvider::appWidgetClicked(const QString &widgetname, const QDBusVariant &value)
{
    qDebug() << "TestProvider::appWidgetClicked";
    qDebug() << widgetname << value.variant();
    searchinterface->asyncCall("showWindow");
}
