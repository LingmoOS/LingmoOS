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

#ifndef LINGMO_QUICK_CONFIG_LOADER_H
#define LINGMO_QUICK_CONFIG_LOADER_H

#include <QObject>
#include "config.h"

namespace LingmoUIQuick {

/**
 * 获取从文件系统加载配置文件
 * 此处负责处理文件的加载路径
 * 根据Domain进行映射：domain {
 *     Local: ~/.config/org.lingmo/appid/id.json
 *     Global: ~/.config/org.lingmo/_lingmo-config-global/id.json
 * }
 */
class ConfigLoader
{
public:
    enum Domain {
        Local = 0,
        Global
    };

    enum Type {
        Json = 0,
        Ini
    };

    /**
     * 加载配置文件
     * @param id 配置文件ID,对应id.json
     * @param app 应用名称
     * @param domain {
     *     Local: ~/.config/org.lingmo/appid/id.json
     *     Global: ~/.config/org.lingmo/_lingmo-config-global/id.json
     * }
     * @return config
     */
    static Config *getConfig(const QString &id, Domain domain = Global, const QString &appid = QString());
    static QString getFullFileName(const QString &id, Type type = Json, Domain domain = Global, const QString &appid = QString());

    static QString globalConfigPath();
    static QString localConfigPath();
    static QString getConfigFileName(const QString &id, Type type = Json);

private:
    static QMap<Domain, QMap<QString, Config*>> globalCache;
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_CONFIG_LOADER_H
