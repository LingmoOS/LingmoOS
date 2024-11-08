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

#ifndef LINGMO_MENU_FAVORITE_CONTEXT_MENU_H
#define LINGMO_MENU_FAVORITE_CONTEXT_MENU_H

#include "../context-menu-extension.h"

namespace LingmoUIMenu {

class FavoriteContextMenu : public ContextMenuExtension
{
public:
    int index() const override;
    QList<QAction *> actions(const DataEntity &data, QMenu *parent,
                             const MenuInfo::Location &location,
                             const QString &locationId) override;

private:
    void appendActionsForAppsInFolder(QObject *parent, const QString &appId, QList<QAction *> &list);
    void appendActionsForAppsOutsideFolder(QObject *parent, const QString &appId, QList<QAction *> &list);
};

} // LingmoUIMenu

#endif //LINGMO_MENU_FAVORITE_CONTEXT_MENU_H
