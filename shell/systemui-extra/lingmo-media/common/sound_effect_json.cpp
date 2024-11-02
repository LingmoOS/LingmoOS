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
#include "sound_effect_json.h"
#include <syslog.h>

SoundEffectJson::SoundEffectJson(const QString& name, const JsonType& type) : IJson(name, type)
{
}

SoundEffectJson::~SoundEffectJson()
{
}

void SoundEffectJson::init()
{
    for (const auto& it : m_keys.toStdMap()) {
        insert(it.first, it.second);
    }
    update();
}

void SoundEffectJson::remove(const QString& key)
{
    if (m_rootObj.contains(key)) {
        m_rootObj.remove(key);
    }
    update();
}

void SoundEffectJson::insert(const QString& key, const QJsonValue& value)
{
    // 如果json文件中存在相同的键值对则不进行任何操作
    if (m_rootObj.contains(key) && m_rootObj.value(key) == value) {
        return;
    }

    m_rootObj.insert(key, value);
    update();
}

QJsonValue SoundEffectJson::getValue(QString key) const
{
    if (!m_rootObj.contains(key)) {
        syslog(LOG_INFO, "the json file does not contain the %s key", key.toLatin1().data());
        return QJsonValue();
    }

    return m_rootObj.value(key);
}
