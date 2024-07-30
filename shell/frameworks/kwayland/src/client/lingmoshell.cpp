/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "lingmoshell.h"
#include "event_queue.h"
#include "output.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Wayland
#include <wayland-lingmo-shell-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN LingmoShell::Private
{
public:
    WaylandPointer<org_kde_lingmo_shell, org_kde_lingmo_shell_destroy> shell;
    EventQueue *queue = nullptr;
};

class Q_DECL_HIDDEN LingmoShellSurface::Private
{
public:
    Private(LingmoShellSurface *q);
    ~Private();
    void setup(org_kde_lingmo_surface *surface);

    WaylandPointer<org_kde_lingmo_surface, org_kde_lingmo_surface_destroy> surface;
    QSize size;
    QPointer<Surface> parentSurface;
    LingmoShellSurface::Role role;

    static LingmoShellSurface *get(Surface *surface);

private:
    static void autoHidingPanelHiddenCallback(void *data, org_kde_lingmo_surface *org_kde_lingmo_surface);
    static void autoHidingPanelShownCallback(void *data, org_kde_lingmo_surface *org_kde_lingmo_surface);

    LingmoShellSurface *q;
    static QList<Private *> s_surfaces;
    static const org_kde_lingmo_surface_listener s_listener;
};

QList<LingmoShellSurface::Private *> LingmoShellSurface::Private::s_surfaces;

