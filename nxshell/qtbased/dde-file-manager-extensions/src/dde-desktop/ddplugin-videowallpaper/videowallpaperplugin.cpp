// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videowallpaperplugin.h"
#include "wallpaperengine.h"

#include <QTranslator>

namespace ddplugin_videowallpaper {
DFM_LOG_REISGER_CATEGORY(DDP_VIDEOWALLPAPER_NAMESPACE)
}

DDP_VIDEOWALLPAPER_USE_NAMESPACE

VideoWallpaperPlugin::VideoWallpaperPlugin(QObject *parent) : Plugin()
{

}

void ddplugin_videowallpaper::VideoWallpaperPlugin::initialize()
{
#ifdef USE_LIBDMR
    // for libdmr
    setlocale(LC_NUMERIC, "C");
#endif
    // load translation
    auto trans = new QTranslator(this);
    if (trans->load(QString(":/translations/ddplugin-videowallpaper_%1.qm").arg(QLocale::system().name())))
        QCoreApplication::installTranslator(trans);
    else
        delete trans;
}

bool VideoWallpaperPlugin::start()
{
    engine = new WallpaperEngine();
    return engine->init();
}

void VideoWallpaperPlugin::stop()
{
    delete engine;
    engine = nullptr;
}
