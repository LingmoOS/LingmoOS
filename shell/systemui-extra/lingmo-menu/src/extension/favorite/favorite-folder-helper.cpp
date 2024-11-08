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
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMenu>

#include "favorite-folder-helper.h"
#include "event-track.h"
#include "favorites-config.h"
#include "libappdata/basic-app-model.h"

#define FOLDER_FILE_PATH ".config/lingmo-menu/"
#define FOLDER_FILE_NAME "folder.json"

namespace LingmoUIMenu {

QString FavoriteFolderHelper::s_folderConfigFile = QDir::homePath() + "/" + FOLDER_FILE_PATH + FOLDER_FILE_NAME;
/**
 * changelog 1.0.0 添加版本号，区分旧文件内容
 */
static const QString FOLDER_CONFIG_VERSION = QStringLiteral("1.0.0");
FavoriteFolderHelper *FavoriteFolderHelper::instance()
{
    static FavoriteFolderHelper FavoriteFolderHelper;
    return &FavoriteFolderHelper;
}

FavoriteFolderHelper::FavoriteFolderHelper()
{
    qRegisterMetaType<FavoritesFolder>("FavoritesFolder");

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

FavoriteFolderHelper::~FavoriteFolderHelper()
{
    saveData();
}

void FavoriteFolderHelper::insertFolder(const FavoritesFolder &folder)
{
    QMutexLocker locker(&m_mutex);
    m_folders.insert(folder.id, folder);
}

void FavoriteFolderHelper::addAppToFolder(const QString &appId, const int &folderId)
{
    if (appId.isEmpty()) {
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        if (!m_folders.contains(folderId)) {
            return;
        }

        FavoritesFolder &folder = m_folders[folderId];
        if (folder.apps.contains(appId)) {
            return;
        }

        folder.apps.append(appId);
    }

    forceSync();
    Q_EMIT folderDataChanged(folderId,appId);

    EventTrack::instance()->sendDefaultEvent("add_app_to_folder", "AppView");
}

void FavoriteFolderHelper::addAppToNewFolder(const QString &appId, const QString &folderName)
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

    FavoritesFolder folder;
    folder.id = ++max;
    folder.name = name;
    folder.apps.append(appId);

    insertFolder(folder);
    forceSync();
    Q_EMIT folderAdded(folder.id, FavoritesConfig::instance().getOrderById(APP_ID_SCHEME + appId));

    EventTrack::instance()->sendDefaultEvent("add_app_to_new_folder", "AppView");
}

void FavoriteFolderHelper::addAppsToNewFolder(const QString &idFrom, const QString &idTo, const QString &folderName)
{
    if (idFrom.isEmpty() || idTo.isEmpty()) {
        return;
    }

    // TODO: max越界处理
    int max = m_folders.isEmpty() ? -1 : m_folders.lastKey();
    QString name = folderName;
    if (name.isEmpty()) {
        name = tr("New Folder %1").arg(m_folders.size() + 1);
    }

    FavoritesFolder folder;
    folder.id = ++max;
    folder.name = name;
    folder.apps.append(idFrom);
    folder.apps.append(idTo);

    insertFolder(folder);
    // 确定folder位置
    int orderTo = FavoritesConfig::instance().getOrderById(APP_ID_SCHEME + idTo);
    int orderFrom = FavoritesConfig::instance().getOrderById(APP_ID_SCHEME + idFrom);
    int folderOrder;
    if (orderFrom > orderTo) {
        folderOrder = orderTo;
    } else {
        folderOrder = orderTo - 1;
    }

    Q_EMIT folderAdded(folder.id, folderOrder);
    forceSync();
}

void FavoriteFolderHelper::removeAppFromFolder(const QString& appId)
{
    if (appId.isEmpty()) {
        return;
    }

    int folderId;

    {
        QMutexLocker locker(&m_mutex);

        for (const auto &folder: m_folders) {
            if (folder.apps.contains(appId)) {
                folderId = folder.getId();
            }
        }

        if (!m_folders.contains(folderId)) {
            return;
        }
        FavoritesFolder &folder = m_folders[folderId];
        if (!folder.apps.contains(appId)) {
            return;
        }
        folder.apps.removeOne(appId);
    }

    if (m_folders[folderId].getApps().isEmpty()) {
        deleteFolder(folderId);
    }

    forceSync();
    Q_EMIT folderDataChanged(folderId, appId);
}

bool FavoriteFolderHelper::deleteFolder(const int& folderId)
{
    Q_EMIT folderToBeDeleted(folderId, m_folders[folderId].getApps());

    {
        QMutexLocker locker(&m_mutex);
        if (!m_folders.contains(folderId)) {
            return false;
        }

        if (!m_folders.remove(folderId)) {
            return false;
        }
    }

    forceSync();
    EventTrack::instance()->sendDefaultEvent("delete_folder", "AppView");
    return true;
}

void FavoriteFolderHelper::renameFolder(const int &folderId, const QString &folderName)
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

