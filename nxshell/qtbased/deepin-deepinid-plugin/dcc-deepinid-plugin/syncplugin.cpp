// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "syncplugin.h"

#include "syncmodule.h"

SyncPlugin::SyncPlugin(QObject *parent):PluginInterface(parent)
{
}

ModuleObject *SyncPlugin::module()
{
    return new SyncModule();
}

QString SyncPlugin::name() const
{
    return QStringLiteral("dcc-deepinid-plugin");
}

QString SyncPlugin::location() const
{
    return QStringLiteral("0");
}
