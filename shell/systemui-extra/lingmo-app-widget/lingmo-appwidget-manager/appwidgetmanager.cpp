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

#include "appwidgetmanager.h"
#include "appwidgetproviderproxy.h"
#include "appwidgetproxy.h"
#include "mappingmanager.h"
#include "appwidgetconfiguration.h"
#include <kappwidgetcommon.h>
#include <QDebug>
#include <QDir>
#include <QDBusMetaType>

namespace AppWidget {

AppWidgetManager::AppWidgetManager(QObject *parent)
    : QObject(parent),
      m_mappingmanager(new MappingManager(this)),
      m_configmanager(new AppWidgetConfiguration(this))
{
    qDBusRegisterMetaType<QMap<QString, QString>>();
    registerCommunicationService();

    connect(m_configmanager, &AppWidgetConfiguration::appwidgetAdded, this, [this](QString appwidgetName) {
        Q_EMIT appWidgetAdded(appwidgetName);
    });
    connect(m_configmanager, &AppWidgetConfiguration::appwidgetRemoved, this, [this](QString appwidgetName) {
        Q_EMIT appWidgetDeleted(appwidgetName);
    });
}
AppWidgetManager::~AppWidgetManager()
{

}
//public slots
QStringList AppWidgetManager::getAppWidgetList()
{
    QStringList appwidgetlist;
    QDir dirinfo = QDir(APP_WIDGET_LIST);
    if (!dirinfo.exists()) {
        return QStringList();
    }
    QStringList filename ;
    filename << "*.qml";
    QStringList results;
    results = dirinfo.entryList(filename, QDir::Files, QDir::Name);
    for(int i=0; i<results.count(); i++) {
        QString str = results.at(i);
        str.remove(".qml");
        appwidgetlist << str;
    }
    return appwidgetlist;
}
QString AppWidgetManager::getAppWidgetUiFile(QString appwidgetname)
{
    if (appwidgetname.isEmpty()) {
        return QString();
    }
    return m_configmanager->getAppWidgetUiFile(appwidgetname);

}
QString AppWidgetManager::getAppWidgetConfig(QString appwidgetname, QString config)
{

    if (appwidgetname.isEmpty() || config.isEmpty()) {
        return QString();
    }
    return m_configmanager->getAppWidgetConfig(appwidgetname, config);
}
QMap<QString, QVariant> AppWidgetManager::getAppWidgetConfigs(QString appwidgetname)
{
    if (appwidgetname.isEmpty()) {
        return QMap<QString, QVariant>();
    }
    return m_configmanager->getAppWidgetConfigs(appwidgetname, "AppWidget");
}

QMap<QString, QString> AppWidgetManager::getAppWidgetLanguageFiles(QString appwidgetname)
{
    if (appwidgetname.isEmpty()) {
        return QMap<QString, QString>();
    }
    QMap<QString, QVariant> languageFiles = m_configmanager->getAppWidgetConfigs(appwidgetname, "LanguageFiles");
    QMap<QString, QString> appWidgetLanguageFiles;
    for (auto it = languageFiles.begin(); it != languageFiles.end(); ++it) {
        appWidgetLanguageFiles.insert(it.key(), it.value().toString());
    }

    return appWidgetLanguageFiles;
}

int AppWidgetManager::registerAppWidget(QString appwidgetname,QString username)
{
    if (appwidgetname.isEmpty() | username.isEmpty()) {
        return -1;
    }
    QString providername = m_configmanager->getProviderNameByAppWidgetName(appwidgetname);
    if (providername==NULL) {
        return -1;
    }
    AppWidgetProviderProxy* providerproxy = m_mappingmanager->findProviderProxyByProviderName(providername);
    if (!providerproxy) {
        providerproxy = createProviderProxy(providername);
    }
    AppWidgetProxy* appwidget = createAppWidgetProxy(providerproxy, appwidgetname, username);
    if (!appwidget) {
        return -1;
    }
    return appwidget->getAppWidgetProxyId();
}
bool AppWidgetManager::unregisterAppWidget(int appwidgetid)
{
    AppWidgetProviderProxy* providerproxy = m_mappingmanager->findProviderProxyByAppWidgetId(appwidgetid);
    if (!providerproxy) {
        qWarning() << __FILE__ << __FUNCTION__ <<  "Providerproxy is null";
        return true;
    }
    if (deleteAppWidgetProxy(providerproxy, appwidgetid)) {
        if (providerproxy->appWidgetProxyIsEmpty()) {
            deleteProviderProxy(providerproxy);
        }
    }else {
        return false;
    }
    return true;
}

QMap<QString, QVariant> AppWidgetManager::getAppWidgetData(int appwidgetid)
{
    AppWidgetProviderProxy* providerproxy = m_mappingmanager->findProviderProxyByAppWidgetId(appwidgetid);
    if (!providerproxy) {
        return QMap<QString, QVariant>();
    }
    return providerproxy->getAppWidgetProviderProxyData();
}
void AppWidgetManager::updateAppWidget(QString providername, QMap<QString, QVariant> data)
{
    qDebug() <<  __FILE__ << __FUNCTION__ << ": Providername = " << providername;
    AppWidgetProviderProxy * providerproxy = m_mappingmanager->findProviderProxyByProviderName(providername);
    if (!providerproxy) {
        return ;
    }
    providerproxy->updateAppWidgetProviderProxyData(data);
}
void AppWidgetManager::updateUserStatus(QString username, QString status)
{
    QVector<AppWidgetProxy*> appvec = m_mappingmanager->findAppWidgetProxyByUserName(username);
    for (int var = 0; var < appvec.size(); ++var) {
        appvec.at(var)->updateUserStatus(status);
    }
}
void AppWidgetManager::notifyProviderUpdate(int id) {
    AppWidgetProviderProxy* providerproxy = m_mappingmanager->findProviderProxyByAppWidgetId(id);
    providerproxy->appWidgetUpdate();
}

void AppWidgetManager::registerCommunicationService()
{
    QDBusConnection systembus = QDBusConnection::sessionBus();
    QString object = KAppWidgetCommon::getManagerDbusObject();
    QString objectPath = KAppWidgetCommon::getManagerDbusPath();
    QString interface = KAppWidgetCommon::getManagerDbusInterface();

    if (systembus.registerService(object)) {
        qDebug() << "Regist Dbus Service success";
        systembus.registerObject(objectPath, interface, this, QDBusConnection :: ExportAllSlots | QDBusConnection :: ExportAllSignals);
    }
    else
        qCritical() << "Register DBus Service Error:" << systembus.lastError().message();
}
bool AppWidgetManager::registerScheduledUpdate(AppWidgetProviderProxy* providerproxy , int timingperiod)
{
    Q_UNUSED(providerproxy);
    Q_UNUSED(timingperiod);
    return false;
}
bool AppWidgetManager::unregisterScheduledUpdate(AppWidgetProviderProxy* providerproxy)
{
    Q_UNUSED(providerproxy);
    return false;
}
AppWidgetProviderProxy* AppWidgetManager::createProviderProxy(QString providername)
{
    AppWidgetProviderProxy* providerproxy = m_mappingmanager->asyncCreateProviderProxy(providername);
    if (!providerproxy) {
        qWarning() << __FILE__ << __FUNCTION__ <<  "providerproxy is null";
        return nullptr;
    }
    providerproxy->appWidgetEnable();
    return providerproxy;
}
void AppWidgetManager::deleteProviderProxy(AppWidgetProviderProxy* providerproxy)
{
    if (!providerproxy) {
        qWarning() << __FILE__ << __FUNCTION__ <<  "providerproxy is null";
        return;
    }
    providerproxy->appWidgetDisable();
//    if(通过配置管理查看配置是否注册定时任务){
//        unregisterScheduledUpdate(providerproxy);
//    }
    m_mappingmanager->destroyProviderProxy(providerproxy);
}
AppWidgetProxy* AppWidgetManager::createAppWidgetProxy(AppWidgetProviderProxy* providerproxy, QString appwidgetname, QString username)
{
    if (!providerproxy) {
        qWarning() << __FILE__ << __FUNCTION__ <<  "providerproxy is null";
        return nullptr ;
    }
    AppWidgetProxy* appwidgetproxy = providerproxy->createAppWidgetProxy(appwidgetname, username);
    if (!appwidgetproxy) {
        qWarning() << __FILE__ << __FUNCTION__ <<  "appwidgetproxy is null";
        return nullptr;
    }
    appwidgetproxy->setProviderProxy(providerproxy);
    return appwidgetproxy;
}
bool AppWidgetManager::deleteAppWidgetProxy(AppWidgetProviderProxy* providerproxy, int id)
{
    qDebug() << __FILE__ << __FUNCTION__ << "delete appwidget`s  id:" << id;
    if (!providerproxy) {
        qWarning() << __FILE__ << __FUNCTION__ <<  "providerproxy is null";
        return false;
    }
    if (providerproxy->destroyAppWidgetProxy(id)) {
        providerproxy->appWidgetDeleted();
    } else {
        return false;
    }
    return true;
}

} // namespace AppWidget
