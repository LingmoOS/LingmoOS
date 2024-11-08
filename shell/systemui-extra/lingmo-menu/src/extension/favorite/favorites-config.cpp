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
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>
#include "favorites-config.h"
#include "favorite-folder-helper.h"

#define FOLDER_FILE_PATH ".config/lingmo-menu/"
#define FOLDER_FILE_NAME "favorite.json"

namespace LingmoUIMenu {

QString FavoritesConfig::s_favoritesConfigFile = QDir::homePath() + "/" + FOLDER_FILE_PATH + FOLDER_FILE_NAME;

FavoritesConfig &FavoritesConfig::instance()
{
    static FavoritesConfig favoritesConfig;
    return favoritesConfig;
}

FavoritesConfig::FavoritesConfig(QObject *parent)
{
    initConfig();
}

QString FavoritesConfig::getValue(const int &index) const
{
    if (index < 0 || index > m_favoritesList.count()) {
        return {};
    }

    return m_favoritesList.at(index);
}

void FavoritesConfig::insertValue(const QString &id, const int &index)
{
    if (m_favoritesList.indexOf(id) > -1) {
        return;
    }

    m_favoritesList.insert(index, id);
    sync();
}

void FavoritesConfig::removeValueById(const QString &id)
{
    int index = m_favoritesList.indexOf(id);

    if (index > -1 && index < m_favoritesList.count()) {
        m_favoritesList.removeAt(index);
    }

    sync();
}

int FavoritesConfig::getOrderById(const QString &id)
{
    return m_favoritesList.indexOf(id);
}

void FavoritesConfig::changeOrder(const int &indexFrom, const int &indexTo)
{
    m_favoritesList.move(indexFrom, indexTo);
    sync();
}

int FavoritesConfig::configSize() const
{
    return m_favoritesList.size();
}

void FavoritesConfig::sync()
{
    QDir dir;
    QString folderConfigDir(QDir::homePath() + "/" + FOLDER_FILE_PATH);
    if (!dir.exists(folderConfigDir)) {
        if (!dir.mkdir(folderConfigDir)) {
            qWarning() << "FavoritesConfig::Unable to create configuration file.";
            return;
        }
    }

    QFile file(s_favoritesConfigFile);
    file.open(QFile::WriteOnly);

    QJsonDocument jsonDocument;
    jsonDocument.setArray(QJsonArray::fromStringList(m_favoritesList));

    if (file.write(jsonDocument.toJson()) == -1) {
        qWarning() << "FavoritesConfig::Error saving configuration file.";
    }

    file.flush();
    file.close();

    Q_EMIT configChanged();
}

void FavoritesConfig::clear()
{
    m_favoritesList.clear();
    sync();
}

void FavoritesConfig::initConfig()
{
    QFile file(s_favoritesConfigFile);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "FavoritesConfig: Incorrect configuration files open failed.";
        return;
    }

    QJsonDocument jsonDocument(QJsonDocument::fromJson(file.readAll()));
    file.close();

    if (jsonDocument.isNull() || jsonDocument.isEmpty() || !jsonDocument.isArray()) {
        qWarning() << "FavoritesConfig: Incorrect configuration files are ignored.";
        return;
    }

    m_favoritesList.clear();
    QJsonArray array = jsonDocument.array();
    bool needSync = false;
    for (int i = 0; i < array.size(); i++) {
        if (array.at(i).isString()) {
            QString id = array.at(i).toString();
            // 配置文件出现错误, 某一个应用 同时出现在了 应用组的内部和外部
            if (id.startsWith(APP_ID_SCHEME) && FavoriteFolderHelper::instance()->containApp(id.mid(APP_ID_SCHEME.length()))) {
                needSync = true;
                continue;
            }
            m_favoritesList.append(id);
        } else {
            needSync = true;
        }
    }

    if (needSync) {
        sync();
    }
}

} // LingmoUIMenu
