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

#ifndef LINGMO_MENU_FAVORITE_FOLDER_HELPER_H
#define LINGMO_MENU_FAVORITE_FOLDER_HELPER_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QMap>
#include <QString>
#include <QStringList>

namespace LingmoUIMenu {

#define FOLDER_MAX_ICON_NUM 16

class FavoriteFolderHelper;

class FavoritesFolder
{
Q_GADGET
    Q_PROPERTY(int id READ getId WRITE setId)
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(QStringList apps READ getApps)
    friend class FavoriteFolderHelper;
public:
    void setId(int folderId) {id = folderId;}
    int getId() const { return id; }
    QString getName() const { return name; }
    QStringList getApps() const { return apps; }

private:
    int id;           // 文件夹唯一Id，文件夹排序值
    QString name;     // 名称
    QStringList apps; // 应用列表
};

class FavoriteFolderHelper : public QObject
{
    Q_OBJECT
public:
    static FavoriteFolderHelper *instance();
    static QStringList folderIcon(const FavoritesFolder &folder);
    ~FavoriteFolderHelper() override;

    bool getFolderFromId(const int& folderId, FavoritesFolder& folder);
    bool containApp(const QString& appId);
    bool deleteFolder(const int& folderId);

    QList<FavoritesFolder> folderData();

    void addAppToFolder(const QString& appId, const int& folderId);
    void addAppToNewFolder(const QString& appId, const QString& folderName);
    void addAppsToNewFolder(const QString& idFrom, const QString& idTo, const QString& folderName);
    void removeAppFromFolder(const QString& appId);
    void renameFolder(const int& folderId, const QString& folderName);

    void forceSync();
    QStringList appsInFolders();

Q_SIGNALS:
    void folderAdded(int folderId, const int& order);
    void folderToBeDeleted(int folderId, const QStringList& apps);
    void folderDataChanged(int folderId, const QString& app);

private:
    FavoriteFolderHelper();
    void readData();
    void saveData();
    void insertFolder(const FavoritesFolder& folder);
    // TODO 配置文件监听

private:
    QMutex m_mutex;
    //TODO 指针
    QMap<int, FavoritesFolder> m_folders;
    static QString s_folderConfigFile;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_FAVORITE_FOLDER_HELPER_H
