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
#ifndef UKMEDIACOMMON_H
#define UKMEDIACOMMON_H

#include <QDBusArgument>
#include <pulse/introspect.h>
#include <libkycpu.h>
#include <lingmosdk/lingmosdk-system/libkysysinfo.h>

#define UKMedia_Equal(a,b) (strcmp((a),(b)) == 0)

#define SETTINGSWIDGET_WIDTH  760
#define SETTINGSWIDGET_HEIGHT 520
#define PA_VOLUME_NORMAL      65536.0
#define PA_VOLUME_MIN         0.0
#define UKMEDIA_VOLUME_NORMAL 100.0
#define UKMEDIA_VOLUME_MIN    0.0

//  Init Port Volume
#define MIC_VOLUME       100
#define HDMI_VOLUME      100
#define OUTPUT_VOLUME    67
#define HEADPHONE_VOLUME 17

#define HDMI      "hdmi"
#define BLUEZ     "bluez"
#define SPEAKER   "speaker"
#define HEADPHONE "headphone"

#define HUAWEI_BTO_SINK   "bt_sco_sink"
#define HUAWEI_BTO_SOURCE "bt_sco_source"
#define HEADSET_HEAD_UNIT "headset_head_unit"

#define PA_ID_INVALID  ((uint32_t)0xffffffff)
#define PANGUW_SINK "alsa_output.platform-raoliang-sndcard.analog-stereo"
#define HISTEN_SINK "histen_sink"
#define NOISE_REDUCE_SOURCE "noiseReduceSource"

//  LINGMO Audio
#define KEY_SOUNDS_SCHEMA      "org.lingmo.sound"
#define ALERT_VOLUME           "alert-volume"
#define AUDIO_VOLUME_CHANGE    "audio-volume-change"
#define SOUND_CUSTOM_THEME_KEY "custom-theme"
#define DNS_NOISE_REDUCTION    "dns-noise-reduction"
#define EVENT_SOUNDS_KEY       "event-sounds"
#define LOOP_BACK              "loopback"
#define MONO_AUDIO             "mono-audio"
#define NOTIFICATION_GENGERAL  "notification-general"
#define SOUND_THEME_KEY        "theme-name"
#define VOLUME_INCREASE        "volume-increase"
#define VOLUME_INCREASE_VALUE  "volume-increase-value"
#define SOUND_FILE_PATH        "/usr/share/sounds/xxxTheme/stereo/xxxFile.ogg"

//  LINGMO Session
#define LINGMO_SWITCH_SETTING     "org.lingmo.session"
#define LINGMO_STARTUP_MUSIC_KEY  "startup-music"
#define LINGMO_POWEROFF_MUSIC_KEY "poweroff-music"
#define LINGMO_LOGOUT_MUSIC_KEY   "logout-music"
#define LINGMO_WAKEUP_MUSIC_KEY   "weakup-music"

//  LINGMO Theme
#define LINGMO_THEME_SETTING "org.lingmo.style"
#define LINGMO_THEME_NAME    "style-name"
#define LINGMO_THEME_LIGHT   "lingmo-light"
#define LINGMO_THEME_DARK    "lingmo-dark"
#define LINGMO_THEME_DEFAULT "lingmo-default"

//  LINGMO Global Theme
#define LINGMO_GLOBALTHEME_SETTINGS "org.lingmo.globaltheme.settings"
#define GLOBAL_THEME_NAME         "global-theme-name"

#define SYSTEM_VOLUME_CTRL                 "lingmo-settings-system"
#define LINGMO_MEDIA_PATH                    "/org/lingmo/media"
#define LINGMO_MEDIA_CONTROL_PATH            "/org/lingmo/media/control"
#define LINGMO_MEDIA_SERVICE                 "org.lingmo.media"
#define LINGMO_MEDIA_INTERFACE               "org.lingmo.media"
#define PULSEAUDIO_DEVICECONTROL_PATH      "/org/pulseaudio/device_control"
#define PULSEAUDIO_DEVICECONTROL_SERVICE   "org.PulseAudio.DeviceControl"
#define PULSEAUDIO_DEVICECONTROL_INTERFACE "org.PulseAudio.DeviceControl"

//设备插入状态
typedef enum pa_device_plugged_stauts
{
    PORT_STAUTS_UNKNOWN   = 0,
    PORT_STAUTS_PLUGGED   = 1,
    PORT_STAUTS_UNPLUGGED = 2,

} pa_device_plugged_stauts;


enum SoundType {
    SINK,
    SOURCE,
};

struct portInfo
{
    QString name;                   /**< Name of this port */
    QString description;            /**< Description of this port */
    uint32_t priority;              /**< The higher this value is, the more useful this port is as a default. */
    int available;                  /**< A flags (see #pa_port_available), indicating availability status of this port. \since 2.0 */
};

