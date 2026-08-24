/***************************************************************************
 *   Copyright (C) 2024 Reion Wong     <aj@lingmo.org>                 *
 *   Copyright (C) 2020 Konrad Materka <materka@gmail.com>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "systemtraymodel.h"

#include <QApplication>
#include <QDebug>

#include <KWindowSystem>

static QStringList noColorOverlayList = {
    "netease-cloud-music",
    "chrome_status_icon_1",
    "35682", // obs studio
    "lark_status_icon_1"
};

SystemTrayModel::SystemTrayModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_watcher = new StatusNotifierWatcher;
    m_sniHost = StatusNotifierItemHost::self();

    connect(m_sniHost, &StatusNotifierItemHost::itemAdded, this, &SystemTrayModel::onItemAdded);
    connect(m_sniHost, &StatusNotifierItemHost::itemRemoved, this, &SystemTrayModel::onItemRemoved);

    for (auto service : m_sniHost->services()) {
        onItemAdded(service);
    }
}

SystemTrayModel::~SystemTrayModel()
{
    QDBusConnection::sessionBus().unregisterService(m_hostName);
}

int SystemTrayModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_items.size();
}

QHash<int, QByteArray> SystemTrayModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[IconNameRole] = "iconName";
    roles[IconRole] = "icon";
    roles[TitleRole] = "title";
    roles[ToolTipRole] = "toolTip";
    roles[CanColorOverlay] = "canColorOverlay";
    return roles;
}

QVariant SystemTrayModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    StatusNotifierItemSource *item = m_items.at(index.row());

    switch (role) {
    case IdRole:
        return item->id();
    case IconNameRole:
        return item->iconName();
    case IconRole: {
        if (!item->icon().isNull())
            return item->icon();
        else
            return QVariant();
    }
    case TitleRole:
        return item->title();
    case ToolTipRole:
        return item->tooltip();
    case CanColorOverlay:
        return !noColorOverlayList.contains(item->appId());
    }

    return QVariant();
}

int SystemTrayModel::indexOf(const QString &id)
{
    for (StatusNotifierItemSource *item : qAsConst(m_items)) {
        if (item->id() == id)
            return m_items.indexOf(item);
    }

    return -1;
}

StatusNotifierItemSource *SystemTrayModel::findItemById(const QString &id)
{
    int index = indexOf(id);

    if (index == -1)
        return nullptr;

    return m_items.at(index);
}

void SystemTrayModel::leftButtonClick(const QString &id, int x, int y)
{
    StatusNotifierItemSource *item = findItemById(id);

    if (item) {
        item->activate(x * qApp->devicePixelRatio(),
                       y * qApp->devicePixelRatio());
    }
}

void SystemTrayModel::rightButtonClick(const QString &id, QQuickItem *iconItem, int x, int y)
{
    StatusNotifierItemSource *item = findItemById(id);

    if (item) {
        item->contextMenu(x * qApp->devicePixelRatio(),
                          y * qApp->devicePixelRatio(),
                          iconItem);
    }
}

void SystemTrayModel::middleButtonClick(const QString &id, int x, int y)
{
    StatusNotifierItemSource *item = findItemById(id);

    if (item) {
        item->secondaryActivate(x * qApp->devicePixelRatio(),
                                y * qApp->devicePixelRatio());
    }
}

void SystemTrayModel::move(int from, int to)
{
    if (from == to)
        return;

    m_items.move(from, to);

    if (from < to)
        beginMoveRows(QModelIndex(), from, from, QModelIndex(), to + 1);
    else
        beginMoveRows(QModelIndex(), from, from, QModelIndex(), to);

    endMoveRows();
}

QPointF SystemTrayModel::popupPosition(QQuickItem *visualParent, int x, int y)
{
    if (!visualParent) {
        return QPointF(0, 0);
    }

    QPointF pos = visualParent->mapToScene(QPointF(x, y));

    if (visualParent->window() && visualParent->window()->screen()) {
        pos = visualParent->window()->mapToGlobal(pos.toPoint());
    } else {
        return QPoint();
    }

    return pos;
}

void SystemTrayModel::onItemAdded(const QString &service)
{
    StatusNotifierItemSource *source = m_sniHost->itemForService(service);

    connect(source, &StatusNotifierItemSource::updated, this, &SystemTrayModel::updated);

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items.append(source);
    endInsertRows();
}

void SystemTrayModel::onItemRemoved(const QString &service)
{
    int index = indexOf(service);

    if (index != -1) {
        beginRemoveRows(QModelIndex(), index, index);
        StatusNotifierItemSource *item = m_items.at(index);
        m_items.removeAll(item);
        endRemoveRows();
    }
}

void SystemTrayModel::updated(StatusNotifierItemSource *item)
{
    if (!item)
        return;

    int idx = indexOf(item->id());

    // update
    if (idx != -1) {
        emit dataChanged(index(idx, 0), index(idx, 0));
    }
}
