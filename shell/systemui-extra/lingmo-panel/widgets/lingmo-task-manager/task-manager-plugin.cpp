/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

#include "task-manager-plugin.h"
#include "window-thumbnail-view.h"
#include "lingmo-task-manager.h"
#include "actions.h"
#include "task-manager-filter-model.h"

#include <QQmlEngine>
#include <QQmlContext>

void TaskManagerPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.lingmo.panel.taskManager"));

    qmlRegisterSingletonType<TaskManager::LingmoUITaskManager>(uri, 1, 0, "TaskManager", [] (QQmlEngine *, QJSEngine *) -> QObject* {
        return &TaskManager::LingmoUITaskManager::self();
    });

    qmlRegisterType<TaskManager::TaskManagerFilterModel>(uri, 1, 0, "TaskManagerFilterModel");
    qmlRegisterUncreatableType<TaskManager::Action>(uri, 1, 0, "Action", "Only enumeration variables are required");

    qRegisterMetaType<TaskManager::Action::Type>("TaskManager::Action::Type");
    qRegisterMetaType<TaskManager::Actions>("TaskManager::Actions");
}
