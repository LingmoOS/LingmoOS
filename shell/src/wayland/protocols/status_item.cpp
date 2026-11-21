// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/status_item.h"

#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <string>

#include <lingmo-status-item-v1-server-protocol.h>

#include "base/strings.h"
#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "wayland/protocols/xdg_shell.h"
#include "wayland/server_util.h"
#include "wm/status_items.h"

namespace Lingmo::Wayland {
namespace {

// The executable behind a client, which names its application for the places
// the bar remembers; metadata, never a privilege check.
std::string clientExecutable(wl_client *client)
{
    pid_t pid = 0;
    wl_client_get_credentials(client, &pid, nullptr, nullptr);
    if (pid <= 0)
        return {};
    char path[PATH_MAX];
    const std::string link = "/proc/" + std::to_string(pid) + "/exe";
    const ssize_t length = readlink(link.c_str(), path, sizeof(path) - 1);
    if (length <= 0)
        return {};
    return std::string(path, size_t(length));
}

// One lingmo_status_item_v1: the item, and the check that the client
// configures before it draws.
class WaylandStatusItem : public compositor::SurfaceObserver
{
public:
    WaylandStatusItem(wl_resource *resource, wm::StatusItems *items, compositor::Surface *surface,
                      std::string owner)
        : m_resource(resource)
        , m_surface(surface)
    {
        wm::StatusItem::Client client;
        client.configure = [resource](int height, int maxWidth) {
            const uint32_t serial = wl_display_next_serial(wl_client_get_display(wl_resource_get_client(resource)));
            lingmo_status_item_v1_send_configure(resource, serial, height, maxWidth);
            wl_client_flush(wl_resource_get_client(resource));
            return serial;
        };
        client.appearance = [resource](bool dark, uint32_t foreground) {
            lingmo_status_item_v1_send_appearance(resource,
                                                    dark ? LINGMO_STATUS_ITEM_V1_APPEARANCE_DARK
                                                         : LINGMO_STATUS_ITEM_V1_APPEARANCE_LIGHT,
                                                    foreground);
        };
        client.highlight = [resource](bool highlighted) {
            lingmo_status_item_v1_send_highlight(resource, highlighted ? 1 : 0);
        };
        client.placed = [resource](const base::Rect &rect) {
            lingmo_status_item_v1_send_placed(resource, rect.x(), rect.y(), rect.width(), rect.height());
            wl_client_flush(wl_resource_get_client(resource));
        };
        m_surface->addSurfaceObserver(this);
        m_item = std::make_unique<wm::StatusItem>(items, surface, std::move(owner), std::move(client));
    }

    ~WaylandStatusItem() override
    {
        m_item.reset();
        if (m_surface)
            m_surface->removeSurfaceObserver(this);
    }

    wm::StatusItem *item() const { return m_item.get(); }

    void onSurfaceDestroying(compositor::Surface *surface) override
    {
        surface->removeSurfaceObserver(this);
        m_surface = nullptr;
    }

    std::function<void()> onSurfaceCommitRequested(compositor::Surface *surface) override
    {
        if (!m_item->isConfigured() && surface->lastCommitAttachedBuffer()) {
            wl_resource_post_error(m_resource, LINGMO_STATUS_ITEM_V1_ERROR_UNCONFIGURED_BUFFER,
                                   "a buffer was committed before the first configure was acknowledged");
        }
        return {};
    }

private:
    wl_resource *m_resource;
    compositor::Surface *m_surface;
    std::unique_ptr<wm::StatusItem> m_item;
};

void itemDestroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void itemAckConfigure(wl_client *, wl_resource *resource, uint32_t serial)
{
    if (!getUserDataAs<WaylandStatusItem>(resource)->item()->acknowledgeConfigure(serial)) {
        wl_resource_post_error(resource, LINGMO_STATUS_ITEM_V1_ERROR_INVALID_SERIAL,
                               "the serial was never sent, or was already acknowledged");
    }
}

void itemSetIdentifier(wl_client *, wl_resource *resource, const char *identifier)
{
    getUserDataAs<WaylandStatusItem>(resource)->item()->setIdentifier(base::strings::sanitizedUtf8(identifier));
}

void itemSetHighlight(wl_client *, wl_resource *resource, uint32_t highlighted)
{
    if (highlighted > 1) {
        wl_resource_post_error(resource, LINGMO_STATUS_ITEM_V1_ERROR_INVALID_VALUE,
                               "highlighted must be zero or one");
        return;
    }
    getUserDataAs<WaylandStatusItem>(resource)->item()->setHighlightRequested(highlighted != 0);
}

void itemGetPopup(wl_client *, wl_resource *resource, wl_resource *popup)
{
    wm::StatusItem *item = getUserDataAs<WaylandStatusItem>(resource)->item();
    wm::ShellSurface *parent = item->isPlaced() ? item->popupParent() : nullptr;
    const ParentlessPopupResult result = adoptParentlessPopup(
            popup, parent, item->position(),
            [item](wm::ShellSurface *shellSurface, const base::Rect &bounds) {
                return item->adoptPopup(shellSurface, bounds);
            });
    if (result == ParentlessPopupResult::HasParent) {
        wl_resource_post_error(resource, LINGMO_STATUS_ITEM_V1_ERROR_INVALID_POPUP,
                               "the popup already has a parent");
    }
}

const struct lingmo_status_item_v1_interface itemImplementation = {
        itemDestroy, itemAckConfigure, itemSetIdentifier, itemSetHighlight, itemGetPopup};

void managerDestroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void managerGetStatusItem(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *surfaceResource)
{
    auto *surface = getUserDataAs<compositor::Surface>(surfaceResource);
    if (surface->hasRole()) {
        wl_resource_post_error(resource, LINGMO_STATUS_ITEM_MANAGER_V1_ERROR_ROLE,
                               "the surface already has a role");
        return;
    }
    wl_resource *itemResource = wl_resource_create(client, &lingmo_status_item_v1_interface, 1, id);
    if (!itemResource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto *items = static_cast<wm::StatusItems *>(wl_resource_get_user_data(resource));
    setImplementation(itemResource, &itemImplementation,
                      std::make_unique<WaylandStatusItem>(itemResource, items, surface, clientExecutable(client)));
}

const struct lingmo_status_item_manager_v1_interface managerImplementation = {managerDestroy,
                                                                                  managerGetStatusItem};

void bind(wl_client *client, void *data, uint32_t, uint32_t id)
{
    auto *resource = wl_resource_create(client, &lingmo_status_item_manager_v1_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &managerImplementation, data, nullptr);
}

} // namespace

wl_global *createStatusItemGlobal(wl_display *display, wm::StatusItems *items)
{
    return wl_global_create(display, &lingmo_status_item_manager_v1_interface, 1, items, bind);
}

} // namespace Lingmo::Wayland
