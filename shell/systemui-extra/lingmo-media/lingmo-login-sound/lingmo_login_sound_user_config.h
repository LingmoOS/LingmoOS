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
#ifndef LINGMOLOGINSOUNDUSERCONFIG_H
#define LINGMOLOGINSOUNDUSERCONFIG_H

#include <QFile>
#include <qbytearray.h>
#include <qstring.h>
#include <QSettings>
#include <QObject>
#include <memory>
#include <syslog.h>

#include "../common/user_config.h"
#include "../common/user_info_json.h"
#include "../common/sound_effect_json.h"

class LingmoUILoginSoundUserConfig : public UserConfig
{
public:
    LingmoUILoginSoundUserConfig();
    ~LingmoUILoginSoundUserConfig();

public:
    const QString getAutoLoginUser() const;
    const QString getLastLoginUser() const;
    std::unordered_map<JsonType, std::shared_ptr<IJson>> getJsonMap() const;
    virtual void initJsonMap() override;

private:
    void initSettings();
    QString getSoundEffectFullPath();
    QString getUserInfoFullPath();

private:
    std::unordered_map<JsonType, std::function<QString()>> m_keys {
        {JsonType::JSON_TYPE_SOUNDEFFECT, [this](){ return getSoundEffectFullPath();}},
        {JsonType::JSON_TYPE_USERINFO, [this](){ return getUserInfoFullPath(); }},
    };

    std::unique_ptr<QSettings> m_pLastLoginSetting;
    std::unique_ptr<QSettings> m_pAutoLoginSetting;
};

#endif // LINGMOLOGINSOUNDUSERCONFIG_H
