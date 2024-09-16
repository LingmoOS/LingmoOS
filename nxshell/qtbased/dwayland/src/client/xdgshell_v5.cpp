/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "../compat/wayland-xdg-shell-v5-client-protocol.h"
#include "event_queue.h"
#include "output.h"
#include "seat.h"
#include "surface.h"
#include "wayland_pointer_p.h"
#include "xdgshell_p.h"

namespace KWayland
{
namespace Client
{
class XdgShellUnstableV5::Private : public XdgShell::Private
{
public:
    void setupV5(xdg_shell *shell) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;
    XdgShellSurface *getXdgSurface(Surface *surface, QObject *parent) override;
    XdgShellPopup *getXdgPopup(Surface *surface, Surface *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent) override;

    using XdgShell::Private::operator xdg_wm_base *;
    using XdgShell::Private::operator zxdg_shell_v6 *;
    operator xdg_shell *() override
    {
        return xdgshellv5;
    }
    operator xdg_shell *() const override
    {
        return xdgshellv5;
    }

    static void pingCallback(void *data, struct xdg_shell *shell, uint32_t serial);

    WaylandPointer<xdg_shell, zxdg_shell_v5_destroy> xdgshellv5;
    static const struct zxdg_shell_v5_listener s_shellListener;
};

const struct zxdg_shell_v5_listener XdgShellUnstableV5::Private::s_shellListener = {
    pingCallback,
};

void XdgShellUnstableV5::Private::pingCallback(void *data, struct xdg_shell *shell, uint32_t serial)
{
    Q_UNUSED(data);
    zxdg_shell_v5_pong(shell, serial);
}

void XdgShellUnstableV5::Private::setupV5(xdg_shell *shell)
{
    Q_ASSERT(shell);
    Q_ASSERT(!xdgshellv5);
    xdgshellv5.setup(shell);
    zxdg_shell_v5_use_unstable_version(xdgshellv5, 5);
    zxdg_shell_v5_add_listener(shell, &s_shellListener, this);
}

void XdgShellUnstableV5::Private::release()
{
    xdgshellv5.release();
}

void XdgShellUnstableV5::Private::destroy()
{
    xdgshellv5.destroy();
}

bool XdgShellUnstableV5::Private::isValid() const
{
    return xdgshellv5.isValid();
}

XdgShellSurface *XdgShellUnstableV5::Private::getXdgSurface(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    XdgShellSurface *s = new XdgShellSurfaceUnstableV5(parent);
    auto w = zxdg_shell_v5_get_xdg_surface(xdgshellv5, *surface);
    if (queue) {
        queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

XdgShellPopup *
XdgShellUnstableV5::Private::getXdgPopup(Surface *surface, Surface *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent)
{
    Q_ASSERT(isValid());
    XdgShellPopup *s = new XdgShellPopupUnstableV5(parent);
    auto w = zxdg_shell_v5_get_xdg_popup(xdgshellv5, *surface, *parentSurface, *seat, serial, parentPos.x(), parentPos.y());
    if (queue) {
        queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

XdgShellUnstableV5::XdgShellUnstableV5(QObject *parent)
    : XdgShell(new Private, parent)
{
}

XdgShellUnstableV5::~XdgShellUnstableV5() = default;

class XdgShellSurfaceUnstableV5::Private : public XdgShellSurface::Private
{
public:
    Private(XdgShellSurface *q);
    WaylandPointer<xdg_surface, zxdg_surface_v5_destroy> xdgsurfacev5;

    void setupV5(xdg_surface *surface) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;

    using XdgShellSurface::Private::operator zxdg_surface_v6 *;
    using XdgShellSurface::Private::operator zxdg_toplevel_v6 *;
    using XdgShellSurface::Private::operator xdg_toplevel *;
    operator xdg_surface *() override
    {
        return xdgsurfacev5;
    }
    operator xdg_surface *() const override
    {
        return xdgsurfacev5;
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
    static void configureCallback(void *data, xdg_surface *xdg_surface, int32_t width, int32_t height, wl_array *states, uint32_t serial);
    static void closeCallback(void *data, xdg_surface *xdg_surface);

    static const struct zxdg_surface_v5_listener s_listener;
};

const struct zxdg_surface_v5_listener XdgShellSurfaceUnstableV5::Private::s_listener = {configureCallback, closeCallback};

void XdgShellSurfaceUnstableV5::Private::configureCallback(void *data,
                                                           xdg_surface *xdg_surface,
                                                           int32_t width,
                                                           int32_t height,
                                                           wl_array *wlStates,
                                                           uint32_t serial)
{
    auto s = reinterpret_cast<XdgShellSurfaceUnstableV5::Private *>(data);
    Q_ASSERT(s->xdgsurfacev5 == xdg_surface);
    uint32_t *state = reinterpret_cast<uint32_t *>(wlStates->data);
    size_t numStates = wlStates->size / sizeof(uint32_t);
    States states;
    for (size_t i = 0; i < numStates; i++) {
        switch (state[i]) {
        case ZXDG_SURFACE_V5_STATE_MAXIMIZED:
            states = states | XdgShellSurface::State::Maximized;
            break;
        case ZXDG_SURFACE_V5_STATE_FULLSCREEN:
            states = states | XdgShellSurface::State::Fullscreen;
            break;
        case ZXDG_SURFACE_V5_STATE_RESIZING:
            states = states | XdgShellSurface::State::Resizing;
            break;
        case ZXDG_SURFACE_V5_STATE_ACTIVATED:
            states = states | XdgShellSurface::State::Activated;
            break;
        }
    }
    const QSize size = QSize(width, height);
    Q_EMIT s->q->configureRequested(size, states, serial);
    if (!size.isNull()) {
        s->q->setSize(size);
    }
}

void XdgShellSurfaceUnstableV5::Private::closeCallback(void *data, xdg_surface *xdg_surface)
{
    auto s = reinterpret_cast<XdgShellSurfaceUnstableV5::Private *>(data);
    Q_ASSERT(s->xdgsurfacev5 == xdg_surface);
    Q_EMIT s->q->closeRequested();
}

XdgShellSurfaceUnstableV5::Private::Private(XdgShellSurface *q)
    : XdgShellSurface::Private(q)
{
}

void XdgShellSurfaceUnstableV5::Private::setupV5(xdg_surface *surface)
{
    Q_ASSERT(surface);
    Q_ASSERT(!xdgsurfacev5);
    xdgsurfacev5.setup(surface);
    zxdg_surface_v5_add_listener(xdgsurfacev5, &s_listener, this);
}

void XdgShellSurfaceUnstableV5::Private::release()
{
    xdgsurfacev5.release();
}

void XdgShellSurfaceUnstableV5::Private::destroy()
{
    xdgsurfacev5.destroy();
}

bool XdgShellSurfaceUnstableV5::Private::isValid() const
{
    return xdgsurfacev5.isValid();
}

void XdgShellSurfaceUnstableV5::Private::setTransientFor(XdgShellSurface *parent)
{
    xdg_surface *parentSurface = nullptr;
    if (parent) {
        parentSurface = *parent;
    }
    zxdg_surface_v5_set_parent(xdgsurfacev5, parentSurface);
}

void XdgShellSurfaceUnstableV5::Private::setTitle(const QString &title)
{
    zxdg_surface_v5_set_title(xdgsurfacev5, title.toUtf8().constData());
}

void XdgShellSurfaceUnstableV5::Private::setAppId(const QByteArray &appId)
{
    zxdg_surface_v5_set_app_id(xdgsurfacev5, appId.constData());
}

void XdgShellSurfaceUnstableV5::Private::showWindowMenu(Seat *seat, quint32 serial, qint32 x, qint32 y)
{
    zxdg_surface_v5_show_window_menu(xdgsurfacev5, *seat, serial, x, y);
}

void XdgShellSurfaceUnstableV5::Private::move(Seat *seat, quint32 serial)
{
    zxdg_surface_v5_move(xdgsurfacev5, *seat, serial);
}

void XdgShellSurfaceUnstableV5::Private::resize(Seat *seat, quint32 serial, Qt::Edges edges)
{
    uint wlEdge = ZXDG_SURFACE_V5_RESIZE_EDGE_NONE;
    if (edges.testFlag(Qt::TopEdge)) {
        if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::TopEdge)) {
            wlEdge = ZXDG_SURFACE_V5_RESIZE_EDGE_TOP_LEFT;
        } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::TopEdge)) {
            wlEdge = ZXDG_SURFACE_V5_RESIZE_EDGE_TOP_RIGHT;
        } else if ((edges & ~Qt::TopEdge) == Qt::Edges()) {
            wlEdge = ZXDG_SURFACE_V5_RESIZE_EDGE_TOP;
        }
    } else if (edges.testFlag(Qt::BottomEdge)) {
        if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::BottomEdge)) {
            wlEdge = ZXDG_SURFACE_V5_RESIZE_EDGE_BOTTOM_LEFT;
        } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::BottomEdge)) {
            wlEdge = ZXDG_SURFACE_V5_RESIZE_EDGE_BOTTOM_RIGHT;
        } else if ((edges & ~Qt::BottomEdge) == Qt::Edges()) {
            wlEdge = ZXDG_SURFACE_V5_RESIZE_EDGE_BOTTOM;
        }
    } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::Edges())) {
        wlEdge = ZXDG_SURFACE_V5_RESIZE_EDGE_RIGHT;
    } else if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::Edges())) {
        wlEdge = ZXDG_SURFACE_V5_RESIZE_EDGE_LEFT;
    }
    zxdg_surface_v5_resize(xdgsurfacev5, *seat, serial, wlEdge);
}

