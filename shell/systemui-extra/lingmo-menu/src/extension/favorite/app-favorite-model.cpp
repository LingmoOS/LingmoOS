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

#include "favorites-config.h"
#include "app-favorite-model.h"
#include "favorite-folder-helper.h"

#include <QMimeDatabase>
#include <QFile>
#include <QUrl>

namespace LingmoUIMenu {

AppFavoritesModel &AppFavoritesModel::instance()
{
    static AppFavoritesModel appFavoritesModel;
    return appFavoritesModel;
}

AppFavoritesModel::AppFavoritesModel(QObject *parent) : QAbstractListModel(parent)
{
    m_sourceModel = BasicAppModel::instance();

    updateData();

    connect(m_sourceModel, &BasicAppModel::dataChanged, this, &AppFavoritesModel::onAppUpdated);
    connect(m_sourceModel, &BasicAppModel::rowsAboutToBeRemoved, this, &AppFavoritesModel::onAppRemoved);
    connect(FavoriteFolderHelper::instance(), &FavoriteFolderHelper::folderAdded, this,&AppFavoritesModel::onFolderAdded);
    connect(FavoriteFolderHelper::instance(), &FavoriteFolderHelper::folderToBeDeleted, this, &AppFavoritesModel::onFolderDeleted);
    connect(FavoriteFolderHelper::instance(), &FavoriteFolderHelper::folderDataChanged, this, &AppFavoritesModel::onFolderChanged);
}

AppFavoritesModel::~AppFavoritesModel()
{
}

QHash<int, QByteArray> AppFavoritesModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names.insert(DataEntity::Id, "id");
    names.insert(DataEntity::Icon, "icon");
    names.insert(DataEntity::Name, "name");
    names.insert(DataEntity::Type, "type");
    return names;
}

int AppFavoritesModel::rowCount(const QModelIndex &parent) const
{
    return m_favoritesApps.count() + m_folders.count() + m_favoritesFiles.count();
}

QVariant AppFavoritesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return {};
    }

    if (index.row() < m_favoritesApps.count()) {
        return m_favoritesApps.at(index.row()).data(role);

    } else if (index.row() < m_favoritesApps.count() + m_folders.count() && index.row() >= m_favoritesApps.count()) {
        return folderData(index, role);

    } else {
        return fileData(index, role);
    }
}

void AppFavoritesModel::getFavoritesApps()
{
    m_favoritesApps.clear();
    for (int i = 0; i < m_sourceModel->rowCount(QModelIndex()); i++) {
        updateFavoritesApps(m_sourceModel->appOfIndex(i), m_sourceModel->index(i));
    }
}

void AppFavoritesModel::onAppUpdated(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    // 更新全部信息
    if (roles.isEmpty()) {
        QPersistentModelIndex topLeftIndex(topLeft);
        QPersistentModelIndex bottomRightIndex(bottomRight);

        int leftIndex = m_favoritesApps.indexOf(topLeftIndex);
        int rightIndex = m_favoritesApps.indexOf(topLeftIndex);

        Q_EMIT dataChanged(index(leftIndex, 0, QModelIndex()), index(rightIndex, 0, QModelIndex()));
        // 更新某一项信息
    } else {
        for (int row = topLeft.row(); row <= bottomRight.row(); row ++) {
            if (roles.contains(DataEntity::Favorite) ) {
                updateFavoritesApps(m_sourceModel->appOfIndex(row), m_sourceModel->index(row, 0, QModelIndex()));
            }
        }
    }
}

void AppFavoritesModel::getFoldersId()
{
    QVector<int> foldersId;
    for (const auto &folder : FavoriteFolderHelper::instance()->folderData()) {
        foldersId.append(folder.getId());
        FavoritesConfig::instance().insertValue(FOLDER_ID_SCHEME + QString::number(folder.getId()));
    }
    m_folders.swap(foldersId);
}

void AppFavoritesModel::getFavoritesFiles()
{
}

void AppFavoritesModel::updateFavoritesApps(const DataEntity &app, const QModelIndex &sourceIndex)
{
    if (app.id().isEmpty()) {
        return;
    }
    QPersistentModelIndex index(sourceIndex);

    if (app.favorite() > 0 && !FavoriteFolderHelper::instance()->containApp(app.id())) {
        if (m_favoritesApps.contains(index)) {
            return;
        }
        addFavoriteApp(index);
        FavoritesConfig::instance().insertValue(APP_ID_SCHEME + index.data(DataEntity::Id).toString());

    } else if (app.favorite() == 0) {
        if (FavoriteFolderHelper::instance()->containApp(app.id())) {
            FavoriteFolderHelper::instance()->removeAppFromFolder(app.id());
        }
        removeFavoriteApp(index);
    }
}

