/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#include "audio-play-manager.h"
#include <QTimer>

using namespace Peony;

static AudioPlayManager *audio_global_instance = nullptr;

AudioPlayManager::AudioPlayManager(QObject *parent) : QObject(parent)
{
//    ca_context_create(&m_ca_context);
}

AudioPlayManager::~AudioPlayManager()
{

}

AudioPlayManager *AudioPlayManager::getInstance()
{
    if (! audio_global_instance) {
        audio_global_instance = new AudioPlayManager;
    }
    return audio_global_instance;
}

void AudioPlayManager::close()
{
    if (audio_global_instance)
        audio_global_instance->deleteLater();
}

void AudioPlayManager::init()
{
    AudioPlayManager::getInstance();
}

void AudioPlayManager::playWarningAudio()
{
//    ca_context_play(m_ca_context, 0,
//                     CA_PROP_EVENT_ID, m_event_id,
//                     CA_PROP_EVENT_DESCRIPTION,
//                     tr("Operation file Warning"),
//                     NULL);
}
