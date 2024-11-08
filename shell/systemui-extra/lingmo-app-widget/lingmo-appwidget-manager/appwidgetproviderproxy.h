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

#ifndef APPWIDGETPROVIDERPROXY_H
#define APPWIDGETPROVIDERPROXY_H

#include <QDBusInterface>
#include <QVector>

namespace AppWidget {
class AppWidgetProxy;
class AppWidgetProviderProxy : public QDBusInterface
{
    Q_OBJECT
public:
    AppWidgetProviderProxy(int id, QString providerName);
    ~AppWidgetProviderProxy();


    void updateAppWidgetProviderProxyData(QMap<QString, QVariant> data);
    QMap<QString, QVariant> getAppWidgetProviderProxyData();

    AppWidgetProxy* createAppWidgetProxy(QString appwidgetname, QString username);
    bool destroyAppWidgetProxy(int id);
    int* getAllAppWidgetIds();

    void appWidgetRecevie(QString eventname, QString widgetname, QDBusVariant value);
    void appWidgetUpdate();
    void appWidgetDisable();
    void appWidgetDeleted();
    void appWidgetEnable();
    void appWidgetClicked(QString widgetname, QDBusVariant value);
    bool appWidgetProxyIsEmpty();
    AppWidgetProxy* getAppWidgetProxy(int id);
    int getAppWidgetProviderProxyIndex();
    QString getAppWidgetProviderProxyProviderName();

    QVector<AppWidgetProxy*> getAppwidgetProxyByUser(QString username);
private:
    bool addAppWidgetProxy(int index, AppWidgetProxy* appwidgetproxy);
    bool deleteAppWidgetProxy(int index);
    int getUsableIndex();

private:
    QMap<QString, QVariant> m_data;
    QVector<AppWidgetProxy*> m_appwidgetproxy;
    int m_index;
    QString m_providername;
};
} // namespace AppWidget
#endif // APPWIDGETPROVIDERPROXY_H
