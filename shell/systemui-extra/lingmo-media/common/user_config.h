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
#ifndef USERCONFIG_H
#define USERCONFIG_H

#include <memory>
#include <unordered_map>

#include "json.h"

#define LIGHTDM_CONF_FILE      "/etc/lightdm/lightdm.conf"
#define LIGHTDM_CONF_KEY       "SeatDefaults/autologin-user"
#define LINGMO_GREETER_CONF_FILE "/var/lib/lightdm/.cache/lingmo-greeter.conf"
#define LINGMO_GREETER_CONF_KEY  "Greeter/lastLoginUser"

class UserConfig
{
public:
    UserConfig();
    ~UserConfig();

public:
    virtual void initJsonMap() = 0;
    virtual void insert(std::shared_ptr<IJson> json, const QString& key, const QJsonValue& vaule);
    QJsonValue getValueFromJson(std::shared_ptr<IJson> json, const QString& key) const;
    std::shared_ptr<IJson> getJsonByType(std::unordered_map<JsonType, std::shared_ptr<IJson>> map, const JsonType& type);
    std::function<QString()> getJsonFileNameByType(std::unordered_map<JsonType, std::function<QString()>> keys, const JsonType& type);

protected:
    std::unordered_map<JsonType, std::shared_ptr<IJson>> m_jsonMap;
};

#endif // USERCONFIG_H
