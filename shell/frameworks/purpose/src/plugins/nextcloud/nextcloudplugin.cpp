/*
    SPDX-FileCopyrightText: 2017 Lim Yuen Hoe <yuenhoe86@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "nextcloudjob.h"
#include <purpose/pluginbase.h>

#include <KPluginFactory>
#include <QUrl>

class NextcloudPlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    using PluginBase::PluginBase;
    Purpose::Job *createJob() const override
    {
        return new NextcloudJob(nullptr);
    }
};

K_PLUGIN_CLASS_WITH_JSON(NextcloudPlugin, "nextcloudplugin.json")

#include "nextcloudplugin.moc"
