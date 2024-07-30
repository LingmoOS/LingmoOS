/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "lingmoshell.h"
#include "display.h"
#include "surface.h"
#include "utils/resource.h"

#include <qwayland-server-lingmo-shell.h>

#include <QPointer>

namespace KWin
{
static const quint32 s_version = 8;
static QList<LingmoShellSurfaceInterface *> s_shellSurfaces;

class LingmoShellInterfacePrivate : public QtWaylandServer::org_kde_lingmo_shell
{
public:
    LingmoShellInterfacePrivate(LingmoShellInterface *q, Display *display);

private:
    void org_kde_lingmo_shell_get_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface) override;
    LingmoShellInterface *q;
};

LingmoShellInterfacePrivate::LingmoShellInterfacePrivate(LingmoShellInterface *_q, Display *display)
    : QtWaylandServer::org_kde_lingmo_shell(*display, s_version)
    , q(_q)
{
}

class LingmoShellSurfaceInterfacePrivate : public QtWaylandServer::org_kde_lingmo_surface
{
public:
    LingmoShellSurfaceInterfacePrivate(LingmoShellSurfaceInterface *q, SurfaceInterface *surface, wl_resource *resource);

    QPointer<SurfaceInterface> surface;
    LingmoShellSurfaceInterface *q;
    QPoint m_globalPos;
    LingmoShellSurfaceInterface::Role m_role = LingmoShellSurfaceInterface::Role::Normal;
    LingmoShellSurfaceInterface::PanelBehavior m_panelBehavior = LingmoShellSurfaceInterface::PanelBehavior::AlwaysVisible;
    bool m_positionSet = false;
    bool m_skipTaskbar = false;
    bool m_skipSwitcher = false;
    bool m_panelTakesFocus = false;
    bool m_openUnderCursorRequested = false;

private:
    void org_kde_lingmo_surface_destroy_resource(Resource *resource) override;
    void org_kde_lingmo_surface_destroy(Resource *resource) override;
    void org_kde_lingmo_surface_set_output(Resource *resource, struct ::wl_resource *output) override;
    void org_kde_lingmo_surface_set_position(Resource *resource, int32_t x, int32_t y) override;
    void org_kde_lingmo_surface_set_role(Resource *resource, uint32_t role) override;
    void org_kde_lingmo_surface_set_panel_behavior(Resource *resource, uint32_t flag) override;
    void org_kde_lingmo_surface_set_skip_taskbar(Resource *resource, uint32_t skip) override;
    void org_kde_lingmo_surface_panel_auto_hide_hide(Resource *resource) override;
    void org_kde_lingmo_surface_panel_auto_hide_show(Resource *resource) override;
    void org_kde_lingmo_surface_set_panel_takes_focus(Resource *resource, uint32_t takes_focus) override;
    void org_kde_lingmo_surface_set_skip_switcher(Resource *resource, uint32_t skip) override;
    void org_kde_lingmo_surface_open_under_cursor(Resource *resource) override;
};

LingmoShellInterface::LingmoShellInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new LingmoShellInterfacePrivate(this, display))
{
}

LingmoShellInterface::~LingmoShellInterface() = default;

void LingmoShellInterfacePrivate::org_kde_lingmo_shell_get_surface(QtWaylandServer::org_kde_lingmo_shell::Resource *resource,
                                                                   uint32_t id,
                                                                   struct ::wl_resource *surface)
{
    SurfaceInterface *s = SurfaceInterface::get(surface);
    if (!s) {
        wl_resource_post_error(resource->handle, 0, "Invalid  surface");
        return;
    }

    if (LingmoShellSurfaceInterface::get(s)) {
        wl_resource_post_error(resource->handle, 0, "org_kde_lingmo_shell_surface already exists");
        return;
    }

    wl_resource *shell_resource = wl_resource_create(resource->client(), &org_kde_lingmo_surface_interface, resource->version(), id);

    auto shellSurface = new LingmoShellSurfaceInterface(s, shell_resource);
    s_shellSurfaces.append(shellSurface);

    QObject::connect(shellSurface, &QObject::destroyed, [shellSurface]() {
        s_shellSurfaces.removeOne(shellSurface);
    });

