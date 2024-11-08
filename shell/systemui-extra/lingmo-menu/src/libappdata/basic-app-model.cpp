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

#include "basic-app-model.h"
#include "user-config.h"

#include <QDebug>

namespace LingmoUIMenu {

BasicAppModel *BasicAppModel::instance()
{
    static BasicAppModel model;
    return &model;
}

BasicAppModel::BasicAppModel(QObject *parent) : QAbstractListModel(parent)
    , m_databaseInterface(new AppDatabaseInterface(this))
{
    connect(m_databaseInterface, &AppDatabaseInterface::appDatabaseOpenFailed, this, [this] {
        qWarning() << "BasicAppModel: app database open failed.";
        m_apps.clear();
        // TODO: 显示错误信息到界面
    });

    m_apps = m_databaseInterface->apps();

    connect(m_databaseInterface, &AppDatabaseInterface::appAdded, this, &BasicAppModel::onAppAdded);
    connect(m_databaseInterface, &AppDatabaseInterface::appUpdated, this, &BasicAppModel::onAppUpdated);
    connect(m_databaseInterface, &AppDatabaseInterface::appDeleted, this, &BasicAppModel::onAppDeleted);
}

int BasicAppModel::rowCount(const QModelIndex &parent) const
{
    return m_apps.count();
}

int BasicAppModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant BasicAppModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const DataEntity &app = m_apps[index.row()];
    return app.getValue(static_cast<DataEntity::PropertyName>(role));
}

bool BasicAppModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return false;
    }

    DataEntity &app = m_apps[index.row()];
    DataEntity::PropertyName propertyName;
    switch (propertyName) {
        case DataEntity::Group:
            app.setGroup(value.toString());
            return true;
        case DataEntity::IsLaunched: {
            if (value.toBool() == app.launched()) {
                return false;
            }
            m_databaseInterface->updateApLaunchedState(app.id(), value.toBool());
            return true;
        }
        case DataEntity::Favorite:
            if (value.toInt() == app.favorite()) {
                return false;
            }
            m_databaseInterface->fixAppToFavorite(app.id(), value.toInt());
            return true;
        case DataEntity::Top:
            if (value.toInt() == app.top()) {
                return false;
            }
            m_databaseInterface->fixAppToTop(app.id(), value.toInt());
            return true;
        case DataEntity::RecentInstall:
            app.setRecentInstall(value.toBool());
            return true;
        default:
            break;
    }

    return QAbstractItemModel::setData(index, value, role);
}

QHash<int, QByteArray> BasicAppModel::roleNames() const
{
    return DataEntity::AppRoleNames();
}

const AppDatabaseInterface *BasicAppModel::databaseInterface() const
{
    return m_databaseInterface;
}

void BasicAppModel::onAppAdded(const DataEntityVector &apps)
{
    DataEntityVector appItems;
    for (auto const & app : apps) {
        if (indexOfApp(app.id()) < 0) {
            appItems.append(app);
        }
    }
    if (appItems.isEmpty()) return;
    beginInsertRows(QModelIndex(), m_apps.size(), m_apps.size() + appItems.size() - 1);
    m_apps.append(appItems);
    endInsertRows();
}

void BasicAppModel::onAppUpdated(const QVector<QPair<DataEntity, QVector<int> > > &updates)
{
    for (const auto &pair : updates) {
        int index = indexOfApp(pair.first.id());
        if (index < 0) {
            continue;
        }

        DataEntity &app = m_apps[index];
        QVector<int> roles = pair.second;
        if (!roles.isEmpty()) {
            for (const auto &role : roles) {
                app.setValue(static_cast<DataEntity::PropertyName>(role), pair.first.getValue(static_cast<DataEntity::PropertyName>(role)));
            }
        } else {
            app = pair.first;
        }

        dataChanged(QAbstractListModel::index(index), QAbstractListModel::index(index), roles);
    }
}

void BasicAppModel::onAppDeleted(const QStringList &apps)
{
    for (const auto &appid : apps) {
        UserConfig::instance()->removePreInstalledApp(appid);
        int index = indexOfApp(appid);
        if (index < 0) {
            continue;
        }

        beginRemoveRows(QModelIndex(), index, index);
        m_apps.takeAt(index);
        endRemoveRows();
    }
}

int BasicAppModel::indexOfApp(const QString &appid) const
{
    if (appid.isEmpty()) {
        return -1;
    }

    auto it = std::find_if(m_apps.constBegin(), m_apps.constEnd(), [&appid] (const DataEntity &app) {
        return app.id() == appid;
    });

    if (it == m_apps.constEnd()) {
        return -1;
    }

    return std::distance(m_apps.constBegin(), it);
}

DataEntity BasicAppModel::appOfIndex(int row) const
{
    QModelIndex idx = QAbstractListModel::index(row);
    if (!checkIndex(idx, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return {};
    }

    return m_apps.at(row);
}

bool BasicAppModel::getAppById(const QString &appid, DataEntity &app) const
{
    int idx = indexOfApp(appid);
    if (idx < 0) {
        return false;
    }

    app = m_apps.at(idx);
    return true;
}

} // LingmoUIMenu
