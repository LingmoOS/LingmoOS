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

#ifndef LINGMO_MENU_APP_FAVORITE_MODEL_H
#define LINGMO_MENU_APP_FAVORITE_MODEL_H

#include "data-entity.h"
#include "libappdata/basic-app-model.h"

#include <QObject>
#include <QStringList>
#include <QAbstractListModel>

namespace LingmoUIMenu {

class AppFavoritesModel : public QAbstractListModel
{
Q_OBJECT
public:
    static AppFavoritesModel &instance();
    ~AppFavoritesModel() override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void clearFavorites();
    void addAppToFavorites(const QString &id, const int &index);
    void removeAppFromFavorites(const QString &id);
    void changeFileState(const QString &url, const bool &favorite);
    bool isAppIncluded(const QString &appid);

private:
    explicit AppFavoritesModel(QObject *parent = nullptr);

    void getFavoritesApps();
    void getFoldersId();
    void getFavoritesFiles();
    void updateData();

    void addFavoriteApp(const QPersistentModelIndex &modelIndex);
    void removeFavoriteApp(const QPersistentModelIndex &modelIndex);
    void onAppRemoved(const QModelIndex &parent, int first, int last);
    void onAppUpdated(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    void updateFavoritesApps(const DataEntity &app, const QModelIndex &sourceIndex);
    QPersistentModelIndex getIndexFromAppId(const QString &id) const;

    void onFolderAdded(const int &folderId, const int &order);
    void onFolderDeleted(const int &folderId, const QStringList &apps);
    void onFolderChanged(const int &folderId, const QString &appId);

    QVariant folderData(const QModelIndex &index, int role) const;
    QVariant fileData(const QModelIndex &index, int role) const;

private:
    /**
     *不在应用组 的收藏应用在baseModel的对应index
     */
    QVector<QPersistentModelIndex> m_favoritesApps;
    /**
     *应用组的唯一Id
     */
    QVector<int> m_folders;
    /**
     *收藏文件夹的唯一路径
     */
    QVector<QString> m_favoritesFiles;
    BasicAppModel *m_sourceModel = nullptr;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_FAVORITE_MODEL_H
