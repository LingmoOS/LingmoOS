// Copyright 2022  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "oceanseat_interface.h"
#include "oceankeyboard_interface.h"
#include "display.h"
#include "logging.h"
#include "utils.h"

#include <QPointF>

#include "oceanseat_interface_p.h"
#include "oceankeyboard_interface_p.h"

namespace KWaylandServer
{
static const int s_version = 1;
static const int s_oceanPointerVersion = 1;
static const int s_oceanTouchVersion = 7;
static const int s_oceanKeyboardVersion = 7;

/*********************************
 * OCEANSeatInterface
 *********************************/
OCEANSeatInterfacePrivate::OCEANSeatInterfacePrivate(OCEANSeatInterface *q, Display *d)
    : QtWaylandServer::ocean_seat(*d, s_version)
    , q(q)
    , display(d)
{
}

OCEANSeatInterfacePrivate *OCEANSeatInterfacePrivate::get(OCEANSeatInterface *oceanseat)
{
    return oceanseat->d.data();
}

void OCEANSeatInterfacePrivate::ocean_seat_get_ocean_pointer(Resource *resource, uint32_t id) {
    if (oceanpointer) {
        OCEANPointerInterfacePrivate *pointerPrivate = OCEANPointerInterfacePrivate::get(oceanpointer.data());
        pointerPrivate->add(resource->client(), id, resource->version());
    } else {
        wl_resource *pointer_resource = wl_resource_create(resource->client(), &ocean_pointer_interface, s_oceanPointerVersion, id);
        oceanpointer.reset(new OCEANPointerInterface(q, pointer_resource));
        Q_EMIT q->oceanPointerCreated(oceanpointer.data());
    }
}

void OCEANSeatInterfacePrivate::ocean_seat_get_ocean_keyboard(Resource *resource, uint32_t id) {
    if (oceankeyboard) {
        OCEANKeyboardInterfacePrivate *keyboardPrivate = OCEANKeyboardInterfacePrivate::get(oceankeyboard.data());
        keyboardPrivate->add(resource->client(), id, resource->version());
    } else {
        wl_resource *keyboard_resource = wl_resource_create(resource->client(), &ocean_keyboard_interface, s_oceanKeyboardVersion, id);
        oceankeyboard.reset(new OCEANKeyboardInterface(q, keyboard_resource));
        Q_EMIT q->oceanKeyboardCreated(oceankeyboard.data());
    }
}

void OCEANSeatInterfacePrivate::ocean_seat_get_ocean_touch(Resource *resource, uint32_t id) {
    if (oceantouch) {
        OCEANTouchInterfacePrivate *touchPrivate = OCEANTouchInterfacePrivate::get(oceantouch.data());
        touchPrivate->add(resource->client(), id, resource->version());
    } else {
        wl_resource *touch_resource = wl_resource_create(resource->client(), &ocean_touch_interface, s_oceanTouchVersion, id);
        oceantouch.reset(new OCEANTouchInterface(q, touch_resource));
        Q_EMIT q->oceanTouchCreated(oceantouch.data());
    }
}

bool OCEANSeatInterfacePrivate::updateKey(quint32 key, Keyboard::State state)
{
    auto it = keys.states.find(key);
    if (it == keys.states.end()) {
        keys.states.insert(key, state);
        return true;
    }
    if (it.value() == state) {
        return false;
    }
    it.value() = state;
    return true;
}

OCEANSeatInterface::OCEANSeatInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new OCEANSeatInterfacePrivate(this, display))
{
}

OCEANSeatInterface::~OCEANSeatInterface() = default;

OCEANSeatInterface *OCEANSeatInterface::get(wl_resource* native)
{
    if (OCEANSeatInterfacePrivate *seatPrivate = resource_cast<OCEANSeatInterfacePrivate *>(native)) {
        return seatPrivate->q;
    }
    return nullptr;
}

QPointF OCEANSeatInterface::pointerPos() const
{
    return d->globalPos;
}

void OCEANSeatInterface::setPointerPos(const QPointF &pos)
{
    if (!d->oceanpointer) {
        return;
    }
    if (d->globalPos == pos) {
        return;
    }
    d->globalPos = pos;
    d->oceanpointer->sendMotion(pos);
}

