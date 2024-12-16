// Copyright (C) 2023 rewine <luhongxu@lingmo.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "wallpapercolor.h"

#include "impl/wallpaper_color_impl.h"

#include <woutput.h>
#include <wserver.h>

#include <qwdisplay.h>

#include <QDebug>
#include <QQmlInfo>

extern "C" {
#include <wayland-server-core.h>
}

WallpaperColorV1::WallpaperColorV1(QObject *parent)
    : QObject(parent)
{
}

void WallpaperColorV1::create(WServer *server)
{
    m_handle = wallpaper_color_manager_v1::create(server->handle());
}

void WallpaperColorV1::destroy(WServer *server) { }

wl_global *WallpaperColorV1::global() const
{
    return m_handle->global;
}

void WallpaperColorV1::updateWallpaperColor(const QString &output, bool isDarkType)
{
    m_handle->updateWallpaperColor(output, isDarkType);
}
