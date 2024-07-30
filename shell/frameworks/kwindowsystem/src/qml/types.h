/*
    SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KWINDOWSYSTEM_QML_TYPES_H
#define KWINDOWSYSTEM_QML_TYPES_H

#include <QQmlEngine>

#include <KWindowSystem>

#include <config-kwindowsystem.h>
#if KWINDOWSYSTEM_HAVE_X11
#include <KX11Extras>
#endif

struct KWindowSystemForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(KWindowSystem)
    QML_SINGLETON
    QML_FOREIGN(KWindowSystem)

public:
    static KWindowSystem *create(QQmlEngine *, QJSEngine *)
    {
        QQmlEngine::setObjectOwnership(KWindowSystem::self(), QQmlEngine::CppOwnership);
        return KWindowSystem::self();
    }
};

#if KWINDOWSYSTEM_HAVE_X11
struct KX11ExtrasForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(KX11Extras)
    QML_SINGLETON
    QML_FOREIGN(KX11Extras)

public:
    static KX11Extras *create(QQmlEngine *, QJSEngine *)
    {
        QQmlEngine::setObjectOwnership(KX11Extras::self(), QQmlEngine::CppOwnership);
        return KX11Extras::self();
    }
};
#endif

#endif
