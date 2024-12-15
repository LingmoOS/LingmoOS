// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudioplugin_daemon.h"

#include "daudiomanager_daemon.h"

#include <QDebug>

DAUDIOMANAGER_BEGIN_NAMESPACE

DDaemonAudioPlugin::DDaemonAudioPlugin(QObject *parent)
    : DAudioPlugin(parent)
{

}

DAudioManagerPrivate *DDaemonAudioPlugin::createAudioManager()
{
    return new DDaemonAudioManager();
}
DAUDIOMANAGER_END_NAMESPACE