void OCEANSeatInterface::pointerButtonPressed(quint32 button)
{
    if (!d->oceanpointer) {
        return;
    }
    d->oceanpointer->buttonPressed(button);
}

void OCEANSeatInterface::pointerButtonReleased(quint32 button)
{
    if (!d->oceanpointer) {
        return;
    }
    d->oceanpointer->buttonReleased(button);
}

void OCEANSeatInterface::pointerAxis(Qt::Orientation orientation, qint32 delta)
{
    if (!d->oceanpointer) {
        return;
    }
    d->oceanpointer->axis(orientation, delta);
}

quint32 OCEANSeatInterface::timestamp() const
{
    return d->timestamp;
}

void OCEANSeatInterface::setTimestamp(quint32 time)
{
    if (d->timestamp == time) {
        return;
    }
    d->timestamp = time;
}

quint32 OCEANSeatInterface::touchtimestamp() const
{
    return d->touchtimestamp;
}

void OCEANSeatInterface::setTouchTimestamp(quint32 time)
{
    if (d->touchtimestamp == time) {
        return;
    }
    d->touchtimestamp = time;
}

void OCEANSeatInterface::setKeymap(int fd, quint32 size)
{
    if (!d->oceankeyboard) {
        return;
    }
    d->keys.keymap.xkbcommonCompatible = true;
    d->keys.keymap.fd = fd;
    d->keys.keymap.size = size;

    d->oceankeyboard->setKeymap(fd, size);
}

void OCEANSeatInterface::keyPressed(quint32 key)
{
    if (!d->oceankeyboard) {
        return;
    }
    d->keys.lastStateSerial = d->display->nextSerial();
    if (!d->updateKey(key, OCEANSeatInterfacePrivate::Keyboard::State::Pressed)) {
        return;
    }

    d->oceankeyboard->keyPressed(key, d->keys.lastStateSerial);
}

void OCEANSeatInterface::keyReleased(quint32 key)
{
    if (!d->oceankeyboard) {
        return;
    }
    d->keys.lastStateSerial = d->display->nextSerial();
    if (!d->updateKey(key, OCEANSeatInterfacePrivate::Keyboard::State::Released)) {
        return;
    }

    d->oceankeyboard->keyReleased(key, d->keys.lastStateSerial);
}

void OCEANSeatInterface::touchDown(qint32 id, const QPointF &pos)
{
    if (!d->oceantouch) {
        return;
    }
    d->oceantouch->touchDown(id, pos);
}

void OCEANSeatInterface::touchMotion(qint32 id, const QPointF &pos)
{
    if (!d->oceantouch) {
        return;
    }
    d->oceantouch->touchMotion(id, pos);
}

void OCEANSeatInterface::touchUp(qint32 id)
{
    if (!d->oceantouch) {
        return;
    }
    d->oceantouch->touchUp(id);
}

void OCEANSeatInterface::updateKeyboardModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group)
{
    if (!d->oceankeyboard) {
        return;
    }
    bool changed = false;
#define UPDATE( value ) \
    if (d->keys.modifiers.value != value) { \
        d->keys.modifiers.value = value; \
        changed = true; \
    }
    UPDATE(depressed)
    UPDATE(latched)
    UPDATE(locked)
    UPDATE(group)
    if (!changed) {
        return;
    }
    const quint32 serial = d->display->nextSerial();
    d->keys.modifiers.serial = serial;

    d->oceankeyboard->updateModifiers(depressed, latched, locked, group, serial);
}

quint32 OCEANSeatInterface::depressedModifiers() const
{
    return d->keys.modifiers.depressed;
}

quint32 OCEANSeatInterface::groupModifiers() const
{
    return d->keys.modifiers.group;
}

quint32 OCEANSeatInterface::latchedModifiers() const
{
    return d->keys.modifiers.latched;
}

quint32 OCEANSeatInterface::lockedModifiers() const
{
    return d->keys.modifiers.locked;
}

quint32 OCEANSeatInterface::lastModifiersSerial() const
{
    return d->keys.modifiers.serial;
}

void OCEANSeatInterface::setHasKeyboard(bool has)
{
    Q_UNUSED(has)
}

void OCEANSeatInterface::setHasPointer(bool has)
{
    Q_UNUSED(has)
}

void OCEANSeatInterface::setHasTouch(bool has)
{
    Q_UNUSED(has)
}

/*********************************
 * OCEANPointerInterface
 *********************************/
