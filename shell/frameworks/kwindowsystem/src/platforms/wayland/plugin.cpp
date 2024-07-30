/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "plugin.h"
#include "windoweffects.h"
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

KWindowShadowTilePrivate *KWaylandPlugin::createWindowShadowTile()
{
    return new WindowShadowTile();
}

KWindowShadowPrivate *KWaylandPlugin::createWindowShadow()
{
    return new WindowShadow();
}

#include "moc_plugin.cpp"
