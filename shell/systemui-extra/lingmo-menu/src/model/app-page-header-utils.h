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

#ifndef LINGMO_MENU_APP_PAGE_HEADER_UTILS_H
#define LINGMO_MENU_APP_PAGE_HEADER_UTILS_H

#include <QObject>
#include <QString>

#include "data-provider-plugin-iface.h"

namespace LingmoUIMenu {

class ProviderModel;

class AppPageHeaderUtils : public QObject
{
    Q_OBJECT
public:
    explicit AppPageHeaderUtils(QObject *parent = nullptr);

    // 激活某插件
    Q_INVOKABLE void activateProvider(const QString &name) const;
    // 获取不同的model
    Q_INVOKABLE ProviderModel *model(PluginGroup::Group group) const;

    Q_INVOKABLE void startSearch(QString key) const;
    Q_INVOKABLE QString getPluginTitle(QString id) const;
    Q_INVOKABLE QString currentPluginId() const;

Q_SIGNALS:
    void pluginChanged(const QString &id);

private Q_SLOTS:
    void onPluginChanged(const QString &id, PluginGroup::Group group);

private:
    QHash<PluginGroup::Group, ProviderModel*> m_models;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_PAGE_HEADER_UTILS_H
