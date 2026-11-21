// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/xdg_dialog.h"

#include <xdg-dialog-v1-server-protocol.h>
#include <xdg-shell-server-protocol.h>

#include <algorithm>
#include <memory>

#include "wayland/protocols/xdg_shell.h"
#include "wayland/server_util.h"
#include "wm/shell_surface.h"

namespace Lingmo::Wayland {

namespace {

using Lingmo::WM::ShellSurface;

// The user data of an xdg_dialog_v1. The protocol lets the xdg_toplevel go
// first; the dialog object is inert from then on.
class WaylandXdgDialog
{
public:
    explicit WaylandXdgDialog(wl_resource *toplevel)
        : m_toplevel(toplevel)
    {
        m_toplevelDestroy.owner = this;
        m_toplevelDestroy.listener.notify = &onToplevelDestroyed;
        wl_resource_add_destroy_listener(m_toplevel, &m_toplevelDestroy.listener);
        if (ShellSurface *shellSurface = this->shellSurface())
            shellSurface->setHasDialogObject(true);
    }

    WaylandXdgDialog(const WaylandXdgDialog &) = delete;
    WaylandXdgDialog &operator=(const WaylandXdgDialog &) = delete;

    ~WaylandXdgDialog()
    {
        if (!m_toplevel)
            return;
        wl_list_remove(&m_toplevelDestroy.listener.link);
        // Destroying the dialog object ends the dialog, modality included.
        if (ShellSurface *shellSurface = this->shellSurface()) {
            shellSurface->setHasDialogObject(false);
            shellSurface->setModal(false);
        }
    }

    void setModal(bool modal)
    {
        if (ShellSurface *shellSurface = this->shellSurface())
            shellSurface->setModal(modal);
    }

private:
    struct DestroyListener {
        wl_listener listener;
        WaylandXdgDialog *owner;
    };

    ShellSurface *shellSurface() const
    {
        return m_toplevel ? shellSurfaceForToplevel(m_toplevel) : nullptr;
    }

    static void onToplevelDestroyed(wl_listener *listener, void *)
    {
        auto *self = reinterpret_cast<DestroyListener *>(listener)->owner;
        wl_list_remove(&self->m_toplevelDestroy.listener.link);
        self->m_toplevel = nullptr;
    }

    wl_resource *m_toplevel = nullptr;
    DestroyListener m_toplevelDestroy {};
};

////////////////////////////////////////////////////////////////////////////////
// xdg_dialog_v1_interface:

void dialog_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void dialog_set_modal(wl_client *, wl_resource *resource)
{
    getUserDataAs<WaylandXdgDialog>(resource)->setModal(true);
}

void dialog_unset_modal(wl_client *, wl_resource *resource)
{
    getUserDataAs<WaylandXdgDialog>(resource)->setModal(false);
}

const struct xdg_dialog_v1_interface dialog_implementation = {
        dialog_destroy,
        dialog_set_modal,
        dialog_unset_modal,
};

////////////////////////////////////////////////////////////////////////////////
// xdg_wm_dialog_v1_interface:

void wm_dialog_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void wm_dialog_get_xdg_dialog(wl_client *client, wl_resource *resource, uint32_t id,
                              wl_resource *toplevel)
{
    if (ShellSurface *shellSurface = shellSurfaceForToplevel(toplevel);
        shellSurface && shellSurface->hasDialogObject()) {
        wl_resource_post_error(resource, XDG_WM_DIALOG_V1_ERROR_ALREADY_USED,
                               "the toplevel already has a dialog object");
        return;
    }

    wl_resource *dialog = wl_resource_create(client, &xdg_dialog_v1_interface,
                                             wl_resource_get_version(resource), id);
    if (!dialog) {
        wl_client_post_no_memory(client);
        return;
    }
    setImplementation(dialog, &dialog_implementation, std::make_unique<WaylandXdgDialog>(toplevel));
}

const struct xdg_wm_dialog_v1_interface wm_dialog_implementation = {
        wm_dialog_destroy,
        wm_dialog_get_xdg_dialog,
};

void bindXdgWmDialog(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    wl_resource *resource = wl_resource_create(client, &xdg_wm_dialog_v1_interface,
                                               std::min(kXdgWmDialogVersion, version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &wm_dialog_implementation, data, nullptr);
}

} // namespace

wl_global *createXdgDialogGlobal(wl_display *display)
{
    return wl_global_create(display, &xdg_wm_dialog_v1_interface, kXdgWmDialogVersion, nullptr,
                            &bindXdgWmDialog);
}

} // namespace Lingmo::Wayland
