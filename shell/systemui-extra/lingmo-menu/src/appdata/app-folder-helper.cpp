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

#include "app-folder-helper.h"
#include "context-menu-manager.h"
#include "app-data-manager.h"
#include "model-manager.h"
#include "app-model.h"
#include "event-track.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMenu>

#define FOLDER_FILE_PATH ".config/lingmo-menu/"
#define FOLDER_FILE_NAME "folder.json"

namespace LingmoUIMenu {

//class FolderMenuProvider : public MenuProvider
//{
//public:
//    FolderMenuProvider();
//    int index() override { return 256; }
//    bool isSupport(const RequestType &type) override;
//    QList<QAction *> generateActions(QObject *parent, const QVariant &data, const MenuInfo::Location &location, const QString &locationId) override;
//private:
//    static void folderMenuForNormal(QObject *parent, const QString &appId, QList<QAction *> &list);
//    static void dissolveFolder(QObject *parent, const QString &folderId, QList<QAction *> &list);
//    static void renameFolder(QObject *parent, const QString &folderId, QList<QAction *> &list);
//};

//FolderMenuProvider::FolderMenuProvider()
//{
//
//}
//
//bool FolderMenuProvider::isSupport(const MenuProvider::RequestType &type)
//{
//    return type == DataType;
//}
//
//QList<QAction *>
//FolderMenuProvider::generateActions(QObject *parent, const QVariant &data, const MenuInfo::Location &location,
//                                    const QString &locationId)
//{
//    if (!parent) {
//        return {};
//    }
//
//    DataEntity app = data.value<DataEntity>();
//    QList<QAction *> list;
//
//    switch (location) {
//        case MenuInfo::FullScreen: {
//            if (app.type() == DataType::Folder) {
//                dissolveFolder(parent, app.id(), list);
//                break;
//            }
//        }
//        case MenuInfo::AppList: {
//            if (app.type() == DataType::Folder) {
//                renameFolder(parent, app.id(), list);
//                dissolveFolder(parent, app.id(), list);
//                break;
//            }
//
//            if (app.type() != DataType::Normal || locationId != "all") {
//                break;
//            }
//        }
//        case MenuInfo::FullScreenFolder:
//        case MenuInfo::FolderPage: {
//            if (app.type() == DataType::Normal) {
//                folderMenuForNormal(parent, app.id(), list);
//            }
//            break;
//        }
//        case MenuInfo::Extension:
//        default:
//            break;
//    }
//
//    return list;
//}
//
//void FolderMenuProvider::folderMenuForNormal(QObject *parent, const QString &appId, QList<QAction *> &list)
//{
//    Folder folder;
//    if (AppFolderHelper::instance()->searchFolderByAppName(appId, folder)) {
//        //从当前文件夹移除
//        list << new QAction(QObject::tr("Remove from folder"), parent);
//        QObject::connect(list.last(), &QAction::triggered, parent, [appId, folder] {
//            AppFolderHelper::instance()->removeAppFromFolder(appId, folder.getId());
//            AppFolderHelper::instance()->forceSync();
//        });
//        return;
//    }
//
//    QMenu *menu = static_cast<QMenu*>(parent);
//    //添加到应用组
//    QMenu *subMenu = new QMenu(QObject::tr("Add to folder"), menu);
//
//    QAction* newAct = new QAction(QObject::tr("Add to new folder"), subMenu);
//    QObject::connect(newAct, &QAction::triggered, parent, [appId] {
//        AppFolderHelper::instance()->addAppToNewFolder(appId, "");
//        AppFolderHelper::instance()->forceSync();
//    });
//    subMenu->addAction(newAct);
//
//    QList<Folder> folders = AppFolderHelper::instance()->folderData();
//    for (const Folder &f : folders) {
//        QString name = QObject::tr("Add to \"%1\"").arg(f.getName());
//        QAction* act = new QAction(name, subMenu);
//        QObject::connect(act, &QAction::triggered, parent, [appId, f] {
//            AppFolderHelper::instance()->addAppToFolder(appId, f.getId());
//        });
//        subMenu->addAction(act);
//    }
//    menu->addMenu(subMenu);
//}
//
//void FolderMenuProvider::dissolveFolder(QObject *parent, const QString &folderId, QList<QAction *> &list)
//{
//    list << new QAction(QObject::tr("Dissolve folder"), parent);
//    QObject::connect(list.last(), &QAction::triggered, parent, [folderId] {
//        AppFolderHelper::instance()->deleteFolder(folderId.toInt());
//        AppFolderHelper::instance()->forceSync();
//    });
//}
//
//void FolderMenuProvider::renameFolder(QObject *parent, const QString &folderId, QList<QAction *> &list)
//{
//    list << new QAction(QObject::tr("Rename"), parent);
//    QObject::connect(list.last(), &QAction::triggered, parent, [folderId] {
//        ModelManager::instance()->getAppModel()->toRenameFolder(folderId);
//    });
//}

QString AppFolderHelper::s_folderConfigFile = QDir::homePath() + "/" + FOLDER_FILE_PATH + FOLDER_FILE_NAME;

AppFolderHelper *AppFolderHelper::instance()
{
    static AppFolderHelper appFolderHelper;
    return &appFolderHelper;
}

AppFolderHelper::AppFolderHelper()
{
    qRegisterMetaType<Folder>("Folder");

    if (!QFile::exists(s_folderConfigFile)) {
        QDir dir;
        QString folderConfigDir(QDir::homePath() + "/" + FOLDER_FILE_PATH);
        if (!dir.exists(folderConfigDir)) {
            if (!dir.mkdir(folderConfigDir)) {
                qWarning() << "Unable to create profile folder.";
                return;
            }
        }

        QFile file(s_folderConfigFile);
        file.open(QFile::WriteOnly);
        file.close();
    }

    readData();
}

AppFolderHelper::~AppFolderHelper()
{
    saveData();
}

void AppFolderHelper::insertFolder(const Folder &folder)
{
    QMutexLocker locker(&m_mutex);
    m_folders.insert(folder.id, folder);
}

void AppFolderHelper::addAppToFolder(const QString &appId, const int &folderId)
{
    if (appId.isEmpty()) {
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        if (!m_folders.contains(folderId)) {
            return;
        }

        Folder &folder = m_folders[folderId];
        if (folder.apps.contains(appId)) {
            return;
        }

        folder.apps.append(appId);
    }

    forceSync();
    Q_EMIT folderDataChanged(folderId);

    EventTrack::instance()->sendDefaultEvent("add_app_to_folder", "AppView");
}

void AppFolderHelper::addAppToNewFolder(const QString &appId, const QString &folderName)
{
    if (appId.isEmpty()) {
        return;
    }

    // TODO: max越界处理
    int max = m_folders.isEmpty() ? -1 : m_folders.lastKey();
    QString name = folderName;
    if (name.isEmpty()) {
        name = tr("New Folder %1").arg(m_folders.size() + 1);
    }

    Folder folder;
    folder.id = ++max;
    folder.name = name;
    folder.apps.append(appId);

    insertFolder(folder);
    Q_EMIT folderAdded(folder.id);

    EventTrack::instance()->sendDefaultEvent("add_app_to_new_folder", "AppView");
}

void AppFolderHelper::addAppsToNewFolder(const QString &appIdA, const QString &appIdB, const QString &folderName)
{
    if (appIdA.isEmpty() || appIdB.isEmpty()) {
        return;
    }

    // TODO: max越界处理
    int max = m_folders.isEmpty() ? -1 : m_folders.lastKey();
    QString name = folderName;
    if (name.isEmpty()) {
        name = tr("New Folder %1").arg(m_folders.size() + 1);
    }

    Folder folder;
    folder.id = ++max;
    folder.name = name;
    folder.apps.append(appIdA);
    folder.apps.append(appIdB);

    insertFolder(folder);
    Q_EMIT folderAdded(folder.id);
    forceSync();
}

void AppFolderHelper::removeAppFromFolder(const QString &appId, const int &folderId)
{
    if (appId.isEmpty()) {
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        if (!m_folders.contains(folderId)) {
            return;
        }
        Folder &folder = m_folders[folderId];
        if (!folder.apps.contains(appId)) {
            return;
        }
        folder.apps.removeOne(appId);
    }

    if (m_folders[folderId].getApps().isEmpty()) {
        deleteFolder(folderId);
    }

    Q_EMIT folderDataChanged(folderId);
}

bool AppFolderHelper::deleteFolder(const int& folderId)
{
    {
        QMutexLocker locker(&m_mutex);
        if (!m_folders.contains(folderId)) {
            return false;
        }

        if (!m_folders.remove(folderId)) {
            return false;
        }
    }

    Q_EMIT folderDeleted(folderId);
    EventTrack::instance()->sendDefaultEvent("delete_folder", "AppView");
    return true;
}

void AppFolderHelper::renameFolder(const int &folderId, const QString &folderName)
{
    {
        QMutexLocker locker(&m_mutex);
        if (!m_folders.contains(folderId)) {
            return;
        }
        if (m_folders[folderId].name == folderName) {
            return;
        }
        m_folders[folderId].name = folderName;
    }

    Q_EMIT folderDataChanged(folderId);
    forceSync();
}

QList<Folder> AppFolderHelper::folderData()
{
    QMutexLocker locker(&m_mutex);
    return m_folders.values();
}

bool AppFolderHelper::searchFolder(const int &folderId, Folder &folder)
{
    QMutexLocker locker(&m_mutex);
    if (!m_folders.contains(folderId)) {
        return false;
    }

    const Folder &tmp = m_folders[folderId];
    folder = tmp;

    return true;
}

bool AppFolderHelper::searchFolderByAppName(const QString &appId, Folder &folder)
{
    if (appId.isEmpty()) {
        return false;
    }

    QMutexLocker locker(&m_mutex);
    for (const auto &tmp: m_folders) {
        if (tmp.apps.contains(appId)) {
            folder = tmp;
            return true;
        }
    }

    return false;
}

bool AppFolderHelper::containFolder(int folderId)
{
    QMutexLocker locker(&m_mutex);
    return m_folders.contains(folderId);
}

bool AppFolderHelper::containApp(const QString &appId)
{
    QMutexLocker locker(&m_mutex);
    return std::any_of(m_folders.constBegin(), m_folders.constEnd(), [appId] (const Folder &folder) {
        return folder.apps.contains(appId);
    });
}

void AppFolderHelper::forceSync()
{
    saveData();
    //readData();
}

void AppFolderHelper::readData()
{
    QFile file(s_folderConfigFile);
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    // 读取json数据
    QByteArray byteArray = file.readAll();
    file.close();

    QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray));
    if (jsonDocument.isNull() || jsonDocument.isEmpty() || !jsonDocument.isArray()) {
        qWarning() << "AppFolderHelper: Incorrect configuration files are ignored.";
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        m_folders.clear();
    }

    // 遍历json数据节点
    QJsonArray jsonArray = jsonDocument.array();
    for (const auto &value : jsonArray) {
        QJsonObject object = value.toObject();
        if (object.contains("name") && object.contains("id") && object.contains("apps")) {
            Folder folder;
            folder.name = object.value(QLatin1String("name")).toString();
            folder.id = object.value(QLatin1String("id")).toInt();

            QJsonArray apps = object.value(QLatin1String("apps")).toArray();
            for (const auto &app : apps) {
                folder.apps.append(app.toString());
            }

            if (!folder.apps.isEmpty()) {
                insertFolder(folder);
            }
        }
    }
}

void AppFolderHelper::saveData()
{
    QFile file(s_folderConfigFile);
    if (!file.open(QFile::WriteOnly)) {
        return;
    }

    QJsonDocument jsonDocument;
    QJsonArray folderArray;

    {
        QMutexLocker locker(&m_mutex);
        for (const auto &folder : m_folders) {
            QJsonObject object;
            QJsonArray apps;

            for (const auto &app : folder.apps) {
                apps.append(app);
            }

            object.insert("name", folder.name);
            object.insert("id", folder.id);
            object.insert("apps", apps);

            folderArray.append(object);
        }
    }

    jsonDocument.setArray(folderArray);

    if (file.write(jsonDocument.toJson()) == -1) {
        qWarning() << "Error saving configuration file.";
    }
    file.flush();
    file.close();
}

QStringList AppFolderHelper::folderIcon(const Folder &folder)
{
    // TODO: 使用绘图API生成图片
    QStringList icons;
    DataEntity app;

    int count = qMin(folder.apps.count(), FOLDER_MAX_ICON_NUM);
    for (int i = 0; i < count; ++i) {
        if (AppDataManager::instance()->getApp(folder.apps.at(i), app)) {
            icons.append(app.icon());
        }
    }

    return icons;
}

} // LingmoUIMenu