void AppFavoritesModel::updateData()
{
    getFavoritesApps();
    getFoldersId();
    getFavoritesFiles();
}

QVariant AppFavoritesModel::folderData(const QModelIndex &index, int role) const
{
    FavoritesFolder folder;
    int folderId = m_folders.at(index.row() - m_favoritesApps.count());
    if (!FavoriteFolderHelper::instance()->getFolderFromId(folderId, folder)) {
        return {};
    }

    switch (role) {
        case DataEntity::Id:
            return QString::number(folder.getId());
        case DataEntity::Icon:
            return FavoriteFolderHelper::folderIcon(folder).join(" ");
        case DataEntity::Name:
            return folder.getName();
        case DataEntity::Type:
            return DataType::Folder;
        default:
            break;
    }

    return {};
}

QVariant AppFavoritesModel::fileData(const QModelIndex &index, int role) const
{
    int fileIndex = index.row() -  m_favoritesApps.count() - m_folders.count();
    QString url = m_favoritesFiles.at(fileIndex);

    switch (role) {
        case DataEntity::Id:
            return url;
        case DataEntity::Icon: {
            QMimeDatabase mimeDatabase;
            return mimeDatabase.mimeTypeForFile(url).iconName();
        }
        case DataEntity::Name:
            return QUrl(url).fileName();
        case DataEntity::Type:
            return DataType::Files;
        default:
            break;
    }

    return {};
}

void AppFavoritesModel::clearFavorites()
{
    for(const auto &appIndex : m_favoritesApps) {
        removeAppFromFavorites(appIndex.data(DataEntity::Id).toString());
    }

    for (const auto &id : FavoriteFolderHelper::instance()->appsInFolders()) {
        removeAppFromFavorites(id);
    }

    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_favoritesApps.clear();
    m_folders.clear();
    m_favoritesFiles.clear();
    endRemoveRows();

    FavoritesConfig::instance().clear();
}

bool AppFavoritesModel::isAppIncluded(const QString &appid)
{
    QModelIndex sourceIndex = m_sourceModel->index(m_sourceModel->indexOfApp(appid), 0, {});
    if (!sourceIndex.isValid()) {
        return false;
    }

    QPersistentModelIndex index(sourceIndex);
    return m_favoritesApps.contains(index);
}

void AppFavoritesModel::onAppRemoved(const QModelIndex &parent, int first, int last)
{
    for(int row = first; row <= last; ++row) {
        QModelIndex sourceIndex = m_sourceModel->index(row, 0, {});
        QPersistentModelIndex index(sourceIndex);

        if (index.data(DataEntity::Favorite).toInt() > 0) {
            QString appId = index.data(DataEntity::Id).toString();

            if (!m_favoritesApps.contains(index) && FavoriteFolderHelper::instance()->containApp(appId)) {
                FavoriteFolderHelper::instance()->removeAppFromFolder(appId);
            }

            removeFavoriteApp(index);
        }
    }
}

void AppFavoritesModel::addFavoriteApp(const QPersistentModelIndex &modelIndex)
{
    if (m_favoritesApps.contains(modelIndex) || !modelIndex.isValid()) {
        return;
    }

    beginInsertRows(QModelIndex(), m_favoritesApps.count(), m_favoritesApps.count());
    m_favoritesApps.append(modelIndex);
    endInsertRows();
}

void AppFavoritesModel::removeFavoriteApp(const QPersistentModelIndex &modelIndex)
{
    if (!m_favoritesApps.contains(modelIndex)) {
        return;
    }

    beginRemoveRows(QModelIndex(), m_favoritesApps.indexOf(modelIndex), m_favoritesApps.indexOf(modelIndex));
    m_favoritesApps.removeOne(modelIndex);
    endRemoveRows();

    FavoritesConfig::instance().removeValueById(APP_ID_SCHEME + modelIndex.data(DataEntity::Id).toString());
}

