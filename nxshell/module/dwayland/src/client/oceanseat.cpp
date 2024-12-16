// Copyright 2020  wugang <wugang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "oceanseat.h"
#include "oceankeyboard.h"
#include "event_queue.h"
#include "logging.h"
#include "wayland_pointer_p.h"
// Qt
#include <QDebug>
#include <QVector>
#include <QPointF>
#include <QPointer>
// wayland
#include "wayland-ocean-seat-client-protocol.h"
#include <wayland-client-protocol.h>

namespace KWayland
{

namespace Client
{

class Q_DECL_HIDDEN OCEANSeat::Private
{
public:
    Private(OCEANSeat *q);
    void setup(ocean_seat *o);

    WaylandPointer<ocean_seat, ocean_seat_destroy> oceanSeat;
    EventQueue *queue = nullptr;

private:
    OCEANSeat *q;
};

OCEANSeat::Private::Private(OCEANSeat *q)
    : q(q)
{
}

void OCEANSeat::Private::setup(ocean_seat *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!oceanSeat);
    oceanSeat.setup(o);
}

OCEANSeat::OCEANSeat(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

OCEANSeat::~OCEANSeat()
{
    release();
}

void OCEANSeat::destroy()
{
    if (!d->oceanSeat) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->oceanSeat.destroy();
}

void OCEANSeat::release()
{
    if (!d->oceanSeat) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->oceanSeat.release();
}

void OCEANSeat::setup(ocean_seat *oceanSeat)
{
    d->setup(oceanSeat);
}

EventQueue *OCEANSeat::eventQueue() const
{
    return d->queue;
}

void OCEANSeat::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

OCEANPointer *OCEANSeat::createDDePointer(QObject *parent)
{
    Q_ASSERT(isValid());

    OCEANPointer *s = new OCEANPointer(parent);
    connect(this, &OCEANSeat::interfaceAboutToBeReleased, s, &OCEANPointer::release);
    connect(this, &OCEANSeat::interfaceAboutToBeDestroyed, s, &OCEANPointer::destroy);
    auto w = ocean_seat_get_ocean_pointer(d->oceanSeat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

OCEANKeyboard *OCEANSeat::createOCEANKeyboard(QObject *parent)
{
    Q_ASSERT(isValid());

    OCEANKeyboard *s = new OCEANKeyboard(parent);
    connect(this, &OCEANSeat::interfaceAboutToBeReleased, s, &OCEANKeyboard::release);
    connect(this, &OCEANSeat::interfaceAboutToBeDestroyed, s, &OCEANKeyboard::destroy);
    auto w = ocean_seat_get_ocean_keyboard(d->oceanSeat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

OCEANTouch *OCEANSeat::createOCEANTouch(QObject *parent)
{
    Q_ASSERT(isValid());

    OCEANTouch *s = new OCEANTouch(parent);
    connect(this, &OCEANSeat::interfaceAboutToBeReleased, s, &OCEANTouch::release);
    connect(this, &OCEANSeat::interfaceAboutToBeDestroyed, s, &OCEANTouch::destroy);
    auto w = ocean_seat_get_ocean_touch(d->oceanSeat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

bool OCEANSeat::isValid() const
{
    return d->oceanSeat.isValid();
}

OCEANSeat::operator ocean_seat*() {
    return d->oceanSeat;
}

OCEANSeat::operator ocean_seat*() const {
    return d->oceanSeat;
}


class Q_DECL_HIDDEN OCEANPointer::Private
{
public:
    Private(OCEANPointer *q);
    void setup(ocean_pointer *p);

    WaylandPointer<ocean_pointer, ocean_pointer_destroy> oceanPointer;
    QPointF globalPointerPos;
private:
    static void motionCallback(void *data, ocean_pointer *oceanPointer, wl_fixed_t sx, wl_fixed_t sy);
    static void buttonCallback(void *data, ocean_pointer *oceanPointer, wl_fixed_t sx, wl_fixed_t sy, uint32_t button, uint32_t state);
    static void axisCallback(void *data, ocean_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value);

    OCEANPointer *q;
    static const ocean_pointer_listener s_listener;
};

OCEANPointer::Private::Private(OCEANPointer *q)
    : q(q)
{
}

void OCEANPointer::Private::setup(ocean_pointer *p)
{
    Q_ASSERT(p);
    Q_ASSERT(!oceanPointer);
    oceanPointer.setup(p);
    ocean_pointer_add_listener(oceanPointer, &s_listener, this);
}

const ocean_pointer_listener OCEANPointer::Private::s_listener = {
    motionCallback,
    buttonCallback,
    axisCallback,
};

OCEANPointer::OCEANPointer(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

OCEANPointer::~OCEANPointer()
{
    release();
}

void OCEANPointer::release()
{
    d->oceanPointer.release();
}

void OCEANPointer::destroy()
{
    d->oceanPointer.destroy();
}

void OCEANPointer::setup(ocean_pointer *oceanPointer)
{
    d->setup(oceanPointer);
}

void OCEANPointer::Private::motionCallback(void *data, ocean_pointer *oceanPointer, wl_fixed_t sx, wl_fixed_t sy)
{
    auto p = reinterpret_cast<OCEANPointer::Private*>(data);
    Q_ASSERT(p->oceanPointer == oceanPointer);
    p->globalPointerPos = QPointF(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
    Q_EMIT p->q->motion(p->globalPointerPos);
}

void OCEANPointer::Private::buttonCallback(void *data, ocean_pointer *oceanPointer, wl_fixed_t sx, wl_fixed_t sy, uint32_t button, uint32_t state)
{
    auto p = reinterpret_cast<OCEANPointer::Private*>(data);
    Q_ASSERT(p->oceanPointer == oceanPointer);
    auto toState = [state] {
        if (state == OCEAN_POINTER_BUTTON_STATE_RELEASED) {
            return ButtonState::Released;
        } else {
            return ButtonState::Pressed;
        }
    };
    p->globalPointerPos = QPointF(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
    Q_EMIT p->q->buttonStateChanged(p->globalPointerPos, button, toState());
}

void OCEANPointer::Private::axisCallback(void *data, ocean_pointer *oceanPointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
    auto p = reinterpret_cast<OCEANPointer::Private*>(data);
    Q_ASSERT(p->oceanPointer == oceanPointer);
    auto toAxis = [axis] {
        if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
            return Axis::Horizontal;
        } else {
            return Axis::Vertical;
        }
    };
    Q_EMIT p->q->axisChanged(time, toAxis(), wl_fixed_to_double(value));
}

bool OCEANPointer::isValid() const
{
    return d->oceanPointer.isValid();
}

OCEANPointer::operator ocean_pointer*() const
{
    return d->oceanPointer;
}

OCEANPointer::operator ocean_pointer*()
{
    return d->oceanPointer;
}

void OCEANPointer::getMotion()
{
    ocean_pointer_get_motion(d->oceanPointer);
}

QPointF OCEANPointer::getGlobalPointerPos()
{
    return d->globalPointerPos;
}

//OCEANTouch
class Q_DECL_HIDDEN OCEANTouch::Private
{
public:
    Private(OCEANTouch *q);
    void setup(ocean_touch *k);

    WaylandPointer<ocean_touch, ocean_touch_release> oceanTouch;

private:
    static void downCallback(void *data, struct ocean_touch *touch, int32_t id, uint32_t time, wl_fixed_t x, wl_fixed_t y);
    static void motionCallback(void *data, struct ocean_touch *touch, int32_t id, uint32_t time, wl_fixed_t x, wl_fixed_t y);
    static void upCallback(void *data, struct ocean_touch *touch, int32_t id, uint32_t time);
    OCEANTouch *q;
    static const ocean_touch_listener s_listener;
};

OCEANTouch::Private::Private(OCEANTouch *q)
    : q(q)
{
}

void OCEANTouch::Private::setup(ocean_touch *t)
{
    Q_ASSERT(t);
    Q_ASSERT(!oceanTouch);
    oceanTouch.setup(t);
    ocean_touch_add_listener(oceanTouch, &s_listener, this);
}

const ocean_touch_listener OCEANTouch::Private::s_listener = {
    downCallback,
    upCallback,
    motionCallback,
};

OCEANTouch::OCEANTouch(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

OCEANTouch::~OCEANTouch()
{
    release();
}

void OCEANTouch::release()
{
    d->oceanTouch.release();
}

void OCEANTouch::destroy()
{
    d->oceanTouch.destroy();
}

void OCEANTouch::setup(ocean_touch *keyboard)
{
    d->setup(keyboard);
}

bool OCEANTouch::isValid() const
{
    return d->oceanTouch.isValid();
}

OCEANTouch::operator ocean_touch*()
{
    return d->oceanTouch;
}

OCEANTouch::operator ocean_touch*() const
{
    return d->oceanTouch;
}

void OCEANTouch::Private::downCallback(void *data, struct ocean_touch *touch, int32_t id, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    auto t = reinterpret_cast<OCEANTouch::Private*>(data);
    Q_ASSERT(t->oceanTouch == touch);
    Q_EMIT t->q->touchDown(id, QPointF(wl_fixed_to_double(x), wl_fixed_to_double(y)));
}

void OCEANTouch::Private::motionCallback(void *data, struct ocean_touch *touch, int32_t id, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    auto t = reinterpret_cast<OCEANTouch::Private*>(data);
    Q_ASSERT(t->oceanTouch == touch);
    Q_EMIT t->q->touchMotion(id, QPointF(wl_fixed_to_double(x), wl_fixed_to_double(y)));
}

void OCEANTouch::Private::upCallback(void *data, struct ocean_touch *touch, int32_t id, uint32_t time)
{
    auto t = reinterpret_cast<OCEANTouch::Private*>(data);
    Q_ASSERT(t->oceanTouch == touch);
    Q_EMIT t->q->touchUp(id);
}

}
}
