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

#ifndef LINGMO_MENU_APP_CATEGORY_PLUGIN_H
#define LINGMO_MENU_APP_CATEGORY_PLUGIN_H

#include "app-list-plugin.h"

namespace LingmoUIMenu {

class CombinedListModel;
class AppCategoryModel;
class RecentlyInstalledModel;

class AppCategoryPlugin : public AppListPluginInterface
{
    Q_OBJECT
public:
    explicit AppCategoryPlugin(QObject *parent = nullptr);

    AppListPluginGroup::Group group() override;
    QString name() override;
    QString title() override;
    QList<QAction *> actions() override;
    QAbstractItemModel *dataModel() override;
    LabelBottle *labelBottle() override;

private:
    void setTitle(const QString &title);
    void updateLabelBottle();

private:
    QString m_title;
    QList<QAction *> m_actions;
    LabelBottle *m_labelBottle {nullptr};

    AppCategoryModel *m_categoryModel {nullptr};
    RecentlyInstalledModel *m_recentlyModel {nullptr};

    CombinedListModel *m_dataModel {nullptr};
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_CATEGORY_PLUGIN_H