struct sinkInfo
{
    QString name;                      /**< Name of the sink */
    uint32_t index;                    /**< Index of the sink */
    QString description;               /**< Description of this sink */
    pa_cvolume volume;                 /**< Volume of the sink */
    int mute;                          /**< Mute switch of the sink */
    uint32_t card;                     /**< Card index, or PA_INVALID_INDEX. \since 0.9.15 */
    QString active_port_name = "";
    QString active_port_description = "";
    QString master_device = "";
    QList<portInfo> sink_port_list;
};

struct sourceInfo
{
    QString name;                      /**< Name of the source */
    uint32_t index;                    /**< Index of the source */
    QString description;               /**< Description of this source */
    pa_cvolume volume;                 /**< Volume of the source */
    int mute;                          /**< Mute switch of the source */
    uint32_t card;                     /**< Card index, or PA_INVALID_INDEX. \since 0.9.15 */
    QString active_port_name = "";
    QString active_port_description = "";
    QString master_device = "";
    QList<portInfo> source_port_list;
};

struct pa_device_port_info
{
    /*card index*/
    int card;

    /* port direction. Input or Output */
    int direction;

    /* port available. */
    int available;

    /* port plugged stauts. */
    int plugged_stauts;

    /* port name. Example: analog-output-speak */
    QString name;

    /* port description */
    QString description;

    /* device description */
    QString device_description;

    /* device product name */
    QString device_product_name;

    friend const QDBusArgument& operator>>(const QDBusArgument &argument, pa_device_port_info &info) {
        argument.beginStructure();
        argument >> info.card;
        argument >> info.direction;
        argument >> info.available;
        argument >> info.plugged_stauts;
        argument >> info.name;
        argument >> info.description;
        argument >> info.device_description;
        argument >> info.device_product_name;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator<<(QDBusArgument &argument, const pa_device_port_info &info) {
        argument.beginStructure();
        argument << info.card;
        argument << info.direction;
        argument << info.available;
        argument << info.plugged_stauts;
        argument << info.name;
        argument << info.description;
        argument << info.device_description;
        argument << info.device_product_name;
        argument.endStructure();
        return argument;
    }
};

typedef pa_device_port_info pa_device_port_info;
Q_DECLARE_METATYPE(pa_device_port_info)

/* some information of sink-input or source-output */
struct appInfo
{
    int index;

    int volume;

    int channel;

    bool mute;

    int direction;

#ifdef PA_PROP_APPLICATION_MOVE
    QString move;
#endif

    /* name eg: lingmo-music*/
    QString name;

    /* sink-input's sink index*/
    int masterIndex;

    /* sink-input's sink name*/
    QString masterDevice;

    friend QDBusArgument &operator<<(QDBusArgument &argument, const appInfo &mystruct)
    {
        argument.beginStructure();
        argument << mystruct.index
                 << mystruct.volume
                 << mystruct.channel
                 << mystruct.mute
                 << mystruct.direction
#ifdef PA_PROP_APPLICATION_MOVE
                 << mystruct.move
#endif
                 << mystruct.name
                 << mystruct.masterIndex
                 << mystruct.masterDevice;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, appInfo &mystruct)
    {
        argument.beginStructure();
        argument >> mystruct.index
                 >> mystruct.volume
                 >> mystruct.channel
                 >> mystruct.mute
                 >> mystruct.direction
#ifdef PA_PROP_APPLICATION_MOVE
                >> mystruct.move
#endif
                 >> mystruct.name
                 >> mystruct.masterIndex
                 >> mystruct.masterDevice;
        argument.endStructure();
        return argument;
    }
};

typedef appInfo appInfo;
Q_DECLARE_METATYPE(appInfo)


enum class SystemVersion {
    SYSTEM_VERSION_LINGMO,     // 主线版本
    SYSTEM_VERSION_OKYANGTZE, // lingmo yangtze版本
    SYSTEM_VERSION_OKNILE,    // lingmo nile版本
    SYSTEM_VERSION_UNKNOWN
};

class UkmediaCommon
{
public:
    static UkmediaCommon& getInstance();

    /**
    * @brief  获取系统版本信息
    *
    * @return 返回当前系统版本信息
    */
    SystemVersion getSystemVersion();

    /**
    * @brief  判断该机器是不是华为平台
    *
    * @return 是返回true,否则返回false
    */
    bool isHuaweiPlatform();

    /**
    * @brief  判断当前系统语言环境变量是不是哈维柯
    *
    * @param  str 语言环境变量 维吾尔语:ug_CN 哈萨克语:kk_KZ 柯尔克孜语:ky_KG
    *
    * @return 是返回true,否则返回false
    */
    bool isHWKLanguageEnv(const QString& str);

private:
    UkmediaCommon();
    UkmediaCommon(const UkmediaCommon&) = delete;
    UkmediaCommon(UkmediaCommon&&) = delete;
    UkmediaCommon operator=(const UkmediaCommon&) = delete;
    UkmediaCommon operator=(UkmediaCommon&&) = delete;
    ~UkmediaCommon() = default;
};

#endif // UKMEDIACOMMON_H
