// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wallpaperconfig_p.h"

#include <QApplication>
#include <QFileInfo>
#include <QThread>
#include <QDir>
#include <QStandardPaths>
#include <QVariant>
#include <QDebug>

DCORE_USE_NAMESPACE

using namespace ddplugin_videowallpaper;

class WallpaperConfigGlobal : public WallpaperConfig{};
Q_GLOBAL_STATIC(WallpaperConfigGlobal, wallpaperConfig)

static constexpr char kConfName[] = "org.deepin.dde.file-manager.desktop.videowallpaper";
static constexpr char kKeyEnable[] = "enable";

WallpaperConfigPrivate::WallpaperConfigPrivate(WallpaperConfig *qq)
    : q(qq)
{

}

bool WallpaperConfigPrivate::getEnable() const
{
    bool ret = false;
    if (settings)
        ret = settings->value(kKeyEnable, false).toBool();
    return ret;
}

WallpaperConfig *WallpaperConfig::instance()
{
    return wallpaperConfig;
}

bool WallpaperConfig::enable() const
{
    return d->enable;
}

void WallpaperConfig::setEnable(bool e)
{
    if (d->enable == e)
        return;

    d->enable = e;

    if (d->settings && d->getEnable() != e)
        d->settings->setValue(kKeyEnable, e);
}

WallpaperConfig::WallpaperConfig(QObject *parent)
    : QObject(parent)
    , d(new WallpaperConfigPrivate(this))
{
    // 只能在主线程创建
    Q_ASSERT(qApp->thread() == thread());
    d->settings = DConfig::create("org.deepin.dde.file-manager", kConfName, "", this);
    if (!d->settings)
        qCritical() << "can not create dconfig for" << kConfName;
}

void WallpaperConfig::initialize()
{
    d->enable = d->getEnable();
    if (d->settings)
        connect(d->settings, &DConfig::valueChanged,
                this, &WallpaperConfig::configChanged, Qt::UniqueConnection);

}

void WallpaperConfig::configChanged(const QString &key)
{
    if (key == kKeyEnable) {
        bool e = d->getEnable();
        if (e != d->enable)
            emit changeEnableState(e);
    }
}
