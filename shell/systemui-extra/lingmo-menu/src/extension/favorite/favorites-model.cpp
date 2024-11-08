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
 */

#include <QDir>

#include "../context-menu-manager.h"
#include "favorites-model.h"
#include "app-favorite-model.h"
#include "favorite-folder-helper.h"

namespace LingmoUIMenu {

FavoritesModel &FavoritesModel::instance()
{
    static FavoritesModel favoritesModel;
    return favoritesModel;
}

FavoritesModel::FavoritesModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    connect(&FavoritesConfig::instance(), &FavoritesConfig::configChanged, this, &FavoritesModel::invalidate);
}

bool FavoritesModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    int index_left = -1;
    int index_right = -1;

    QString id_left = urlFromModelIndex(source_left);
    QString id_right = urlFromModelIndex(source_right);

    index_left = FavoritesConfig::instance().getOrderById(id_left);
    index_right = FavoritesConfig::instance().getOrderById(id_right);

    return index_left < index_right;
}

QString FavoritesModel::urlFromModelIndex(const QModelIndex &modelIndex) const
{
    QString url;
    if (modelIndex.data(DataEntity::Type).value<DataType::Type>() == DataType::Folder) {
        url = FOLDER_ID_SCHEME + modelIndex.data(DataEntity::Id).toString();
    } else if (modelIndex.data(DataEntity::Type).value<DataType::Type>() == DataType::Files) {
        url = FILE_ID_SCHEME + modelIndex.data(DataEntity::Id).toString();
    } else if (modelIndex.data(DataEntity::Type).value<DataType::Type>() == DataType::Normal) {
        url = APP_ID_SCHEME + modelIndex.data(DataEntity::Id).toString();
    }

    return url;
}

void FavoritesModel::openMenu(const int &row)
{
    if (row < 0 || row >= sourceModel()->rowCount()) {
        return;
    }

    DataEntity data;
    data.setId(sourceModel()->data(mapToSource(index(row, 0)), DataEntity::Id).toString());
    data.setFavorite(sourceModel()->data(mapToSource(index(row, 0)), DataEntity::Favorite).toInt());
    data.setType(sourceModel()->data(mapToSource(index(row, 0)), DataEntity::Type).value<DataType::Type>());

    ContextMenuManager::instance()->showMenu(data, MenuInfo::Extension, "favorite");
}

void FavoritesModel::addAppToFavorites(const QString &id, const int &index)
{
    if (AppFavoritesModel::instance().isAppIncluded(id) || FavoriteFolderHelper::instance()->containApp(id)) {
        qWarning() << "This application is already included in the favorite apps!";
        return;
    }

    int appOrder;
    if (index == -1) {
        appOrder = FavoritesConfig::instance().configSize();
    } else {
        appOrder = index;
    }
    AppFavoritesModel::instance().addAppToFavorites(id, appOrder);
}

void FavoritesModel::removeAppFromFavorites(const QString &id)
{
    AppFavoritesModel::instance().removeAppFromFavorites(id);
}

void FavoritesModel::exchangedAppsOrder(const int &indexFrom, const int &indexTo)
{
    if (indexFrom == indexTo) {
        return;
    }
    FavoritesConfig::instance().changeOrder(indexFrom, indexTo);
}

void FavoritesModel::addAppsToNewFolder(const QString &idFrom, const QString &idTo)
{
    if (idFrom == idTo) {
        return;
    }

    if (AppFavoritesModel::instance().isAppIncluded(idFrom) && AppFavoritesModel::instance().isAppIncluded(idTo)) {
        FavoriteFolderHelper::instance()->addAppsToNewFolder(idFrom, idTo, "");
    }
}

void FavoritesModel::addAppToFolder(const QString &appId, const QString &folderId)
{
    if (folderId == "") {
        FavoriteFolderHelper::instance()->addAppToNewFolder(appId, "");
        return;
    }

    FavoriteFolderHelper::instance()->addAppToFolder(appId, folderId.toInt());
}

void FavoritesModel::addFileToFavorites(const QString &url)
{
    AppFavoritesModel::instance().changeFileState(url, true);
}

void FavoritesModel::clearFavorites()
{
    AppFavoritesModel::instance().clearFavorites();
}
} // LingmoUIMenu
