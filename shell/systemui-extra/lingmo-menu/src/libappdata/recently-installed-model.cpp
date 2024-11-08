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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "recently-installed-model.h"
#include "basic-app-model.h"
#include "user-config.h"

#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QTimerEvent>

namespace LingmoUIMenu {

// ====== RecentlyInstalledModel ====== //
RecentlyInstalledModel::RecentlyInstalledModel(QObject *parent) : QSortFilterProxyModel(parent), m_timer(new QTimer(this))
{
    QSortFilterProxyModel::setSourceModel(BasicAppModel::instance());
    // 触发排序动作
//    QSortFilterProxyModel::sort(0, Qt::DescendingOrder);
    QSortFilterProxyModel::sort(0);

    // 每48小时主动刷新
    m_timer->setInterval(48 * 3600000);
    m_timer->start();
}

bool RecentlyInstalledModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    // 是否为预装应用
    if (UserConfig::instance()->isPreInstalledApps(sourceIndex.data(DataEntity::Id).toString())) {
        return false;
    }
    // 是否打开过
    if (sourceIndex.data(DataEntity::IsLaunched).toInt() != 0) {
        return false;
    }

    // 是否收藏
    if (sourceIndex.data(DataEntity::Favorite).toInt() > 0) {
        return false;
    }

    QDateTime installDate = QDateTime::fromString(sourceIndex.data(DataEntity::InstallationTime).value<QString>(), "yyyy-MM-dd hh:mm:ss");
    if (!installDate.isValid()) {
        return false;
    }

    QDateTime currentDateTime = QDateTime::currentDateTime();
    // 安装时间在30天内
    qint64 xt = currentDateTime.toSecsSinceEpoch() - installDate.toSecsSinceEpoch();
    return (xt >= 0) && (xt <= 30 * 24 * 3600);
}

bool RecentlyInstalledModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QDateTime leftInstallDate = QDateTime::fromString(source_left.data(DataEntity::InstallationTime).value<QString>(), "yyyy-MM-dd hh:mm:ss");
    QDateTime rightInstallDate = QDateTime::fromString(source_right.data(DataEntity::InstallationTime).value<QString>(), "yyyy-MM-dd hh:mm:ss");

    qint64 xt = leftInstallDate.toSecsSinceEpoch() - rightInstallDate.toSecsSinceEpoch();
    if (xt == 0) {
        return source_left.data(DataEntity::FirstLetter).value<QString>() < source_right.data(DataEntity::FirstLetter).value<QString>();
    }

    return xt >= 0;
}

bool RecentlyInstalledModel::event(QEvent *event)
{
    if (event->type() == QEvent::Timer) {
        auto timerEvent = static_cast<QTimerEvent*>(event);
        if (timerEvent->timerId() == m_timer->timerId()) {
            invalidate();
            return true;
        }
    }

    return QObject::event(event);
}

QVariant RecentlyInstalledModel::data(const QModelIndex &index, int role) const
{
    if (role == DataEntity::Group) {
        return tr("Recently Installed");
    }

    if (role == DataEntity::RecentInstall) {
        return true;
    }

    return QSortFilterProxyModel::data(index, role);
}

} // LingmoUIMenu
