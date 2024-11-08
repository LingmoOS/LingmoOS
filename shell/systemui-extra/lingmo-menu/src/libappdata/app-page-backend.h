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

#ifndef LINGMO_MENU_APP_PAGE_BACKEND_H
#define LINGMO_MENU_APP_PAGE_BACKEND_H

#include <QObject>
#include <QAction>
class QAbstractItemModel;
class QSortFilterProxyModel;

#include "app-list-plugin.h"

namespace LingmoUIMenu {

class AppListModel;

/**
 * 此类管理开始菜单的主体功能和接口
 *
 * 提供：应用搜索，应用分类模式的切换功能
 *
 *
 *        AppPage
 *           |
 *     AppListHeader
 *        AppList
 *     /          \
 * Search          Display
 *   |                    \
 * (Model Title Actions)   (Model Title Actions)
 *
 */
class AppPageBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *appModel READ appModel NOTIFY appModelChanged)
    Q_PROPERTY(LingmoUIMenu::AppListPluginGroup::Group group READ group WRITE setGroup NOTIFY groupChanged)
public:
    static AppPageBackend *instance();

    // 开始菜单主要功能，显示应用列表
    AppListModel *appModel() const;
    Q_INVOKABLE void startSearch(const QString &keyword);

    AppListPluginGroup::Group group() const;
    void setGroup(AppListPluginGroup::Group group);

Q_SIGNALS:
    void appModelChanged();
    void groupChanged();

private:
    explicit AppPageBackend(QObject *parent = nullptr);
    void switchGroup();

private:
    AppListPluginGroup::Group m_group {AppListPluginGroup::Display};
    AppListModel *m_appModel {nullptr};
    QMap<AppListPluginGroup::Group, AppListPluginInterface*> m_plugins;

};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_PAGE_BACKEND_H
