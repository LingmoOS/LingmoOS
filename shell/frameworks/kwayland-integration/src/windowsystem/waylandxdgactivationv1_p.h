/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WAYLANDXDGACTIVATIONV1_P_H
#define WAYLANDXDGACTIVATIONV1_P_H

#include "qwayland-xdg-activation-v1.h"
#include <QObject>
#include <QtWaylandClient/QWaylandClientExtension>

class QWaylandSurface;

class WaylandXdgActivationTokenV1 : public QObject, public QtWayland::xdg_activation_token_v1
{
    Q_OBJECT
public:
    void xdg_activation_token_v1_done(const QString &token) override
    {
        Q_EMIT done(token);
    }

Q_SIGNALS:
    void failed();
    void done(const QString &token);
};

class WaylandXdgActivationV1 : public QWaylandClientExtensionTemplate<WaylandXdgActivationV1>, public QtWayland::xdg_activation_v1
{
public:
    WaylandXdgActivationV1();
    ~WaylandXdgActivationV1() override;

    WaylandXdgActivationTokenV1 *requestXdgActivationToken(wl_seat *seat, struct ::wl_surface *surface, uint32_t serial, const QString &app_id);
};

#endif
