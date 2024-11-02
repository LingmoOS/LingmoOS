/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Li ke <like1@kylinos.cn>
 *
 */

#include "sound-effect.h"
#include <QFile>

using namespace Peony;

static SoundEffect * soundeffect_instance = nullptr;

SoundEffect::SoundEffect()
{
    ca_context_create(&caContext);
    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA))
        m_pSoundSettings = g_settings_new (KEY_SOUNDS_SCHEMA);

}

SoundEffect *SoundEffect::getInstance()
{
    if(!soundeffect_instance){
        soundeffect_instance = new SoundEffect;
    }
    return soundeffect_instance;
}

QList<char *> SoundEffect::listExistsPath()
{
    char ** childs;
    int len;

    DConfClient * client = dconf_client_new();
    childs = dconf_client_list (client, KEYBINDINGS_CUSTOM_DIR, &len);
    g_object_unref (client);

    QList<char *> vals;

    for (int i = 0; childs[i] != NULL; i++){
        if (dconf_is_rel_dir (childs[i], NULL)){
            char * val = g_strdup (childs[i]);

            vals.append(val);
        }
    }

    g_strfreev (childs);
    return vals;
}

void SoundEffect::playAlertSound(QString gsettingStr)
{
    if (! m_pSoundSettings)
        return;

    QFile file("/usr/share/sounds/xunguang.xml");
    if(!file.exists()){
        qDebug() << "THE SOUND EFFECT FILE IS NOT EXIST";
        return;
    }

    gint retval = -1;
    const gchar *desc = "Alert Sound";
    QString filenameStr;
    QList<char *> existsPath = this->listExistsPath();
    for (char * path : existsPath) {
        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);
        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba)){
            QGSettings * settings = new QGSettings(ba, bba);
            filenameStr = settings->get(FILENAME_KEY).toString();
            QString nameStr = settings->get(NAME_KEY).toString();
            //此处匹配对应的gsetting接口字段
            if (nameStr == gsettingStr){
                break;
            }
            else{
                filenameStr = "";//新增处理，如果未找到对应组件的音效文件，则不予播放，不做此处理，会存在播放screen音效
            }
        }
    }
    const QByteArray text = filenameStr.toLocal8Bit();
    const char *id = text.data();
    const char *eventId =id;
    if(desc && filenameStr.length() >0){
        bool status = g_settings_get_boolean(m_pSoundSettings, EVENT_SOUNDS_KEY);
        if (status)
            retval = ca_context_play (this->caContext, 0,
                                      CA_PROP_EVENT_ID, eventId,
                                      CA_PROP_EVENT_DESCRIPTION, desc, NULL);

        qDebug() << "ca_context_play" << status << eventId << retval;
    }
}

void SoundEffect::recycleBinClearMusic()
{
    playAlertSound("recycle-bin-clear");
}

void SoundEffect::recycleBinDeleteMusic()
{
    playAlertSound("recycle-bin-delete");
}

void SoundEffect::copyOrMoveSucceedMusic()
{
    playAlertSound("copyormove-succeed");
}

void SoundEffect::copyOrMoveFailedMusic()
{
    playAlertSound("operation-notsupported");
}
