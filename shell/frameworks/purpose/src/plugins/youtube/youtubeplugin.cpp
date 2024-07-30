/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "youtubejob.h"
#include "youtubejobcomposite.h"
#include <purpose/pluginbase.h>

#include <KPluginFactory>

class YoutubePlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    using PluginBase::PluginBase;
    Purpose::Job *createJob() const override
    {
        return new YoutubeJobComposite;
    }
};

K_PLUGIN_CLASS_WITH_JSON(YoutubePlugin, "youtubeplugin.json")

#include "youtubeplugin.moc"
