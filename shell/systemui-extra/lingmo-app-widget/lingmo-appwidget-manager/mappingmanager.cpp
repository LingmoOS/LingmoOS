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

#include "mappingmanager.h"
#include "appwidgetproviderproxy.h"
#include "appwidgetcommon.h"
#include "asyncproviderproxycreator.h"

#include <QDBusInterface>
#include <QDebug>
#include <QCoreApplication>

namespace AppWidget {

MappingManager::MappingManager(QObject *parent)
    :QObject(parent)
{
}
MappingManager::~MappingManager()
{
    for (int i = 0; i < m_providerproxy.count(); i++) {
        if (m_providerproxy.at(i) != nullptr) {
            delete m_providerproxy.at(i);
        }
    }
}
AppWidgetProviderProxy* MappingManager::findProviderProxyByProviderName(const QString providername)
{
    //通过名字从提供端容器查找到提供端代理
    for (int i = 0 ; i < m_providerproxy.size(); i++) {
        if (m_providerproxy.at(i)) {
            if (m_providerproxy.at(i)->getAppWidgetProviderProxyProviderName() == providername) {
                return m_providerproxy.at(i);
            }
        }
    }
    return nullptr;
}
AppWidgetProviderProxy* MappingManager::findProviderProxyByAppWidgetId(int id)
{
    int index = AppWidgetCommon::getProviderIndexFromdecodeAppWidgetId(id);
    if (index < m_providerproxy.size()) {
        return m_providerproxy.at(index);
    }
    return nullptr;
}
AppWidgetProxy* MappingManager::findAppWidgetProxyByAppWidgetId(int id)
{
    qDebug() << __FILE__ << __FUNCTION__ << "appwidget`s id :" << id;
    int index1 = AppWidgetCommon::getProviderIndexFromdecodeAppWidgetId(id);
    AppWidgetProviderProxy* providerproxy;
    if (index1 < m_providerproxy.size()) {
        providerproxy = m_providerproxy.at(index1);
        if (!providerproxy) {
            qWarning() << __FILE__ << __FUNCTION__ << "providerproxy is null";
            return nullptr;
        }
    }
    AppWidgetProxy* appwidgetproxy = providerproxy->getAppWidgetProxy(id);
    if (appwidgetproxy) {
        return appwidgetproxy;
    }else {
        qWarning() << __FILE__ << __FUNCTION__ << "appwidgetproxy is null";
        return nullptr;
    }
}
QVector<AppWidgetProxy*> MappingManager::findAppWidgetProxyByUserName(QString username)
{
    qDebug() << __FILE__ << __FUNCTION__ << "UserName`s :" << username;
    QVector<AppWidgetProxy*> appwidgetlist;
    AppWidgetProviderProxy* providerproxy;
    for (int i = 0; i < m_providerproxy.size(); ++i) {
        providerproxy = m_providerproxy.at(i);
        if (!providerproxy) {
            qWarning() << __FILE__ << __FUNCTION__ << "providerproxy is null";
            continue;
        }
        QVector<AppWidgetProxy*> appvec(providerproxy->getAppwidgetProxyByUser(username));
        for (int var = 0; var < appvec.size(); ++var) {
            appwidgetlist.append(appvec.at(var));
        }
    }
    return appwidgetlist;
}

AppWidgetProviderProxy* MappingManager::asyncCreateProviderProxy(QString providername)
{
    qDebug() << __FILE__ << __FUNCTION__ << "providername :" << providername;
    int index = getUsableIndex();

    AsyncProviderProxyCreator providerProxyCreator;
    providerProxyCreator.startCreatorByIndexAndName(index, providername);

    while (!providerProxyCreator.isFinished()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        QThread::msleep(1);
    }
    if (providerProxyCreator.getProviderProxy()) {
        addProviderProxy(index, providerProxyCreator.getProviderProxy());
    }
    return providerProxyCreator.getProviderProxy();
}

void MappingManager::destroyProviderProxy(AppWidgetProviderProxy* providerproxy)
{
    if (!providerproxy) {
        qWarning() << __FILE__ << __FUNCTION__ << "providerproxy is null";
        return ;
    }
    int index = providerproxy->getAppWidgetProviderProxyIndex();
    deleteProviderProxy(index);
}
int MappingManager::getUsableIndex()
{
    int size = m_providerproxy.size();
    for (int i = 0; i < size; i++) {
        if (m_providerproxy.at(i) == nullptr) {
            return i;
        }
    }
    return size;
}
void MappingManager::addProviderProxy(int index, AppWidgetProviderProxy* providerproxy)
{
    int size = m_providerproxy.size();
    if (index < size){
        m_providerproxy.replace(index, providerproxy);
    } else {
        m_providerproxy.append(providerproxy);
    }
}
void MappingManager::deleteProviderProxy(int index)
{
    qDebug() << __FILE__ << __FUNCTION__;
    int size = m_providerproxy.size();
    if (index < size) {
        AppWidgetProviderProxy * providerproxy = m_providerproxy.at(index);
        m_providerproxy.replace(index, nullptr);
        if (providerproxy) {
            delete providerproxy;
        }
    }
}
} // namespace AppWidget
