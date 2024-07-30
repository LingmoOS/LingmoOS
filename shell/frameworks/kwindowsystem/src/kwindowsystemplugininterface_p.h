/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWINDOWSYSTEMPLUGININTERFACE_P_H
#define KWINDOWSYSTEMPLUGININTERFACE_P_H
#include <kwindowsystem_export.h>

#include <QObject>
#include <QWidgetList> //For WId

class KWindowEffectsPrivate;
class KWindowShadowPrivate;
class KWindowShadowTilePrivate;
class KWindowSystemPrivate;

#define KWindowSystemPluginInterface_iid "org.kde.kwindowsystem.KWindowSystemPluginInterface"

class KWINDOWSYSTEM_EXPORT KWindowSystemPluginInterface : public QObject
{
    Q_OBJECT
public:
    explicit KWindowSystemPluginInterface(QObject *parent = nullptr);
    ~KWindowSystemPluginInterface() override;

    virtual KWindowEffectsPrivate *createEffects();
    virtual KWindowSystemPrivate *createWindowSystem();
    virtual KWindowShadowPrivate *createWindowShadow();
    virtual KWindowShadowTilePrivate *createWindowShadowTile();
};

Q_DECLARE_INTERFACE(KWindowSystemPluginInterface, KWindowSystemPluginInterface_iid)

#endif
