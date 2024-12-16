// Copyright (C) 2024 rewine <luhongxu@lingmo.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "server-protocol.h"

#include <qwdisplay.h>

#include <QMap>
#include <QObject>
#include <QString>

#define TREELAND_COLOR_MANAGER_V1_VERSION 1

struct wallpaper_color_manager_v1 : public QObject
{
    Q_OBJECT
public:
    ~wallpaper_color_manager_v1();
    wl_global *global{ nullptr };

    static wallpaper_color_manager_v1 *create(QW_NAMESPACE::QWDisplay *display);

    QMap<wl_resource *, QList<QString>> watch_lists;
    QMap<QString, bool> color_map;

    void updateWallpaperColor(const QString &output, bool isDarkType);

Q_SIGNALS:
    void beforeDestroy();
};
