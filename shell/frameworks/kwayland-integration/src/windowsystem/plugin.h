/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWINDOWSYSTEM_KWAYLAND_PLUGIN_H
#define KWINDOWSYSTEM_KWAYLAND_PLUGIN_H

#include <private/kwindowsystemplugininterface_p.h>

class KWaylandPlugin : public KWindowSystemPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kwindowsystem.KWindowSystemPluginInterface" FILE "wayland.json")
    Q_INTERFACES(KWindowSystemPluginInterface)

public:
    explicit KWaylandPlugin(QObject *parent = nullptr);
    ~KWaylandPlugin() override;

    KWindowEffectsPrivate *createEffects() override;
    KWindowSystemPrivate *createWindowSystem() override;
    KWindowInfoPrivate *createWindowInfo(WId window, NET::Properties properties, NET::Properties2 properties2) override;
    KWindowShadowTilePrivate *createWindowShadowTile() override;
    KWindowShadowPrivate *createWindowShadow() override;
};

#endif
