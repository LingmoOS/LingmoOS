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
#include <wayland-xdg-shell-v6-client-protocol.h>

#include <QDebug>

namespace KWayland
{
namespace Client
{
class XdgShellUnstableV6::Private : public XdgShell::Private
{
public:
    void setupV6(zxdg_shell_v6 *shell) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;
    XdgShellSurface *getXdgSurface(Surface *surface, QObject *parent) override;

    XdgShellPopup *getXdgPopup(Surface *surface, XdgShellSurface *parentSurface, const XdgPositioner &positioner, QObject *parent) override;
    XdgShellPopup *getXdgPopup(Surface *surface, XdgShellPopup *parentSurface, const XdgPositioner &positioner, QObject *parent) override;

    using XdgShell::Private::operator xdg_wm_base *;
    using XdgShell::Private::operator xdg_shell *;
    operator zxdg_shell_v6 *() override
    {
        return xdgshellv6;
    }
    operator zxdg_shell_v6 *() const override
    {
        return xdgshellv6;
    }

private:
    XdgShellPopup *internalGetXdgPopup(Surface *surface, zxdg_surface_v6 *parentSurface, const XdgPositioner &positioner, QObject *parent);
    static void pingCallback(void *data, struct zxdg_shell_v6 *shell, uint32_t serial);

