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

#ifndef APPWIDGETPROXY_H
#define APPWIDGETPROXY_H

#include <QDBusInterface>

namespace AppWidget {
class AppWidgetProviderProxy;
class AppWidgetProxy : public QDBusInterface
{
    Q_OBJECT
public:
    AppWidgetProxy(int id, QString appwidgetname, QString appwidgetuser);
    ~AppWidgetProxy();


    void updateAppWidget();
    void updateUserStatus(QString status);
    void updateAppWidgetOptions(QVariant key, QVariant value);
    bool updateAppWidgetSize(QSize);
    int  getAppWidgetProxyId();
    QString getAppWidgetProxyAppWidgetName();
    QString getAppWidgetProxyUserName();
    void setProviderProxy(AppWidgetProviderProxy * providerproxy);

public slots:
    void onClicked(QString widgetname, QDBusVariant value);
    void onDefineSignalType(QString eventname, QString widgetname, QDBusVariant value);

private:
    int m_id ;
    QString m_appwidgetname;
    QString m_username;
    AppWidgetProviderProxy* m_provider;


signals:
    void clickedSignal(QString widgetname, QDBusVariant value);
    void defineEventSignal(QString eventname, QString widgetname, QDBusVariant value);

};
} // namespace AppWidget
#endif // APPWIDGETPROXY_H
