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
#include <QString>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <syslog.h>

#include "lingmo_login_sound_user_config.h"
#include "lingmo_login_sound.h"

/* define hw sound card dirvers */
#define HI3XXX_HI6405         "hi3xxx_hi6405"
#define HI3XXX_DA_COMBINE_V5  "hi3xxx_DA_combine_v5"
#define DA_COMBINE_V5_MACHINE "da_combine_v5_machine"
#define HW_AUDIO_SERVICE      "/usr/share/hw-audio/hwaudioservice"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (QFile(SOUND_EFFECT_JSON).exists())
    {
        syslog(LOG_INFO, "this is not first time longin to the system, so the stratup sound effect will not play.");
        return 0;
    }

    std::shared_ptr<UserConfig> info = std::make_shared<LingmoUILoginSoundUserConfig>();

    QString autoLoginUser = std::dynamic_pointer_cast<LingmoUILoginSoundUserConfig>(info)->getAutoLoginUser();
    QString lastLoginUser = std::dynamic_pointer_cast<LingmoUILoginSoundUserConfig>(info)->getLastLoginUser();

    syslog(LOG_DEBUG, "autoLoginUser:%s, lastLoginUser:%s", autoLoginUser.toLatin1().data(), lastLoginUser.toLatin1().data());
    // 1. 正常装机或者OEM装机时第一次登录，使用默认设备播放开机音效
    if ("oem" == autoLoginUser || ("" == autoLoginUser && "" == lastLoginUser))
    {
        if (QFile::exists(HW_AUDIO_SERVICE)) {
            system("/usr/share/hw-audio/hwaudioservice -o");
        }

        syslog(LOG_INFO, "when you first enter the system, use the default output device to play startup sound effect.");
        QString defaultOutputDev = DEFAULT_AUDIO_CARD_VALUE;
        defaultOutputDev = LingmoUILoginSound::getInstance().checkPcm(defaultOutputDev.toLatin1().data());

        LingmoUILoginSound::getInstance().aplayMain(PLAY_STARTUP_WAV, defaultOutputDev.toLatin1().data());
    }
    else
    {
        // 2.当系统设置自动登录时，使用自动登录用户记录的设备信息进行播放，没有设置自动登录用户时，使用上一次登录用户记录的设备信息进行播放
        bool status = info->getValueFromJson(info->getJsonByType(std::dynamic_pointer_cast<LingmoUILoginSoundUserConfig>(info)->getJsonMap(),
                                             JsonType::JSON_TYPE_USERINFO), AUDIO_STRATUP_SOUNDEFFECT_KEY).toBool();
        bool mute = info->getValueFromJson(info->getJsonByType(std::dynamic_pointer_cast<LingmoUILoginSoundUserConfig>(info)->getJsonMap(),
                                           JsonType::JSON_TYPE_USERINFO), AUDIO_MUTE_KEY).toBool();
        int volume = info->getValueFromJson(info->getJsonByType(std::dynamic_pointer_cast<LingmoUILoginSoundUserConfig>(info)->getJsonMap(),
                                            JsonType::JSON_TYPE_USERINFO), AUDIO_VOLUME_KEY).toInt();

        /* Do not play the boot sound effect when the user is closed when the boot sound effect,
         * the mute state, and the volume value are 0 */
        if (!status || mute || !volume) {
            syslog(LOG_DEBUG, "%s user has turned off the system boot sound or it is in mute mode, status: %d, muted: %d, volume: %d.",
                   autoLoginUser == "" ? lastLoginUser.toLatin1().data() : autoLoginUser.toLatin1().data(), status, mute, volume);
            return 0;
        }

        int cardId = info->getValueFromJson(info->getJsonByType(std::dynamic_pointer_cast<LingmoUILoginSoundUserConfig>(info)->getJsonMap(),
                                            JsonType::JSON_TYPE_USERINFO), AUDIO_CARDID_KEY).toInt();
        QString audioDEV = info->getValueFromJson(info->getJsonByType(std::dynamic_pointer_cast<LingmoUILoginSoundUserConfig>(info)->getJsonMap(),
                                                  JsonType::JSON_TYPE_USERINFO), AUDIO_CARD_KEY).toString();

        /* Check whether the PCM device is valid before playing the boot sound */
        audioDEV = LingmoUILoginSound::getInstance().checkPcm(audioDEV.toLatin1().data());
        bool support = LingmoUILoginSound::getInstance().isSupportHardwareVolumeControl(QString("hw:%1").arg(cardId).toLatin1().data());
        syslog(LOG_INFO, "play system sound effects on the device:%s at volume:%d.", audioDEV.toLatin1().data(), volume);

        if (!mute && QFile::exists(HW_AUDIO_SERVICE)) {
            system("/usr/share/hw-audio/hwaudioservice -o");
        }

        /* When the driver does not support hardware volume control, modify the volume of
         * the audio file of the boot sound effect through FFMPEG */
        if (!support && !LingmoUILoginSound::getInstance().wav_convert(volume)) {
            LingmoUILoginSound::getInstance().aplayMain(TMP_STARTUP_WAV_PATH, audioDEV.toLatin1().data());
        }
        else {
            LingmoUILoginSound::getInstance().aplayMain(PLAY_STARTUP_WAV, audioDEV.toLatin1().data());
        }
    }

    return 0;
}
