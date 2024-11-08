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

#ifndef LINGMO_MENU_CONTEXT_MENU_MANAGER_H
#define LINGMO_MENU_CONTEXT_MENU_MANAGER_H

#include <QObject>
#include <QPoint>

#include "context-menu-extension.h"

class QWindow;
class MenuManagerPrivate;

namespace LingmoUIMenu {

class ContextMenuManager : public QObject
{
    Q_OBJECT
public:
    static ContextMenuManager *instance();
    ~ContextMenuManager() override;

    Q_INVOKABLE void showMenu(const QString &appid, MenuInfo::Location location, const QString& lid = QString(), const QPoint &point = QPoint());
    void showMenu(const DataEntity &data, MenuInfo::Location location, const QString& lid = QString(), const QPoint &point = QPoint());

    void setMainWindow(QWindow *mainWindow);
    bool closeMenu();

private:
    ContextMenuManager();
    inline QPoint checkPoint(const QPoint &rawPoint);

private:
    MenuManagerPrivate *d {nullptr};
};

} // LingmoUIMenu

#endif //LINGMO_MENU_CONTEXT_MENU_MANAGER_H
