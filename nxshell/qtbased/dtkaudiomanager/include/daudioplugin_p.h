// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOPLUGIN_P_H
#define DAUDIOPLUGIN_P_H

#include "dtkaudiomanager_global.h"

#include <QObject>

DAUDIOMANAGER_BEGIN_NAMESPACE
class DPlatformAudioPort;
class DPlatformAudioCard;
class DAudioManagerPrivate;
class DAudioPlugin : public QObject
{
    Q_OBJECT
public:
    explicit DAudioPlugin(QObject *parent = nullptr);
    virtual ~DAudioPlugin();

    virtual DAudioManagerPrivate *createAudioManager() = 0;
};
DAUDIOMANAGER_END_NAMESPACE

#define DtkAudioManagerPlugin_iid "org.deepin.dtk.audiomanager.PluginInterface"

Q_DECLARE_INTERFACE(DTK_AUDIOMANAGER_NAMESPACE::DAudioPlugin, DtkAudioManagerPlugin_iid)

#endif