    Q_EMIT q->surfaceCreated(shellSurface);
}

/*********************************
 * ShellSurfaceInterface
 *********************************/
LingmoShellSurfaceInterfacePrivate::LingmoShellSurfaceInterfacePrivate(LingmoShellSurfaceInterface *_q, SurfaceInterface *surface, wl_resource *resource)
    : QtWaylandServer::org_kde_lingmo_surface(resource)
    , surface(surface)
    , q(_q)
{
}

LingmoShellSurfaceInterface::LingmoShellSurfaceInterface(SurfaceInterface *surface, wl_resource *resource)
    : d(new LingmoShellSurfaceInterfacePrivate(this, surface, resource))
{
}

LingmoShellSurfaceInterface::~LingmoShellSurfaceInterface() = default;

SurfaceInterface *LingmoShellSurfaceInterface::surface() const
{
    return d->surface;
}

void LingmoShellSurfaceInterfacePrivate::org_kde_lingmo_surface_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void LingmoShellSurfaceInterfacePrivate::org_kde_lingmo_surface_destroy_resource(Resource *resource)
{
    delete q;
}

void LingmoShellSurfaceInterfacePrivate::org_kde_lingmo_surface_set_output(Resource *resource, struct ::wl_resource *output)
{
    // TODO: implement
}

void LingmoShellSurfaceInterfacePrivate::org_kde_lingmo_surface_set_position(Resource *resource, int32_t x, int32_t y)
{
    QPoint globalPos(x, y);
    if (m_globalPos == globalPos && m_positionSet) {
        return;
    }
    m_positionSet = true;
    m_globalPos = globalPos;
    Q_EMIT q->positionChanged();
}

void LingmoShellSurfaceInterfacePrivate::org_kde_lingmo_surface_open_under_cursor(Resource *resource)
{
    if (surface && surface->buffer()) {
        wl_resource_post_error(resource->handle, -1, "open_under_cursor: surface has a buffer");
    }
    m_openUnderCursorRequested = true;
    Q_EMIT q->openUnderCursorRequested();
}

void LingmoShellSurfaceInterfacePrivate::org_kde_lingmo_surface_set_role(Resource *resource, uint32_t role)
{
    LingmoShellSurfaceInterface::Role r = LingmoShellSurfaceInterface::Role::Normal;
    switch (role) {
    case role_desktop:
        r = LingmoShellSurfaceInterface::Role::Desktop;
        break;
    case role_panel:
        r = LingmoShellSurfaceInterface::Role::Panel;
        break;
    case role_onscreendisplay:
        r = LingmoShellSurfaceInterface::Role::OnScreenDisplay;
        break;
    case role_notification:
        r = LingmoShellSurfaceInterface::Role::Notification;
        break;
    case role_tooltip:
        r = LingmoShellSurfaceInterface::Role::ToolTip;
        break;
    case role_criticalnotification:
        r = LingmoShellSurfaceInterface::Role::CriticalNotification;
        break;
    case role_appletpopup:
        r = LingmoShellSurfaceInterface::Role::AppletPopup;
        break;
    case role_normal:
    default:
        r = LingmoShellSurfaceInterface::Role::Normal;
        break;
    }
    if (r == m_role) {
        return;
    }
    m_role = r;
    Q_EMIT q->roleChanged();
}

void LingmoShellSurfaceInterfacePrivate::org_kde_lingmo_surface_set_panel_behavior(Resource *resource, uint32_t flag)
{
    LingmoShellSurfaceInterface::PanelBehavior newBehavior = LingmoShellSurfaceInterface::PanelBehavior::AlwaysVisible;
    switch (flag) {
    case panel_behavior_auto_hide:
        newBehavior = LingmoShellSurfaceInterface::PanelBehavior::AutoHide;
        break;
    case panel_behavior_windows_can_cover:
        newBehavior = LingmoShellSurfaceInterface::PanelBehavior::WindowsCanCover;
        break;
    case panel_behavior_windows_go_below:
        newBehavior = LingmoShellSurfaceInterface::PanelBehavior::WindowsGoBelow;
        break;
    case panel_behavior_always_visible:
    default:
        break;
    }
    if (m_panelBehavior == newBehavior) {
        return;
    }
    m_panelBehavior = newBehavior;
    Q_EMIT q->panelBehaviorChanged();
}

