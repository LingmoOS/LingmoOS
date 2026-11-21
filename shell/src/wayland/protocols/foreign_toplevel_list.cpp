// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/foreign_toplevel_list.h"

#include <algorithm>
#include <ext-foreign-toplevel-list-v1-server-protocol.h>
#include <wayland-server-core.h>

#include "compositor/capture_source.h"
#include "compositor/compositor.h"
#include "wm/window.h"
#include "wm/window_manager.h"

namespace Lingmo::Wayland {

struct ForeignToplevelList::Client {
    ForeignToplevelList *list = nullptr;
    wl_resource *resource = nullptr;
    // After stop no further toplevel events are sent, but live handles stay.
    bool stopped = false;
};

struct ForeignToplevelList::Entry {
    wm::Window *window = nullptr;
    std::string identifier;
    std::shared_ptr<compositor::CaptureSource> source;
    // The window with its shadow, for the screenshot interface.
    std::shared_ptr<compositor::CaptureSource> shadowed;
    // Announced to the list's clients; a shell window only has its sources.
    bool listed = false;
    // One handle per client that saw this window; owner data points here.
    std::vector<wl_resource *> handles;
};

struct ForeignToplevelListEvents {
    static void stop(wl_client *, wl_resource *resource)
    {
        auto *client = static_cast<ForeignToplevelList::Client *>(wl_resource_get_user_data(resource));
        if (client && !client->stopped) {
            client->stopped = true;
            ext_foreign_toplevel_list_v1_send_finished(resource);
        }
    }

    static void destroy(wl_client *, wl_resource *resource)
    {
        wl_resource_destroy(resource);
    }

    static void handleDestroy(wl_client *, wl_resource *resource)
    {
        wl_resource_destroy(resource);
    }

    static void handleResourceDestroyed(wl_resource *resource)
    {
        auto *entry = static_cast<ForeignToplevelList::Entry *>(wl_resource_get_user_data(resource));
        if (entry)
            std::erase(entry->handles, resource);
    }

    static void listResourceDestroyed(wl_resource *resource)
    {
        auto *client = static_cast<ForeignToplevelList::Client *>(wl_resource_get_user_data(resource));
        if (client) {
            std::erase_if(client->list->m_clients, [client](const auto &owned) {
                return owned.get() == client;
            });
        }
    }

