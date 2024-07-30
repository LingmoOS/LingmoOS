/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "relativepointer.h"
#include "event_queue.h"
#include "pointer.h"
#include "wayland_pointer_p.h"
#include <QSizeF>
#include <wayland-relativepointer-unstable-v1-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN RelativePointerManager::Private
{
public:
    Private() = default;

    WaylandPointer<zwp_relative_pointer_manager_v1, zwp_relative_pointer_manager_v1_destroy> relativepointermanagerunstablev1;
    EventQueue *queue = nullptr;
};

RelativePointerManager::RelativePointerManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

RelativePointerManager::~RelativePointerManager()
{
    release();
}

void RelativePointerManager::setup(zwp_relative_pointer_manager_v1 *relativepointermanagerunstablev1)
{
    Q_ASSERT(relativepointermanagerunstablev1);
    Q_ASSERT(!d->relativepointermanagerunstablev1);
    d->relativepointermanagerunstablev1.setup(relativepointermanagerunstablev1);
}

void RelativePointerManager::release()
{
    d->relativepointermanagerunstablev1.release();
}

void RelativePointerManager::destroy()
{
    d->relativepointermanagerunstablev1.destroy();
}

void RelativePointerManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *RelativePointerManager::eventQueue()
{
    return d->queue;
}

RelativePointerManager::operator zwp_relative_pointer_manager_v1 *()
{
    return d->relativepointermanagerunstablev1;
}

RelativePointerManager::operator zwp_relative_pointer_manager_v1 *() const
{
    return d->relativepointermanagerunstablev1;
}

bool RelativePointerManager::isValid() const
{
    return d->relativepointermanagerunstablev1.isValid();
}

RelativePointer *RelativePointerManager::createRelativePointer(Pointer *pointer, QObject *parent)
{
    Q_ASSERT(isValid());
    RelativePointer *p = new RelativePointer(parent);
    auto w = zwp_relative_pointer_manager_v1_get_relative_pointer(d->relativepointermanagerunstablev1, *pointer);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

class Q_DECL_HIDDEN RelativePointer::Private
{
public:
    Private(RelativePointer *q);

    void setup(zwp_relative_pointer_v1 *relativepointerunstablev1);

    WaylandPointer<zwp_relative_pointer_v1, zwp_relative_pointer_v1_destroy> relativepointerunstablev1;

private:
    static void relativeMotionCallback(void *data,
                                       zwp_relative_pointer_v1 *zwp_relative_pointer_v1,
                                       uint32_t utime_hi,
                                       uint32_t utime_lo,
                                       wl_fixed_t dx,
                                       wl_fixed_t dy,
                                       wl_fixed_t dx_unaccel,
                                       wl_fixed_t dy_unaccel);

    RelativePointer *q;

    static const zwp_relative_pointer_v1_listener s_listener;
};

RelativePointer::Private::Private(RelativePointer *q)
    : q(q)
{
}

const zwp_relative_pointer_v1_listener RelativePointer::Private::s_listener = {relativeMotionCallback};

void RelativePointer::Private::relativeMotionCallback(void *data,
                                                      zwp_relative_pointer_v1 *zwp_relative_pointer_v1,
                                                      uint32_t utime_hi,
                                                      uint32_t utime_lo,
                                                      wl_fixed_t dx,
                                                      wl_fixed_t dy,
                                                      wl_fixed_t dx_unaccel,
                                                      wl_fixed_t dy_unaccel)
{
    auto p = reinterpret_cast<RelativePointer::Private *>(data);
    Q_ASSERT(p->relativepointerunstablev1 == zwp_relative_pointer_v1);
    const QSizeF delta(wl_fixed_to_double(dx), wl_fixed_to_double(dy));
    const QSizeF deltaNonAccel(wl_fixed_to_double(dx_unaccel), wl_fixed_to_double(dy_unaccel));
    const quint64 timestamp = quint64(utime_lo) | (quint64(utime_hi) << 32);
    Q_EMIT p->q->relativeMotion(delta, deltaNonAccel, timestamp);
}

void RelativePointer::Private::setup(zwp_relative_pointer_v1 *v1)
{
    Q_ASSERT(v1);
    Q_ASSERT(!relativepointerunstablev1);
    relativepointerunstablev1.setup(v1);
    zwp_relative_pointer_v1_add_listener(relativepointerunstablev1, &s_listener, this);
}

RelativePointer::RelativePointer(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

RelativePointer::~RelativePointer()
{
    release();
}

void RelativePointer::setup(zwp_relative_pointer_v1 *relativepointerunstablev1)
{
    d->setup(relativepointerunstablev1);
}

void RelativePointer::release()
{
    d->relativepointerunstablev1.release();
}

void RelativePointer::destroy()
{
    d->relativepointerunstablev1.destroy();
}

RelativePointer::operator zwp_relative_pointer_v1 *()
{
    return d->relativepointerunstablev1;
}

RelativePointer::operator zwp_relative_pointer_v1 *() const
{
    return d->relativepointerunstablev1;
}

bool RelativePointer::isValid() const
{
    return d->relativepointerunstablev1.isValid();
}

}
}

#include "moc_relativepointer.cpp"
