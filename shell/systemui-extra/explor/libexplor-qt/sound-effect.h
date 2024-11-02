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

#ifndef SOUNDEFFECT_H
#define SOUNDEFFECT_H
#include <QObject>
#include <QDebug>
#include <QGSettings>
#include "explor-core_global.h"


extern "C" {
#include <dconf/dconf.h>
#include <canberra.h>
}

#define NAME_KEY "name"
#define FILENAME_KEY "filename"
#define EVENT_SOUNDS_KEY "event-sounds"
#define KEY_SOUNDS_SCHEMA "org.lingmo.sound"
#define KEYBINDINGS_CUSTOM_DIR "/org/lingmo/sound/keybindings/"
#define KEYBINDINGS_CUSTOM_SCHEMA "org.lingmo.media.sound"


namespace Peony{
class PEONYCORESHARED_EXPORT SoundEffect : public QObject
{
public:
    SoundEffect();
    static SoundEffect*getInstance();
    QList<char *> listExistsPath();
    void playAlertSound(QString gsettingStr);
    void recycleBinClearMusic();
    void recycleBinDeleteMusic();
    void copyOrMoveSucceedMusic();
    void copyOrMoveFailedMusic();

private:
    ca_context *caContext;
    GSettings *m_pSoundSettings = nullptr;



};
}
#endif // SOUNDEFFECT_H
