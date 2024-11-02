/*
 *
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "config.h"
#include <QDir>
#include <mutex>
#include <QFile>
#include <QDebug>
#include <QMap>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonArray>

static const QString HOME_PATH = QDir::homePath();
static const QString OLD_BLOCK_DIR_SETTINGS = HOME_PATH + "/.config/org.lingmo/lingmo-search/lingmo-search-block-dirs.conf";
static const QString OLD_INDEXABLE_DIR_SETTINGS = HOME_PATH + "/.config/org.lingmo/lingmo-search/lingmo-search-current-indexable-dir.conf";
static const QString SEARCH_DIRS_SETTINGS = HOME_PATH + "/.config/org.lingmo/lingmo-search/lingmo-search-dirs.json";
static const QString SEARCH_DIRS_SETTINGS_DIR = HOME_PATH + "/.config/org.lingmo/lingmo-search";
static const QString INDEXABLE_DIR_KEY = "IndexableDir"; //兼容历史版本
static const QString CONFIG_VERSION_KEY = "ConfigVersion";
static const QString CONFIG_VERSION = "1.0";
static const QString BLOCK_DIRS_FOR_USER_KEY = "BlockDirsForUser";
static const QString GLOBAL_BLACK_LIST_KEY = "GlobalBlackList";
static const QStringList GLOBAL_BLACK_LIST{"/proc", "/sys", "/dev", "/tmp", "/run"};
static const QString GLOBAL_SETTINGS_GROUP = "GlobalSettings";
static const QString SEARCH_DIRS_GROUP = "SearchDirs";

static std::once_flag flag;
static Config *global_instance = nullptr;
Config *Config::self()
{
    std::call_once(flag, [ & ] {
        global_instance = new Config();
    });
    return global_instance;
}

void Config::addDir(const SearchDir &dir)
{
    QJsonObject searchDirData = m_settingsData.value(SEARCH_DIRS_GROUP).toObject();
    QJsonArray blackList;
    for(const QString& path : dir.getBlackList()) {
        blackList.append(path);
    }
    searchDirData.insert(dir.getPath(), blackList);
    m_settingsData.insert(SEARCH_DIRS_GROUP, searchDirData);
    save(m_settingsData);
}

QStringList Config::removeDir(const SearchDir &dir)
{
    QJsonObject searchDirData = m_settingsData.value(SEARCH_DIRS_GROUP).toObject();
    QStringList blackDirs = searchDirData.take(dir.getPath()).toVariant().toStringList();
    m_settingsData.insert(SEARCH_DIRS_GROUP, searchDirData);
    save(m_settingsData);
    return blackDirs;
}

int Config::addBlockDirOfUser(const QString &dir)
{
    if (!QFile::exists(dir)) {
        return SearchDir::NotExists;
    }

    QJsonObject globalSettings = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject();
    QJsonArray blockDirsOfUser = globalSettings.value(BLOCK_DIRS_FOR_USER_KEY).toArray();

    for (auto it = blockDirsOfUser.begin(); it !=blockDirsOfUser.end();) {
        QString blockDir = (*it).toString();
        if (dir == blockDir || dir.startsWith(blockDir + "/") || blockDir == "/") {
            return SearchDir::Duplicated;
        }
        //有子文件夹已被添加，删除这些子文件夹
        if (blockDir.startsWith(dir + "/") || dir == "/") {
            it = blockDirsOfUser.erase(it);
        } else {
            it++;
        }
    }

    blockDirsOfUser.append(dir);
    globalSettings.insert(BLOCK_DIRS_FOR_USER_KEY, blockDirsOfUser);
    m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
    save(m_settingsData);
    return SearchDir::Successful;
}

void Config::removeBlockDirOfUser(const QString &dir)
{
    QJsonObject globalSettings = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject();
    QJsonArray blockDirs = globalSettings.value(BLOCK_DIRS_FOR_USER_KEY).toArray();
    bool removeItem(false);

    for (auto it = blockDirs.begin(); it != blockDirs.end();) {
        if ((*it).toString() == dir) {
            it = blockDirs.erase(it);
            removeItem = true;
        } else {
            it++;
        }
    }
    if (removeItem) {
        globalSettings.insert(BLOCK_DIRS_FOR_USER_KEY, blockDirs);
        m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
        save(m_settingsData);
    }
}

bool Config::isCompatibilityMode()
{
    return m_compatibilityMode;
}

void Config::save(const QJsonObject &jsonDocData)
{
    QDir dir;
    QString configFileDir(SEARCH_DIRS_SETTINGS_DIR);
    if (!dir.exists(configFileDir)) {
        if (!dir.mkdir(configFileDir)) {
            qWarning() << "Unable to create settings config file.";
            return;
        }
    }

    QFile settingsFile(SEARCH_DIRS_SETTINGS);
    if(!settingsFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Fail to open file " << SEARCH_DIRS_SETTINGS;
    }

    if (settingsFile.write(QJsonDocument(jsonDocData).toJson()) == -1) {
        qWarning() << "Error saving configuration file.";
    }
    settingsFile.flush();
    settingsFile.close();
}

void Config::initSettingsData(QJsonObject &data)
{
    //全局配置
    QJsonObject globalSettings;

    QJsonArray blackList;
    for(const QString& path : GLOBAL_BLACK_LIST) {
        blackList.append(path);
    }
    globalSettings.insert(GLOBAL_BLACK_LIST_KEY, blackList);

    data.insert(CONFIG_VERSION_KEY, CONFIG_VERSION);
    data.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
}

QStringList Config::searchDirs()
{
    QStringList tmp;
    bool needSave = false;
    QJsonObject searchDirData = m_settingsData.value(SEARCH_DIRS_GROUP).toObject();
    for(const QString& path : searchDirData.keys()) {
        if(QFile::exists(path)) {
            tmp.append(path);
        } else {
            searchDirData.remove(path);
            needSave = true;
        }
    }
    if(needSave) {
        m_settingsData.insert(SEARCH_DIRS_GROUP, searchDirData);
        save(m_settingsData);
    }
    return tmp;
}

QStringList Config::blackDirs()
{
    QStringList tmp;
    bool needSave = false;
    QJsonObject searchDir = m_settingsData.value(SEARCH_DIRS_GROUP).toObject();
    for(const QString& path : searchDir.keys()) {
        if(QFile::exists(path)) {
            tmp.append(searchDir.value(path).toVariant().toStringList());
        } else {
            searchDir.remove(path);
            needSave = true;
        }
    }
    if(needSave) {
        m_settingsData.insert(SEARCH_DIRS_GROUP, searchDir);
        save(m_settingsData);
    }
    return tmp;
}

QStringList Config::globalBlackList() const
{
    return m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject().value(GLOBAL_BLACK_LIST_KEY).toVariant().toStringList();
}

QStringList Config::blockDirsForUser()
{
    bool needSave;
    QStringList blockDirs;
    QJsonObject globalSettings = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject();
    for (const QString& dir : globalSettings.value(BLOCK_DIRS_FOR_USER_KEY).toVariant().toStringList()) {
        if (QFile::exists(dir)) {
            blockDirs << dir;
        } else {
            needSave = true;
        }
    }

    if (needSave) {
        QJsonArray array;
        for (const QString& dir : blockDirs) {
            array << dir;
        }
        globalSettings.insert(BLOCK_DIRS_FOR_USER_KEY, array);
        m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
        save(m_settingsData);
    }
    return blockDirs;
}

Config::Config()
{
    QJsonArray blockDirsForUser;
    bool oldBlockSettingsExists(false);
    if (QFile::exists(OLD_BLOCK_DIR_SETTINGS)) {
        oldBlockSettingsExists = true;
        QSettings oldBlockSettings(OLD_BLOCK_DIR_SETTINGS, QSettings::IniFormat);
        for(const QString& blockDir : oldBlockSettings.allKeys()) {
            QString wholePath = "/" + blockDir;
            if(QFile::exists(wholePath)) {
                blockDirsForUser.append(wholePath);
            }
        }
        QFile::remove(OLD_BLOCK_DIR_SETTINGS);
    }

    QSettings oldSettings(OLD_INDEXABLE_DIR_SETTINGS, QSettings::IniFormat);
    QFile settingsFile(SEARCH_DIRS_SETTINGS);

    if(!settingsFile.exists()) {
        m_compatibilityMode = true;
        qDebug() << oldSettings.value(INDEXABLE_DIR_KEY + "/" + INDEXABLE_DIR_KEY).toStringList();
        for(const QString& path : oldSettings.value(INDEXABLE_DIR_KEY + "/" + INDEXABLE_DIR_KEY).toStringList()) {
            if(QFile::exists(path)) {
                SearchDir dir(path, false);
                qDebug() << "Found old config path" << path;
                m_compatibleCache.append(dir);
            }
        }

        //老版本配置会在更新后被清空，如果allKeys为空说明已经运行过新版本，此时如果新版配置不存在则添加家目录作为默认搜索目录
        if(m_compatibleCache.isEmpty() && oldSettings.allKeys().isEmpty()) {
            SearchDir dir(HOME_PATH, false);
            m_compatibleCache.append(dir);
        }
        oldSettings.clear();
        oldSettings.sync();

        initSettingsData(m_settingsData);
        if (!blockDirsForUser.isEmpty()) {
            QJsonObject globalSettings = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject();
            globalSettings.insert(BLOCK_DIRS_FOR_USER_KEY, blockDirsForUser);
            m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
        }
        save(m_settingsData);
        return;
    }

    if (!settingsFile.open(QFile::ReadOnly)) {
        qWarning() << "SettingsManagerPrivate: configuration file " << settingsFile.fileName() << "open failed !";
        return;
    }
    QByteArray byteArray = settingsFile.readAll();
    settingsFile.close();
    QJsonParseError errRpt;
    QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray, &errRpt));
    if (errRpt.error != QJsonParseError::NoError) {
        qWarning() << "Incorrect configuration files. JSON parse error";
        initSettingsData(m_settingsData);
        save(m_settingsData);
        return;
    }
    // 读取配置文件
    QJsonObject settingsData = jsonDocument.object();
    QString version = settingsData.value(CONFIG_VERSION_KEY).toString();
    if (version != QString(CONFIG_VERSION)) {
        qWarning() << "Settings version check failed, old: " << version << " new:" << CONFIG_VERSION;
        //TODO:这里做一些兼容处理
    } else {
        m_settingsData.swap(settingsData);
        //旧的配置文件存在则需要进行同步旧配置
        if (oldBlockSettingsExists) {
            QJsonObject globalSettings = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject();
            globalSettings.insert(BLOCK_DIRS_FOR_USER_KEY, blockDirsForUser);
            m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
            save(m_settingsData);
        }
    }

    QStringList pathToRemove;
    for(const QString& path : m_settingsData.value(SEARCH_DIRS_GROUP).toObject().keys()) {
        if(!QFile::exists(path)) {
            pathToRemove.append(path);
        }
    }
    if(!pathToRemove.isEmpty()) {
        removeDir(pathToRemove);
    }
}

void Config::removeDir(const QStringList &paths)
{
    QJsonObject searchDirData = m_settingsData.value(SEARCH_DIRS_GROUP).toObject();
    for(const QString& path : paths) {
        searchDirData.remove(path);
    }
    m_settingsData.insert(SEARCH_DIRS_GROUP, searchDirData);
    save(m_settingsData);
}

void Config::processCompatibleCache()
{
    if(this->isCompatibilityMode() && !m_compatibleCache.isEmpty()) {
        QJsonObject searchDirsdata;
        for (SearchDir dir : m_compatibleCache) {
            dir.generateBlackList();
            if (dir.error() == SearchDir::ErrorInfo::Successful) {
                QJsonArray blackList;
                for(const QString& path : dir.getBlackList()) {
                    blackList.append(path);
                }
                searchDirsdata.insert(dir.getPath(), blackList);
                m_settingsData.insert(SEARCH_DIRS_GROUP, searchDirsdata);
            }
        }
        save(m_settingsData);
        m_compatibleCache.clear();
        m_compatibilityMode = false;
    }
}
