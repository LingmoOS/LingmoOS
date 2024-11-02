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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include <QQmlEngine>
#include <QQmlContext>
#include <QDebug>
#include <QMenu>
#include <QQuickView>
#include <windowmanager/windowmanager.h>
#include <KWindowSystem>

#include "actions.h"
#include "task-manager-view.h"
#include "lingmo-task-manager.h"

TaskManagerView::TaskManagerView(QWindow *parent)
{
    initWindow();
}

void TaskManagerView::openMenu(const QVariant &modelActions, const int &index)
{
    TaskManager::Actions actions;
    if (modelActions.canConvert<TaskManager::Actions>()) {
        actions = modelActions.value<TaskManager::Actions>();
    } else {
        qWarning() << "actions is not available!";
        return;
    }

    if (m_contextMenu) {
        m_contextMenu->close();
    }

    QMenu *menu = new QMenu;
    menu->setAttribute(Qt::WA_DeleteOnClose);

    for(const TaskManager::Action* action : actions) {
        menu->addAction(action->internalAction());
    }

    if (menu->actions().isEmpty()) {
        delete menu;
        return;
    }
    m_contextMenu = menu;
    menu->popup(QCursor::pos());
}

void TaskManagerView::initWindow()
{
    setResizeMode(QQuickView::SizeViewToRootObject);
    m_windowThumbnailView = new WindowThumbnailView();


    engine()->rootContext()->setContextProperty("taskView", this);
    engine()->rootContext()->setContextProperty("taskModel", &TaskManager::LingmoUITaskManager::self());
    engine()->rootContext()->setContextProperty("thumbnailView", m_windowThumbnailView);

    setSource(QUrl("qrc:/main.qml"));

    kdk::WindowManager::setSkipTaskBar(this, true);
//    kdk::WindowManager::setSkipSwitcher(this, true);
//垃圾，不好使
//    kdk::WindowManager::setOnAllDesktops(winId());
//    KWindowSystem::setType(winId(), NET::Dock);
    KWindowSystem::setOnAllDesktops(winId(), true);
}