OCEANPointerInterfacePrivate *OCEANPointerInterfacePrivate::get(OCEANPointerInterface *pointer)
{
    return pointer->d.data();
}

OCEANPointerInterfacePrivate::OCEANPointerInterfacePrivate(OCEANPointerInterface *q, OCEANSeatInterface *seat, wl_resource *resource)
    : QtWaylandServer::ocean_pointer(resource)
    , q(q)
    , oceanSeat(seat)
{
}

OCEANPointerInterfacePrivate::~OCEANPointerInterfacePrivate()
{
}

void OCEANPointerInterfacePrivate::ocean_pointer_get_motion(Resource *resource)
{
    Q_UNUSED(resource)

    const QPointF globalPos = oceanSeat->pointerPos();
    send_motion(wl_fixed_from_double(globalPos.x()), wl_fixed_from_double(globalPos.y()));
}

OCEANPointerInterface::OCEANPointerInterface(OCEANSeatInterface *seat, wl_resource *resource)
    : d(new OCEANPointerInterfacePrivate(this, seat, resource))
{
}

OCEANPointerInterface::~OCEANPointerInterface() = default;

OCEANSeatInterface *OCEANPointerInterface::oceanSeat() const
{
    return d->oceanSeat;
}

void OCEANPointerInterface::buttonPressed(quint32 button)
{
    const QPointF globalPos = d->oceanSeat->pointerPos();
    d->send_button(wl_fixed_from_double(globalPos.x()), wl_fixed_from_double(globalPos.y()), button,
            QtWaylandServer::ocean_pointer::button_state::button_state_pressed);
}

void OCEANPointerInterface::buttonReleased(quint32 button)
{
    const QPointF globalPos = d->oceanSeat->pointerPos();
    d->send_button(wl_fixed_from_double(globalPos.x()), wl_fixed_from_double(globalPos.y()), button,
            QtWaylandServer::ocean_pointer::button_state::button_state_released);
}

void OCEANPointerInterface::axis(Qt::Orientation orientation, qint32 delta)
{
    d->send_axis(0, (orientation == Qt::Vertical) ? WL_POINTER_AXIS_VERTICAL_SCROLL : WL_POINTER_AXIS_HORIZONTAL_SCROLL,
            wl_fixed_from_int(delta));
}

void OCEANPointerInterface::sendMotion(const QPointF &position)
{
    d->send_motion(wl_fixed_from_double(position.x()), wl_fixed_from_double(position.y()));
}

/*********************************
 * OCEANTouchInterface
 *********************************/
OCEANTouchInterfacePrivate *OCEANTouchInterfacePrivate::get(OCEANTouchInterface *touch)
{
    return touch->d.data();
}

OCEANTouchInterfacePrivate::OCEANTouchInterfacePrivate(OCEANTouchInterface *q, OCEANSeatInterface *seat, wl_resource *resource)
    : QtWaylandServer::ocean_touch(resource)
    , q(q)
    , oceanSeat(seat)
{
}

OCEANTouchInterfacePrivate::~OCEANTouchInterfacePrivate() = default;

void OCEANTouchInterfacePrivate::ocean_touch_release(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

OCEANTouchInterface::OCEANTouchInterface(OCEANSeatInterface *seat, wl_resource *resource)
    : d(new OCEANTouchInterfacePrivate(this, seat, resource))
{
}

OCEANTouchInterface *OCEANTouchInterface::get(wl_resource *native)
{
    if (OCEANTouchInterfacePrivate *touchPrivate = resource_cast<OCEANTouchInterfacePrivate *>(native)) {
        return touchPrivate->q;
    }
    return nullptr;
}

OCEANTouchInterface::~OCEANTouchInterface() = default;

OCEANSeatInterface *OCEANTouchInterface::oceanSeat() const
{
    return d->oceanSeat;
}

void OCEANTouchInterface::touchDown(qint32 id, const QPointF &pos)
{
    d->send_down(id, d->oceanSeat->touchtimestamp(), wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()));
}

void OCEANTouchInterface::touchMotion(qint32 id, const QPointF &pos)
{
    d->send_motion(id, d->oceanSeat->touchtimestamp(), wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()));
}

void OCEANTouchInterface::touchUp(qint32 id)
{
    d->send_up(id, d->oceanSeat->touchtimestamp());
}

}