/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kwindowsystemplugininterface_p.h"

KWindowSystemPluginInterface::KWindowSystemPluginInterface(QObject *parent)
    : QObject(parent)
{
}

KWindowSystemPluginInterface::~KWindowSystemPluginInterface()
{
}

KWindowEffectsPrivate *KWindowSystemPluginInterface::createEffects()
{
    return nullptr;
}

KWindowSystemPrivate *KWindowSystemPluginInterface::createWindowSystem()
{
    return nullptr;
}

KWindowShadowPrivate *KWindowSystemPluginInterface::createWindowShadow()
{
    return nullptr;
}

KWindowShadowTilePrivate *KWindowSystemPluginInterface::createWindowShadowTile()
{
    return nullptr;
}

#include "moc_kwindowsystemplugininterface_p.cpp"
