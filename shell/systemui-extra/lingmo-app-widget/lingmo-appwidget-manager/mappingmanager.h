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

#ifndef MAPPINGMANAGER_H
#define MAPPINGMANAGER_H

#include <QString>
#include <QVector>
#include <QObject>

class AsyncProviderProxyCreator;
namespace AppWidget {
class AppWidgetProviderProxy;
class AppWidgetProxy;
class MappingManager:public QObject
{
    Q_OBJECT
public:
    MappingManager(QObject *parent = nullptr);
    ~MappingManager();

    AppWidgetProviderProxy* findProviderProxyByProviderName(const QString providername);
    AppWidgetProviderProxy* findProviderProxyByAppWidgetId(int id);
    AppWidgetProxy* findAppWidgetProxyByAppWidgetId(int id);

    QVector<AppWidgetProxy*> findAppWidgetProxyByUserName(QString username);
    AppWidgetProviderProxy* asyncCreateProviderProxy(QString providername);
    void destroyProviderProxy(AppWidgetProviderProxy* providerproxy);

private:
    int getUsableIndex();
    void addProviderProxy(int index, AppWidgetProviderProxy* providerproxy);
    void deleteProviderProxy(int index);

private:
    QVector<AppWidgetProviderProxy*> m_providerproxy;
};
} // namespace AppWidget

#endif // MAPPINGMANAGER_H
