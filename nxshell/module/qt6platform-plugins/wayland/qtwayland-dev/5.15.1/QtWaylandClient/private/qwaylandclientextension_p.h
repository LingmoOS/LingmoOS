// Copyright (C) 2017 Erik Larsson.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDCLIENTEXTENSION_P_H
#define QWAYLANDCLIENTEXTENSION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/private/qobject_p.h>
#include <QtWaylandClient/QWaylandClientExtension>
#include <QtWaylandClient/private/qwaylandintegration_p.h>

QT_BEGIN_NAMESPACE

class Q_WAYLAND_CLIENT_EXPORT QWaylandClientExtensionPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWaylandClientExtension)
public:
    QWaylandClientExtensionPrivate();
    static void handleRegistryGlobal(void *data, ::wl_registry *registry, uint32_t id,
                                     const QString &interface, uint32_t version);

    QtWaylandClient::QWaylandIntegration *waylandIntegration = nullptr;
    int version = -1;
    bool active = false;
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandClientExtensionTemplatePrivate : public QWaylandClientExtensionPrivate
{
public:
    QWaylandClientExtensionTemplatePrivate()
    { }
};

QT_END_NAMESPACE

#endif  /*QWAYLANDCLIENTEXTENSION_P_H*/