void XdgShellSurfaceUnstableV5::Private::ackConfigure(quint32 serial)
{
    zxdg_surface_v5_ack_configure(xdgsurfacev5, serial);
}

void XdgShellSurfaceUnstableV5::Private::setMaximized()
{
    zxdg_surface_v5_set_maximized(xdgsurfacev5);
}

void XdgShellSurfaceUnstableV5::Private::unsetMaximized()
{
    zxdg_surface_v5_unset_maximized(xdgsurfacev5);
}

void XdgShellSurfaceUnstableV5::Private::setFullscreen(Output *output)
{
    wl_output *o = nullptr;
    if (output) {
        o = *output;
    }
    zxdg_surface_v5_set_fullscreen(xdgsurfacev5, o);
}

void XdgShellSurfaceUnstableV5::Private::unsetFullscreen()
{
    zxdg_surface_v5_unset_fullscreen(xdgsurfacev5);
}

void XdgShellSurfaceUnstableV5::Private::setMinimized()
{
    zxdg_surface_v5_set_minimized(xdgsurfacev5);
}

void XdgShellSurfaceUnstableV5::Private::setMaxSize(const QSize &size)
{
    Q_UNUSED(size)
    // TODO: notify an error?
}

void XdgShellSurfaceUnstableV5::Private::setMinSize(const QSize &size)
{
    Q_UNUSED(size)
    // TODO: notify an error?
}

