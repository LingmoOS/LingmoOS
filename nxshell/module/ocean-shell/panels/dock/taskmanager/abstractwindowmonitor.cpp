// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractwindowmonitor.h"
#include "abstractwindow.h"

namespace dock {
AbstractWindowMonitor::AbstractWindowMonitor(QObject* parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> AbstractWindowMonitor::roleNames() const
{
    return {
        {AbstractWindow::winIdRole, "winId"},
        {AbstractWindow::pidRole, "pid"},
        {AbstractWindow::identityRole, "identity"},
        {AbstractWindow::winIconRole, "icon"},
        {AbstractWindow::winTitleRole, "title"},
        {AbstractWindow::activieRole, "isActive"},
        {AbstractWindow::shouldSkipRole, "shouldSkip"}
    };
}

int AbstractWindowMonitor::rowCount(const QModelIndex &parent) const
{
    return m_trackedWindows.size();
}

QVariant AbstractWindowMonitor::data(const QModelIndex &index, int role) const
{
    auto pos = index.row();
    if (pos >= m_trackedWindows.size())
        return QVariant();
    auto window = m_trackedWindows[pos];

    switch (role) {
        case AbstractWindow::winIdRole:
            return window->id();
        case AbstractWindow::pidRole:
            return window->pid();
        case AbstractWindow::identityRole:
            return window->identity();
        case AbstractWindow::winIconRole:
            return window->icon();
        case AbstractWindow::winTitleRole:
            return window->title();
        case AbstractWindow::activieRole:
            return window->isActive();
        case AbstractWindow::shouldSkipRole:
            return window->shouldSkip();
    }

    return QVariant();
}

void AbstractWindowMonitor::trackWindow(AbstractWindow* window)
{
    beginInsertRows(QModelIndex(), m_trackedWindows.size(), m_trackedWindows.size());
    m_trackedWindows.append(window);
    endInsertRows();

    connect(window, &AbstractWindow::pidChanged, this, [this, window](){
        auto pos = m_trackedWindows.indexOf(window);
        auto modelIndex = index(pos);
        Q_EMIT dataChanged(modelIndex, modelIndex, {AbstractWindow::pidRole});
    });
    connect(window, &AbstractWindow::identityChanged, this, [this, window](){
        auto pos = m_trackedWindows.indexOf(window);
        auto modelIndex = index(pos);
        Q_EMIT dataChanged(modelIndex, modelIndex, {AbstractWindow::identityRole});
    });
    connect(window, &AbstractWindow::iconChanged, this, [this, window](){
        auto pos = m_trackedWindows.indexOf(window);
        auto modelIndex = index(pos);
        Q_EMIT dataChanged(modelIndex, modelIndex, {AbstractWindow::winIconRole});
    });
    connect(window, &AbstractWindow::titleChanged, this, [this, window](){
        auto pos = m_trackedWindows.indexOf(window);
        auto modelIndex = index(pos);
        Q_EMIT dataChanged(modelIndex, modelIndex, {AbstractWindow::winTitleRole});
    });

    connect(window, &AbstractWindow::isActiveChanged, this, [this, window](){
        auto pos = m_trackedWindows.indexOf(window);
        auto modelIndex = index(pos);
        Q_EMIT dataChanged(modelIndex, modelIndex, {AbstractWindow::activieRole});
    });
    connect(window, &AbstractWindow::shouldSkipChanged, this, [this, window](){
        auto pos = m_trackedWindows.indexOf(window);
        auto modelIndex = index(pos);
        Q_EMIT dataChanged(modelIndex, modelIndex, {AbstractWindow::shouldSkipRole});
    });
}

void AbstractWindowMonitor::destroyWindow(AbstractWindow * window)
{
    auto pos = m_trackedWindows.indexOf(window);
    if (pos == -1)
        return;

    beginRemoveRows(QModelIndex(), pos, pos + 1);
    m_trackedWindows.removeOne(window);
    endRemoveRows();
}


}
