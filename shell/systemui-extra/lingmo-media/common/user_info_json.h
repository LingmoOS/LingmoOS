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
#ifndef USERINFOJSON_H
#define USERINFOJSON_H

#include "json.h"

#define INVAILD_PATH                      "invaild path"
#define DEFAULT_PATH                      "/home/"
#define AUDIO_JSON                        "/.config/audio.json"        /* 保存用户默认的输出设备、音量、静音状态及开机音效开关的配置 */
#define AUDIO_CARD_KEY                    "card"                       /* 声卡字符串 */
#define DEFAULT_AUDIO_CARD_VALUE          "default"                    /* 系统默认的音频设备 */
#define AUDIO_CARDID_KEY                  "cardid"                     /* 声卡ID */
#define DEFAULT_AUDIO_CARDID_VALUE        0                            /* 默认声卡ID为0 */
#define AUDIO_MUTE_KEY                    "mute"                       /* 静音状态 */
#define DEFAULT_AUDIO_MUTE_VALUE          false                        /* 默认静音状态为false */
#define AUDIO_VOLUME_KEY                  "volume"                     /* 当前用户的音量值 */
#define DEFAULT_AUDIO_VOLUME_VALUE        67                           /* 默认音量为67 */
#define AUDIO_STRATUP_SOUNDEFFECT_KEY     "startup-soundeffect"        /* 当前用户开机音效的开启状态 */
#define DEFAULT_STARTUP_SOUNDEFFECI_VALUE true                         /* 默认开启状态为true */

class UserInfoJson : public IJson
{
public:
    UserInfoJson(const QString& name, const JsonType& type = JsonType::JSON_TYPE_USERINFO);
    virtual ~UserInfoJson();

public:
    virtual void init() override;
    virtual void remove(const QString& key) override;
    virtual void insert(const QString& key, const QJsonValue& value) override;
    virtual QJsonValue getValue(QString key) const override;

private:
    QMap<QString, QJsonValue> m_keys {
        {AUDIO_CARD_KEY, DEFAULT_AUDIO_CARD_VALUE},
        {AUDIO_CARDID_KEY, DEFAULT_AUDIO_CARDID_VALUE},
        {AUDIO_MUTE_KEY, DEFAULT_AUDIO_MUTE_VALUE},
        {AUDIO_VOLUME_KEY, DEFAULT_AUDIO_VOLUME_VALUE},
        {AUDIO_STRATUP_SOUNDEFFECT_KEY, DEFAULT_STARTUP_SOUNDEFFECI_VALUE},
    };
};

#endif // USERINFOJSON_H
