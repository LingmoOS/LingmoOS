/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_MENU_APP_SEARCH_PLUGIN_H
#define LINGMO_MENU_APP_SEARCH_PLUGIN_H

#include "app-list-plugin.h"

namespace LingmoUIMenu {

class AppSearchPluginPrivate;
class AppSearchModel;

/**
 * @class AppSearchPlugin
 * 应用搜索插件，调用搜索接口进行应用搜索
 */
class AppSearchPlugin : public AppListPluginInterface
{
    Q_OBJECT
public:
    explicit AppSearchPlugin(QObject *parent = nullptr);
    ~AppSearchPlugin() override;

    AppListPluginGroup::Group group() override;
    QString name() override;
    QString title() override;
    QList<QAction *> actions() override;
    QAbstractItemModel *dataModel() override;
    void search(const QString &keyword) override;

private:
    AppSearchModel *m_model {nullptr};
    AppSearchPluginPrivate * m_searchPluginPrivate {nullptr};
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_SEARCH_PLUGIN_H
