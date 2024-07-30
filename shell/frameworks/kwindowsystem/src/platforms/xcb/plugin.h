/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWINDOWSYSTEM_X11_PLUGIN_H
#define KWINDOWSYSTEM_X11_PLUGIN_H

#include "kwindowsystemplugininterface_p.h"

class X11Plugin : public KWindowSystemPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kwindowsystem.KWindowSystemPluginInterface" FILE "xcb.json")
    Q_INTERFACES(KWindowSystemPluginInterface)

public:
    explicit X11Plugin(QObject *parent = nullptr);
    ~X11Plugin() override;

    KWindowEffectsPrivate *createEffects() override;
    KWindowSystemPrivate *createWindowSystem() override;
    KWindowShadowPrivate *createWindowShadow() override final;
    KWindowShadowTilePrivate *createWindowShadowTile() override final;
};

#endif