void AppFavoritesModel::onFolderAdded(const int &folderId, const int &order)
{
    if (!m_folders.contains(folderId)) {
        beginInsertRows(QModelIndex(), m_favoritesApps.count() + m_folders.count(), m_favoritesApps.count() + m_folders.count());
        m_folders.append(folderId);
        endInsertRows();

        FavoritesFolder folder;
        FavoriteFolderHelper::instance()->getFolderFromId(folderId, folder);
        for (auto app : folder.getApps()) {
            removeFavoriteApp(getIndexFromAppId(app));
        }

        FavoritesConfig::instance().insertValue(FOLDER_ID_SCHEME + QString::number(folderId), std::max(0, order));
    }
}

void AppFavoritesModel::onFolderDeleted(const int &folderId, const QStringList &apps)
{
    if (m_folders.contains(folderId)) {
        beginRemoveRows(QModelIndex(), m_favoritesApps.count() + m_folders.indexOf(folderId), m_favoritesApps.count() + m_folders.indexOf(folderId));
        m_folders.removeOne(folderId);
        endRemoveRows();

        int index = FavoritesConfig::instance().getOrderById(FOLDER_ID_SCHEME +QString::number(folderId));
        for (int i = 0; i < apps.count(); i++) {
            QPersistentModelIndex modelIndex(m_sourceModel->index(m_sourceModel->indexOfApp(apps.at(i))));
            addFavoriteApp(modelIndex);
            FavoritesConfig::instance().insertValue(APP_ID_SCHEME + modelIndex.data(DataEntity::Id).toString(), index + i);
        }

        FavoritesConfig::instance().removeValueById(FOLDER_ID_SCHEME + QString::number(folderId));
    }
}

void AppFavoritesModel::onFolderChanged(const int &folderId, const QString &appId)
{
    QVector<int> roles;
    if (appId.isEmpty()) {
        roles.append(DataEntity::Name);
        int row = m_favoritesApps.count() + m_folders.indexOf(folderId);

        Q_EMIT dataChanged(index(row), index(row), roles);
        return;
    }

    if (FavoriteFolderHelper::instance()->containApp(appId)) {
        removeFavoriteApp(getIndexFromAppId(appId));

    } else {
        QPersistentModelIndex modelIndex(m_sourceModel->index(m_sourceModel->indexOfApp(appId)));
        addFavoriteApp(modelIndex);
        FavoritesConfig::instance().insertValue(APP_ID_SCHEME + modelIndex.data(DataEntity::Id).toString());
    }

    roles.append(DataEntity::Icon);
    int row = m_favoritesApps.count() + m_folders.indexOf(folderId);

    Q_EMIT dataChanged(index(row), index(row), roles);
}

QPersistentModelIndex AppFavoritesModel::getIndexFromAppId(const QString &id) const
{
    auto modelIndex = std::find_if(m_favoritesApps.constBegin(), m_favoritesApps.constEnd(), [&id] (const QPersistentModelIndex &index) {
        return index.data(DataEntity::Id).toString() == id;
    });

    if (modelIndex == m_favoritesApps.constEnd()) {
        return {};
    }

    return *modelIndex;
}

void AppFavoritesModel::changeFileState(const QString &url, const bool &favorite)
{
    if (url.isEmpty()) {
        return;
    }
    QString fileId;
    if (url.startsWith(FILE_ID_SCHEME)) {
        fileId = url;
    } else {
        fileId = FILE_ID_SCHEME + url;
    }

    if (favorite) {
        m_favoritesFiles.append(url);
        FavoritesConfig::instance().insertValue(fileId);
    } else {
        m_favoritesFiles.removeAll(url);
        FavoritesConfig::instance().removeValueById(fileId);
    }
}

void AppFavoritesModel::addAppToFavorites(const QString &id, const int &index)
{
    if (id.isEmpty()) {
        return;
    }
    QPersistentModelIndex modelIndex(m_sourceModel->index(m_sourceModel->indexOfApp(id)));
    addFavoriteApp(modelIndex);

    m_sourceModel->databaseInterface()->fixAppToFavorite(id, 1);

    // 同步配置文件，更新应用顺序
    if (index > 0) {
        FavoritesConfig::instance().insertValue(APP_ID_SCHEME + id, index);
    }
}

void AppFavoritesModel::removeAppFromFavorites(const QString &id)
{
    if (id.isEmpty()) {
        return;
    }

    m_sourceModel->databaseInterface()->fixAppToFavorite(id, 0);
}
} // LingmoUIMenu
