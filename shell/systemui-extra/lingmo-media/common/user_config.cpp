/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "user_config.h"

UserConfig::UserConfig()
{

}

UserConfig::~UserConfig()
{

}

void UserConfig::insert(std::shared_ptr<IJson> json, const QString &key, const QJsonValue& vaule)
{
    json->insert(key, vaule);
}

QJsonValue UserConfig::getValueFromJson(std::shared_ptr<IJson> json, const QString &key) const
{
    return json->getValue(key);
}

std::shared_ptr<IJson> UserConfig::getJsonByType(std::unordered_map<JsonType, std::shared_ptr<IJson>> map, const JsonType& type)
{
    if (const auto& itr = map.find(type); itr != map.end() && itr->second != nullptr) {
        return itr->second;
    }

    syslog(LOG_ERR, "JsonType error, jsonMap dosnot exit %d json type.", int(type));
    return nullptr;
}

std::function<QString()> UserConfig::getJsonFileNameByType(std::unordered_map<JsonType, std::function<QString()>> keys, const JsonType& type)
{
    if (const auto& itr = keys.find(type); itr != keys.end() && itr->second != nullptr) {
        return itr->second;
    }

    syslog(LOG_ERR, "JsonType error, keys dosnot exit %d json type.", int(type));
    return nullptr;
}
