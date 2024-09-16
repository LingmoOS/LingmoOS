// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOPLUGIN_DAEMON_H
#define DAUDIOPLUGIN_DAEMON_H

#include "daudioplugin_p.h"

#include <QObject>

DAUDIOMANAGER_BEGIN_NAMESPACE
class DDaemonAudioPlugin : public DAudioPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID DtkAudioManagerPlugin_iid FILE "daemonaudio.json")

public:
    explicit DDaemonAudioPlugin(QObject *parent = nullptr);
    virtual ~DDaemonAudioPlugin() override {}


    virtual DAudioManagerPrivate *createAudioManager() override;
};
DAUDIOMANAGER_END_NAMESPACE

#endif
