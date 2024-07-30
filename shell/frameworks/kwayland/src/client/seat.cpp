/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "seat.h"
#include "event_queue.h"
#include "keyboard.h"
#include "pointer.h"
#include "touch.h"
#include "wayland_pointer_p.h"
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN Seat::Private
{
public:
    Private(Seat *q);
    void resetSeat();
    void setup(wl_seat *seat);

    WaylandPointer<wl_seat, wl_seat_destroy> seat;
    EventQueue *queue = nullptr;
    bool capabilityKeyboard = false;
    bool capabilityPointer = false;
    bool capabilityTouch = false;
    QString name;

private:
    void setHasKeyboard(bool has);
    void setHasPointer(bool has);
    void setHasTouch(bool has);
    void capabilitiesChanged(uint32_t capabilities);
    void setName(const QString &name);
    static void capabilitiesCallback(void *data, wl_seat *seat, uint32_t capabilities);
    static void nameCallback(void *data, wl_seat *wl_seat, const char *name);

    Seat *q;
    static const wl_seat_listener s_listener;
};

Seat::Private::Private(Seat *q)
    : q(q)
{
}

void Seat::Private::setup(wl_seat *s)
{
    Q_ASSERT(s);
    Q_ASSERT(!seat);
    seat.setup(s);
    wl_seat_add_listener(seat, &s_listener, this);
}

const wl_seat_listener Seat::Private::s_listener = {capabilitiesCallback, nameCallback};

Seat::Seat(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Seat::~Seat()
{
    release();
}

void Seat::release()
{
    if (!d->seat) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->seat.release();
    d->resetSeat();
}

void Seat::destroy()
{
    if (!d->seat) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->seat.destroy();
    d->resetSeat();
}

void Seat::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *Seat::eventQueue()
{
    return d->queue;
}

void Seat::Private::resetSeat()
{
    setHasKeyboard(false);
    setHasPointer(false);
    setHasTouch(false);
    setName(QString());
}

void Seat::Private::setHasKeyboard(bool has)
{
    if (capabilityKeyboard == has) {
        return;
    }
    capabilityKeyboard = has;
    Q_EMIT q->hasKeyboardChanged(capabilityKeyboard);
}

void Seat::Private::setHasPointer(bool has)
{
    if (capabilityPointer == has) {
        return;
    }
    capabilityPointer = has;
    Q_EMIT q->hasPointerChanged(capabilityPointer);
}

void Seat::Private::setHasTouch(bool has)
{
    if (capabilityTouch == has) {
        return;
    }
    capabilityTouch = has;
    Q_EMIT q->hasTouchChanged(capabilityTouch);
}

void Seat::setup(wl_seat *seat)
{
    d->setup(seat);
}

void Seat::Private::capabilitiesCallback(void *data, wl_seat *seat, uint32_t capabilities)
{
    auto s = reinterpret_cast<Seat::Private *>(data);
    Q_ASSERT(s->seat == seat);
    s->capabilitiesChanged(capabilities);
}

void Seat::Private::nameCallback(void *data, wl_seat *seat, const char *name)
{
    auto s = reinterpret_cast<Seat::Private *>(data);
    Q_ASSERT(s->seat == seat);
    s->setName(QString::fromUtf8(name));
}

void Seat::Private::capabilitiesChanged(uint32_t capabilities)
{
    setHasKeyboard(capabilities & WL_SEAT_CAPABILITY_KEYBOARD);
    setHasPointer(capabilities & WL_SEAT_CAPABILITY_POINTER);
    setHasTouch(capabilities & WL_SEAT_CAPABILITY_TOUCH);
}

Keyboard *Seat::createKeyboard(QObject *parent)
{
    Q_ASSERT(isValid());
    Q_ASSERT(d->capabilityKeyboard);
    Keyboard *k = new Keyboard(parent);
    connect(this, &Seat::interfaceAboutToBeReleased, k, &Keyboard::release);
    connect(this, &Seat::interfaceAboutToBeDestroyed, k, &Keyboard::destroy);
    auto w = wl_seat_get_keyboard(d->seat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    k->setup(w);
    return k;
}

Pointer *Seat::createPointer(QObject *parent)
{
    Q_ASSERT(isValid());
    Q_ASSERT(d->capabilityPointer);
    Pointer *p = new Pointer(parent);
    connect(this, &Seat::interfaceAboutToBeReleased, p, &Pointer::release);
    connect(this, &Seat::interfaceAboutToBeDestroyed, p, &Pointer::destroy);
    auto w = wl_seat_get_pointer(d->seat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

Touch *Seat::createTouch(QObject *parent)
{
    Q_ASSERT(isValid());
    Q_ASSERT(d->capabilityTouch);
    Touch *t = new Touch(parent);
    connect(this, &Seat::interfaceAboutToBeReleased, t, &Touch::release);
    connect(this, &Seat::interfaceAboutToBeDestroyed, t, &Touch::destroy);
    auto w = wl_seat_get_touch(d->seat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    t->setup(w);
    return t;
}

void Seat::Private::setName(const QString &n)
{
    if (name == n) {
        return;
    }
    name = n;
    Q_EMIT q->nameChanged(name);
}

bool Seat::isValid() const
{
    return d->seat.isValid();
}

bool Seat::hasKeyboard() const
{
    return d->capabilityKeyboard;
}

bool Seat::hasPointer() const
{
    return d->capabilityPointer;
}

bool Seat::hasTouch() const
{
    return d->capabilityTouch;
}

QString Seat::name() const
{
    return d->name;
}

Seat::operator wl_seat *()
{
    return d->seat;
}

Seat::operator wl_seat *() const
{
    return d->seat;
}

}
}

#include "moc_seat.cpp"
