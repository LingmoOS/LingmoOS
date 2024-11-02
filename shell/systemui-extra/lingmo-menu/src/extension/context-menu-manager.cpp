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

#include "context-menu-manager.h"
#include "menu-extension-loader.h"
#include "basic-app-model.h"

#include <QMenu>
#include <QCursor>
#include <QPointer>
#include <QWindow>

using namespace LingmoUIMenu;

ContextMenuManager *ContextMenuManager::instance()
{
    static ContextMenuManager manager;
    return &manager;
}

class MenuManagerPrivate
{
public:
    QPointer<QMenu> contextMenu;
    QWindow *mainWindow {nullptr};
    QList<ContextMenuExtension*> extensions;
};

ContextMenuManager::ContextMenuManager() : d(new MenuManagerPrivate)
{
    d->extensions = MenuExtensionLoader::instance()->menus();
}

ContextMenuManager::~ContextMenuManager()
{
    for (auto &provider : d->extensions) {
        delete provider;
        provider = nullptr;
    }
}

void ContextMenuManager::showMenu(const QString &appid, const MenuInfo::Location location, const QString &lid, const QPoint &point)
{
    DataEntity app;
    if (BasicAppModel::instance()->getAppById(appid, app)) {
        showMenu(app, location, lid, point);
    }
}

void ContextMenuManager::showMenu(const DataEntity &data, MenuInfo::Location location, const QString &lid, const QPoint &point)
{
    if (closeMenu()) {
        return;
    }

    auto menu = new QMenu;
    QList<QAction*> actions;
    for (const auto &extension : d->extensions) {
        actions.append(extension->actions(data, menu, location, lid));
    }

    if (actions.isEmpty() && menu->isEmpty()) {
        delete menu;
        return;
    }

    d->contextMenu = menu;
    menu->setAttribute(Qt::WA_DeleteOnClose);
    if (menu->winId()) {
        menu->windowHandle()->setTransientParent(d->mainWindow);
    }

    menu->addActions(actions);
    menu->popup(checkPoint(point));
}

void ContextMenuManager::setMainWindow(QWindow *mainWindow)
{
    d->mainWindow = mainWindow;
}

bool ContextMenuManager::closeMenu()
{
    if (d->contextMenu) {
        d->contextMenu.data()->close();
        return true;
    }

    return false;
}

inline QPoint ContextMenuManager::checkPoint(const QPoint &rawPoint)
{
    if (rawPoint.isNull()) {
        return QCursor::pos();
    }
    return rawPoint;
}
