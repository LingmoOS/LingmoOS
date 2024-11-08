/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasmoidattached_p.h"

#include "appletcontext_p.h"
#include "appletquickitem.h"

#include <Lingmo/Applet>
#include <Lingmo/Containment>

namespace LingmoQuick
{
///////////////////////PlasmoidAttached

PlasmoidAttached::PlasmoidAttached(QObject *parent)
    : QObject(parent)
{
}

PlasmoidAttached::~PlasmoidAttached()
{
}

Lingmo::Applet *PlasmoidAttached::qmlAttachedProperties(QObject *object)
{
    // Special case: we are asking the attached Plasmoid property of an AppletItem itself, which in this case is its own Applet
    if (auto *appletItem = qobject_cast<AppletQuickItem *>(object)) {
        return appletItem->applet();
    } else if (auto *applet = qobject_cast<Lingmo::Applet *>(object)) {
        // Asked for the Plasmoid of an Applet itself
        return applet;
    }

    QQmlContext *context = qmlContext(object);
    while (context) {
        if (auto *appletContext = qobject_cast<AppletContext *>(context)) {
            return appletContext->applet();
        }

        context = context->parentContext();
    }

    return nullptr;
}

///////////////////////PlasmoidAttached

ContainmentAttached::ContainmentAttached(QObject *parent)
    : QObject(parent)
{
}

ContainmentAttached::~ContainmentAttached()
{
}

Lingmo::Containment *ContainmentAttached::qmlAttachedProperties(QObject *object)
{
    return qobject_cast<Lingmo::Containment *>(PlasmoidAttached::qmlAttachedProperties(object));
}

}

#include "moc_plasmoidattached_p.cpp"
