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

#ifndef LINGMO_MENU_FAVORITES_MODEL_H
#define LINGMO_MENU_FAVORITES_MODEL_H

#include <QSortFilterProxyModel>
#include "favorites-config.h"
#include "app-favorite-model.h"

namespace LingmoUIMenu {

class FavoritesModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    static FavoritesModel &instance();

    Q_INVOKABLE void openMenu(const int &row);
    /**
     * 通过id添加应用到收藏
     * @param id
     * @param index 默认为-1,添加到收藏最后一位;可以添加到收藏指定位置
     */
    Q_INVOKABLE void addAppToFavorites(const QString &id, const int &index = -1);
    /**
     * 通过id从收藏中移除
     * @param id
     */
    Q_INVOKABLE void removeAppFromFavorites(const QString &id);
    /**
     * 拖拽交换位置。
     * @param indexFrom
     * @param indexTo
     */
    Q_INVOKABLE void exchangedAppsOrder(const int &indexFrom, const int &indexTo);
    /**
     * 两个应用推拽合并为应用组。
     * @param idFrom
     * @param idTo 拖拽至某一应用，并在该位置创建应用组
     */
    Q_INVOKABLE void addAppsToNewFolder(const QString &idFrom, const QString &idTo);
    /**
     * 添加应用到应用组，包括新建应用组。
     * @param appId
     * @param folderId id为""时,新建应用组；id为数值时，添加至对应应用组
     */
    Q_INVOKABLE void addAppToFolder(const QString &appId, const QString& folderId);
    /**
     * 添加文件到收藏区域
     * @param url 文件对应的url路径
     */
    Q_INVOKABLE void addFileToFavorites(const QString &url);
    /**
     * 清空所有已收藏应用
     */
    Q_INVOKABLE void clearFavorites();

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    explicit FavoritesModel(QObject *parent = nullptr);

    QString urlFromModelIndex(const QModelIndex &modelIndex) const;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_FAVORITES_MODEL_H
