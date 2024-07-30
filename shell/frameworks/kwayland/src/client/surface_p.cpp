/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "surface_p.h"
#include "surface.h"

#include <QGuiApplication>
#include <private/qwaylandwindow_p.h>
#include <qpa/qplatformnativeinterface.h>

namespace KWayland
{
namespace Client
{

// The file split from surface.cpp because QtWaylandClient private headers require to unset QT_NO_KEYWORDS.
Surface *Surface::fromWindow(QWindow *window)
{
    if (!window) {
        return nullptr;
    }
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native) {
        return nullptr;
    }
    window->create();
    wl_surface *s = reinterpret_cast<wl_surface *>(native->nativeResourceForWindow(QByteArrayLiteral("surface"), window));
    if (!s) {
        return nullptr;
    }
    if (auto surface = get(s)) {
        return surface;
    }
    Surface *surface = new Surface(window);
    surface->d->surface.setup(s, true);

    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
    if (waylandWindow) {
        connect(waylandWindow, &QtWaylandClient::QWaylandWindow::wlSurfaceDestroyed, surface, &QObject::deleteLater);
    }
    return surface;
}

Surface *Surface::fromQtWinId(WId wid)
{
    QWindow *window = nullptr;

    for (auto win : qApp->allWindows()) {
        if (win->winId() == wid) {
            window = win;
            break;
        }
    }

    if (!window) {
        return nullptr;
    }
    return fromWindow(window);
}

}
}
