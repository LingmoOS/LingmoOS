// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/xdg_foreign.h"

#include <xdg-foreign-unstable-v2-server-protocol.h>

#include <algorithm>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "wayland/protocols/xdg_shell.h"
#include "wayland/server_util.h"
#include "wm/shell_surface.h"

namespace Lingmo::Wayland {

namespace {

using Lingmo::WM::ShellSurface;

class ExportedSurface;

std::unordered_map<std::string, ExportedSurface *> &exportedSurfaces()
{
    static std::unordered_map<std::string, ExportedSurface *> surfaces;
    return surfaces;
}

// Handles travel through untrusted channels, so they must not be guessable:
// importing one is enough to parent a window onto somebody else's.
std::string generateHandle()
{
    static std::mt19937_64 generator(std::random_device {}());
    std::string handle;
    do {
        handle.clear();
        for (int i = 0; i < 2; ++i) {
            char buffer[17] = {};
            std::snprintf(buffer, sizeof(buffer), "%016llx",
                          static_cast<unsigned long long>(generator()));
            handle += buffer;
        }
    } while (exportedSurfaces().count(handle));
    return handle;
}

// The user data of a zxdg_exported_v2. The exported wl_surface may go first,
// which invalidates every relationship set up through this handle.
class ExportedSurface
{
public:
    ExportedSurface(wl_resource *resource, wl_resource *surface)
        : m_resource(resource)
        , m_surface(surface)
        , m_handle(generateHandle())
    {
        m_surfaceDestroy.owner = this;
        m_surfaceDestroy.listener.notify = &onSurfaceDestroyed;
        wl_resource_add_destroy_listener(m_surface, &m_surfaceDestroy.listener);
        exportedSurfaces().emplace(m_handle, this);
        zxdg_exported_v2_send_handle(m_resource, m_handle.c_str());
    }

    ExportedSurface(const ExportedSurface &) = delete;
    ExportedSurface &operator=(const ExportedSurface &) = delete;

    ~ExportedSurface()
    {
        if (m_surface)
            wl_list_remove(&m_surfaceDestroy.listener.link);
        exportedSurfaces().erase(m_handle);
        invalidate();
    }

    static ExportedSurface *forHandle(const std::string &handle)
    {
        const auto it = exportedSurfaces().find(handle);
        return it == exportedSurfaces().end() ? nullptr : it->second;
    }

    ShellSurface *shellSurface() const
    {
        return m_surface ? toplevelShellSurfaceForSurface(m_surface) : nullptr;
    }

    void addImport(wl_resource *imported) { m_imports.push_back(imported); }

    void removeImport(wl_resource *imported)
    {
        m_imports.erase(std::remove(m_imports.begin(), m_imports.end(), imported), m_imports.end());
    }

private:
    struct DestroyListener {
        wl_listener listener;
        ExportedSurface *owner;
    };

    // Tells the importers the handle is gone; each one drops the parent it set.
    void invalidate();

    static void onSurfaceDestroyed(wl_listener *listener, void *)
    {
        auto *self = reinterpret_cast<DestroyListener *>(listener)->owner;
        wl_list_remove(&self->m_surfaceDestroy.listener.link);
        self->m_surface = nullptr;
        self->invalidate();
    }

    wl_resource *m_resource = nullptr;
    wl_resource *m_surface = nullptr;
    DestroyListener m_surfaceDestroy {};
    std::string m_handle;
    std::vector<wl_resource *> m_imports;
};

// The user data of a zxdg_imported_v2. Inert once the export is gone; the
// client may still send requests until it destroys the object.
class ImportedSurface
{
public:
    explicit ImportedSurface(wl_resource *resource, ExportedSurface *exported)
        : m_resource(resource)
        , m_exported(exported)
    {
        if (m_exported)
            m_exported->addImport(m_resource);
    }

    ImportedSurface(const ImportedSurface &) = delete;
    ImportedSurface &operator=(const ImportedSurface &) = delete;

    ~ImportedSurface()
    {
        clearParent();
        if (m_exported)
            m_exported->removeImport(m_resource);
    }

    bool setParentOf(wl_resource *surface)
    {
        ShellSurface *child = toplevelShellSurfaceForSurface(surface);
        if (!child)
            return false;
        if (!m_exported) {
            // The handle died between import and this request.
            return true;
        }
        ShellSurface *parent = m_exported->shellSurface();
        if (!parent)
            return true;
        clearParent();
        if (!child->setTransientParent(parent))
            return true;
        m_childSurface = surface;
        m_childDestroy.owner = this;
        m_childDestroy.listener.notify = &onChildDestroyed;
        wl_resource_add_destroy_listener(surface, &m_childDestroy.listener);
        return true;
    }

