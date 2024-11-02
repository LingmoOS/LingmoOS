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
#ifndef SOUNDEFFECTJSON_H
#define SOUNDEFFECTJSON_H

#include "json.h"

#define SOUND_EFFECT_JSON              "/tmp/PlayStartupWav.json"   /* 判断是否为首次登录系统 */
#define FIRST_TIME_LOGIN_KEY           "first-time-login"           /* 首次登录系统 */
#define DEFAULT_FIRST_TIME_LOGIN_VALUE true                         /* 默认为首次登录系统 */

class SoundEffectJson : public IJson
{
public:
    SoundEffectJson(const QString& name, const JsonType& type = JsonType::JSON_TYPE_SOUNDEFFECT);
    virtual ~SoundEffectJson();

public:
    virtual void init() override;
    virtual void remove(const QString& key) override;
    virtual void insert(const QString& key, const QJsonValue& value) override;
    virtual QJsonValue getValue(QString key) const override;

private:
    QMap<QString, QJsonValue> m_keys {
        {FIRST_TIME_LOGIN_KEY, DEFAULT_FIRST_TIME_LOGIN_VALUE},
    };

};

#endif // SOUNDEFFECTJSON_H
