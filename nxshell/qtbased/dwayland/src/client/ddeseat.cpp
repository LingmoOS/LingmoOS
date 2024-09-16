// Copyright 2020  wugang <wugang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "ddeseat.h"
#include "ddekeyboard.h"
#include "event_queue.h"
#include "logging.h"
#include "wayland_pointer_p.h"
// Qt
#include <QDebug>
#include <QVector>
#include <QPointF>
#include <QPointer>
// wayland
#include "wayland-dde-seat-client-protocol.h"
#include <wayland-client-protocol.h>

namespace KWayland
{

namespace Client
{

class Q_DECL_HIDDEN DDESeat::Private
{
public:
    Private(DDESeat *q);
    void setup(dde_seat *o);

    WaylandPointer<dde_seat, dde_seat_destroy> ddeSeat;
    EventQueue *queue = nullptr;

private:
    DDESeat *q;
};

DDESeat::Private::Private(DDESeat *q)
    : q(q)
{
}

void DDESeat::Private::setup(dde_seat *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!ddeSeat);
    ddeSeat.setup(o);
}

DDESeat::DDESeat(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DDESeat::~DDESeat()
{
    release();
}

void DDESeat::destroy()
{
    if (!d->ddeSeat) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->ddeSeat.destroy();
}

void DDESeat::release()
{
    if (!d->ddeSeat) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->ddeSeat.release();
}

void DDESeat::setup(dde_seat *ddeSeat)
{
    d->setup(ddeSeat);
}

EventQueue *DDESeat::eventQueue() const
{
    return d->queue;
}

void DDESeat::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

DDEPointer *DDESeat::createDDePointer(QObject *parent)
{
    Q_ASSERT(isValid());

    DDEPointer *s = new DDEPointer(parent);
    connect(this, &DDESeat::interfaceAboutToBeReleased, s, &DDEPointer::release);
    connect(this, &DDESeat::interfaceAboutToBeDestroyed, s, &DDEPointer::destroy);
    auto w = dde_seat_get_dde_pointer(d->ddeSeat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

DDEKeyboard *DDESeat::createDDEKeyboard(QObject *parent)
{
    Q_ASSERT(isValid());

    DDEKeyboard *s = new DDEKeyboard(parent);
    connect(this, &DDESeat::interfaceAboutToBeReleased, s, &DDEKeyboard::release);
    connect(this, &DDESeat::interfaceAboutToBeDestroyed, s, &DDEKeyboard::destroy);
    auto w = dde_seat_get_dde_keyboard(d->ddeSeat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

DDETouch *DDESeat::createDDETouch(QObject *parent)
{
    Q_ASSERT(isValid());

    DDETouch *s = new DDETouch(parent);
    connect(this, &DDESeat::interfaceAboutToBeReleased, s, &DDETouch::release);
    connect(this, &DDESeat::interfaceAboutToBeDestroyed, s, &DDETouch::destroy);
    auto w = dde_seat_get_dde_touch(d->ddeSeat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

bool DDESeat::isValid() const
{
    return d->ddeSeat.isValid();
}

DDESeat::operator dde_seat*() {
    return d->ddeSeat;
}

DDESeat::operator dde_seat*() const {
    return d->ddeSeat;
}


class Q_DECL_HIDDEN DDEPointer::Private
{
public:
    Private(DDEPointer *q);
    void setup(dde_pointer *p);

    WaylandPointer<dde_pointer, dde_pointer_destroy> ddePointer;
    QPointF globalPointerPos;
private:
    static void motionCallback(void *data, dde_pointer *ddePointer, wl_fixed_t sx, wl_fixed_t sy);
    static void buttonCallback(void *data, dde_pointer *ddePointer, wl_fixed_t sx, wl_fixed_t sy, uint32_t button, uint32_t state);
    static void axisCallback(void *data, dde_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value);

    DDEPointer *q;
    static const dde_pointer_listener s_listener;
};

DDEPointer::Private::Private(DDEPointer *q)
    : q(q)
{
}

void DDEPointer::Private::setup(dde_pointer *p)
{
    Q_ASSERT(p);
    Q_ASSERT(!ddePointer);
    ddePointer.setup(p);
    dde_pointer_add_listener(ddePointer, &s_listener, this);
}

const dde_pointer_listener DDEPointer::Private::s_listener = {
    motionCallback,
    buttonCallback,
    axisCallback,
};

DDEPointer::DDEPointer(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DDEPointer::~DDEPointer()
{
    release();
}

void DDEPointer::release()
{
    d->ddePointer.release();
}

void DDEPointer::destroy()
{
    d->ddePointer.destroy();
}

void DDEPointer::setup(dde_pointer *ddePointer)
{
    d->setup(ddePointer);
}

void DDEPointer::Private::motionCallback(void *data, dde_pointer *ddePointer, wl_fixed_t sx, wl_fixed_t sy)
{
    auto p = reinterpret_cast<DDEPointer::Private*>(data);
    Q_ASSERT(p->ddePointer == ddePointer);
    p->globalPointerPos = QPointF(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
    Q_EMIT p->q->motion(p->globalPointerPos);
}

void DDEPointer::Private::buttonCallback(void *data, dde_pointer *ddePointer, wl_fixed_t sx, wl_fixed_t sy, uint32_t button, uint32_t state)
{
    auto p = reinterpret_cast<DDEPointer::Private*>(data);
    Q_ASSERT(p->ddePointer == ddePointer);
    auto toState = [state] {
        if (state == DDE_POINTER_BUTTON_STATE_RELEASED) {
            return ButtonState::Released;
        } else {
            return ButtonState::Pressed;
        }
    };
    p->globalPointerPos = QPointF(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
    Q_EMIT p->q->buttonStateChanged(p->globalPointerPos, button, toState());
}

void DDEPointer::Private::axisCallback(void *data, dde_pointer *ddePointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
    auto p = reinterpret_cast<DDEPointer::Private*>(data);
    Q_ASSERT(p->ddePointer == ddePointer);
    auto toAxis = [axis] {
        if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
            return Axis::Horizontal;
        } else {
            return Axis::Vertical;
        }
    };
    Q_EMIT p->q->axisChanged(time, toAxis(), wl_fixed_to_double(value));
}

bool DDEPointer::isValid() const
{
    return d->ddePointer.isValid();
}

DDEPointer::operator dde_pointer*() const
{
    return d->ddePointer;
}

DDEPointer::operator dde_pointer*()
{
    return d->ddePointer;
}

void DDEPointer::getMotion()
{
    dde_pointer_get_motion(d->ddePointer);
}

QPointF DDEPointer::getGlobalPointerPos()
{
    return d->globalPointerPos;
}

//DDETouch
class Q_DECL_HIDDEN DDETouch::Private
{
public:
    Private(DDETouch *q);
    void setup(dde_touch *k);

    WaylandPointer<dde_touch, dde_touch_release> ddeTouch;

private:
    static void downCallback(void *data, struct dde_touch *touch, int32_t id, uint32_t time, wl_fixed_t x, wl_fixed_t y);
    static void motionCallback(void *data, struct dde_touch *touch, int32_t id, uint32_t time, wl_fixed_t x, wl_fixed_t y);
    static void upCallback(void *data, struct dde_touch *touch, int32_t id, uint32_t time);
    DDETouch *q;
    static const dde_touch_listener s_listener;
};

DDETouch::Private::Private(DDETouch *q)
    : q(q)
{
}

void DDETouch::Private::setup(dde_touch *t)
{
    Q_ASSERT(t);
    Q_ASSERT(!ddeTouch);
    ddeTouch.setup(t);
    dde_touch_add_listener(ddeTouch, &s_listener, this);
}

const dde_touch_listener DDETouch::Private::s_listener = {
    downCallback,
    upCallback,
    motionCallback,
};

DDETouch::DDETouch(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DDETouch::~DDETouch()
{
    release();
}

void DDETouch::release()
{
    d->ddeTouch.release();
}

void DDETouch::destroy()
{
    d->ddeTouch.destroy();
}

void DDETouch::setup(dde_touch *keyboard)
{
    d->setup(keyboard);
}

bool DDETouch::isValid() const
{
    return d->ddeTouch.isValid();
}

DDETouch::operator dde_touch*()
{
    return d->ddeTouch;
}

DDETouch::operator dde_touch*() const
{
    return d->ddeTouch;
}

void DDETouch::Private::downCallback(void *data, struct dde_touch *touch, int32_t id, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    auto t = reinterpret_cast<DDETouch::Private*>(data);
    Q_ASSERT(t->ddeTouch == touch);
    Q_EMIT t->q->touchDown(id, QPointF(wl_fixed_to_double(x), wl_fixed_to_double(y)));
}

void DDETouch::Private::motionCallback(void *data, struct dde_touch *touch, int32_t id, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    auto t = reinterpret_cast<DDETouch::Private*>(data);
    Q_ASSERT(t->ddeTouch == touch);
    Q_EMIT t->q->touchMotion(id, QPointF(wl_fixed_to_double(x), wl_fixed_to_double(y)));
}

void DDETouch::Private::upCallback(void *data, struct dde_touch *touch, int32_t id, uint32_t time)
{
    auto t = reinterpret_cast<DDETouch::Private*>(data);
    Q_ASSERT(t->ddeTouch == touch);
    Q_EMIT t->q->touchUp(id);
}

}
}