    WaylandPointer<zxdg_shell_v6, zxdg_shell_v6_destroy> xdgshellv6;
    static const struct zxdg_shell_v6_listener s_shellListener;
};

const struct zxdg_shell_v6_listener XdgShellUnstableV6::Private::s_shellListener = {
    pingCallback,
};

void XdgShellUnstableV6::Private::pingCallback(void *data, struct zxdg_shell_v6 *shell, uint32_t serial)
{
    Q_UNUSED(data)
    zxdg_shell_v6_pong(shell, serial);
}

void XdgShellUnstableV6::Private::setupV6(zxdg_shell_v6 *shell)
{
    Q_ASSERT(shell);
    Q_ASSERT(!xdgshellv6);
    xdgshellv6.setup(shell);
    zxdg_shell_v6_add_listener(shell, &s_shellListener, this);
}

void XdgShellUnstableV6::Private::release()
{
    xdgshellv6.release();
}

void XdgShellUnstableV6::Private::destroy()
{
    xdgshellv6.destroy();
}

bool XdgShellUnstableV6::Private::isValid() const
{
    return xdgshellv6.isValid();
}

XdgShellSurface *XdgShellUnstableV6::Private::getXdgSurface(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto ss = zxdg_shell_v6_get_xdg_surface(xdgshellv6, *surface);

    if (!ss) {
        return nullptr;
    }

    auto s = new XdgTopLevelUnstableV6(parent);
    auto toplevel = zxdg_surface_v6_get_toplevel(ss);
    if (queue) {
        queue->addProxy(ss);
        queue->addProxy(toplevel);
    }
    s->setup(ss, toplevel);
    return s;
}

XdgShellPopup *XdgShellUnstableV6::Private::getXdgPopup(Surface *surface, XdgShellSurface *parentSurface, const XdgPositioner &positioner, QObject *parent)
{
    return internalGetXdgPopup(surface, *parentSurface, positioner, parent);
}

XdgShellPopup *XdgShellUnstableV6::Private::getXdgPopup(Surface *surface, XdgShellPopup *parentSurface, const XdgPositioner &positioner, QObject *parent)
{
    return internalGetXdgPopup(surface, *parentSurface, positioner, parent);
}

XdgShellPopup *
XdgShellUnstableV6::Private::internalGetXdgPopup(Surface *surface, zxdg_surface_v6 *parentSurface, const XdgPositioner &positioner, QObject *parent)
{
    Q_ASSERT(isValid());
    auto ss = zxdg_shell_v6_get_xdg_surface(xdgshellv6, *surface);
    if (!ss) {
        return nullptr;
    }

    auto p = zxdg_shell_v6_create_positioner(xdgshellv6);

    auto anchorRect = positioner.anchorRect();
    zxdg_positioner_v6_set_anchor_rect(p, anchorRect.x(), anchorRect.y(), anchorRect.width(), anchorRect.height());

    QSize initialSize = positioner.initialSize();
    zxdg_positioner_v6_set_size(p, initialSize.width(), initialSize.height());

    QPoint anchorOffset = positioner.anchorOffset();
    if (!anchorOffset.isNull()) {
        zxdg_positioner_v6_set_offset(p, anchorOffset.x(), anchorOffset.y());
    }

    uint32_t anchor = 0;
    if (positioner.anchorEdge().testFlag(Qt::LeftEdge)) {
        anchor |= ZXDG_POSITIONER_V6_ANCHOR_LEFT;
    }
    if (positioner.anchorEdge().testFlag(Qt::TopEdge)) {
        anchor |= ZXDG_POSITIONER_V6_ANCHOR_TOP;
    }
    if (positioner.anchorEdge().testFlag(Qt::RightEdge)) {
        anchor |= ZXDG_POSITIONER_V6_ANCHOR_RIGHT;
    }
    if (positioner.anchorEdge().testFlag(Qt::BottomEdge)) {
        anchor |= ZXDG_POSITIONER_V6_ANCHOR_BOTTOM;
    }
    if (anchor != 0) {
        zxdg_positioner_v6_set_anchor(p, anchor);
    }

    uint32_t gravity = 0;
    if (positioner.gravity().testFlag(Qt::LeftEdge)) {
        gravity |= ZXDG_POSITIONER_V6_GRAVITY_LEFT;
    }
    if (positioner.gravity().testFlag(Qt::TopEdge)) {
        gravity |= ZXDG_POSITIONER_V6_GRAVITY_TOP;
    }
    if (positioner.gravity().testFlag(Qt::RightEdge)) {
        gravity |= ZXDG_POSITIONER_V6_GRAVITY_RIGHT;
    }
    if (positioner.gravity().testFlag(Qt::BottomEdge)) {
        gravity |= ZXDG_POSITIONER_V6_GRAVITY_BOTTOM;
    }
    if (gravity != 0) {
        zxdg_positioner_v6_set_gravity(p, gravity);
    }

    uint32_t constraint = 0;

    if (positioner.constraints().testFlag(XdgPositioner::Constraint::SlideX)) {
        constraint |= ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_SLIDE_X;
    }
    if (positioner.constraints().testFlag(XdgPositioner::Constraint::SlideY)) {
        constraint |= ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_SLIDE_Y;
    }
    if (positioner.constraints().testFlag(XdgPositioner::Constraint::FlipX)) {
        constraint |= ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_FLIP_X;
    }
    if (positioner.constraints().testFlag(XdgPositioner::Constraint::FlipY)) {
        constraint |= ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_FLIP_Y;
    }
    if (positioner.constraints().testFlag(XdgPositioner::Constraint::ResizeX)) {
        constraint |= ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_RESIZE_X;
    }
    if (positioner.constraints().testFlag(XdgPositioner::Constraint::ResizeY)) {
        constraint |= ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_RESIZE_Y;
    }
    if (constraint != 0) {
        zxdg_positioner_v6_set_constraint_adjustment(p, constraint);
    }

    XdgShellPopup *s = new XdgShellPopupUnstableV6(parent);
    auto popup = zxdg_surface_v6_get_popup(ss, parentSurface, p);
    if (queue) {
        // deliberately not adding the positioner because the positioner has no events sent to it
        queue->addProxy(ss);
        queue->addProxy(popup);
    }
    s->setup(ss, popup);

    zxdg_positioner_v6_destroy(p);

    return s;
}

XdgShellUnstableV6::XdgShellUnstableV6(QObject *parent)
    : XdgShell(new Private, parent)
{
}

XdgShellUnstableV6::~XdgShellUnstableV6() = default;

// A top level wraps both xdg_surface_v6 and xdg_top_level into the public API XdgShelllSurface
class XdgTopLevelUnstableV6::Private : public XdgShellSurface::Private
{
public:
    Private(XdgShellSurface *q);
    WaylandPointer<zxdg_toplevel_v6, zxdg_toplevel_v6_destroy> xdgtoplevelv6;
    WaylandPointer<zxdg_surface_v6, zxdg_surface_v6_destroy> xdgsurfacev6;

    void setupV6(zxdg_surface_v6 *surface, zxdg_toplevel_v6 *toplevel) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;

