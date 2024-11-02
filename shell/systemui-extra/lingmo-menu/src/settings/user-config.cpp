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

#include "user-config.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMutexLocker>

#define PRE_INSTALLED_APPS_KEY "PreInstalledApps"

namespace LingmoUIMenu {

const QString UserConfig::configFilePath = QDir::homePath() + "/.config/lingmo-menu/";
const QString UserConfig::configFileName = "config.json";

UserConfig *UserConfig::instance()
{
    static UserConfig userConfig;
    return &userConfig;
}

UserConfig::UserConfig(QObject *parent) : QObject(parent)
{
    init();
}

bool UserConfig::isFirstStartUp() const
{
    return m_isFirstStartUp;
}

void UserConfig::init()
{
    QFile configFile(configFilePath + configFileName);
    if ((m_isFirstStartUp = !configFile.exists())) {
        if (!QDir(configFilePath).exists() && !QDir().mkdir(configFilePath)) {
            qWarning() << "Could not create the user profile directory";
            return;
        }

        initConfigFile();
        return;
    }

    // read
    readConfigFile();
}

void UserConfig::initConfigFile()
{
    QFile configFile(configFilePath + configFileName);
    configFile.open(QFile::WriteOnly);
    configFile.close();

    // 已安装应用
    QStringList defaultPaths;
    defaultPaths << "/usr/share/applications/";
    defaultPaths << QDir::homePath() + "/.local/share/applications/";

    for (const auto &path : defaultPaths) {
        QDir dir(path);
        QStringList desktopFiles = dir.entryList(QStringList() << "*.desktop", QDir::Files);
        for (const auto &desktopFile : desktopFiles) {
            m_preInstalledApps.insert(path + desktopFile);
        }
    }

    sync();
}

void UserConfig::sync()
{
    writeConfigFile();
}

QSet<QString> UserConfig::preInstalledApps() const
{
    return m_preInstalledApps;
}

void UserConfig::addPreInstalledApp(const QString &appid)
{
    QMutexLocker mutexLocker(&m_mutex);
    m_preInstalledApps.insert(appid);
}

bool UserConfig::isPreInstalledApps(const QString &appid) const
{
    return m_preInstalledApps.contains(appid);
}

void UserConfig::removePreInstalledApp(const QString &appid)
{
    {
        QMutexLocker mutexLocker(&m_mutex);
        m_preInstalledApps.remove(appid);
    }
    sync();
}

void UserConfig::readConfigFile()
{
    {
        QMutexLocker locker(&m_mutex);
        m_preInstalledApps.clear();
    }

    QFile file(configFilePath + configFileName);
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    QByteArray byteArray = file.readAll();
    file.close();

    QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray));
    if (jsonDocument.isNull() || jsonDocument.isEmpty() || !jsonDocument.isArray()) {
        qWarning() << "UserConfig: Incorrect configuration files are ignored.";
        return;
    }

    QJsonArray jsonArray = jsonDocument.array();
    for (const auto &value : jsonArray) {
        QJsonObject object = value.toObject();
        // 预装app
        if (object.contains(PRE_INSTALLED_APPS_KEY)) {
            QJsonArray apps = object.value(QLatin1String(PRE_INSTALLED_APPS_KEY)).toArray();
            for (const auto &app : apps) {
                m_preInstalledApps.insert(app.toString());
            }
        }
    }
}

void UserConfig::writeConfigFile()
{
    QFile file(configFilePath + configFileName);
    if (!file.open(QFile::WriteOnly)) {
        return;
    }

    QJsonDocument jsonDocument;
    QJsonArray configArray;

    {
        // preInstalledApps
        QMutexLocker locker(&m_mutex);
        QJsonObject preInstalledAppsObject;

        QJsonArray apps;
        for (const auto &appid : m_preInstalledApps) {
            apps.append(appid);
        }

        preInstalledAppsObject.insert(PRE_INSTALLED_APPS_KEY, apps);
        configArray.append(preInstalledAppsObject);
    }

    jsonDocument.setArray(configArray);

    if (file.write(jsonDocument.toJson()) == -1) {
        qWarning() << "Error saving configuration file.";
    }
    file.flush();
    file.close();
}

} // LingmoUIMenu
