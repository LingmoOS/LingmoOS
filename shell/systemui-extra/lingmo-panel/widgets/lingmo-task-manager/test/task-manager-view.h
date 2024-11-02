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

#ifndef TASKMANAGERVIEW_H
#define TASKMANAGERVIEW_H

#include <QQuickView>
#include <QMainWindow>
#include <QPointer>

#include "window-thumbnail-view.h"
#include "lingmo-task-manager.h"

class TaskManagerView : public QQuickView
{
    Q_OBJECT
public:
    TaskManagerView(QWindow *parent = nullptr);
    ~TaskManagerView() = default;

    Q_INVOKABLE void openMenu(const QVariant &modelActions, const int &index);
private:
    void initWindow();

private:
    WindowThumbnailView *m_windowThumbnailView = nullptr;
    TaskManager::LingmoUITaskManager *m_model = nullptr;
    QPointer<QMenu> m_contextMenu;
};

#endif // TASKMANAGERVIEW_H