    using XdgShellSurface::Private::operator xdg_surface *;
    using XdgShellSurface::Private::operator xdg_toplevel *;
    operator zxdg_surface_v6 *() override
    {
        return xdgsurfacev6;
    }
    operator zxdg_surface_v6 *() const override
    {
        return xdgsurfacev6;
    }
    operator zxdg_toplevel_v6 *() override
    {
        return xdgtoplevelv6;
    }
    operator zxdg_toplevel_v6 *() const override
    {
        return xdgtoplevelv6;
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

private:
    QSize pendingSize;
    States pendingState;

    static void configureCallback(void *data, struct zxdg_toplevel_v6 *xdg_toplevel, int32_t width, int32_t height, struct wl_array *state);
    static void closeCallback(void *data, zxdg_toplevel_v6 *xdg_toplevel);
    static void surfaceConfigureCallback(void *data, zxdg_surface_v6 *xdg_surface, uint32_t serial);

    static const struct zxdg_toplevel_v6_listener s_toplevelListener;
    static const struct zxdg_surface_v6_listener s_surfaceListener;
};

const struct zxdg_toplevel_v6_listener XdgTopLevelUnstableV6::Private::s_toplevelListener = {configureCallback, closeCallback};

const struct zxdg_surface_v6_listener XdgTopLevelUnstableV6::Private::s_surfaceListener = {surfaceConfigureCallback};

void XdgTopLevelUnstableV6::Private::surfaceConfigureCallback(void *data, struct zxdg_surface_v6 *surface, uint32_t serial)
{
    Q_UNUSED(surface)
    auto s = reinterpret_cast<Private *>(data);
    s->q->configureRequested(s->pendingSize, s->pendingState, serial);
    if (!s->pendingSize.isNull()) {
        s->q->setSize(s->pendingSize);
        s->pendingSize = QSize();
    }
    s->pendingState = {};
}

void XdgTopLevelUnstableV6::Private::configureCallback(void *data, struct zxdg_toplevel_v6 *xdg_toplevel, int32_t width, int32_t height, struct wl_array *state)
{
    Q_UNUSED(xdg_toplevel)
    auto s = reinterpret_cast<Private *>(data);
    States states;

    uint32_t *statePtr = reinterpret_cast<uint32_t *>(state->data);
    for (size_t i = 0; i < state->size / sizeof(uint32_t); i++) {
        switch (statePtr[i]) {
        case ZXDG_TOPLEVEL_V6_STATE_MAXIMIZED:
            states = states | XdgShellSurface::State::Maximized;
            break;
        case ZXDG_TOPLEVEL_V6_STATE_FULLSCREEN:
            states = states | XdgShellSurface::State::Fullscreen;
            break;
        case ZXDG_TOPLEVEL_V6_STATE_RESIZING:
            states = states | XdgShellSurface::State::Resizing;
            break;
        case ZXDG_TOPLEVEL_V6_STATE_ACTIVATED:
            states = states | XdgShellSurface::State::Activated;
            break;
        }
    }
    s->pendingSize = QSize(width, height);
    s->pendingState = states;
}

void XdgTopLevelUnstableV6::Private::closeCallback(void *data, zxdg_toplevel_v6 *xdg_toplevel)
{
    auto s = reinterpret_cast<XdgTopLevelUnstableV6::Private *>(data);
    Q_ASSERT(s->xdgtoplevelv6 == xdg_toplevel);
    Q_EMIT s->q->closeRequested();
}

XdgTopLevelUnstableV6::Private::Private(XdgShellSurface *q)
    : XdgShellSurface::Private(q)
{
}

void XdgTopLevelUnstableV6::Private::setupV6(zxdg_surface_v6 *surface, zxdg_toplevel_v6 *topLevel)
{
    Q_ASSERT(surface);
    Q_ASSERT(!xdgtoplevelv6);
    xdgsurfacev6.setup(surface);
    xdgtoplevelv6.setup(topLevel);
    zxdg_surface_v6_add_listener(xdgsurfacev6, &s_surfaceListener, this);
    zxdg_toplevel_v6_add_listener(xdgtoplevelv6, &s_toplevelListener, this);
}

void XdgTopLevelUnstableV6::Private::release()
{
    xdgtoplevelv6.release();
    xdgsurfacev6.release();
}

void XdgTopLevelUnstableV6::Private::destroy()
{
    xdgtoplevelv6.destroy();
    xdgsurfacev6.destroy();
}

bool XdgTopLevelUnstableV6::Private::isValid() const
{
    return xdgtoplevelv6.isValid() && xdgsurfacev6.isValid();
}

void XdgTopLevelUnstableV6::Private::setTransientFor(XdgShellSurface *parent)
{
    zxdg_toplevel_v6 *parentSurface = nullptr;
    if (parent) {
        parentSurface = *parent;
    }
    zxdg_toplevel_v6_set_parent(xdgtoplevelv6, parentSurface);
}

void XdgTopLevelUnstableV6::Private::setTitle(const QString &title)
{
    zxdg_toplevel_v6_set_title(xdgtoplevelv6, title.toUtf8().constData());
}

void XdgTopLevelUnstableV6::Private::setAppId(const QByteArray &appId)
{
    zxdg_toplevel_v6_set_app_id(xdgtoplevelv6, appId.constData());
}

void XdgTopLevelUnstableV6::Private::showWindowMenu(Seat *seat, quint32 serial, qint32 x, qint32 y)
{
    zxdg_toplevel_v6_show_window_menu(xdgtoplevelv6, *seat, serial, x, y);
}

void XdgTopLevelUnstableV6::Private::move(Seat *seat, quint32 serial)
{
    zxdg_toplevel_v6_move(xdgtoplevelv6, *seat, serial);
}

void XdgTopLevelUnstableV6::Private::resize(Seat *seat, quint32 serial, Qt::Edges edges)
{
    uint wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_NONE;
    if (edges.testFlag(Qt::TopEdge)) {
        if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::TopEdge)) {
            wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP_LEFT;
        } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::TopEdge)) {
            wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP_RIGHT;
        } else if ((edges & ~Qt::TopEdge) == Qt::Edges()) {
            wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP;
        }
    } else if (edges.testFlag(Qt::BottomEdge)) {
        if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::BottomEdge)) {
            wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM_LEFT;
        } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::BottomEdge)) {
            wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM_RIGHT;
        } else if ((edges & ~Qt::BottomEdge) == Qt::Edges()) {
            wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM;
        }
    } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::Edges())) {
        wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_RIGHT;
    } else if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::Edges())) {
        wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_LEFT;
    }
    zxdg_toplevel_v6_resize(xdgtoplevelv6, *seat, serial, wlEdge);
}

