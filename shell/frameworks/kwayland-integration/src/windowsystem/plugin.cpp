/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "plugin.h"
#include "windoweffects.h"
#include "windowinfo.h"
#include "windowshadow.h"
#include "windowsystem.h"

KWaylandPlugin::KWaylandPlugin(QObject *parent)
    : KWindowSystemPluginInterface(parent)
{
}

KWaylandPlugin::~KWaylandPlugin()
{
}

KWindowEffectsPrivate *KWaylandPlugin::createEffects()
{
    return new WindowEffects();
}

KWindowSystemPrivate *KWaylandPlugin::createWindowSystem()
{
    return new WindowSystem();
}

KWindowInfoPrivate *KWaylandPlugin::createWindowInfo(WId window, NET::Properties properties, NET::Properties2 properties2)
{
    return new WindowInfo(window, properties, properties2);
}

KWindowShadowTilePrivate *KWaylandPlugin::createWindowShadowTile()
{
    return new WindowShadowTile();
}

KWindowShadowPrivate *KWaylandPlugin::createWindowShadow()
{
    return new WindowShadow();
}
