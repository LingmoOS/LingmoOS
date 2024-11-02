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

#include "task-manager-filter-model.h"
#include <QQuickWindow>
#include <QTimer>
#include "lingmo-task-manager.h"
#include <window-manager.h>
#include <QGuiApplication>
namespace TaskManager {
TaskManagerFilterModel::TaskManagerFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setFilterRole(LingmoUITaskManager::CurrentWinIdList);
    //读取taskmanager配置文件中的任务栏图标显示在哪个任务栏taskBarIconsShowedOn项并保存
    if (!m_config) {
        m_config = LingmoUIQuick::ConfigLoader::getConfig("org.lingmo.panel.taskManager");
        if(m_config->data().contains("taskBarIconsShowedOn")) {
            m_iconsShowedonScreen = m_config->getValue("taskBarIconsShowedOn").value<TaskBarIconsShowedOn>();
        } else {
            m_iconsShowedonScreen = TaskBarIconsShowedOn::PanelWhereWindowIsOpen;
            m_config->setValue(QStringLiteral("taskBarIconsShowedOn"), TaskBarIconsShowedOn::PanelWhereWindowIsOpen);
        }
        //当taskBarIconsShowedOn变化时，更新筛选条件,并发送事件，更新数据
        connect(m_config, &LingmoUIQuick::Config::configChanged, this, [&](QString key) {
            if(key == "taskBarIconsShowedOn") {
                m_iconsShowedonScreen = m_config->getValue(key).value<TaskBarIconsShowedOn>();
                invalidateFilter();
                m_sourceModel->requestDataChange();
            }
        });
    }
    //主屏改变时，发送事件，更新数据
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, [&]() {
        m_sourceModel->requestDataChange();
    });
}

void TaskManagerFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    m_sourceModel = qobject_cast<LingmoUITaskManager*>(sourceModel);
    QSortFilterProxyModel::setSourceModel(sourceModel);
    connect(sourceModel, &QAbstractItemModel::dataChanged, this, &TaskManagerFilterModel::onDataChanged);
    m_sourceModel->sort(0, Qt::AscendingOrder);
}

QVariant TaskManagerFilterModel::data(const QModelIndex &index, int role) const
{
    if(role == LingmoUITaskManager::CurrentWinIdList && m_screen) {
        return currentWinIdList(index);
    } else if(role == LingmoUITaskManager::HasActiveWindow) {
        if(m_screen && mapToSource(index).data(role).toBool()) {
            return currentWinIdList(index).contains(LingmoUIQuick::WindowManager::currentActiveWindow());
        }
        return false;
    } else if (role == LingmoUITaskManager::Actions) {
        Actions actions = mapToSource(index).data(role).value<Actions>();
        for(auto action : actions) {
            if(action->type() == Action::Type::Exit) {
                QStringList wids = currentWinIdList(index);
                action->setEnabled(!wids.isEmpty());
                action->setParam(wids);
                break;
            }
        }
        return QVariant::fromValue(actions);
    }

    return sourceModel()->data(mapToSource(index), role);
}

void TaskManagerFilterModel::setOrder(const QModelIndex &index, int order)
{
    if(order >= rowCount()) {
        order = rowCount() - 1;
    }
    if(order < 0) {
        order = 0;
    }

    // 将setOrder操作放到下一次消息循环，避免wlcom上事件double free崩溃问题
    QTimer::singleShot(0, this, [this, index, order] {
        m_sourceModel->setOrder(mapToSource(index), mapToSource(this->index(order, 0)).row());
    });
}

void TaskManagerFilterModel::addQuickLauncher(const QString &desktopFile, int order)
{
    m_sourceModel->setQuickLauncher(desktopFile, mapToSource(this->index(order, 0)).row());
}

QScreen *TaskManagerFilterModel::screen() const
{
    return m_screen;;
}

void TaskManagerFilterModel::setScreen(QScreen *screen)
{
    m_screen = screen;
    invalidateFilter();
    m_sourceModel->requestDataChange();
}

bool TaskManagerFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.data(LingmoUITaskManager::HasLauncher).toBool()) {
        QMap<QString, QScreen *> widToScreen = sourceIndex.data(
                LingmoUITaskManager::WinIdsOnScreens).value<QMap<QString, QScreen *>>();
        if (widToScreen.isEmpty()) {
            return false;
        }
        for (const QString &wid: sourceIndex.data(LingmoUITaskManager::CurrentDesktopWinIdList).toStringList()) {
            switch(m_iconsShowedonScreen) {
            case TaskBarIconsShowedOn::AllPanels:
                return true;
            case TaskBarIconsShowedOn::PrimaryScreenPanelAndPanelWhereWindowIsOpen:
                if (widToScreen.value(wid) == m_screen || m_screen == qApp->primaryScreen()) {
                    return true;
                }
                break;
            case TaskBarIconsShowedOn::PanelWhereWindowIsOpen:
                if (widToScreen.value(wid) == m_screen) {
                    return true;
                }
                break;
            default:
                return false;
            }
        }
        return false;
    } else {
        return true;
    }
}

bool TaskManagerFilterModel::openMenu(const bool &showDisabledActions, const TaskManager::Actions &actions, QQuickItem *item)
{
    if (actions.isEmpty()) {
        return false;
    }

    if (m_contextMenu) {
        m_contextMenu->close();
    }

    auto *menu = new QMenu;
    menu->setAttribute(Qt::WA_DeleteOnClose);

    for(const TaskManager::Action* action : actions) {
        if(action->enabled() || showDisabledActions) {
            menu->addAction(action->internalAction());
        }
    }

    connect(menu, &QMenu::aboutToHide, this, [this] {
            Q_EMIT menuAboutToHide();
    });
    m_contextMenu = menu;
    m_contextMenu->winId();
    if(m_contextMenu->windowHandle()) {
        m_contextMenu->windowHandle()->setTransientParent(item->window());
    }
    m_contextMenu->popup(QCursor::pos());
    return true;
}

QStringList TaskManagerFilterModel::currentWinIdList(const QModelIndex &index) const
{
    QStringList wins;
    auto winIdOnScreen = mapToSource(index).data(LingmoUITaskManager::WinIdsOnScreens).value<QMap<QString, QScreen *>>();
    for(const QString &wid : mapToSource(index).data(LingmoUITaskManager::CurrentDesktopWinIdList).toStringList()) {
        switch(m_iconsShowedonScreen) {
        case TaskBarIconsShowedOn::AllPanels:
            wins.append(wid);
            break;
        case TaskBarIconsShowedOn::PrimaryScreenPanelAndPanelWhereWindowIsOpen:
            if (winIdOnScreen.value(wid) == m_screen || m_screen == qApp->primaryScreen()) {
                wins.append(wid);
            }
            break;
        case TaskBarIconsShowedOn::PanelWhereWindowIsOpen:
            if (winIdOnScreen.value(wid) == m_screen) {
                wins.append(wid);
            }
            break;
        default:
            continue;
        }
    }
    return wins;
}

void TaskManagerFilterModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                           const QVector<int> &roles)
{
    Q_EMIT dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
    if(roles.contains(LingmoUITaskManager::CurrentDesktopWinIdList) || roles.contains(LingmoUITaskManager::WinIdsOnScreens)) {
        Q_EMIT dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), {LingmoUITaskManager::HasActiveWindow});
    }
}

void TaskManagerFilterModel::launch(const QModelIndex &index)
{
    m_sourceModel->launch(mapToSource(index));
}
}
