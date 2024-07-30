/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "waylandxdgactivationv1_p.h"
#include <QGuiApplication>

WaylandXdgActivationV1::WaylandXdgActivationV1()
    : QWaylandClientExtensionTemplate<WaylandXdgActivationV1>(1)
{
}

WaylandXdgActivationV1::~WaylandXdgActivationV1()
{
    if (qGuiApp && isActive()) {
        destroy();
    }
}

WaylandXdgActivationTokenV1 *
WaylandXdgActivationV1::requestXdgActivationToken(wl_seat *seat, struct ::wl_surface *surface, uint32_t serial, const QString &app_id)
{
    auto wl = get_activation_token();
    auto provider = new WaylandXdgActivationTokenV1;
    provider->init(wl);

    if (surface)
        provider->set_surface(surface);

    if (!app_id.isEmpty())
        provider->set_app_id(app_id);

    if (seat)
        provider->set_serial(serial, seat);
    provider->commit();
    return provider;
}
