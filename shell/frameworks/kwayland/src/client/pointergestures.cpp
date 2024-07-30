/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "pointergestures.h"
#include "event_queue.h"
#include "pointer.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <wayland-pointer-gestures-unstable-v1-client-protocol.h>

#include <QSizeF>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN PointerGestures::Private
{
public:
    Private() = default;

    WaylandPointer<zwp_pointer_gestures_v1, zwp_pointer_gestures_v1_destroy> pointergestures;
    EventQueue *queue = nullptr;
};

PointerGestures::PointerGestures(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

PointerGestures::~PointerGestures()
{
    release();
}

void PointerGestures::setup(zwp_pointer_gestures_v1 *pointergestures)
{
    Q_ASSERT(pointergestures);
    Q_ASSERT(!d->pointergestures);
    d->pointergestures.setup(pointergestures);
}

void PointerGestures::release()
{
    d->pointergestures.release();
}

void PointerGestures::destroy()
{
    d->pointergestures.destroy();
}

PointerGestures::operator zwp_pointer_gestures_v1 *()
{
    return d->pointergestures;
}

PointerGestures::operator zwp_pointer_gestures_v1 *() const
{
    return d->pointergestures;
}

bool PointerGestures::isValid() const
{
    return d->pointergestures.isValid();
}

void PointerGestures::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *PointerGestures::eventQueue()
{
    return d->queue;
}

PointerSwipeGesture *PointerGestures::createSwipeGesture(Pointer *pointer, QObject *parent)
{
    Q_ASSERT(isValid());
    PointerSwipeGesture *p = new PointerSwipeGesture(parent);
    auto w = zwp_pointer_gestures_v1_get_swipe_gesture(d->pointergestures, *pointer);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

PointerPinchGesture *PointerGestures::createPinchGesture(Pointer *pointer, QObject *parent)
{
    Q_ASSERT(isValid());
    PointerPinchGesture *p = new PointerPinchGesture(parent);
    auto w = zwp_pointer_gestures_v1_get_pinch_gesture(d->pointergestures, *pointer);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

class Q_DECL_HIDDEN PointerSwipeGesture::Private
{
public:
    Private(PointerSwipeGesture *q);

    void setup(zwp_pointer_gesture_swipe_v1 *pg);

    WaylandPointer<zwp_pointer_gesture_swipe_v1, zwp_pointer_gesture_swipe_v1_destroy> pointerswipegesture;
    quint32 fingerCount = 0;
    QPointer<Surface> surface;

private:
    static void beginCallback(void *data,
                              zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1,
                              uint32_t serial,
                              uint32_t time,
                              wl_surface *surface,
                              uint32_t fingers);
    static void updateCallback(void *data, zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1, uint32_t time, wl_fixed_t dx, wl_fixed_t dy);
    static void endCallback(void *data, zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1, uint32_t serial, uint32_t time, int32_t cancelled);

    PointerSwipeGesture *q;
    static const zwp_pointer_gesture_swipe_v1_listener s_listener;
};

const zwp_pointer_gesture_swipe_v1_listener PointerSwipeGesture::Private::s_listener = {beginCallback, updateCallback, endCallback};

void PointerSwipeGesture::Private::beginCallback(void *data,
                                                 zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1,
                                                 uint32_t serial,
                                                 uint32_t time,
                                                 wl_surface *surface,
                                                 uint32_t fingers)
{
    auto p = reinterpret_cast<PointerSwipeGesture::Private *>(data);
    Q_ASSERT(p->pointerswipegesture == zwp_pointer_gesture_swipe_v1);
    p->fingerCount = fingers;
    p->surface = QPointer<Surface>(Surface::get(surface));
    Q_EMIT p->q->started(serial, time);
}

void PointerSwipeGesture::Private::updateCallback(void *data,
                                                  zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1,
                                                  uint32_t time,
                                                  wl_fixed_t dx,
                                                  wl_fixed_t dy)
{
    auto p = reinterpret_cast<PointerSwipeGesture::Private *>(data);
    Q_ASSERT(p->pointerswipegesture == zwp_pointer_gesture_swipe_v1);
    Q_EMIT p->q->updated(QSizeF(wl_fixed_to_double(dx), wl_fixed_to_double(dy)), time);
}

void PointerSwipeGesture::Private::endCallback(void *data,
                                               zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1,
                                               uint32_t serial,
                                               uint32_t time,
                                               int32_t cancelled)
{
    auto p = reinterpret_cast<PointerSwipeGesture::Private *>(data);
    Q_ASSERT(p->pointerswipegesture == zwp_pointer_gesture_swipe_v1);
    if (cancelled) {
        Q_EMIT p->q->cancelled(serial, time);
    } else {
        Q_EMIT p->q->ended(serial, time);
    }
    p->fingerCount = 0;
    p->surface.clear();
}

PointerSwipeGesture::Private::Private(PointerSwipeGesture *q)
    : q(q)
{
}

PointerSwipeGesture::PointerSwipeGesture(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

PointerSwipeGesture::~PointerSwipeGesture()
{
    release();
}

quint32 PointerSwipeGesture::fingerCount() const
{
    return d->fingerCount;
}

QPointer<Surface> PointerSwipeGesture::surface() const
{
    return d->surface;
}

void PointerSwipeGesture::Private::setup(zwp_pointer_gesture_swipe_v1 *pg)
{
    Q_ASSERT(pg);
    Q_ASSERT(!pointerswipegesture);
    pointerswipegesture.setup(pg);
    zwp_pointer_gesture_swipe_v1_add_listener(pointerswipegesture, &s_listener, this);
}

void PointerSwipeGesture::setup(zwp_pointer_gesture_swipe_v1 *pointerswipegesture)
{
    d->setup(pointerswipegesture);
}

void PointerSwipeGesture::release()
{
    d->pointerswipegesture.release();
}

void PointerSwipeGesture::destroy()
{
    d->pointerswipegesture.destroy();
}

PointerSwipeGesture::operator zwp_pointer_gesture_swipe_v1 *()
{
    return d->pointerswipegesture;
}

PointerSwipeGesture::operator zwp_pointer_gesture_swipe_v1 *() const
{
    return d->pointerswipegesture;
}

bool PointerSwipeGesture::isValid() const
{
    return d->pointerswipegesture.isValid();
}

class Q_DECL_HIDDEN PointerPinchGesture::Private
{
public:
    Private(PointerPinchGesture *q);

    void setup(zwp_pointer_gesture_pinch_v1 *pg);

    WaylandPointer<zwp_pointer_gesture_pinch_v1, zwp_pointer_gesture_pinch_v1_destroy> pointerpinchgesture;
    quint32 fingerCount = 0;
    QPointer<Surface> surface;

private:
    static void beginCallback(void *data,
                              zwp_pointer_gesture_pinch_v1 *zwp_pointer_gesture_pinch_v1,
                              uint32_t serial,
                              uint32_t time,
                              wl_surface *surface,
                              uint32_t fingers);
    static void updateCallback(void *data,
                               zwp_pointer_gesture_pinch_v1 *zwp_pointer_gesture_pinch_v1,
                               uint32_t time,
                               wl_fixed_t dx,
                               wl_fixed_t dy,
                               wl_fixed_t scale,
                               wl_fixed_t rotation);
    static void endCallback(void *data, zwp_pointer_gesture_pinch_v1 *zwp_pointer_gesture_pinch_v1, uint32_t serial, uint32_t time, int32_t cancelled);

    PointerPinchGesture *q;
    static const zwp_pointer_gesture_pinch_v1_listener s_listener;
};

const zwp_pointer_gesture_pinch_v1_listener PointerPinchGesture::Private::s_listener = {beginCallback, updateCallback, endCallback};

void PointerPinchGesture::Private::beginCallback(void *data,
                                                 zwp_pointer_gesture_pinch_v1 *pg,
                                                 uint32_t serial,
                                                 uint32_t time,
                                                 wl_surface *surface,
                                                 uint32_t fingers)
{
    auto p = reinterpret_cast<PointerPinchGesture::Private *>(data);
    Q_ASSERT(p->pointerpinchgesture == pg);
    p->fingerCount = fingers;
    p->surface = QPointer<Surface>(Surface::get(surface));
    Q_EMIT p->q->started(serial, time);
}

void PointerPinchGesture::Private::updateCallback(void *data,
                                                  zwp_pointer_gesture_pinch_v1 *pg,
                                                  uint32_t time,
                                                  wl_fixed_t dx,
                                                  wl_fixed_t dy,
                                                  wl_fixed_t scale,
                                                  wl_fixed_t rotation)
{
    auto p = reinterpret_cast<PointerPinchGesture::Private *>(data);
    Q_ASSERT(p->pointerpinchgesture == pg);
    Q_EMIT p->q->updated(QSizeF(wl_fixed_to_double(dx), wl_fixed_to_double(dy)), wl_fixed_to_double(scale), wl_fixed_to_double(rotation), time);
}

void PointerPinchGesture::Private::endCallback(void *data, zwp_pointer_gesture_pinch_v1 *pg, uint32_t serial, uint32_t time, int32_t cancelled)
{
    auto p = reinterpret_cast<PointerPinchGesture::Private *>(data);
    Q_ASSERT(p->pointerpinchgesture == pg);
    if (cancelled) {
        Q_EMIT p->q->cancelled(serial, time);
    } else {
        Q_EMIT p->q->ended(serial, time);
    }
    p->fingerCount = 0;
    p->surface.clear();
}

PointerPinchGesture::Private::Private(PointerPinchGesture *q)
    : q(q)
{
}

PointerPinchGesture::PointerPinchGesture(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

PointerPinchGesture::~PointerPinchGesture()
{
    release();
}

void PointerPinchGesture::Private::setup(zwp_pointer_gesture_pinch_v1 *pg)
{
    Q_ASSERT(pg);
    Q_ASSERT(!pointerpinchgesture);
    pointerpinchgesture.setup(pg);
    zwp_pointer_gesture_pinch_v1_add_listener(pointerpinchgesture, &s_listener, this);
}

void PointerPinchGesture::setup(zwp_pointer_gesture_pinch_v1 *pointerpinchgesture)
{
    d->setup(pointerpinchgesture);
}

void PointerPinchGesture::release()
{
    d->pointerpinchgesture.release();
}

void PointerPinchGesture::destroy()
{
    d->pointerpinchgesture.destroy();
}

PointerPinchGesture::operator zwp_pointer_gesture_pinch_v1 *()
{
    return d->pointerpinchgesture;
}

PointerPinchGesture::operator zwp_pointer_gesture_pinch_v1 *() const
{
    return d->pointerpinchgesture;
}

bool PointerPinchGesture::isValid() const
{
    return d->pointerpinchgesture.isValid();
}

quint32 PointerPinchGesture::fingerCount() const
{
    return d->fingerCount;
}

QPointer<Surface> PointerPinchGesture::surface() const
{
    return d->surface;
}

}
}

#include "moc_pointergestures.cpp"
