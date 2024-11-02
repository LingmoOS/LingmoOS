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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "config-loader.h"
#include <QStandardPaths>
#include <QDir>
#include <QMap>
#include <QDebug>
#include <QVariant>
#include <QStringLiteral>

namespace LingmoUIQuick {

QMap<ConfigLoader::Domain, QMap<QString, Config*>> ConfigLoader::globalCache = QMap<ConfigLoader::Domain, QMap<QString, Config*>>();

Config *ConfigLoader::getConfig(const QString &id, ConfigLoader::Domain domain, const QString &appid)
{
    QString filename = getFullFileName(id, Json, domain, appid);
    if (filename.isEmpty()) {
        return nullptr;
    }

    const QString cacheKey = (domain == Local) ? (appid + QStringLiteral("_") + id) : id;
    auto &cache = ConfigLoader::globalCache[domain];
    if (cache.contains(cacheKey)) {
        return cache.value(cacheKey);
    }

    auto config = new Config(filename);
    cache.insert(cacheKey, config);

    return config;
}

QString ConfigLoader::getFullFileName(const QString &id, Type type, Domain domain, const QString &appid)
{
    const QString filename = ConfigLoader::getConfigFileName(id, type);
    if (filename.isEmpty()) {
        return {};
    }

    if (domain == Global) {
        return ConfigLoader::globalConfigPath() + filename;
    }

    if (appid.isEmpty()) {
        return {};
    }

    return ConfigLoader::localConfigPath() + appid + QStringLiteral("/") + filename;
}

QString ConfigLoader::globalConfigPath()
{
    return QDir::homePath() + QStringLiteral("/.config/org.lingmo/_lingmo-config-global/");
}

QString ConfigLoader::localConfigPath()
{
    return QDir::homePath() + QStringLiteral("/.config/org.lingmo/");
}

QString ConfigLoader::getConfigFileName(const QString &id, ConfigLoader::Type type)
{
    if (id.isEmpty()) {
        return {};
    }

    if (type == Json) {
        return id + QStringLiteral(".json");
    }

    return id + QStringLiteral(".conf");
}

} // LingmoUIQuick