    Q_EMIT folderDataChanged(folderId, "");
    forceSync();
}

QList<FavoritesFolder> FavoriteFolderHelper::folderData()
{
    QMutexLocker locker(&m_mutex);
    return m_folders.values();
}

bool FavoriteFolderHelper::getFolderFromId(const int &folderId, FavoritesFolder& folder)
{
    QMutexLocker locker(&m_mutex);
    if (!m_folders.contains(folderId)) {
        return false;
    }

    const FavoritesFolder &tmp = m_folders[folderId];
    folder = tmp;

    return true;
}

bool FavoriteFolderHelper::containApp(const QString &appId)
{
    QMutexLocker locker(&m_mutex);
    return std::any_of(m_folders.constBegin(), m_folders.constEnd(), [appId] (const FavoritesFolder &folder) {
        return folder.apps.contains(appId);
    });
}

void FavoriteFolderHelper::forceSync()
{
    saveData();
}

void FavoriteFolderHelper::readData()
{
    QFile file(s_folderConfigFile);
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    // 读取json数据
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString str = stream.readAll();
    file.close();

    QJsonParseError jsonError;
    QJsonDocument jsonDocument(QJsonDocument::fromJson(str.toUtf8(),&jsonError));
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qWarning() << "FavoriteFolderHelper: Incorrect configuration files are ignored." << jsonError.error;
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        m_folders.clear();
    }

    QJsonObject fileObject = jsonDocument.object();
    if (fileObject.value("version").toString() == FOLDER_CONFIG_VERSION) {
        if (fileObject.contains("folders")) {
            QJsonArray jsonArray = fileObject.value(QLatin1String("folders")).toArray();
            for (const auto &value : jsonArray) {
                QJsonObject object = value.toObject();
                if (object.contains("name") && object.contains("id") && object.contains("apps")) {
                    FavoritesFolder folder;
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

    } else {
        saveData();
        return;
    }
}

void FavoriteFolderHelper::saveData()
{
    QFile file(s_folderConfigFile);
    if (!file.open(QFile::WriteOnly)) {
        return;
    }

    QJsonDocument jsonDocument;
    QJsonObject fileObject;
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

    fileObject.insert("version", FOLDER_CONFIG_VERSION);
    fileObject.insert("folders", folderArray);
    jsonDocument.setObject(fileObject);

    if (file.write(jsonDocument.toJson()) == -1) {
        qWarning() << "Error saving configuration file.";
    }
    file.flush();
    file.close();
}

QStringList FavoriteFolderHelper::folderIcon(const FavoritesFolder &folder)
{
    // TODO: 使用绘图API生成图片
    QStringList icons;
    DataEntity app;

    int count = qMin(folder.apps.count(), FOLDER_MAX_ICON_NUM);
    for (int i = 0; i < count; ++i) {
        if (BasicAppModel::instance()->getAppById(folder.apps.at(i), app)) {
            icons.append(app.icon());
        }
    }

    return icons;
}

QStringList FavoriteFolderHelper::appsInFolders()
{
    QStringList apps;
    for (const auto &folder : m_folders) {
        apps.append(folder.apps);
    }

    return apps;
}
} // LingmoUIMenu