    static void bind(wl_client *wlClient, void *data, uint32_t version, uint32_t id);
};

namespace {

const struct ext_foreign_toplevel_list_v1_interface listImplementation = {
        ForeignToplevelListEvents::stop, ForeignToplevelListEvents::destroy};

const struct ext_foreign_toplevel_handle_v1_interface handleImplementation = {
        ForeignToplevelListEvents::handleDestroy};

} // namespace

void ForeignToplevelListEvents::bind(wl_client *wlClient, void *data, uint32_t version,
                                     uint32_t id)
{
    auto *list = static_cast<ForeignToplevelList *>(data);
    if (!list->m_authorize(wlClient)) {
        wl_client_post_implementation_error(wlClient, "toplevel list access denied");
        return;
    }
    auto *resource = wl_resource_create(wlClient, &ext_foreign_toplevel_list_v1_interface,
                                        int(std::min(version, 1u)), id);
    if (!resource) {
        wl_client_post_no_memory(wlClient);
        return;
    }
    auto client = std::make_unique<ForeignToplevelList::Client>();
    client->list = list;
    client->resource = resource;
    wl_resource_set_implementation(resource, &listImplementation, client.get(),
                                   listResourceDestroyed);
    list->m_clients.push_back(std::move(client));
    for (auto &[window, entry] : list->m_entries) {
        if (entry->listed)
            list->announce(*list->m_clients.back(), *entry);
    }
}

ForeignToplevelList::ForeignToplevelList(wl_display *display, wm::WindowManager *manager,
                                         compositor::Compositor *compositor, Authorize authorize)
    : m_manager(manager)
    , m_compositor(compositor)
    , m_authorize(std::move(authorize))
{
    m_global = wl_global_create(display, &ext_foreign_toplevel_list_v1_interface, 1, this,
                                ForeignToplevelListEvents::bind);
    for (wm::Window *window : m_manager->windows())
        add(window);
    m_windowAdded = m_manager->windowAdded.connect([this](wm::Window *window) {
        add(window);
    });
    m_windowRemoved = m_manager->windowRemoved.connect([this](wm::Window *window) {
        remove(window);
    });
    if (m_compositor) {
        m_outputViewsChanged = m_compositor->outputViewsChanged.connect([this] {
            updateAllGeometry();
        });
    }
}

ForeignToplevelList::~ForeignToplevelList()
{
    shutdown();
}

void ForeignToplevelList::shutdown()
{
    if (!m_global)
        return;
    wl_global_destroy(std::exchange(m_global, nullptr));
    while (!m_entries.empty())
        remove(m_entries.begin()->first);
    for (auto &client : m_clients) {
        if (client->resource && !client->stopped) {
            client->stopped = true;
            ext_foreign_toplevel_list_v1_send_finished(client->resource);
        }
        // A list resource that outlives this object must not reach its Client.
        wl_resource_set_user_data(client->resource, nullptr);
    }
    m_clients.clear();
}

bool ForeignToplevelList::listable(const wm::Window *window)
{
    return window && !window->isPopup() && window->role() == wm::WindowRole::Normal
            && !window->isSystemDialog();
}

ForeignToplevelList::Entry &ForeignToplevelList::entryFor(wm::Window *window)
{
    if (const auto it = m_entries.find(window); it != m_entries.end())
        return *it->second;
    auto entry = std::make_unique<Entry>();
    entry->window = window;
    // The window id is never reused, so the identifier names one window for
    // the life of the compositor.
    entry->identifier = "lingmo-" + std::to_string(window->id());
    entry->source = std::make_shared<compositor::CaptureSource>();
    entry->source->kind = compositor::CaptureSource::Kind::Window;
    entry->source->node = window->node();
    entry->shadowed = std::make_shared<compositor::CaptureSource>();
    entry->shadowed->kind = compositor::CaptureSource::Kind::Window;
    entry->shadowed->node = window->node();
    Entry *raw = entry.get();
    m_entries.emplace(window, std::move(entry));
    updateGeometry(*raw);
    window->addObserver(this);
    return *raw;
}

void ForeignToplevelList::add(wm::Window *window)
{
    if (!listable(window))
        return;
    // A capture source may have made the entry before the window became listable.
    Entry &entry = entryFor(window);
    if (entry.listed)
        return;
    entry.listed = true;
    for (auto &client : m_clients)
        announce(*client, entry);
}

void ForeignToplevelList::remove(wm::Window *window)
{
    const auto it = m_entries.find(window);
    if (it == m_entries.end())
        return;
    std::unique_ptr<Entry> entry = std::move(it->second);
    m_entries.erase(it);
    window->removeObserver(this);
    for (auto &source : {entry->source, entry->shadowed}) {
        source->node = nullptr;
        source->active = false;
        source->changed();
    }
    // Handles outlive the window until the client destroys them; they simply
    // no longer resolve to a source.
    for (wl_resource *handle : entry->handles) {
        wl_resource_set_user_data(handle, nullptr);
        ext_foreign_toplevel_handle_v1_send_closed(handle);
    }
}

void ForeignToplevelList::announce(Client &client, Entry &entry)
{
    if (!client.resource || client.stopped)
        return;
    auto *handle = wl_resource_create(wl_resource_get_client(client.resource),
                                      &ext_foreign_toplevel_handle_v1_interface, 1, 0);
    if (!handle) {
        wl_client_post_no_memory(wl_resource_get_client(client.resource));
        return;
    }
    wl_resource_set_implementation(handle, &handleImplementation, &entry,
                                   ForeignToplevelListEvents::handleResourceDestroyed);
    entry.handles.push_back(handle);
    ext_foreign_toplevel_list_v1_send_toplevel(client.resource, handle);
    ext_foreign_toplevel_handle_v1_send_identifier(handle, entry.identifier.c_str());
    ext_foreign_toplevel_handle_v1_send_title(handle, entry.window->title().c_str());
    ext_foreign_toplevel_handle_v1_send_app_id(handle,
                                               entry.window->applicationId().c_str());
    ext_foreign_toplevel_handle_v1_send_done(handle);
}

// The output whose composition carries the window is the one holding most of
// it; its scale decides the capture's pixel size.
void ForeignToplevelList::updateGeometry(Entry &entry)
{
    const base::Rect bounds = entry.window->bounds();
    std::string outputName;
    double scale = 1;
    int best = -1;
    for (const auto &view : m_compositor->outputViews()) {
        const base::Rect overlap = view.bounds.intersected(bounds);
        const int area = overlap.width() * overlap.height();
        if (area > best) {
            best = area;
            outputName = view.waylandName.empty() ? view.id : view.waylandName;
            scale = view.scale.value();
        }
    }
    const auto update = [&](compositor::CaptureSource &source, const base::Rect &bounds) {
        const base::Size before = source.pixelSize();
        source.bounds = bounds;
        source.scale = scale;
        source.outputName = outputName;
        if (source.pixelSize() != before)
            source.changed();
    };
    update(*entry.source, bounds);
    update(*entry.shadowed, m_shadowBounds ? m_shadowBounds(entry.window) : bounds);
}

void ForeignToplevelList::setShadowBounds(ShadowBounds bounds)
{
    m_shadowBounds = std::move(bounds);
    updateAllGeometry();
}

std::shared_ptr<compositor::CaptureSource> ForeignToplevelList::shadowedSourceFor(wm::Window *window)
{
    if (!window || !m_global)
        return nullptr;
    Entry &entry = entryFor(window);
    updateGeometry(entry);
    return entry.shadowed;
}

void ForeignToplevelList::updateAllGeometry()
{
    for (auto &[window, entry] : m_entries)
        updateGeometry(*entry);
}

std::shared_ptr<compositor::CaptureSource> ForeignToplevelList::sourceFor(wl_resource *handle) const
{
    if (!handle
        || !wl_resource_instance_of(handle, &ext_foreign_toplevel_handle_v1_interface,
                                    &handleImplementation))
        return nullptr;
    auto *entry = static_cast<Entry *>(wl_resource_get_user_data(handle));
    return entry ? entry->source : nullptr;
}

void ForeignToplevelList::onWindowBoundsChanged(wm::Window *window, const base::Rect &)
{
    if (const auto it = m_entries.find(window); it != m_entries.end())
        updateGeometry(*it->second);
}

// Maximized and fullscreen windows cast no shadow, so the shadowed bounds
// follow the state as well as the bounds.
void ForeignToplevelList::onWindowStateTypeChanged(wm::Window *window, wm::WindowStateType)
{
    if (const auto it = m_entries.find(window); it != m_entries.end())
        updateGeometry(*it->second);
}

void ForeignToplevelList::onWindowApplicationIdChanged(wm::Window *window)
{
    const auto it = m_entries.find(window);
    if (it == m_entries.end())
        return;
    for (wl_resource *handle : it->second->handles) {
        ext_foreign_toplevel_handle_v1_send_app_id(handle,
                                                   window->applicationId().c_str());
        ext_foreign_toplevel_handle_v1_send_done(handle);
    }
}

void ForeignToplevelList::onWindowTitleChanged(wm::Window *window)
{
    const auto it = m_entries.find(window);
    if (it == m_entries.end())
        return;
    for (wl_resource *handle : it->second->handles) {
        ext_foreign_toplevel_handle_v1_send_title(handle, window->title().c_str());
        ext_foreign_toplevel_handle_v1_send_done(handle);
    }
}

} // namespace Lingmo::Wayland
