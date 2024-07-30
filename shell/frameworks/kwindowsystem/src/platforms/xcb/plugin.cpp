/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "plugin.h"
#include "kwindoweffects_x11.h"
#include "kwindowshadow_p_x11.h"
#include "kwindowsystem_p_x11.h"

X11Plugin::X11Plugin(QObject *parent)
    : KWindowSystemPluginInterface(parent)
{
}

X11Plugin::~X11Plugin()
{
}

KWindowEffectsPrivate *X11Plugin::createEffects()
{
    return new KWindowEffectsPrivateX11();
}

KWindowSystemPrivate *X11Plugin::createWindowSystem()
{
    return new KWindowSystemPrivateX11();
}

KWindowShadowPrivate *X11Plugin::createWindowShadow()
{
    return new KWindowShadowPrivateX11();
}

KWindowShadowTilePrivate *X11Plugin::createWindowShadowTile()
{
    return new KWindowShadowTilePrivateX11();
}

#include "moc_plugin.cpp"