void LingmoShellSurfaceInterfacePrivate::org_kde_lingmo_surface_set_skip_taskbar(Resource *resource, uint32_t skip)
{
    m_skipTaskbar = (bool)skip;
    Q_EMIT q->skipTaskbarChanged();
}

void LingmoShellSurfaceInterfacePrivate::org_kde_lingmo_surface_set_skip_switcher(Resource *resource, uint32_t skip)
{
    m_skipSwitcher = (bool)skip;
    Q_EMIT q->skipSwitcherChanged();
}

void LingmoShellSurfaceInterfacePrivate::org_kde_lingmo_surface_panel_auto_hide_hide(Resource *resource)
{
    if (m_role != LingmoShellSurfaceInterface::Role::Panel
        || (m_panelBehavior != LingmoShellSurfaceInterface::PanelBehavior::AutoHide
            && m_panelBehavior != LingmoShellSurfaceInterface::PanelBehavior::WindowsCanCover)) {
        wl_resource_post_error(resource->handle, error_panel_not_auto_hide, "Not an auto hide panel");
        return;
    }
    Q_EMIT q->panelAutoHideHideRequested();
}

void LingmoShellSurfaceInterfacePrivate::org_kde_lingmo_surface_panel_auto_hide_show(Resource *resource)
{
    if (m_role != LingmoShellSurfaceInterface::Role::Panel || m_panelBehavior != LingmoShellSurfaceInterface::PanelBehavior::AutoHide) {
        wl_resource_post_error(resource->handle, error_panel_not_auto_hide, "Not an auto hide panel");
        return;
    }
    Q_EMIT q->panelAutoHideShowRequested();
}

void LingmoShellSurfaceInterfacePrivate::org_kde_lingmo_surface_set_panel_takes_focus(Resource *resource, uint32_t takesFocus)
{
    if (m_panelTakesFocus == takesFocus) {
        return;
    }
    m_panelTakesFocus = takesFocus;
    Q_EMIT q->panelTakesFocusChanged();
}

QPoint LingmoShellSurfaceInterface::position() const
{
    return d->m_globalPos;
}

LingmoShellSurfaceInterface::Role LingmoShellSurfaceInterface::role() const
{
    return d->m_role;
}

bool LingmoShellSurfaceInterface::isPositionSet() const
{
    return d->m_positionSet;
}

bool LingmoShellSurfaceInterface::wantsOpenUnderCursor() const
{
    return d->m_openUnderCursorRequested;
}

LingmoShellSurfaceInterface::PanelBehavior LingmoShellSurfaceInterface::panelBehavior() const
{
    return d->m_panelBehavior;
}

bool LingmoShellSurfaceInterface::skipTaskbar() const
{
    return d->m_skipTaskbar;
}

bool LingmoShellSurfaceInterface::skipSwitcher() const
{
    return d->m_skipSwitcher;
}

void LingmoShellSurfaceInterface::hideAutoHidingPanel()
{
    d->send_auto_hidden_panel_hidden();
}

void LingmoShellSurfaceInterface::showAutoHidingPanel()
{
    d->send_auto_hidden_panel_shown();
}

bool LingmoShellSurfaceInterface::panelTakesFocus() const
{
    return d->m_panelTakesFocus;
}

LingmoShellSurfaceInterface *LingmoShellSurfaceInterface::get(wl_resource *native)
{
    if (auto surfacePrivate = resource_cast<LingmoShellSurfaceInterfacePrivate *>(native)) {
        return surfacePrivate->q;
    }
    return nullptr;
}

LingmoShellSurfaceInterface *LingmoShellSurfaceInterface::get(SurfaceInterface *surface)
{
    for (LingmoShellSurfaceInterface *shellSurface : std::as_const(s_shellSurfaces)) {
        if (shellSurface->surface() == surface) {
            return shellSurface;
        }
    }
    return nullptr;
}

}

#include "moc_lingmoshell.cpp"
