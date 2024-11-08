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

#include "appwidgetproxy.h"

#include "appwidgetproviderproxy.h"
#include "appwidgetcommon.h"
#include <kappwidgetcommon.h>

#include <QDBusPendingCall>
#include <QDebug>

namespace AppWidget {
AppWidgetProxy::AppWidgetProxy(int id, QString appwidgetname, QString appwidgetuser)
    :QDBusInterface(KAppWidgetCommon::getAppWidgetDbusObject(appwidgetuser,id),
                    KAppWidgetCommon::getAppWidgetDbusPath(appwidgetuser,id),
                    KAppWidgetCommon::getAppWidgetDbusInterface(appwidgetuser,id)),
     m_id(id),
     m_appwidgetname(appwidgetname),
     m_username(appwidgetuser),
     m_provider(nullptr)
{
    connect(this, &AppWidgetProxy::clickedSignal, this, &AppWidgetProxy::onClicked);
    connect(this, &AppWidgetProxy::defineEventSignal, this, &AppWidgetProxy::onDefineSignalType);
}

AppWidgetProxy::~AppWidgetProxy() 
{
}

void AppWidgetProxy::setProviderProxy(AppWidgetProviderProxy * providerproxy)
{
    m_provider = providerproxy;
}

int AppWidgetProxy::getAppWidgetProxyId()
{
    return m_id;
}

QString AppWidgetProxy::getAppWidgetProxyAppWidgetName()
{
    return m_appwidgetname;
}
QString AppWidgetProxy::getAppWidgetProxyUserName()
{
    return m_username;
}
void AppWidgetProxy::updateAppWidget()
{
    qDebug() << "dbus 调用提供端小插件代理"  << m_id;
    this->asyncCall("updateAppWidget");
}
void AppWidgetProxy::updateUserStatus(QString status)
{
    qDebug() << "dbus updateUserStatus"  << status;
    this->asyncCall("updateUserStatus", status);
}
void AppWidgetProxy::onClicked(QString widgetname, QDBusVariant value)
{
    m_provider->appWidgetClicked(widgetname, value);
}

void AppWidgetProxy::onDefineSignalType(QString eventname, QString widgetname, QDBusVariant value)
{
    m_provider->appWidgetRecevie(eventname, widgetname, value);
}

} // namespace AppWidget
