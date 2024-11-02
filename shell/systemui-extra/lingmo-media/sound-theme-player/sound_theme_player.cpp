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
#include "sound_theme_player.h"

SoundThemePlayer::SoundThemePlayer()
{
    createCaContext();//注册
    UkmediaPlayerDbusRegister();
    initGSettings();
    addSoundFileList();
}

/**
 * @brief create for the libcanberra context
 * @return create successfully or not
 */
bool SoundThemePlayer::createCaContext()
{
    if (nullptr != m_caContext)
    {
        ca_context_cancel(m_caContext, 0);
        ca_context_destroy(m_caContext);
    }

    return (ca_context_create(&m_caContext) == 0) ? true : false;
}

/**
 * @brief Register for the dbus service
 */
void SoundThemePlayer::UkmediaPlayerDbusRegister()
{
    QDBusConnection con = QDBusConnection::sessionBus();

    if (!con.registerService("org.lingmo.sound.theme.player"))
        qDebug() << "registerService failed" << con.lastError().message();

    if (!con.registerObject("/org/lingmo/sound/theme/player", this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals))
        qDebug() << "registerObject failed" << con.lastError().message();
}


/**
 * @brief Play the corresponding sound effects
 *
 * @param soundEvent Application sound type
 * @return Play successfully or not
 */
bool SoundThemePlayer::playAlertSound(QString soundEvent)
{
    bool soundSwitch;

    if (m_pSoundSettings->keys().contains("eventSounds"))
        soundSwitch = m_pSoundSettings->get(EVENT_SOUNDS_KEY).toBool();

    if (!soundSwitch)
        return false;

    gint retval;
    QByteArray ba;
    const gchar *eventId;
    const gchar *desc = "Sound Theme Player";

    if (m_pSoundFileList.contains(soundEvent))
        ba = soundEvent.toLatin1();
    else {
        QString theme = m_pSoundSettings->get(SOUND_THEME_KEY).toString();
        if (isFileExist(theme, soundEvent))
            ba = soundEvent.toLatin1();
        else
            ba = NOTIFICATION_GENERAL;
    }

    eventId = ba.data();

    if (ba == AUDIO_VOLUME_CHANGE && m_pSoundSettings->get(SOUND_CUSTOM_THEME_KEY).toBool()) {
        QString soundPath = m_pSoundSettings->get(AUDIO_VOLUME_CHANGE).toString();
        retval = playCustomAlertSound(soundPath);
        return retval;
    }
    else if (ba == NOTIFICATION_GENERAL && m_pSoundSettings->get(SOUND_CUSTOM_THEME_KEY).toBool()) {
        QString soundPath = m_pSoundSettings->get(NOTIFICATION_GENERAL).toString();
        retval = playCustomAlertSound(soundPath);
        return retval;
    }

    if (createCaContext()) {
        retval = ca_context_play(m_caContext, 0,
                                 CA_PROP_EVENT_ID, eventId,
                                 CA_PROP_EVENT_DESCRIPTION, desc,
                                 CA_PROP_CANBERRA_VOLUME, m_dBValue,
                                 NULL);
    }
    qDebug() << __func__ << soundEvent << eventId << retval;

    return (retval == 0 ? 1 : 0);
}


/**
 * @brief Find the sound file according to the sound type
 *
 * @param action Application sound type
 * @return Sound file
 */
QString SoundThemePlayer::readJson(QString action)
{
    QFile file("/usr/share/lingmo-media/sounds/audio.json");

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "can't open error!";
        return "";
    }

    // 读取文件的全部内容
    QTextStream stream(&file);
    stream.setCodec("UTF-8");		// 设置读取编码是UTF8
    QString str = stream.readAll();

    file.close();

    // QJsonParseError类用于在JSON解析期间报告错误。
    QJsonParseError jsonError;
    // 将json解析为UTF-8编码的json文档，并从中创建一个QJsonDocument。
    // 如果解析成功，返回QJsonDocument对象，否则返回null
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &jsonError);
    // 判断是否解析失败
    if (jsonError.error != QJsonParseError::NoError && !doc.isNull()) {
        qDebug() << "Json格式错误！" << jsonError.error;
        return "";
    }

    QJsonObject rootObj = doc.object();

    //获取键值对
    QJsonValue card = rootObj.value(action);

    if (card.toString().isEmpty())
        return NOTIFICATION_GENERAL;

    return card.toString();
}

