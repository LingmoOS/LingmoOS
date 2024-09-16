// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WALLPAPERCONFIG_P_H
#define WALLPAPERCONFIG_P_H

#include "wallpaperconfig.h"

#include <DConfig>

namespace ddplugin_videowallpaper {

class WallpaperConfigPrivate
{
public:
    WallpaperConfigPrivate(WallpaperConfig *qq);
    bool getEnable() const;
    bool enable = false;
    DTK_CORE_NAMESPACE::DConfig *settings = nullptr;
private:
    WallpaperConfig *q;
};

}
#endif // WALLPAPERCONFIG_P_H
