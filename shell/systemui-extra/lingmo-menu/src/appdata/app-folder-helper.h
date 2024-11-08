/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef LINGMO_MENU_APP_FOLDER_HELPER_H
#define LINGMO_MENU_APP_FOLDER_HELPER_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QMap>
#include <QString>
#include <QStringList>

namespace LingmoUIMenu {

#define FOLDER_MAX_ICON_NUM 16

class AppFolderHelper;

class Folder
{
    Q_GADGET
    Q_PROPERTY(int id READ getId)
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(QStringList apps READ getApps)
    friend class AppFolderHelper;
public:
    int getId() const { return id; }
    QString getName() const { return name; }
    QStringList getApps() const { return apps; }

private:
    int id;           // 文件夹唯一Id，文件夹排序值
    QString name;     // 名称
    QStringList apps; // 应用列表
};

class AppFolderHelper : public QObject
{
    Q_OBJECT
public:
    static AppFolderHelper *instance();
    static QStringList folderIcon(const Folder &folder);
    ~AppFolderHelper() override;

    AppFolderHelper(const AppFolderHelper& obj) = delete;
    AppFolderHelper &operator=(const AppFolderHelper& obj) = delete;
    AppFolderHelper(AppFolderHelper&& obj) = delete;
    AppFolderHelper &operator=(AppFolderHelper&& obj) = delete;

    // TODO 修改文件夹信息
    // xxxx
    bool searchFolder(const int& folderId, Folder &folder);
    bool searchFolderByAppName(const QString& appId, Folder &folder);
    bool containFolder(int folderId);
    bool containApp(const QString& appId);

    QList<Folder> folderData();

    void addAppToFolder(const QString& appId, const int& folderId);
    void addAppToNewFolder(const QString& appId, const QString& folderName);
    void addAppsToNewFolder(const QString& appIdA, const QString& appIdB, const QString& folderName);
    void removeAppFromFolder(const QString& appId, const int& folderId);
    bool deleteFolder(const int& folderId);
    void renameFolder(const int& folderId, const QString& folderName);

    void forceSync();

Q_SIGNALS:
    void folderAdded(int folderId);
    void folderDeleted(int folderId);
    void folderDataChanged(int folderId);

private:
    AppFolderHelper();
    void readData();
    void saveData();
    void insertFolder(const Folder& folder);
    // TODO 配置文件监听

private:
    QMutex m_mutex;
    QMap<int, Folder> m_folders;
    static QString s_folderConfigFile;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_FOLDER_HELPER_H