    // The export is gone: the relationship it set up goes with it.
    void invalidate()
    {
        clearParent();
        m_exported = nullptr;
        zxdg_imported_v2_send_destroyed(m_resource);
    }

private:
    struct DestroyListener {
        wl_listener listener;
        ImportedSurface *owner;
    };

    // The shell surface is looked up again rather than kept: the client may have
    // destroyed the xdg_toplevel while holding on to its wl_surface.
    void clearParent()
    {
        if (!m_childSurface)
            return;
        wl_list_remove(&m_childDestroy.listener.link);
        if (ShellSurface *child = toplevelShellSurfaceForSurface(m_childSurface))
            child->setTransientParent(nullptr);
        m_childSurface = nullptr;
    }

    static void onChildDestroyed(wl_listener *listener, void *)
    {
        auto *self = reinterpret_cast<DestroyListener *>(listener)->owner;
        wl_list_remove(&self->m_childDestroy.listener.link);
        self->m_childSurface = nullptr;
    }

    wl_resource *m_resource = nullptr;
    ExportedSurface *m_exported = nullptr;
    wl_resource *m_childSurface = nullptr;
    DestroyListener m_childDestroy {};
};

void ExportedSurface::invalidate()
{
    const std::vector<wl_resource *> imports = m_imports;
    m_imports.clear();
    for (wl_resource *imported : imports)
        getUserDataAs<ImportedSurface>(imported)->invalidate();
}

////////////////////////////////////////////////////////////////////////////////
// zxdg_exported_v2_interface:

void exported_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

const struct zxdg_exported_v2_interface exported_implementation = {
        exported_destroy,
};

////////////////////////////////////////////////////////////////////////////////
// zxdg_imported_v2_interface:

void imported_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void imported_set_parent_of(wl_client *, wl_resource *resource, wl_resource *surface)
{
    if (!getUserDataAs<ImportedSurface>(resource)->setParentOf(surface)) {
        wl_resource_post_error(resource, ZXDG_IMPORTED_V2_ERROR_INVALID_SURFACE,
                               "the surface is not an xdg_toplevel");
    }
}

const struct zxdg_imported_v2_interface imported_implementation = {
        imported_destroy,
        imported_set_parent_of,
};

////////////////////////////////////////////////////////////////////////////////
// zxdg_exporter_v2_interface:

void exporter_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void exporter_export_toplevel(wl_client *client, wl_resource *resource, uint32_t id,
                              wl_resource *surface)
{
    if (!toplevelShellSurfaceForSurface(surface)) {
        wl_resource_post_error(resource, ZXDG_EXPORTER_V2_ERROR_INVALID_SURFACE,
                               "the surface is not an xdg_toplevel");
        return;
    }

    wl_resource *exported = wl_resource_create(client, &zxdg_exported_v2_interface,
                                               wl_resource_get_version(resource), id);
    if (!exported) {
        wl_client_post_no_memory(client);
        return;
    }
    setImplementation(exported, &exported_implementation,
                      std::make_unique<ExportedSurface>(exported, surface));
}

const struct zxdg_exporter_v2_interface exporter_implementation = {
        exporter_destroy,
        exporter_export_toplevel,
};

void bindXdgExporter(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    wl_resource *resource = wl_resource_create(client, &zxdg_exporter_v2_interface,
                                               std::min(kXdgExporterV2Version, version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &exporter_implementation, data, nullptr);
}

////////////////////////////////////////////////////////////////////////////////
// zxdg_importer_v2_interface:

void importer_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void importer_import_toplevel(wl_client *client, wl_resource *resource, uint32_t id,
                              const char *handle)
{
    wl_resource *imported = wl_resource_create(client, &zxdg_imported_v2_interface,
                                               wl_resource_get_version(resource), id);
    if (!imported) {
        wl_client_post_no_memory(client);
        return;
    }

    ExportedSurface *exported = ExportedSurface::forHandle(handle ? handle : "");
    setImplementation(imported, &imported_implementation,
                      std::make_unique<ImportedSurface>(imported, exported));
    // An unknown handle still yields an object, an inert one.
    if (!exported)
        zxdg_imported_v2_send_destroyed(imported);
}

const struct zxdg_importer_v2_interface importer_implementation = {
        importer_destroy,
        importer_import_toplevel,
};

void bindXdgImporter(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    wl_resource *resource = wl_resource_create(client, &zxdg_importer_v2_interface,
                                               std::min(kXdgImporterV2Version, version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &importer_implementation, data, nullptr);
}

} // namespace

wl_global *createXdgExporterV2Global(wl_display *display)
{
    return wl_global_create(display, &zxdg_exporter_v2_interface, kXdgExporterV2Version, nullptr,
                            &bindXdgExporter);
}

wl_global *createXdgImporterV2Global(wl_display *display)
{
    return wl_global_create(display, &zxdg_importer_v2_interface, kXdgImporterV2Version, nullptr,
                            &bindXdgImporter);
}

} // namespace Lingmo::Wayland