bool SoundThemePlayer::playCustomAlertSound(QString soundPath)
{
    int retval = 1;
    if (createCaContext()) {
        retval = ca_context_play(m_caContext, 0,
                                      CA_PROP_APPLICATION_NAME, "Sound Theme Player",
                                      CA_PROP_MEDIA_FILENAME, soundPath.toLatin1().data(),
                                      CA_PROP_EVENT_DESCRIPTION, "Testing event sound",
                                      CA_PROP_CANBERRA_CACHE_CONTROL, "never",
                                      CA_PROP_APPLICATION_ID, "org.lingmo.VolumeControl",
                                      CA_PROP_CANBERRA_VOLUME, m_dBValue,
                                  #ifdef CA_PROP_CANBERRA_ENABLE
                                      CA_PROP_CANBERRA_ENABLE, "1",
                                  #endif
                                      NULL);
        qDebug() << __func__ << soundPath << retval;
    }

    return (retval == 0 ? 1 : 0);
}

bool SoundThemePlayer::isFileExist(QString theme, QString soundTye)
{
    QString path = SOUND_FILE_PATH;
    path.replace("xxxTheme", theme);
    path.replace("xxxFile", soundTye);

    QFileInfo fileInfo(path);
    path.replace("ogg", "oga");
    QFileInfo fileInfoOga(path);

    if (fileInfo.isFile())
        return true;
    else if (fileInfoOga.isFile())
        return true;
    else
        return false;
}

void SoundThemePlayer::addSoundFileList()
{
    m_pSoundFileList << "audio-volume-change"
                     << "battery-low"
                     << "complete"
                     << "device-added-failed"
                     << "device-added"
                     << "device-removed"
                     << "dialog-error"
                     << "dialog-warning"
                     << "notification-general"
                     << "notification-special"
                     << "operation-file"
                     << "operation-unsupported"
                     << "power-plug"
                     << "screen-capture"
                     << "screen-record"
                     << "start-up"
                     << "trash-empty" ;
}

void SoundThemePlayer::initGSettings()
{
    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)) {
        m_pSoundSettings = new QGSettings(KEY_SOUNDS_SCHEMA);
    }

    if (nullptr == m_pSoundSettings) {
        qDebug() << "m_pSoundSettings create fail";
        return;
    }

    if (m_pSoundSettings->keys().contains("alertVolume")) {
        int n_AlertVolume = m_pSoundSettings->get(ALERT_VOLUME).toInt();
        m_dBValue = alertVolumeTodB(n_AlertVolume);
    }

    connect(m_pSoundSettings, &QGSettings::changed, this, &SoundThemePlayer::onSettingChanged);
}

void SoundThemePlayer::onSettingChanged(const QString &key)
{
    if (key == "alertVolume") {
        int n_AlertVolume = m_pSoundSettings->get(ALERT_VOLUME).toInt();
        m_dBValue = alertVolumeTodB(n_AlertVolume);
        qDebug() << "The alert volume is changed, now is" << n_AlertVolume << m_dBValue;
    }
}

char *SoundThemePlayer::alertVolumeTodB(int value)
{
    if (value < 0 || value > 100)
        value = 100;

    pa_volume_t volume = value / 100.0 * 65536.0;
    double dBValue = pa_sw_volume_to_dB(volume);

    QByteArray byteArray = QByteArray::number(dBValue, 'f', 2);
    char* buffer = new char[byteArray.size() + 1];
    strcpy(buffer, byteArray.constData());
    m_dBValue = buffer;

    return m_dBValue;
}

SoundThemePlayer::~SoundThemePlayer()
{

}