LingmoShell::LingmoShell(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

LingmoShell::~LingmoShell()
{
    release();
}

void LingmoShell::destroy()
{
    if (!d->shell) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->shell.destroy();
}

void LingmoShell::release()
{
    if (!d->shell) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->shell.release();
}

void LingmoShell::setup(org_kde_lingmo_shell *shell)
{
    Q_ASSERT(!d->shell);
    Q_ASSERT(shell);
    d->shell.setup(shell);
}

void LingmoShell::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *LingmoShell::eventQueue()
{
    return d->queue;
}

LingmoShellSurface *LingmoShell::createSurface(wl_surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto kwS = Surface::get(surface);
    if (kwS) {
        if (auto s = LingmoShellSurface::Private::get(kwS)) {
            return s;
        }
    }
    LingmoShellSurface *s = new LingmoShellSurface(parent);
    connect(this, &LingmoShell::interfaceAboutToBeReleased, s, &LingmoShellSurface::release);
    connect(this, &LingmoShell::interfaceAboutToBeDestroyed, s, &LingmoShellSurface::destroy);
    auto w = org_kde_lingmo_shell_get_surface(d->shell, surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    s->d->parentSurface = QPointer<Surface>(kwS);
    return s;
}

LingmoShellSurface *LingmoShell::createSurface(Surface *surface, QObject *parent)
{
    return createSurface(*surface, parent);
}

bool LingmoShell::isValid() const
{
    return d->shell.isValid();
}

LingmoShell::operator org_kde_lingmo_shell *()
{
    return d->shell;
}

LingmoShell::operator org_kde_lingmo_shell *() const
{
    return d->shell;
}

LingmoShellSurface::Private::Private(LingmoShellSurface *q)
    : role(LingmoShellSurface::Role::Normal)
    , q(q)
{
    s_surfaces << this;
}

LingmoShellSurface::Private::~Private()
{
    s_surfaces.removeAll(this);
}

LingmoShellSurface *LingmoShellSurface::Private::get(Surface *surface)
{
    if (!surface) {
        return nullptr;
    }
    for (auto it = s_surfaces.constBegin(); it != s_surfaces.constEnd(); ++it) {
        if ((*it)->parentSurface == surface) {
            return (*it)->q;
        }
    }
    return nullptr;
}

void LingmoShellSurface::Private::setup(org_kde_lingmo_surface *s)
{
    Q_ASSERT(s);
    Q_ASSERT(!surface);
    surface.setup(s);
    org_kde_lingmo_surface_add_listener(surface, &s_listener, this);
}

const org_kde_lingmo_surface_listener LingmoShellSurface::Private::s_listener = {autoHidingPanelHiddenCallback, autoHidingPanelShownCallback};

void LingmoShellSurface::Private::autoHidingPanelHiddenCallback(void *data, org_kde_lingmo_surface *org_kde_lingmo_surface)
{
    auto p = reinterpret_cast<LingmoShellSurface::Private *>(data);
    Q_ASSERT(p->surface == org_kde_lingmo_surface);
    Q_EMIT p->q->autoHidePanelHidden();
}

void LingmoShellSurface::Private::autoHidingPanelShownCallback(void *data, org_kde_lingmo_surface *org_kde_lingmo_surface)
{
    auto p = reinterpret_cast<LingmoShellSurface::Private *>(data);
    Q_ASSERT(p->surface == org_kde_lingmo_surface);
    Q_EMIT p->q->autoHidePanelShown();
}

LingmoShellSurface::LingmoShellSurface(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

LingmoShellSurface::~LingmoShellSurface()
{
    release();
}

void LingmoShellSurface::release()
{
    d->surface.release();
}

void LingmoShellSurface::destroy()
{
    d->surface.destroy();
}

void LingmoShellSurface::setup(org_kde_lingmo_surface *surface)
{
    d->setup(surface);
}

LingmoShellSurface *LingmoShellSurface::get(Surface *surface)
{
    if (auto s = LingmoShellSurface::Private::get(surface)) {
        return s;
    }

    return nullptr;
}

bool LingmoShellSurface::isValid() const
{
    return d->surface.isValid();
}

LingmoShellSurface::operator org_kde_lingmo_surface *()
{
    return d->surface;
}

LingmoShellSurface::operator org_kde_lingmo_surface *() const
{
    return d->surface;
}

void LingmoShellSurface::setPosition(const QPoint &point)
{
    Q_ASSERT(isValid());
    org_kde_lingmo_surface_set_position(d->surface, point.x(), point.y());
}

void LingmoShellSurface::openUnderCursor()
{
    org_kde_lingmo_surface_open_under_cursor(d->surface);
}

void LingmoShellSurface::setRole(LingmoShellSurface::Role role)
{
    Q_ASSERT(isValid());
    uint32_t wlRole = ORG_KDE_LINGMO_SURFACE_ROLE_NORMAL;
    switch (role) {
    case Role::Normal:
        wlRole = ORG_KDE_LINGMO_SURFACE_ROLE_NORMAL;
        break;
    case Role::Desktop:
        wlRole = ORG_KDE_LINGMO_SURFACE_ROLE_DESKTOP;
        break;
    case Role::Panel:
        wlRole = ORG_KDE_LINGMO_SURFACE_ROLE_PANEL;
        break;
    case Role::OnScreenDisplay:
        wlRole = ORG_KDE_LINGMO_SURFACE_ROLE_ONSCREENDISPLAY;
        break;
    case Role::Notification:
        wlRole = ORG_KDE_LINGMO_SURFACE_ROLE_NOTIFICATION;
        break;
    case Role::ToolTip:
        wlRole = ORG_KDE_LINGMO_SURFACE_ROLE_TOOLTIP;
        break;
    case Role::CriticalNotification:
        if (wl_proxy_get_version(d->surface) < ORG_KDE_LINGMO_SURFACE_ROLE_CRITICALNOTIFICATION_SINCE_VERSION) {
            // Fall back to generic notification type if not supported
            wlRole = ORG_KDE_LINGMO_SURFACE_ROLE_NOTIFICATION;
        } else {
            wlRole = ORG_KDE_LINGMO_SURFACE_ROLE_CRITICALNOTIFICATION;
        }
        break;
    case Role::AppletPopup:
        if (wl_proxy_get_version(d->surface) < ORG_KDE_LINGMO_SURFACE_ROLE_APPLETPOPUP_SINCE_VERSION) {
            // dock is what applet popups were before
            wlRole = ORG_KDE_LINGMO_SURFACE_ROLE_PANEL;
            setPanelBehavior(PanelBehavior::WindowsGoBelow);
        } else {
            wlRole = ORG_KDE_LINGMO_SURFACE_ROLE_APPLETPOPUP;
        }
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    org_kde_lingmo_surface_set_role(d->surface, wlRole);
    d->role = role;
}

LingmoShellSurface::Role LingmoShellSurface::role() const
{
    return d->role;
}

void LingmoShellSurface::setPanelBehavior(LingmoShellSurface::PanelBehavior behavior)
{
    Q_ASSERT(isValid());
    uint32_t wlRole = ORG_KDE_LINGMO_SURFACE_PANEL_BEHAVIOR_ALWAYS_VISIBLE;
    switch (behavior) {
    case PanelBehavior::AlwaysVisible:
        wlRole = ORG_KDE_LINGMO_SURFACE_PANEL_BEHAVIOR_ALWAYS_VISIBLE;
        break;
    case PanelBehavior::AutoHide:
        wlRole = ORG_KDE_LINGMO_SURFACE_PANEL_BEHAVIOR_AUTO_HIDE;
        break;
    case PanelBehavior::WindowsCanCover:
        wlRole = ORG_KDE_LINGMO_SURFACE_PANEL_BEHAVIOR_WINDOWS_CAN_COVER;
        break;
    case PanelBehavior::WindowsGoBelow:
        wlRole = ORG_KDE_LINGMO_SURFACE_PANEL_BEHAVIOR_WINDOWS_GO_BELOW;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    org_kde_lingmo_surface_set_panel_behavior(d->surface, wlRole);
}

void LingmoShellSurface::setSkipTaskbar(bool skip)
{
    org_kde_lingmo_surface_set_skip_taskbar(d->surface, skip);
}

void LingmoShellSurface::setSkipSwitcher(bool skip)
{
    org_kde_lingmo_surface_set_skip_switcher(d->surface, skip);
}

void LingmoShellSurface::requestHideAutoHidingPanel()
{
    org_kde_lingmo_surface_panel_auto_hide_hide(d->surface);
}

void LingmoShellSurface::requestShowAutoHidingPanel()
{
    org_kde_lingmo_surface_panel_auto_hide_show(d->surface);
}

void LingmoShellSurface::setPanelTakesFocus(bool takesFocus)
{
    org_kde_lingmo_surface_set_panel_takes_focus(d->surface, takesFocus);
}

}
}

#include "moc_lingmoshell.cpp"
