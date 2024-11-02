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
#include "lingmo_volume_control_user_config.h"
#include <filesystem>

LingmoUIVolumeControlUserConfig::LingmoUIVolumeControlUserConfig()
{
    initJsonMap();
}

LingmoUIVolumeControlUserConfig::~LingmoUIVolumeControlUserConfig()
{

}

std::unordered_map<JsonType, std::shared_ptr<IJson>> LingmoUIVolumeControlUserConfig::getJsonMap() const
{
    return m_jsonMap;
}

void LingmoUIVolumeControlUserConfig::initJsonMap()
{
    for (const auto& [k, v] : m_keys) {
        std::shared_ptr<IJson> json = nullptr;

        switch (k) {
        case JsonType::JSON_TYPE_SOUNDEFFECT: {
            json = std::make_shared<SoundEffectJson>(v());
            break;
        }
        case JsonType::JSON_TYPE_USERINFO: {
            json = std::make_shared<UserInfoJson>(v());
            json->init();
            break;
        }
        default:
            break;
        }

        if (json) {
            m_jsonMap.emplace(k, json);
        }
    }
}

QString LingmoUIVolumeControlUserConfig::getSoundEffectFullPath()
{
    return SOUND_EFFECT_JSON;
}

QString LingmoUIVolumeControlUserConfig::getUserInfoFullPath()
{
    QString path = getenv("HOME");
    path.append(AUDIO_JSON);
    syslog(LOG_DEBUG, "usr info file path: %s.", path.toLatin1().data());
    return path;
}

