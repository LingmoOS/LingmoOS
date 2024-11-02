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
 */

#ifndef LINGMO_MENU_DATA_PROVIDER_MANAGER_H
#define LINGMO_MENU_DATA_PROVIDER_MANAGER_H

#include <QObject>
#include <QThread>
#include <QVariant>

#include "data-provider-plugin-iface.h"

namespace LingmoUIMenu {

class ProviderInfo
{
public:
    int index;
    QString id;
    QString name;
    QString icon;
    QString title;
    PluginGroup::Group group;
};

class DataProviderManager : public QObject
{
    Q_OBJECT
public:
    static DataProviderManager *instance();
    ~DataProviderManager() override;

    QStringList providers() const;
    QVector<ProviderInfo> providers(PluginGroup::Group group) const;
    ProviderInfo providerInfo(const QString &id) const;
    QString activatedProvider() const;
    void activateProvider(const QString &id, bool record = true);
    QVector<DataEntity> data() const;
    QVector<LabelItem> labels() const;
    void forceUpdate() const;
    void forceUpdate(QString &key) const;

Q_SIGNALS:
    void pluginChanged(const QString &id, PluginGroup::Group group);
    void dataChanged(QVector<DataEntity> data, DataUpdateMode::Mode mode, quint32 index);
    void labelChanged();
    void toUpdate(bool isShowed);

private:
    DataProviderManager();
    Q_DISABLE_COPY(DataProviderManager);

    void initProviders();
    void registerProvider(DataProviderPluginIFace *provider);

private:
    QThread m_worker;
    QString m_activatedPlugin{""};
    QHash<QString, DataProviderPluginIFace*> m_providers;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_DATA_PROVIDER_MANAGER_H
