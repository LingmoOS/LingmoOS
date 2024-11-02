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
#ifndef SOUND_THEME_PLAYER_H
#define SOUND_THEME_PLAYER_H

#include <QMap>
#include <QFile>
#include <QDebug>
#include <canberra.h>
#include <QGSettings>
#include <QDomDocument>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <QFileInfo>
#include <pulse/pulseaudio.h>

extern "C" {
#include <dconf/dconf.h>
#include <canberra.h>
}

#define KEY_SOUNDS_SCHEMA   "org.lingmo.sound"
#define EVENT_SOUNDS_KEY "event-sounds"
#define SOUND_THEME_KEY "theme-name"
#define SOUND_CUSTOM_THEME_KEY "custom-theme"
#define AUDIO_VOLUME_CHANGE "audio-volume-change"
#define NOTIFICATION_GENERAL "notification-general"
#define ALERT_VOLUME "alert-volume"
#define SOUND_FILE_PATH "/usr/share/sounds/xxxTheme/stereo/xxxFile.ogg"

class SoundThemePlayer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","org.lingmo.sound.theme.player")
public:
    SoundThemePlayer();
    ~SoundThemePlayer();
    bool createCaContext();
    void UkmediaPlayerDbusRegister();
    QString readJson(QString action);
    void addSoundFileList();
    bool playCustomAlertSound(QString soundPath);
    bool isFileExist(QString theme, QString soundTye);
    void initGSettings();
    void onSettingChanged(const QString &key);
    char *alertVolumeTodB(int value);

public Q_SLOTS: //METHODS
    bool playAlertSound(QString soundEvent);

private:
    ca_context *m_caContext = nullptr;
    QGSettings *m_pSoundSettings = nullptr;
    QStringList m_pSoundFileList;
    char *m_dBValue;
};

#endif // SOUND_THEME_PLAYER_H
