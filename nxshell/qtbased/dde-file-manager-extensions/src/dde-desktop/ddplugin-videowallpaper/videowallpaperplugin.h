// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOWALLPAPERPLUGIN_H
#define VIDEOWALLPAPERPLUGIN_H

#include "ddplugin_videowallpaper_global.h"

#include <dfm-framework/dpf.h>

namespace ddplugin_videowallpaper {

class WallpaperEngine;
class VideoWallpaperPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "videowallpaper.json")
public:
    explicit VideoWallpaperPlugin(QObject *parent = nullptr);
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;
protected:
    WallpaperEngine *engine = nullptr;
};

}

#endif // VIDEOWALLPAPERPLUGIN_H