void XdgTopLevelUnstableV6::Private::ackConfigure(quint32 serial)
{
    zxdg_surface_v6_ack_configure(xdgsurfacev6, serial);
}

void XdgTopLevelUnstableV6::Private::setMaximized()
{
    zxdg_toplevel_v6_set_maximized(xdgtoplevelv6);
}

void XdgTopLevelUnstableV6::Private::unsetMaximized()
{
    zxdg_toplevel_v6_unset_maximized(xdgtoplevelv6);
}

void XdgTopLevelUnstableV6::Private::setFullscreen(Output *output)
{
    wl_output *o = nullptr;
    if (output) {
        o = *output;
    }
    zxdg_toplevel_v6_set_fullscreen(xdgtoplevelv6, o);
}

void XdgTopLevelUnstableV6::Private::unsetFullscreen()
{
    zxdg_toplevel_v6_unset_fullscreen(xdgtoplevelv6);
}

void XdgTopLevelUnstableV6::Private::setMinimized()
{
    zxdg_toplevel_v6_set_minimized(xdgtoplevelv6);
}

void XdgTopLevelUnstableV6::Private::setMaxSize(const QSize &size)
{
    zxdg_toplevel_v6_set_max_size(xdgtoplevelv6, size.width(), size.height());
}

void XdgTopLevelUnstableV6::Private::setMinSize(const QSize &size)
{
    zxdg_toplevel_v6_set_min_size(xdgtoplevelv6, size.width(), size.height());
}

XdgTopLevelUnstableV6::XdgTopLevelUnstableV6(QObject *parent)
    : XdgShellSurface(new Private(this), parent)
{
}

XdgTopLevelUnstableV6::~XdgTopLevelUnstableV6() = default;

class XdgShellPopupUnstableV6::Private : public XdgShellPopup::Private
{
public:
    Private(XdgShellPopup *q);

