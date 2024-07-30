/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef PLUGINWRAPPER_P_H
#define PLUGINWRAPPER_P_H

#include <QWidgetList> //For WId
#include <memory>

class KWindowEffectsPrivate;
class KWindowShadowPrivate;
class KWindowShadowTilePrivate;
class KWindowSystemPluginInterface;
class KWindowSystemPrivate;

class KWindowSystemPluginWrapper
{
public:
    KWindowSystemPluginWrapper();
    ~KWindowSystemPluginWrapper();
    static const KWindowSystemPluginWrapper &self();

    KWindowEffectsPrivate *effects() const;
    KWindowSystemPrivate *createWindowSystem() const;
    KWindowShadowPrivate *createWindowShadow() const;
    KWindowShadowTilePrivate *createWindowShadowTile() const;

private:
    std::unique_ptr<KWindowSystemPluginInterface> m_plugin;
    std::unique_ptr<KWindowEffectsPrivate> m_effects;
};

#endif
