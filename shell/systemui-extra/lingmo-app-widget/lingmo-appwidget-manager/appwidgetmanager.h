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

#ifndef APPWIDGETMANAGER_H
#define APPWIDGETMANAGER_H

#include <QObject>
#include <QDBusVariant>

namespace AppWidget {
class AppWidgetProviderProxy;
class AppWidgetProxy;
class MappingManager;
class AppWidgetConfiguration;
class AppWidgetManager : public QObject
{
    Q_OBJECT

public:
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.appwidget.manager")
    explicit AppWidgetManager(QObject *parent = nullptr);
    ~AppWidgetManager();

public slots:
    QStringList getAppWidgetList();
    QString getAppWidgetUiFile(QString appwidgetname);
    QString getAppWidgetConfig(QString appwidgetname, QString config);
    QMap<QString, QVariant> getAppWidgetConfigs(QString appwidgetname);
    QMap<QString, QString> getAppWidgetLanguageFiles(QString appwidgetname);
    int registerAppWidget(QString appwidgetname, QString username);
    bool unregisterAppWidget(int appwidgetid);
    QMap<QString, QVariant> getAppWidgetData(int appwidgetid);
    void updateAppWidget(QString providername, QMap<QString, QVariant> data);
    void updateUserStatus(QString username, QString status);
    void notifyProviderUpdate(int id);

signals:
    void appWidgetAdded(QString appwidgetname);
    void appWidgetDeleted (QString appwidgetname);

private:
    void registerCommunicationService();//注册dbus服务
    bool registerScheduledUpdate(AppWidgetProviderProxy* providerproxy, int timingperiod);//注册定时更新
    bool unregisterScheduledUpdate(AppWidgetProviderProxy* providerproxy);//注销定时更新
    AppWidgetProviderProxy* createProviderProxy(QString providername);
    void deleteProviderProxy(AppWidgetProviderProxy* providerproxy);
    AppWidgetProxy* createAppWidgetProxy(AppWidgetProviderProxy* providerproxy, QString appwidgetname, QString username);
    bool deleteAppWidgetProxy(AppWidgetProviderProxy* providerproxy, int id);

    //定时更新
    //映射管理
    MappingManager* m_mappingmanager = nullptr;
    //小插件列表管理
    //小插件配置管理
    AppWidgetConfiguration * m_configmanager = nullptr;
};
} // namespace AppWidget
Q_DECLARE_METATYPE(QString)
#endif // APPWIDGETMANAGER_H
