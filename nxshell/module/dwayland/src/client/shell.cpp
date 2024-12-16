/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "shell.h"
#include "event_queue.h"
#include "output.h"
#include "seat.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Qt
#include <QGuiApplication>
#include <QVector>
#include <qpa/qplatformnativeinterface.h>
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN Shell::Private
{
public:
    WaylandPointer<wl_shell, wl_shell_destroy> shell;
    EventQueue *queue = nullptr;
};

Shell::Shell(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Shell::~Shell()
{
    release();
}

void Shell::destroy()
{
    if (!d->shell) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->shell.destroy();
}

void Shell::release()
{
    if (!d->shell) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->shell.release();
}

void Shell::setup(wl_shell *shell)
{
    Q_ASSERT(!d->shell);
    Q_ASSERT(shell);
    d->shell.setup(shell);
}

void Shell::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *Shell::eventQueue()
{
    return d->queue;
}

ShellSurface *Shell::createSurface(wl_surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    ShellSurface *s = new ShellSurface(parent);
    connect(this, &Shell::interfaceAboutToBeReleased, s, &ShellSurface::release);
    connect(this, &Shell::interfaceAboutToBeDestroyed, s, &ShellSurface::destroy);
    auto w = wl_shell_get_shell_surface(d->shell, surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

ShellSurface *Shell::createSurface(Surface *surface, QObject *parent)
{
    Q_ASSERT(surface);
    return createSurface(*surface, parent);
}

bool Shell::isValid() const
{
    return d->shell.isValid();
}

Shell::operator wl_shell *()
{
    return d->shell;
}

Shell::operator wl_shell *() const
{
    return d->shell;
}

class Q_DECL_HIDDEN ShellSurface::Private
{
public:
    Private(ShellSurface *q);
    void setup(wl_shell_surface *surface);

    WaylandPointer<wl_shell_surface, wl_shell_surface_destroy> surface;
    QSize size;
    static QVector<ShellSurface *> s_surfaces;

private:
    void ping(uint32_t serial);
    static void pingCallback(void *data, struct wl_shell_surface *shellSurface, uint32_t serial);
    static void configureCallback(void *data, struct wl_shell_surface *shellSurface, uint32_t edges, int32_t width, int32_t height);
    static void popupDoneCallback(void *data, struct wl_shell_surface *shellSurface);

    ShellSurface *q;
    static const struct wl_shell_surface_listener s_listener;
};

QVector<ShellSurface *> ShellSurface::Private::s_surfaces = QVector<ShellSurface *>();

ShellSurface::Private::Private(ShellSurface *q)
    : q(q)
{
}

void ShellSurface::Private::setup(wl_shell_surface *s)
{
    Q_ASSERT(s);
    Q_ASSERT(!surface);
    surface.setup(s);
    wl_shell_surface_add_listener(surface, &s_listener, this);
}

ShellSurface *ShellSurface::fromWindow(QWindow *window)
{
    if (!window) {
        return nullptr;
    }
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native) {
        return nullptr;
    }
    window->create();
    wl_shell_surface *s = reinterpret_cast<wl_shell_surface *>(native->nativeResourceForWindow(QByteArrayLiteral("wl_shell_surface"), window));
    if (!s) {
        return nullptr;
    }
    if (auto surface = get(s)) {
        return surface;
    }
    ShellSurface *surface = new ShellSurface(window);
    surface->d->surface.setup(s, true);
    return surface;
}

ShellSurface *ShellSurface::fromQtWinId(WId wid)
{
    QWindow *window = nullptr;

    for (auto win : qApp->allWindows()) {
        if (win->winId() == wid) {
            window = win;
            break;
        }
    }

    if (!window) {
        return nullptr;
    }
    return fromWindow(window);
}

ShellSurface *ShellSurface::get(wl_shell_surface *native)
{
    auto it = std::find_if(Private::s_surfaces.constBegin(), Private::s_surfaces.constEnd(), [native](ShellSurface *s) {
        return s->d->surface == native;
    });
    if (it != Private::s_surfaces.constEnd()) {
        return *(it);
    }
    return nullptr;
}

ShellSurface::ShellSurface(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    Private::s_surfaces << this;
}

ShellSurface::~ShellSurface()
{
    Private::s_surfaces.removeOne(this);
    release();
}

void ShellSurface::release()
{
    d->surface.release();
}

void ShellSurface::destroy()
{
    d->surface.destroy();
}

#ifndef K_DOXYGEN
const struct wl_shell_surface_listener ShellSurface::Private::s_listener = {pingCallback, configureCallback, popupDoneCallback};
#endif

void ShellSurface::Private::configureCallback(void *data, wl_shell_surface *shellSurface, uint32_t edges, int32_t width, int32_t height)
{
    Q_UNUSED(edges)
    auto s = reinterpret_cast<ShellSurface::Private *>(data);
    Q_ASSERT(s->surface == shellSurface);
    s->q->setSize(QSize(width, height));
}

void ShellSurface::Private::pingCallback(void *data, wl_shell_surface *shellSurface, uint32_t serial)
{
    auto s = reinterpret_cast<ShellSurface::Private *>(data);
    Q_ASSERT(s->surface == shellSurface);
    s->ping(serial);
}

void ShellSurface::Private::popupDoneCallback(void *data, wl_shell_surface *shellSurface)
{
    auto s = reinterpret_cast<ShellSurface::Private *>(data);
    Q_ASSERT(s->surface == shellSurface);
    Q_EMIT s->q->popupDone();
}

void ShellSurface::setup(wl_shell_surface *surface)
{
    d->setup(surface);
}

void ShellSurface::Private::ping(uint32_t serial)
{
    wl_shell_surface_pong(surface, serial);
    Q_EMIT q->pinged();
}

void ShellSurface::setSize(const QSize &size)
{
    if (d->size == size) {
        return;
    }
    d->size = size;
    Q_EMIT sizeChanged(size);
}

void ShellSurface::setFullscreen(Output *output)
{
    Q_ASSERT(isValid());
    wl_shell_surface_set_fullscreen(d->surface, WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT, 0, output ? output->output() : nullptr);
}

void ShellSurface::setMaximized(Output *output)
{
    Q_ASSERT(isValid());
    wl_shell_surface_set_maximized(d->surface, output ? output->output() : nullptr);
}

void ShellSurface::setToplevel()
{
    Q_ASSERT(isValid());
    wl_shell_surface_set_toplevel(d->surface);
}

void ShellSurface::setTransient(Surface *parent, const QPoint &offset, TransientFlags flags)
{
    Q_ASSERT(isValid());
    uint32_t wlFlags = 0;
    if (flags.testFlag(TransientFlag::NoFocus)) {
        wlFlags |= WL_SHELL_SURFACE_TRANSIENT_INACTIVE;
    }
    wl_shell_surface_set_transient(d->surface, *parent, offset.x(), offset.y(), wlFlags);
}

void ShellSurface::setTransientPopup(Surface *parent, Seat *grabbedSeat, quint32 grabSerial, const QPoint &offset, TransientFlags flags)
{
    Q_ASSERT(isValid());
    Q_ASSERT(parent);
    Q_ASSERT(grabbedSeat);
    uint32_t wlFlags = 0;
    if (flags.testFlag(TransientFlag::NoFocus)) {
        wlFlags |= WL_SHELL_SURFACE_TRANSIENT_INACTIVE;
    }
    wl_shell_surface_set_popup(d->surface, *grabbedSeat, grabSerial, *parent, offset.x(), offset.y(), wlFlags);
}

void ShellSurface::requestMove(Seat *seat, quint32 serial)
{
    Q_ASSERT(isValid());
    Q_ASSERT(seat);

    wl_shell_surface_move(d->surface, *seat, serial);
}

void ShellSurface::requestResize(Seat *seat, quint32 serial, Qt::Edges edges)
{
    Q_ASSERT(isValid());
    Q_ASSERT(seat);

    uint wlEdge = WL_SHELL_SURFACE_RESIZE_NONE;
    if (edges.testFlag(Qt::TopEdge)) {
        if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::TopEdge)) {
            wlEdge = WL_SHELL_SURFACE_RESIZE_TOP_LEFT;
        } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::TopEdge)) {
            wlEdge = WL_SHELL_SURFACE_RESIZE_TOP_RIGHT;
        } else if ((edges & ~Qt::TopEdge) == Qt::Edges()) {
            wlEdge = WL_SHELL_SURFACE_RESIZE_TOP;
        }
    } else if (edges.testFlag(Qt::BottomEdge)) {
        if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::BottomEdge)) {
            wlEdge = WL_SHELL_SURFACE_RESIZE_BOTTOM_LEFT;
        } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::BottomEdge)) {
            wlEdge = WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT;
        } else if ((edges & ~Qt::BottomEdge) == Qt::Edges()) {
            wlEdge = WL_SHELL_SURFACE_RESIZE_BOTTOM;
        }
    } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::Edges())) {
        wlEdge = WL_SHELL_SURFACE_RESIZE_RIGHT;
    } else if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::Edges())) {
        wlEdge = WL_SHELL_SURFACE_RESIZE_LEFT;
    }

    wl_shell_surface_resize(d->surface, *seat, serial, wlEdge);
}

void ShellSurface::setTitle(const QString &title)
{
    wl_shell_surface_set_title(d->surface, title.toUtf8().constData());
}

void ShellSurface::setWindowClass(const QByteArray &windowClass)
{
    wl_shell_surface_set_class(d->surface, windowClass.constData());
}

QSize ShellSurface::size() const
{
    return d->size;
}

bool ShellSurface::isValid() const
{
    return d->surface.isValid();
}

ShellSurface::operator wl_shell_surface *()
{
    return d->surface;
}

ShellSurface::operator wl_shell_surface *() const
{
    return d->surface;
}

}
}
