// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopdbusinterface.h"

#include <QDBusInterface>
#include <QDBusPendingCall>

using namespace ocean_desktop;

DesktopDBusInterface::DesktopDBusInterface(QObject *parent) : QObject(parent)
{

}

void DesktopDBusInterface::Refresh(bool silent)
{
    QDBusInterface ifs(kDesktopServiceName,
                       "/org/lingmo/ocean/desktop/canvas",
                       "org.lingmo.ocean.desktop.canvas");
    ifs.asyncCall("Refresh", silent);
}

void DesktopDBusInterface::ShowWallpaperChooser(const QString &screen)
{
    QDBusInterface ifs(kDesktopServiceName,
                       "/org/lingmo/ocean/desktop/wallpapersettings",
                       "org.lingmo.ocean.desktop.wallpapersettings");
    ifs.asyncCall("ShowWallpaperChooser", screen);
}

void DesktopDBusInterface::ShowScreensaverChooser(const QString &screen)
{
    QDBusInterface ifs(kDesktopServiceName,
                       "/org/lingmo/ocean/desktop/wallpapersettings",
                       "org.lingmo.ocean.desktop.wallpapersettings");
    ifs.asyncCall("ShowScreensaverChooser", screen);
}
