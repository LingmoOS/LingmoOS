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

#include "appwidgetproviderproxy.h"
#include "appwidgetcommon.h"
#include "appwidgetproxy.h"

#include <kappwidgetcommon.h>

#include <QDebug>
#include <QProcess>
#include <QDBusPendingCall>

namespace AppWidget {
AppWidgetProviderProxy::AppWidgetProviderProxy(int id, QString providername)
    :QDBusInterface(KAppWidgetCommon::getProviderDbusObject(providername),
                    KAppWidgetCommon::getProviderDbusPath(providername),
                    KAppWidgetCommon::getProviderDbusInterface(providername)),
    m_index(id),
    m_providername(providername)
{
}

AppWidgetProviderProxy::~AppWidgetProviderProxy()
{
    for (int i = 0; i < m_appwidgetproxy.count(); i++) {
        if(m_appwidgetproxy.at(i) != nullptr) {
             delete m_appwidgetproxy.at(i);
        }
    }
}

int AppWidgetProviderProxy::getAppWidgetProviderProxyIndex()
{
    return m_index;
}

QString AppWidgetProviderProxy::getAppWidgetProviderProxyProviderName()
{
    return m_providername;
}
QVector<AppWidgetProxy*> AppWidgetProviderProxy::getAppwidgetProxyByUser(QString username)
{
    QVector<AppWidgetProxy*> appwidgetvec;
    for (int i = 0; i < m_appwidgetproxy.size(); ++i) {
        if (m_appwidgetproxy.at(i) == nullptr || !m_appwidgetproxy.at(i)->isValid()) {
            continue;
        }
        if(m_appwidgetproxy.at(i)->getAppWidgetProxyUserName() == username) {
            appwidgetvec.append(m_appwidgetproxy.at(i));
        }
    }
    return appwidgetvec;
}

QMap<QString, QVariant> AppWidgetProviderProxy::getAppWidgetProviderProxyData()
{
    return m_data;
}

AppWidgetProxy* AppWidgetProviderProxy::getAppWidgetProxy(int id)
{
    int index = AppWidgetCommon::getWidgetIndexFromdecodeAppWidgetId(id);
    if(m_appwidgetproxy.at(index)) {
        return m_appwidgetproxy.at(index);
    }else {
        return nullptr;
    }
}

void AppWidgetProviderProxy::updateAppWidgetProviderProxyData(QMap<QString, QVariant> data)
{
    m_data = data;
    int size = m_appwidgetproxy.size();
    for(int i = 0; i < size; i++) {
        if(m_appwidgetproxy.at(i)) {
            m_appwidgetproxy.at(i)->updateAppWidget();
        }
    }
}

void AppWidgetProviderProxy::appWidgetRecevie(QString eventname, QString controlname, QDBusVariant value)
{
    QVariant date;
    date.setValue(value);
    asyncCall("appWidgetRecevie", eventname, controlname, date);
}

void AppWidgetProviderProxy::appWidgetUpdate()
{
    asyncCall("appWidgetUpdate");
}

void AppWidgetProviderProxy::appWidgetDisable()
{
    asyncCall("appWidgetDisable");
}

void AppWidgetProviderProxy::appWidgetEnable()
{
    asyncCall("appWidgetEnable");
}

void AppWidgetProviderProxy::appWidgetDeleted()
{
    asyncCall("appWidgetDeleted");
}

void AppWidgetProviderProxy::appWidgetClicked(QString cotrolname, QDBusVariant value)
{
    qDebug() << __FILE__ << __FUNCTION__ << ": Appwidgetname: " << cotrolname;
    QVariant date;
    date.setValue(value);
    asyncCall("appWidgetClicked", cotrolname, date);
//    system("lingmo-control-center");
}

bool AppWidgetProviderProxy::appWidgetProxyIsEmpty()
{
    int size = m_appwidgetproxy.size();
    for(int i = 0; i < size; i++) {
        if(m_appwidgetproxy.at(i) != nullptr) {
            return false;
        }
    }
    return true;
}

AppWidgetProxy* AppWidgetProviderProxy::createAppWidgetProxy(QString appwidgetname, QString username)
{
    int index1 = m_index;
    int index2 = getUsableIndex();
    int id = AppWidgetCommon::createAppWidgetId(index1, index2);
    AppWidgetProxy* appwidgetproxy = new AppWidgetProxy(id, appwidgetname, username);
    addAppWidgetProxy(index2, appwidgetproxy);
    return appwidgetproxy;
}

bool AppWidgetProviderProxy::destroyAppWidgetProxy(int id)
{
    int index = AppWidgetCommon::getWidgetIndexFromdecodeAppWidgetId(id);
    return deleteAppWidgetProxy(index);
}

int* AppWidgetProviderProxy::getAllAppWidgetIds()
{
    return nullptr;
}

bool AppWidgetProviderProxy::addAppWidgetProxy(int index, AppWidgetProxy* appwidgetproxy)
{
    if(appwidgetproxy) {
        int size = m_appwidgetproxy.size();
        if(index < size) {
            m_appwidgetproxy.replace(index, appwidgetproxy);
        }else {
            m_appwidgetproxy.append(appwidgetproxy);
        }
        return true;
    }
    qWarning() << __FILE__ << __FUNCTION__ << "Appwidgetproxy is nullptr";
    return false;
}

bool AppWidgetProviderProxy::deleteAppWidgetProxy(int index)
{
    int size = m_appwidgetproxy.size();
    if(index < size) {
        if(m_appwidgetproxy.at(index)) {
            AppWidgetProxy* appwidgetproxy = m_appwidgetproxy.at(index);
            m_appwidgetproxy.replace(index, nullptr);
            if(appwidgetproxy != nullptr) {
                delete appwidgetproxy;
                appwidgetproxy = nullptr;
            }
            return true;
        }
    }
    qWarning() << __FILE__ << __FUNCTION__ << "Appwidgetproxy deleted failed";
    return false;
}

int AppWidgetProviderProxy::getUsableIndex()
{
    int size = m_appwidgetproxy.size();
    for(int i = 0; i < size; i++) {
       if(m_appwidgetproxy.at(i) == nullptr) {
           return i;
       }
    }
    return size;
}
} // namespace AppWidget