XdgShellSurfaceUnstableV5::XdgShellSurfaceUnstableV5(QObject *parent)
    : XdgShellSurface(new Private(this), parent)
{
}

XdgShellSurfaceUnstableV5::~XdgShellSurfaceUnstableV5() = default;

class XdgShellPopupUnstableV5::Private : public XdgShellPopup::Private
{
public:
    Private(XdgShellPopup *q);

    void setupV5(xdg_popup *p) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;

    using XdgShellPopup::Private::operator xdg_surface *;
    using XdgShellPopup::Private::operator zxdg_popup_v6 *;
    using XdgShellPopup::Private::operator zxdg_surface_v6 *;
    operator xdg_popup *() override
    {
        return xdgpopupv5;
    }
    operator xdg_popup *() const override
    {
        return xdgpopupv5;
    }
    WaylandPointer<xdg_popup, zxdg_popup_v5_destroy> xdgpopupv5;

private:
    static void popupDoneCallback(void *data, xdg_popup *xdg_popup);
    static const struct zxdg_popup_v5_listener s_listener;
};

const struct zxdg_popup_v5_listener XdgShellPopupUnstableV5::Private::s_listener = {popupDoneCallback};

void XdgShellPopupUnstableV5::Private::popupDoneCallback(void *data, xdg_popup *xdg_popup)
{
    auto s = reinterpret_cast<XdgShellPopupUnstableV5::Private *>(data);
    Q_ASSERT(s->xdgpopupv5 == xdg_popup);
    Q_EMIT s->q->popupDone();
}

XdgShellPopupUnstableV5::Private::Private(XdgShellPopup *q)
    : XdgShellPopup::Private(q)
{
}

void XdgShellPopupUnstableV5::Private::setupV5(xdg_popup *p)
{
    Q_ASSERT(p);
    Q_ASSERT(!xdgpopupv5);
    xdgpopupv5.setup(p);
    zxdg_popup_v5_add_listener(xdgpopupv5, &s_listener, this);
}

void XdgShellPopupUnstableV5::Private::release()
{
    xdgpopupv5.release();
}

void XdgShellPopupUnstableV5::Private::destroy()
{
    xdgpopupv5.destroy();
}

bool XdgShellPopupUnstableV5::Private::isValid() const
{
    return xdgpopupv5.isValid();
}

XdgShellPopupUnstableV5::XdgShellPopupUnstableV5(QObject *parent)
    : XdgShellPopup(new Private(this), parent)
{
}

XdgShellPopupUnstableV5::~XdgShellPopupUnstableV5() = default;

}
}
