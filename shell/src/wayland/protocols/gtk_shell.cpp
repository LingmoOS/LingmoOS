// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/gtk_shell.h"

#include <algorithm>
#include <gtk-shell-server-protocol.h>
#include <new>
#include <string>
#include <string_view>

#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {
namespace {

constexpr uint32_t kGtkShellVersion = 1;
constexpr uint32_t kGlobalMenuBarCapability = 1u << (GTK_SHELL1_CAPABILITY_GLOBAL_MENU_BAR - 1);

bool validBusName(std::string_view name)
{
    if (name.empty() || name.size() > 255 || name.front() != ':')
        return false;
    size_t dots = 0;
    bool previousDot = true;
    for (char c : name.substr(1)) {
        if (c == '.') {
            if (previousDot)
                return false;
            ++dots;
            previousDot = true;
        } else {
            if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
                  || c == '_' || c == '-'))
                return false;
            previousDot = false;
        }
    }
    return dots >= 1 && !previousDot;
}

bool validObjectPath(std::string_view path)
{
    if (path.empty() || path.front() != '/' || path.size() > 1024)
        return false;
    if (path == "/")
        return true;
    if (path.back() == '/')
        return false;
    for (size_t i = 1; i < path.size(); ++i) {
        const char c = path[i];
        if (c == '/') {
            if (path[i - 1] == '/')
                return false;
        } else if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
                     || c == '_')) {
            return false;
        }
    }
    return true;
}

class GtkSurface final : public compositor::SurfaceObserver
{
public:
    explicit GtkSurface(compositor::Surface *surface)
        : m_surface(surface)
        , m_source(surface->addMenuSource())
    {
        surface->addSurfaceObserver(this);
    }

    ~GtkSurface() override
    {
        if (m_surface) {
            m_surface->removeSurfaceObserver(this);
            m_surface->removeMenuSource(m_source);
        }
    }

    void onSurfaceDestroying(compositor::Surface *surface) override
    {
        surface->removeSurfaceObserver(this);
        m_surface = nullptr;
    }

    void setProperties(const char *appMenuPath, const char *menuPath, const char *windowPath, const char *applicationPath,
                       const char *busName)
    {
        if (!m_surface)
            return;
        compositor::Surface::MenuAddress address;
        const std::string_view bus(busName ? busName : "");
        const std::string_view menu(menuPath ? menuPath : "");
        if (validBusName(bus) && validObjectPath(menu)) {
            address.gtkBusName = bus;
            const std::string_view appMenu(appMenuPath ? appMenuPath : "");
            if (validObjectPath(appMenu))
                address.gtkAppMenuPath = appMenu;
            address.gtkMenuPath = menu;
            const std::string_view window(windowPath ? windowPath : "");
            const std::string_view application(applicationPath ? applicationPath : "");
            if (validObjectPath(window))
                address.gtkWindowPath = window;
            if (validObjectPath(application))
                address.gtkApplicationPath = application;
        }
        m_surface->setMenuAddress(m_source, address);
    }

private:
    compositor::Surface *m_surface;
    uint64_t m_source;
};

void setDBusProperties(wl_client *, wl_resource *resource, const char *, const char *appMenuPath, const char *menuPath,
                       const char *windowPath, const char *applicationPath, const char *busName)
{
    getUserDataAs<GtkSurface>(resource)->setProperties(appMenuPath, menuPath, windowPath, applicationPath, busName);
}

void noOp(wl_client *, wl_resource *) {}
void present(wl_client *, wl_resource *, uint32_t) {}
void setStartupId(wl_client *, wl_resource *, const char *) {}
void systemBell(wl_client *, wl_resource *, wl_resource *) {}

const struct gtk_surface1_interface gtkSurfaceImplementation = {
    setDBusProperties, noOp, noOp, present
};

void getGtkSurface(wl_client *client, wl_resource *shell, uint32_t id, wl_resource *surfaceResource)
{
    compositor::Surface *surface = surfaceFromResource(surfaceResource);
    if (!surface) {
        wl_client_post_implementation_error(client, "gtk_surface1 requires a live wl_surface");
        return;
    }
    wl_resource *extension = wl_resource_create(client, &gtk_surface1_interface,
                                                wl_resource_get_version(shell), id);
    if (!extension) {
        wl_client_post_no_memory(client);
        return;
    }
    try {
        setImplementation(extension, &gtkSurfaceImplementation, std::make_unique<GtkSurface>(surface));
    } catch (const std::bad_alloc &) {
        wl_resource_destroy(extension);
        wl_client_post_no_memory(client);
    }
}

const struct gtk_shell1_interface gtkShellImplementation = {getGtkSurface, setStartupId, systemBell};

void bindGtkShell(wl_client *client, void *, uint32_t version, uint32_t id)
{
    wl_resource *resource = wl_resource_create(client, &gtk_shell1_interface,
                                               std::min(version, kGtkShellVersion), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &gtkShellImplementation, nullptr, nullptr);
    gtk_shell1_send_capabilities(resource, kGlobalMenuBarCapability);
}

} // namespace

wl_global *createGtkShellGlobal(wl_display *display)
{
    return wl_global_create(display, &gtk_shell1_interface, kGtkShellVersion, nullptr, bindGtkShell);
}

} // namespace Lingmo::Wayland
