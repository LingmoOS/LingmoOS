/*
 * liblingmosdk-waylandhelper's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "lingmo-decoration-manager.h"

#include "lingmo-decoration-client.h"
#include <QDebug>
#include <QApplication>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/connection_thread.h>

static const u_int32_t lingmo_decoration_surpported_version = 1;
static LINGMODecorationManager *global_instance = nullptr;

static wl_display *display = nullptr;
static lingmo_decoration *lingmo_decoration_manager = nullptr;

static void handle_global(void *data, struct wl_registry *registry,
                          uint32_t name, const char *interface, uint32_t version) {
    if (strcmp(interface, lingmo_decoration_interface.name) == 0) {
        u_int32_t surpportedVersion = qMin(lingmo_decoration_surpported_version , version);
        lingmo_decoration_manager = (lingmo_decoration *) wl_registry_bind(registry, name, &lingmo_decoration_interface, surpportedVersion);
    }
}

static void handle_global_remove(void *data, struct wl_registry *registry,
                                 uint32_t name) {
    // Who cares
}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};

LINGMODecorationManager *LINGMODecorationManager::getInstance()
{
    if (!global_instance)
        global_instance = new LINGMODecorationManager;
    return global_instance;
}

bool LINGMODecorationManager::supportLINGMODecoration()
{
    return lingmo_decoration_manager;
}

bool LINGMODecorationManager::moveWindow(QWindow *windowHandle)
{
    if (!supportLINGMODecoration())
        return false;

    auto kde_surface = KWayland::Client::Surface::fromWindow(windowHandle);
    if (!kde_surface)
        return false;
    wl_surface *surface = *kde_surface;
    if (!surface)
        return false;

    lingmo_decoration_move_surface(lingmo_decoration_manager, surface);
    wl_surface_commit(surface);
    wl_display_roundtrip(display);
    return true;
}

bool LINGMODecorationManager::removeHeaderBar(QWindow *windowHandle)
{
    if (!supportLINGMODecoration())
        return false;

    auto kde_surface = KWayland::Client::Surface::fromWindow(windowHandle);
    if (!kde_surface)
    {
        qDebug() << "LINGMODecorationManager::get kde_surface failed!";
        return false;
    }
    wl_surface *surface = *kde_surface;
    if (!surface)
    {
        qDebug() << "LINGMODecorationManager::get wl_surface failed!";
        return false;
    }
    lingmo_decoration_set_lingmo_decoration_mode(lingmo_decoration_manager, surface, 1);
    wl_surface_commit(surface);
    return true;
}

bool LINGMODecorationManager::setCornerRadius(QWindow *windowHandle, int topleft, int topright, int bottomleft, int bottomright)
{
    if (!supportLINGMODecoration())
        return false;

    auto kde_surface = KWayland::Client::Surface::fromWindow(windowHandle);
    if (!kde_surface)
        return false;
    wl_surface *surface = *kde_surface;
    if (!surface)
        return false;

    lingmo_decoration_set_unity_border_radius(lingmo_decoration_manager, surface, topleft, topright, bottomleft, bottomright);
    wl_surface_commit(surface);
    wl_display_roundtrip(display);
    return true;
}

LINGMODecorationManager::LINGMODecorationManager()
{
    auto connectionThread = KWayland::Client::ConnectionThread::fromApplication(qApp);
    display = connectionThread->display();

    struct wl_registry *registry = wl_display_get_registry(display);

    // get lingmo_decoration_manager
    wl_registry_add_listener(registry, &registry_listener, nullptr);
    wl_display_roundtrip(display);
}
