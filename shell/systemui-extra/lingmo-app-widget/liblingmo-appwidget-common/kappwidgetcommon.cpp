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

#include "kappwidgetcommon.h"
#include <QDebug>
namespace AppWidget
{
KAppWidgetCommon::KAppWidgetCommon(QObject *parent) : QObject(parent)
{

}
QString KAppWidgetCommon::getProviderDbusObject(const QString &providername)
{
    if(providername.isEmpty()) {
        qWarning() << "providername isEmpty!";
        return QString();
    }
    QString object = QString("org.lingmo.appwidget.provider.%1").arg(providername);
    return object;
}
QString KAppWidgetCommon::getProviderDbusPath(const QString &providername)
{
    if(providername.isEmpty()) {
        qWarning() << "providername isEmpty!";
        return QString();
    }
    QString objectpath = QString("/org/lingmo/appwidget/provider");
    return objectpath;
}
QString KAppWidgetCommon::getProviderDbusInterface(const QString &providername)
{
    if(providername.isEmpty()) {
        qWarning() << "providername isEmpty!";
        return QString();
    }
    QString interface = QString("org.lingmo.appwidget.provider.%1").arg(providername);
    return interface;
}


QString KAppWidgetCommon::getManagerDbusObject()
{
    QString object = QString("org.lingmo.appwidget.manager");
    return object;
}
QString KAppWidgetCommon::getManagerDbusPath()
{
    QString objectpath = QString("/");
    return objectpath;
}
QString KAppWidgetCommon::getManagerDbusInterface()
{
    QString interface = QString("org.lingmo.appwidget.manager");
    return interface;
}

QString KAppWidgetCommon::getAppWidgetDbusObject(const QString &username, const int &id)
{
    Q_UNUSED(id);
    QString object = QString("org.lingmo.appwidget.%1").arg(username);
    return object;
}
QString KAppWidgetCommon::getAppWidgetDbusPath(const QString &username, const int &id)
{
    Q_UNUSED(username);
    QString objectpath = QString("/%1").arg(id);
    return objectpath;
}

QString KAppWidgetCommon::getAppWidgetDbusInterface(const QString &username, const int &id)
{
    Q_UNUSED(username);
    Q_UNUSED(id);
    QString interface = QString("org.lingmo.appwidget");
    return interface;
}

}// namespace AppWidget
