/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "surface.h"
#include "output.h"
#include "region.h"
#include "surface_p.h"
#include "wayland_pointer_p.h"

#include <QGuiApplication>
#include <QList>
#include <QRegion>
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

QList<Surface *> Surface::Private::s_surfaces = QList<Surface *>();

Surface::Private::Private(Surface *q)
    : q(q)
{
}

Surface::Surface(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    Private::s_surfaces << this;
}

Surface::~Surface()
{
    Private::s_surfaces.removeAll(this);
    release();
}


void Surface::release()
{
    d->surface.release();
}

void Surface::destroy()
{
    d->surface.destroy();
}

void Surface::setup(wl_surface *surface)
{
    d->setup(surface);
}

void Surface::Private::setup(wl_surface *s)
{
    Q_ASSERT(s);
    Q_ASSERT(!surface);
    surface.setup(s);
    wl_surface_add_listener(s, &s_surfaceListener, this);
}

void Surface::Private::frameCallback(void *data, wl_callback *callback, uint32_t time)
{
    Q_UNUSED(time)
    auto s = reinterpret_cast<Surface::Private *>(data);
    if (callback) {
        wl_callback_destroy(callback);
    }
    s->handleFrameCallback();
}

void Surface::Private::handleFrameCallback()
{
    frameCallbackInstalled = false;
    Q_EMIT q->frameRendered();
}

#ifndef K_DOXYGEN
const struct wl_callback_listener Surface::Private::s_listener = {frameCallback};

const struct wl_surface_listener Surface::Private::s_surfaceListener = {enterCallback, leaveCallback};
#endif

void Surface::Private::removeOutput(Output *o)
{
    if (o && outputs.removeOne(o)) {
        Q_EMIT q->outputLeft(o);
    }
}

void Surface::Private::enterCallback(void *data, wl_surface *surface, wl_output *output)
{
    Q_UNUSED(surface);
    auto s = reinterpret_cast<Surface::Private *>(data);
    Output *o = Output::get(output);
    if (!o) {
        return;
    }
    s->outputs << o;
    QObject::connect(o, &Output::removed, s->q, [s, o]() {
        s->removeOutput(o);
    });
    Q_EMIT s->q->outputEntered(o);
}

void Surface::Private::leaveCallback(void *data, wl_surface *surface, wl_output *output)
{
    Q_UNUSED(surface);
    auto s = reinterpret_cast<Surface::Private *>(data);
    s->removeOutput(Output::get(output));
}

void Surface::Private::setupFrameCallback()
{
    Q_ASSERT(!frameCallbackInstalled);
    wl_callback *callback = wl_surface_frame(surface);
    wl_callback_add_listener(callback, &s_listener, this);
    frameCallbackInstalled = true;
}

void Surface::setupFrameCallback()
{
    Q_ASSERT(isValid());
    d->setupFrameCallback();
}

void Surface::commit(Surface::CommitFlag flag)
{
    Q_ASSERT(isValid());
    if (flag == CommitFlag::FrameCallback) {
        setupFrameCallback();
    }
    wl_surface_commit(d->surface);
}

void Surface::damage(const QRegion &region)
{
    for (const QRect &rect : region) {
        damage(rect);
    }
}

void Surface::damage(const QRect &rect)
{
    Q_ASSERT(isValid());
    wl_surface_damage(d->surface, rect.x(), rect.y(), rect.width(), rect.height());
}

void Surface::damageBuffer(const QRegion &region)
{
    for (const QRect &r : region) {
        damageBuffer(r);
    }
}

void Surface::damageBuffer(const QRect &rect)
{
    Q_ASSERT(isValid());
    wl_surface_damage_buffer(d->surface, rect.x(), rect.y(), rect.width(), rect.height());
}

void Surface::attachBuffer(wl_buffer *buffer, const QPoint &offset)
{
    Q_ASSERT(isValid());
    wl_surface_attach(d->surface, buffer, offset.x(), offset.y());
}

void Surface::attachBuffer(Buffer *buffer, const QPoint &offset)
{
    attachBuffer(buffer ? buffer->buffer() : nullptr, offset);
}

void Surface::attachBuffer(Buffer::Ptr buffer, const QPoint &offset)
{
    attachBuffer(buffer.toStrongRef().data(), offset);
}

void Surface::setInputRegion(const Region *region)
{
    Q_ASSERT(isValid());
    if (region) {
        wl_surface_set_input_region(d->surface, *region);
    } else {
        wl_surface_set_input_region(d->surface, nullptr);
    }
}

void Surface::setOpaqueRegion(const Region *region)
{
    Q_ASSERT(isValid());
    if (region) {
        wl_surface_set_opaque_region(d->surface, *region);
    } else {
        wl_surface_set_opaque_region(d->surface, nullptr);
    }
}

void Surface::setSize(const QSize &size)
{
    if (d->size == size) {
        return;
    }
    d->size = size;
    Q_EMIT sizeChanged(d->size);
}

Surface *Surface::get(wl_surface *native)
{
    auto it = std::find_if(Private::s_surfaces.constBegin(), Private::s_surfaces.constEnd(), [native](Surface *s) {
        return s->d->surface == native;
    });
    if (it != Private::s_surfaces.constEnd()) {
        return *(it);
    }
    return nullptr;
}

const QList<Surface *> &Surface::all()
{
    return Private::s_surfaces;
}

bool Surface::isValid() const
{
    return d->surface.isValid();
}

QSize Surface::size() const
{
    return d->size;
}

Surface::operator wl_surface *()
{
    return d->surface;
}

Surface::operator wl_surface *() const
{
    return d->surface;
}

quint32 Surface::id() const
{
    wl_surface *s = *this;
    return wl_proxy_get_id(reinterpret_cast<wl_proxy *>(s));
}

qint32 Surface::scale() const
{
    return d->scale;
}

void Surface::setScale(qint32 scale)
{
    d->scale = scale;
    wl_surface_set_buffer_scale(d->surface, scale);
}

QList<Output *> Surface::outputs() const
{
    return d->outputs;
}

}
}

#include "moc_surface.cpp"
