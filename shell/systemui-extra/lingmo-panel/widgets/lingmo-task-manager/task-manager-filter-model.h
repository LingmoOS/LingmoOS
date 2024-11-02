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

#ifndef LINGMO_PANEL_TASK_MANAGER_FILTER_MODEL_H
#define LINGMO_PANEL_TASK_MANAGER_FILTER_MODEL_H

#include <QSortFilterProxyModel>
#include <QScreen>
#include <QMenu>
#include <QPointer>
#include <QQuickItem>
#include "actions.h"
#include "lingmo-task-manager.h"
#include "config-loader.h"

namespace TaskManager {
class TaskManagerFilterModel : public QSortFilterProxyModel
{
Q_OBJECT
    Q_PROPERTY(QScreen *screen READ screen WRITE setScreen)
public:
    explicit TaskManagerFilterModel(QObject *parent = nullptr);

    Q_INVOKABLE void setSourceModel(QAbstractItemModel *sourceModel) override;
    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void setOrder(const QModelIndex &index, int order);
    /**
     * 右键菜单
     * @param showDisabledActions
     * @param actions
     */
    Q_INVOKABLE bool openMenu(const bool &showDisabledActions, const TaskManager::Actions &actions, QQuickItem *item);
    Q_INVOKABLE void addQuickLauncher(const QString &desktopFile, int order);
    Q_INVOKABLE void launch(const QModelIndex &index);

    QScreen *screen() const;
    void setScreen(QScreen *screen);
    /**
     * 多屏时，任务栏图标显示在哪个任务栏 枚举值
     * 0-所有屏幕，1-主屏和打开了窗口的任务栏 2-只显示在打开了窗口的任务栏
     */
    enum TaskBarIconsShowedOn {
        AllPanels = 0,
        PrimaryScreenPanelAndPanelWhereWindowIsOpen,
        PanelWhereWindowIsOpen
    };
    Q_ENUM(TaskBarIconsShowedOn)

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QStringList currentWinIdList(const QModelIndex &index) const;
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    LingmoUITaskManager *m_sourceModel = nullptr;
    QScreen *m_screen = nullptr;
    QPointer<QMenu> m_contextMenu;

    //任务栏图标显示在哪个任务栏
    TaskBarIconsShowedOn m_iconsShowedonScreen;
    LingmoUIQuick::Config *m_config {nullptr};

Q_SIGNALS:
    void menuAboutToHide();
};
}

#endif //LINGMO_PANEL_TASK_MANAGER_FILTER_MODEL_H
