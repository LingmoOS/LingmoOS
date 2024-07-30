/*
    SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "event_queue.h"
#include "output.h"
#include "seat.h"
#include "surface.h"
#include "wayland_pointer_p.h"
#include "xdgshell_p.h"
#include <wayland-xdg-shell-client-protocol.h>

namespace KWayland
{
namespace Client
{
class XdgShellStable::Private : public XdgShell::Private
{
public:
    void setup(xdg_wm_base *shell) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;
    XdgShellSurface *getXdgSurface(Surface *surface, QObject *parent) override;

    XdgShellPopup *getXdgPopup(Surface *surface, XdgShellSurface *parentSurface, const XdgPositioner &positioner, QObject *parent) override;
    XdgShellPopup *getXdgPopup(Surface *surface, XdgShellPopup *parentSurface, const XdgPositioner &positioner, QObject *parent) override;

    using XdgShell::Private::operator xdg_shell *;
    using XdgShell::Private::operator zxdg_shell_v6 *;
    operator xdg_wm_base *() override
    {
        return xdg_shell_base;
    }
    operator xdg_wm_base *() const override
    {
        return xdg_shell_base;
    }

private:
    XdgShellPopup *internalGetXdgPopup(Surface *surface, xdg_surface *parentSurface, const XdgPositioner &positioner, QObject *parent);
    static void pingCallback(void *data, struct xdg_wm_base *shell, uint32_t serial);

    WaylandPointer<xdg_wm_base, xdg_wm_base_destroy> xdg_shell_base;
    static const struct xdg_wm_base_listener s_shellListener;
};

const struct xdg_wm_base_listener XdgShellStable::Private::s_shellListener = {
    pingCallback,
};

void XdgShellStable::Private::pingCallback(void *data, struct xdg_wm_base *shell, uint32_t serial)
{
    Q_UNUSED(data)
    xdg_wm_base_pong(shell, serial);
}

void XdgShellStable::Private::setup(xdg_wm_base *shell)
{
    Q_ASSERT(shell);
    Q_ASSERT(!xdg_shell_base);
    xdg_shell_base.setup(shell);
    xdg_wm_base_add_listener(shell, &s_shellListener, this);
}

void XdgShellStable::Private::release()
{
    xdg_shell_base.release();
}

void XdgShellStable::Private::destroy()
{
    xdg_shell_base.destroy();
}

bool XdgShellStable::Private::isValid() const
{
    return xdg_shell_base.isValid();
}

XdgShellSurface *XdgShellStable::Private::getXdgSurface(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto ss = xdg_wm_base_get_xdg_surface(xdg_shell_base, *surface);

    if (!ss) {
        return nullptr;
    }

    auto s = new XdgTopLevelStable(parent);
    auto toplevel = xdg_surface_get_toplevel(ss);
    if (queue) {
        queue->addProxy(ss);
        queue->addProxy(toplevel);
    }
    s->setup(ss, toplevel);
    return s;
}

XdgShellPopup *XdgShellStable::Private::getXdgPopup(Surface *surface, XdgShellSurface *parentSurface, const XdgPositioner &positioner, QObject *parent)
{
    return internalGetXdgPopup(surface, *parentSurface, positioner, parent);
}

XdgShellPopup *XdgShellStable::Private::getXdgPopup(Surface *surface, XdgShellPopup *parentSurface, const XdgPositioner &positioner, QObject *parent)
{
    return internalGetXdgPopup(surface, *parentSurface, positioner, parent);
}

XdgShellPopup *XdgShellStable::Private::internalGetXdgPopup(Surface *surface, xdg_surface *parentSurface, const XdgPositioner &positioner, QObject *parent)
{
    Q_ASSERT(isValid());
    auto ss = xdg_wm_base_get_xdg_surface(xdg_shell_base, *surface);
    if (!ss) {
        return nullptr;
    }

    auto p = xdg_wm_base_create_positioner(xdg_shell_base);

    auto anchorRect = positioner.anchorRect();
    xdg_positioner_set_anchor_rect(p, anchorRect.x(), anchorRect.y(), anchorRect.width(), anchorRect.height());

    QSize initialSize = positioner.initialSize();
    xdg_positioner_set_size(p, initialSize.width(), initialSize.height());

    QPoint anchorOffset = positioner.anchorOffset();
    if (!anchorOffset.isNull()) {
        xdg_positioner_set_offset(p, anchorOffset.x(), anchorOffset.y());
    }

    uint32_t anchor = XDG_POSITIONER_ANCHOR_NONE;
    if (positioner.anchorEdge().testFlag(Qt::TopEdge)) {
        if (positioner.anchorEdge().testFlag(Qt::LeftEdge) && ((positioner.anchorEdge() & ~Qt::LeftEdge) == Qt::TopEdge)) {
            anchor = XDG_POSITIONER_ANCHOR_TOP_LEFT;
        } else if (positioner.anchorEdge().testFlag(Qt::RightEdge) && ((positioner.anchorEdge() & ~Qt::RightEdge) == Qt::TopEdge)) {
            anchor = XDG_POSITIONER_ANCHOR_TOP_RIGHT;
        } else if ((positioner.anchorEdge() & ~Qt::TopEdge) == Qt::Edges()) {
            anchor = XDG_POSITIONER_ANCHOR_TOP;
        }
    } else if (positioner.anchorEdge().testFlag(Qt::BottomEdge)) {
        if (positioner.anchorEdge().testFlag(Qt::LeftEdge) && ((positioner.anchorEdge() & ~Qt::LeftEdge) == Qt::BottomEdge)) {
            anchor = XDG_POSITIONER_ANCHOR_BOTTOM_LEFT;
        } else if (positioner.anchorEdge().testFlag(Qt::RightEdge) && ((positioner.anchorEdge() & ~Qt::RightEdge) == Qt::BottomEdge)) {
            anchor = XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT;
        } else if ((positioner.anchorEdge() & ~Qt::BottomEdge) == Qt::Edges()) {
            anchor = XDG_POSITIONER_ANCHOR_BOTTOM;
        }
    } else if (positioner.anchorEdge().testFlag(Qt::RightEdge) && ((positioner.anchorEdge() & ~Qt::RightEdge) == Qt::Edges())) {
        anchor = XDG_POSITIONER_ANCHOR_RIGHT;
    } else if (positioner.anchorEdge().testFlag(Qt::LeftEdge) && ((positioner.anchorEdge() & ~Qt::LeftEdge) == Qt::Edges())) {
        anchor = XDG_POSITIONER_ANCHOR_LEFT;
    }
    if (anchor != 0) {
        xdg_positioner_set_anchor(p, anchor);
    }

    uint32_t gravity = XDG_POSITIONER_GRAVITY_NONE;
    if (positioner.gravity().testFlag(Qt::TopEdge)) {
        if (positioner.gravity().testFlag(Qt::LeftEdge) && ((positioner.gravity() & ~Qt::LeftEdge) == Qt::TopEdge)) {
            gravity = XDG_POSITIONER_GRAVITY_TOP_LEFT;
        } else if (positioner.gravity().testFlag(Qt::RightEdge) && ((positioner.gravity() & ~Qt::RightEdge) == Qt::TopEdge)) {
            gravity = XDG_POSITIONER_GRAVITY_TOP_RIGHT;
        } else if ((positioner.gravity() & ~Qt::TopEdge) == Qt::Edges()) {
            gravity = XDG_POSITIONER_GRAVITY_TOP;
        }
    } else if (positioner.gravity().testFlag(Qt::BottomEdge)) {
        if (positioner.gravity().testFlag(Qt::LeftEdge) && ((positioner.gravity() & ~Qt::LeftEdge) == Qt::BottomEdge)) {
            gravity = XDG_POSITIONER_GRAVITY_BOTTOM_LEFT;
        } else if (positioner.gravity().testFlag(Qt::RightEdge) && ((positioner.gravity() & ~Qt::RightEdge) == Qt::BottomEdge)) {
            gravity = XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT;
        } else if ((positioner.gravity() & ~Qt::BottomEdge) == Qt::Edges()) {
            gravity = XDG_POSITIONER_GRAVITY_BOTTOM;
        }
    } else if (positioner.gravity().testFlag(Qt::RightEdge) && ((positioner.gravity() & ~Qt::RightEdge) == Qt::Edges())) {
        gravity = XDG_POSITIONER_GRAVITY_RIGHT;
    } else if (positioner.gravity().testFlag(Qt::LeftEdge) && ((positioner.gravity() & ~Qt::LeftEdge) == Qt::Edges())) {
        gravity = XDG_POSITIONER_GRAVITY_LEFT;
    }
    if (gravity != 0) {
        xdg_positioner_set_gravity(p, gravity);
    }

    uint32_t constraint = XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_NONE;
    if (positioner.constraints().testFlag(XdgPositioner::Constraint::SlideX)) {
        constraint |= XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X;
    }
    if (positioner.constraints().testFlag(XdgPositioner::Constraint::SlideY)) {
        constraint |= XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y;
    }
    if (positioner.constraints().testFlag(XdgPositioner::Constraint::FlipX)) {
        constraint |= XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X;
    }
    if (positioner.constraints().testFlag(XdgPositioner::Constraint::FlipY)) {
        constraint |= XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y;
    }
    if (positioner.constraints().testFlag(XdgPositioner::Constraint::ResizeX)) {
        constraint |= XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X;
    }
    if (positioner.constraints().testFlag(XdgPositioner::Constraint::ResizeY)) {
        constraint |= XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y;
    }
    if (constraint != 0) {
        xdg_positioner_set_constraint_adjustment(p, constraint);
    }

    XdgShellPopup *s = new XdgShellPopupStable(parent);
    auto popup = xdg_surface_get_popup(ss, parentSurface, p);
    if (queue) {
        // deliberately not adding the positioner because the positioner has no events sent to it
        queue->addProxy(ss);
        queue->addProxy(popup);
    }
    s->setup(ss, popup);

    xdg_positioner_destroy(p);

    return s;
}

XdgShellStable::XdgShellStable(QObject *parent)
    : XdgShell(new Private, parent)
{
}

XdgShellStable::~XdgShellStable() = default;

// A top level wraps both xdg_surface and xdg_top_level into the public API XdgShelllSurface
class XdgTopLevelStable::Private : public XdgShellSurface::Private
{
public:
    Private(XdgShellSurface *q);
    WaylandPointer<xdg_toplevel, xdg_toplevel_destroy> xdgtoplevel;
    WaylandPointer<xdg_surface, xdg_surface_destroy> xdgsurface;

    void setup(xdg_surface *surface, xdg_toplevel *toplevel) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;

    using XdgShellSurface::Private::operator zxdg_toplevel_v6 *;
    using XdgShellSurface::Private::operator zxdg_surface_v6 *;
    operator xdg_surface *() override
    {
        return xdgsurface;
    }
    operator xdg_surface *() const override
    {
        return xdgsurface;
    }
    operator xdg_toplevel *() override
    {
        return xdgtoplevel;
    }
    operator xdg_toplevel *() const override
    {
        return xdgtoplevel;
    }

    void setTransientFor(XdgShellSurface *parent) override;
    void setTitle(const QString &title) override;
    void setAppId(const QByteArray &appId) override;
    void showWindowMenu(Seat *seat, quint32 serial, qint32 x, qint32 y) override;
    void move(Seat *seat, quint32 serial) override;
    void resize(Seat *seat, quint32 serial, Qt::Edges edges) override;
    void ackConfigure(quint32 serial) override;
    void setMaximized() override;
    void unsetMaximized() override;
    void setFullscreen(Output *output) override;
    void unsetFullscreen() override;
    void setMinimized() override;
    void setMaxSize(const QSize &size) override;
    void setMinSize(const QSize &size) override;
    void setWindowGeometry(const QRect &windowGeometry) override;

private:
    QSize pendingSize;
    States pendingState;

    static void configureCallback(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *state);
    static void closeCallback(void *data, xdg_toplevel *xdg_toplevel);
    static void surfaceConfigureCallback(void *data, xdg_surface *xdg_surface, uint32_t serial);

    static const struct xdg_toplevel_listener s_toplevelListener;
    static const struct xdg_surface_listener s_surfaceListener;
};

const struct xdg_toplevel_listener XdgTopLevelStable::Private::s_toplevelListener = {configureCallback, closeCallback};

const struct xdg_surface_listener XdgTopLevelStable::Private::s_surfaceListener = {surfaceConfigureCallback};

void XdgTopLevelStable::Private::surfaceConfigureCallback(void *data, struct xdg_surface *surface, uint32_t serial)
{
    Q_UNUSED(surface)
    auto s = static_cast<Private *>(data);
    s->q->configureRequested(s->pendingSize, s->pendingState, serial);
    if (!s->pendingSize.isNull()) {
        s->q->setSize(s->pendingSize);
        s->pendingSize = QSize();
    }
    s->pendingState = {};
}

void XdgTopLevelStable::Private::configureCallback(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *state)
{
    Q_UNUSED(xdg_toplevel)
    auto s = static_cast<Private *>(data);
    States states;

    uint32_t *statePtr = static_cast<uint32_t *>(state->data);
    for (size_t i = 0; i < state->size / sizeof(uint32_t); i++) {
        switch (statePtr[i]) {
        case XDG_TOPLEVEL_STATE_MAXIMIZED:
            states = states | XdgShellSurface::State::Maximized;
            break;
        case XDG_TOPLEVEL_STATE_FULLSCREEN:
            states = states | XdgShellSurface::State::Fullscreen;
            break;
        case XDG_TOPLEVEL_STATE_RESIZING:
            states = states | XdgShellSurface::State::Resizing;
            break;
        case XDG_TOPLEVEL_STATE_ACTIVATED:
            states = states | XdgShellSurface::State::Activated;
            break;
        }
    }
    s->pendingSize = QSize(width, height);
    s->pendingState = states;
}

void XdgTopLevelStable::Private::closeCallback(void *data, xdg_toplevel *xdg_toplevel)
{
    auto s = static_cast<XdgTopLevelStable::Private *>(data);
    Q_ASSERT(s->xdgtoplevel == xdg_toplevel);
    Q_EMIT s->q->closeRequested();
}

XdgTopLevelStable::Private::Private(XdgShellSurface *q)
    : XdgShellSurface::Private(q)
{
}

void XdgTopLevelStable::Private::setup(xdg_surface *surface, xdg_toplevel *topLevel)
{
    Q_ASSERT(surface);
    Q_ASSERT(!xdgtoplevel);
    xdgsurface.setup(surface);
    xdgtoplevel.setup(topLevel);
    xdg_surface_add_listener(xdgsurface, &s_surfaceListener, this);
    xdg_toplevel_add_listener(xdgtoplevel, &s_toplevelListener, this);
}

void XdgTopLevelStable::Private::release()
{
    xdgtoplevel.release();
    xdgsurface.release();
}

void XdgTopLevelStable::Private::destroy()
{
    xdgtoplevel.destroy();
    xdgsurface.destroy();
}

bool XdgTopLevelStable::Private::isValid() const
{
    return xdgtoplevel.isValid() && xdgsurface.isValid();
}

void XdgTopLevelStable::Private::setTransientFor(XdgShellSurface *parent)
{
    xdg_toplevel *parentSurface = nullptr;
    if (parent) {
        parentSurface = *parent;
    }
    xdg_toplevel_set_parent(xdgtoplevel, parentSurface);
}

void XdgTopLevelStable::Private::setTitle(const QString &title)
{
    xdg_toplevel_set_title(xdgtoplevel, title.toUtf8().constData());
}

void XdgTopLevelStable::Private::setAppId(const QByteArray &appId)
{
    xdg_toplevel_set_app_id(xdgtoplevel, appId.constData());
}

void XdgTopLevelStable::Private::showWindowMenu(Seat *seat, quint32 serial, qint32 x, qint32 y)
{
    xdg_toplevel_show_window_menu(xdgtoplevel, *seat, serial, x, y);
}

void XdgTopLevelStable::Private::move(Seat *seat, quint32 serial)
{
    xdg_toplevel_move(xdgtoplevel, *seat, serial);
}

void XdgTopLevelStable::Private::resize(Seat *seat, quint32 serial, Qt::Edges edges)
{
    uint wlEdge = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
    if (edges.testFlag(Qt::TopEdge)) {
        if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::TopEdge)) {
            wlEdge = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
        } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::TopEdge)) {
            wlEdge = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
        } else if ((edges & ~Qt::TopEdge) == Qt::Edges()) {
            wlEdge = XDG_TOPLEVEL_RESIZE_EDGE_TOP;
        }
    } else if (edges.testFlag(Qt::BottomEdge)) {
        if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::BottomEdge)) {
            wlEdge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
        } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::BottomEdge)) {
            wlEdge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
        } else if ((edges & ~Qt::BottomEdge) == Qt::Edges()) {
            wlEdge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
        }
    } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::Edges())) {
        wlEdge = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
    } else if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::Edges())) {
        wlEdge = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
    }
    xdg_toplevel_resize(xdgtoplevel, *seat, serial, wlEdge);
}

void XdgTopLevelStable::Private::ackConfigure(quint32 serial)
{
    xdg_surface_ack_configure(xdgsurface, serial);
}

void XdgTopLevelStable::Private::setMaximized()
{
    xdg_toplevel_set_maximized(xdgtoplevel);
}

void XdgTopLevelStable::Private::unsetMaximized()
{
    xdg_toplevel_unset_maximized(xdgtoplevel);
}

void XdgTopLevelStable::Private::setFullscreen(Output *output)
{
    wl_output *o = nullptr;
    if (output) {
        o = *output;
    }
    xdg_toplevel_set_fullscreen(xdgtoplevel, o);
}

void XdgTopLevelStable::Private::unsetFullscreen()
{
    xdg_toplevel_unset_fullscreen(xdgtoplevel);
}

void XdgTopLevelStable::Private::setMinimized()
{
    xdg_toplevel_set_minimized(xdgtoplevel);
}

void XdgTopLevelStable::Private::setMaxSize(const QSize &size)
{
    xdg_toplevel_set_max_size(xdgtoplevel, size.width(), size.height());
}

void XdgTopLevelStable::Private::setMinSize(const QSize &size)
{
    xdg_toplevel_set_min_size(xdgtoplevel, size.width(), size.height());
}

void XdgTopLevelStable::Private::setWindowGeometry(const QRect &windowGeometry)
{
    xdg_surface_set_window_geometry(xdgsurface, windowGeometry.x(), windowGeometry.y(), windowGeometry.width(), windowGeometry.height());
}

XdgTopLevelStable::XdgTopLevelStable(QObject *parent)
    : XdgShellSurface(new Private(this), parent)
{
}

XdgTopLevelStable::~XdgTopLevelStable() = default;

class XdgShellPopupStable::Private : public XdgShellPopup::Private
{
public:
    Private(XdgShellPopup *q);

    void setup(xdg_surface *s, xdg_popup *p) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;
    void requestGrab(Seat *seat, quint32 serial) override;
    void ackConfigure(quint32 serial) override;
    void setWindowGeometry(const QRect &windowGeometry) override;

    using XdgShellPopup::Private::operator zxdg_popup_v6 *;
    using XdgShellPopup::Private::operator zxdg_surface_v6 *;
    operator xdg_surface *() override
    {
        return xdgsurface;
    }
    operator xdg_surface *() const override
    {
        return xdgsurface;
    }
    operator xdg_popup *() override
    {
        return xdgpopup;
    }
    operator xdg_popup *() const override
    {
        return xdgpopup;
    }
    WaylandPointer<xdg_surface, xdg_surface_destroy> xdgsurface;
    WaylandPointer<xdg_popup, xdg_popup_destroy> xdgpopup;

    QRect pendingRect;

private:
    static void configureCallback(void *data, xdg_popup *xdg_popup, int32_t x, int32_t y, int32_t width, int32_t height);
    static void popupDoneCallback(void *data, xdg_popup *xdg_popup);
    static void surfaceConfigureCallback(void *data, xdg_surface *xdg_surface, uint32_t serial);

    static const struct xdg_popup_listener s_popupListener;
    static const struct xdg_surface_listener s_surfaceListener;
};

const struct xdg_popup_listener XdgShellPopupStable::Private::s_popupListener = {configureCallback, popupDoneCallback};

const struct xdg_surface_listener XdgShellPopupStable::Private::s_surfaceListener = {
    surfaceConfigureCallback,
};

void XdgShellPopupStable::Private::configureCallback(void *data, xdg_popup *xdg_popup, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(xdg_popup)
    auto s = static_cast<Private *>(data);
    s->pendingRect = QRect(x, y, width, height);
}

void XdgShellPopupStable::Private::surfaceConfigureCallback(void *data, struct xdg_surface *surface, uint32_t serial)
{
    Q_UNUSED(surface)
    auto s = static_cast<Private *>(data);
    s->q->configureRequested(s->pendingRect, serial);
    s->pendingRect = QRect();
}

void XdgShellPopupStable::Private::popupDoneCallback(void *data, xdg_popup *xdg_popup)
{
    auto s = static_cast<XdgShellPopupStable::Private *>(data);
    Q_ASSERT(s->xdgpopup == xdg_popup);
    Q_EMIT s->q->popupDone();
}

XdgShellPopupStable::Private::Private(XdgShellPopup *q)
    : XdgShellPopup::Private(q)
{
}

void XdgShellPopupStable::Private::setup(xdg_surface *s, xdg_popup *p)
{
    Q_ASSERT(p);
    Q_ASSERT(!xdgsurface);
    Q_ASSERT(!xdgpopup);

    xdgsurface.setup(s);
    xdgpopup.setup(p);
    xdg_surface_add_listener(xdgsurface, &s_surfaceListener, this);
    xdg_popup_add_listener(xdgpopup, &s_popupListener, this);
}

void XdgShellPopupStable::Private::release()
{
    xdgpopup.release();
}

void XdgShellPopupStable::Private::destroy()
{
    xdgpopup.destroy();
}

bool XdgShellPopupStable::Private::isValid() const
{
    return xdgpopup.isValid();
}

void XdgShellPopupStable::Private::requestGrab(Seat *seat, quint32 serial)
{
    xdg_popup_grab(xdgpopup, *seat, serial);
}

void XdgShellPopupStable::Private::ackConfigure(quint32 serial)
{
    xdg_surface_ack_configure(xdgsurface, serial);
}

void XdgShellPopupStable::Private::setWindowGeometry(const QRect &windowGeometry)
{
    xdg_surface_set_window_geometry(xdgsurface, windowGeometry.x(), windowGeometry.y(), windowGeometry.width(), windowGeometry.height());
}

XdgShellPopupStable::XdgShellPopupStable(QObject *parent)
    : XdgShellPopup(new Private(this), parent)
{
}

XdgShellPopupStable::~XdgShellPopupStable() = default;

}
}