    void setupV6(zxdg_surface_v6 *s, zxdg_popup_v6 *p) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;
    void requestGrab(Seat *seat, quint32 serial) override;
    void ackConfigure(quint32 serial) override;

    using XdgShellPopup::Private::operator xdg_popup *;
    using XdgShellPopup::Private::operator xdg_surface *;
    operator zxdg_surface_v6 *() override
    {
        return xdgsurfacev6;
    }
    operator zxdg_surface_v6 *() const override
    {
        return xdgsurfacev6;
    }
    operator zxdg_popup_v6 *() override
    {
        return xdgpopupv6;
    }
    operator zxdg_popup_v6 *() const override
    {
        return xdgpopupv6;
    }
    WaylandPointer<zxdg_surface_v6, zxdg_surface_v6_destroy> xdgsurfacev6;
    WaylandPointer<zxdg_popup_v6, zxdg_popup_v6_destroy> xdgpopupv6;

    QRect pendingRect;

private:
    static void configureCallback(void *data, zxdg_popup_v6 *xdg_popup, int32_t x, int32_t y, int32_t width, int32_t height);
    static void popupDoneCallback(void *data, zxdg_popup_v6 *xdg_popup);
    static void surfaceConfigureCallback(void *data, zxdg_surface_v6 *xdg_surface, uint32_t serial);

    static const struct zxdg_popup_v6_listener s_popupListener;
    static const struct zxdg_surface_v6_listener s_surfaceListener;
};

const struct zxdg_popup_v6_listener XdgShellPopupUnstableV6::Private::s_popupListener = {configureCallback, popupDoneCallback};

const struct zxdg_surface_v6_listener XdgShellPopupUnstableV6::Private::s_surfaceListener = {
    surfaceConfigureCallback,
};

void XdgShellPopupUnstableV6::Private::configureCallback(void *data, zxdg_popup_v6 *xdg_popup, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(xdg_popup);
    auto s = reinterpret_cast<Private *>(data);
    s->pendingRect = QRect(x, y, width, height);
}

void XdgShellPopupUnstableV6::Private::surfaceConfigureCallback(void *data, struct zxdg_surface_v6 *surface, uint32_t serial)
{
    Q_UNUSED(surface);
    auto s = reinterpret_cast<Private *>(data);
    s->q->configureRequested(s->pendingRect, serial);
    s->pendingRect = QRect();
}

void XdgShellPopupUnstableV6::Private::popupDoneCallback(void *data, zxdg_popup_v6 *xdg_popup)
{
    auto s = reinterpret_cast<XdgShellPopupUnstableV6::Private *>(data);
    Q_ASSERT(s->xdgpopupv6 == xdg_popup);
    Q_EMIT s->q->popupDone();
}

XdgShellPopupUnstableV6::Private::Private(XdgShellPopup *q)
    : XdgShellPopup::Private(q)
{
}

void XdgShellPopupUnstableV6::Private::setupV6(zxdg_surface_v6 *s, zxdg_popup_v6 *p)
{
    Q_ASSERT(p);
    Q_ASSERT(!xdgsurfacev6);
    Q_ASSERT(!xdgpopupv6);

    xdgsurfacev6.setup(s);
    xdgpopupv6.setup(p);
    zxdg_surface_v6_add_listener(xdgsurfacev6, &s_surfaceListener, this);
    zxdg_popup_v6_add_listener(xdgpopupv6, &s_popupListener, this);
}

void XdgShellPopupUnstableV6::Private::release()
{
    xdgpopupv6.release();
}

void XdgShellPopupUnstableV6::Private::destroy()
{
    xdgpopupv6.destroy();
}

bool XdgShellPopupUnstableV6::Private::isValid() const
{
    return xdgpopupv6.isValid();
}

void XdgShellPopupUnstableV6::Private::requestGrab(Seat *seat, quint32 serial)
{
    zxdg_popup_v6_grab(xdgpopupv6, *seat, serial);
}

void XdgShellPopupUnstableV6::Private::ackConfigure(quint32 serial)
{
    zxdg_surface_v6_ack_configure(xdgsurfacev6, serial);
}

XdgShellPopupUnstableV6::XdgShellPopupUnstableV6(QObject *parent)
    : XdgShellPopup(new Private(this), parent)
{
}

XdgShellPopupUnstableV6::~XdgShellPopupUnstableV6() = default;

}
}
