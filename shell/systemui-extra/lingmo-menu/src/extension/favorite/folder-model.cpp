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

#include "folder-model.h"
#include "app-folder-helper.h"
#include "app-data-manager.h"
#include "favorite/favorite-folder-helper.h"
#include "context-menu-manager.h"
#include "libappdata/basic-app-model.h"

namespace LingmoUIMenu {

FolderModel::FolderModel(QObject *parent) : QAbstractListModel(parent)
{
    connect(FavoriteFolderHelper::instance(), &FavoriteFolderHelper::folderDataChanged, this, &FolderModel::loadFolderData);
    connect(FavoriteFolderHelper::instance(), &FavoriteFolderHelper::folderToBeDeleted, this, [this] (int id) {
        if (id == m_folderId) {
            beginResetModel();
            m_folderId = -1;
            m_apps.clear();
            endResetModel();
        }
    });
}

void FolderModel::setFolderId(const QString &folderId)
{
    bool ok;
    int id = folderId.toInt(&ok);
    if (!ok) {
        return;
    }

    loadFolderData(id);
}

void FolderModel::renameFolder(const QString &folderName)
{
    FavoriteFolderHelper::instance()->renameFolder(m_folderId, folderName);
}

void FolderModel::loadFolderData(int id)
{
    FavoritesFolder folder;
    if (!FavoriteFolderHelper::instance()->getFolderFromId(id, folder)) {
        return;
    }

    beginResetModel();
    m_folderId = id;
    m_apps = folder.getApps();
    endResetModel();

    Q_EMIT countChanged();
}

int FolderModel::rowCount(const QModelIndex &parent) const
{
    return m_apps.count();
}

QVariant FolderModel::data(const QModelIndex &index, int role) const
{
    int i = index.row();
    if (i < 0 || i >= m_apps.size()) {
        return {};
    }

    DataEntity app;

    if (!BasicAppModel::instance()->getAppById(m_apps.at(i), app)) {
        return {};
    }

    switch (role) {
        case DataEntity::Id:
            return app.id();
        case DataEntity::Type:
            return app.type();
        case DataEntity::Icon:
            return app.icon();
        case DataEntity::Name:
            return app.name();
        case DataEntity::Comment:
            return app.comment();
        case DataEntity::ExtraData:
            return app.extraData();
        default:
            break;
    }

    return {};
}

QHash<int, QByteArray> FolderModel::roleNames() const
{
    return DataEntity::AppRoleNames();
}

int FolderModel::count()
{
    return m_apps.count();
}

FolderModel &FolderModel::instance()
{
    static FolderModel folderModel;
    return folderModel;
}

} // LingmoUIMenu
