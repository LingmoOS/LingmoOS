/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_SURFACE_P_H
#define WAYLAND_SURFACE_P_H

#include "surface.h"
#include "wayland_pointer_p.h"
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN Surface::Private
{
public:
    Private(Surface *q);
    void setupFrameCallback();

    WaylandPointer<wl_surface, wl_surface_destroy> surface;
    bool frameCallbackInstalled = false;
    QSize size;
    bool foreign = false;
    qint32 scale = 1;
    QList<Output *> outputs;

    void setup(wl_surface *s);

    static QList<Surface *> s_surfaces;

private:
    void handleFrameCallback();
    static void frameCallback(void *data, wl_callback *callback, uint32_t time);
    static void enterCallback(void *data, wl_surface *wl_surface, wl_output *output);
    static void leaveCallback(void *data, wl_surface *wl_surface, wl_output *output);
    void removeOutput(Output *o);

    Surface *q;
    static const wl_callback_listener s_listener;
    static const wl_surface_listener s_surfaceListener;
};

}
}

#endif
