// Copyright 2022  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "ddeseat_interface.h"
#include "ddekeyboard_interface.h"
#include "display.h"
#include "logging.h"
#include "utils.h"

#include <QPointF>

#include "ddeseat_interface_p.h"
#include "ddekeyboard_interface_p.h"

namespace KWaylandServer
{
static const int s_version = 1;
static const int s_ddePointerVersion = 1;
static const int s_ddeTouchVersion = 7;
static const int s_ddeKeyboardVersion = 7;

/*********************************
 * DDESeatInterface
 *********************************/
DDESeatInterfacePrivate::DDESeatInterfacePrivate(DDESeatInterface *q, Display *d)
    : QtWaylandServer::dde_seat(*d, s_version)
    , q(q)
    , display(d)
{
}

DDESeatInterfacePrivate *DDESeatInterfacePrivate::get(DDESeatInterface *ddeseat)
{
    return ddeseat->d.data();
}

void DDESeatInterfacePrivate::dde_seat_get_dde_pointer(Resource *resource, uint32_t id) {
    if (ddepointer) {
        DDEPointerInterfacePrivate *pointerPrivate = DDEPointerInterfacePrivate::get(ddepointer.data());
        pointerPrivate->add(resource->client(), id, resource->version());
    } else {
        wl_resource *pointer_resource = wl_resource_create(resource->client(), &dde_pointer_interface, s_ddePointerVersion, id);
        ddepointer.reset(new DDEPointerInterface(q, pointer_resource));
        Q_EMIT q->ddePointerCreated(ddepointer.data());
    }
}

void DDESeatInterfacePrivate::dde_seat_get_dde_keyboard(Resource *resource, uint32_t id) {
    if (ddekeyboard) {
        DDEKeyboardInterfacePrivate *keyboardPrivate = DDEKeyboardInterfacePrivate::get(ddekeyboard.data());
        keyboardPrivate->add(resource->client(), id, resource->version());
    } else {
        wl_resource *keyboard_resource = wl_resource_create(resource->client(), &dde_keyboard_interface, s_ddeKeyboardVersion, id);
        ddekeyboard.reset(new DDEKeyboardInterface(q, keyboard_resource));
        Q_EMIT q->ddeKeyboardCreated(ddekeyboard.data());
    }
}

void DDESeatInterfacePrivate::dde_seat_get_dde_touch(Resource *resource, uint32_t id) {
    if (ddetouch) {
        DDETouchInterfacePrivate *touchPrivate = DDETouchInterfacePrivate::get(ddetouch.data());
        touchPrivate->add(resource->client(), id, resource->version());
    } else {
        wl_resource *touch_resource = wl_resource_create(resource->client(), &dde_touch_interface, s_ddeTouchVersion, id);
        ddetouch.reset(new DDETouchInterface(q, touch_resource));
        Q_EMIT q->ddeTouchCreated(ddetouch.data());
    }
}

bool DDESeatInterfacePrivate::updateKey(quint32 key, Keyboard::State state)
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

DDESeatInterface::DDESeatInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new DDESeatInterfacePrivate(this, display))
{
}

DDESeatInterface::~DDESeatInterface() = default;

DDESeatInterface *DDESeatInterface::get(wl_resource* native)
{
    if (DDESeatInterfacePrivate *seatPrivate = resource_cast<DDESeatInterfacePrivate *>(native)) {
        return seatPrivate->q;
    }
    return nullptr;
}

QPointF DDESeatInterface::pointerPos() const
{
    return d->globalPos;
}

void DDESeatInterface::setPointerPos(const QPointF &pos)
{
    if (!d->ddepointer) {
        return;
    }
    if (d->globalPos == pos) {
        return;
    }
    d->globalPos = pos;
    d->ddepointer->sendMotion(pos);
}

void DDESeatInterface::pointerButtonPressed(quint32 button)
{
    if (!d->ddepointer) {
        return;
    }
    d->ddepointer->buttonPressed(button);
}

void DDESeatInterface::pointerButtonReleased(quint32 button)
{
    if (!d->ddepointer) {
        return;
    }
    d->ddepointer->buttonReleased(button);
}

void DDESeatInterface::pointerAxis(Qt::Orientation orientation, qint32 delta)
{
    if (!d->ddepointer) {
        return;
    }
    d->ddepointer->axis(orientation, delta);
}

quint32 DDESeatInterface::timestamp() const
{
    return d->timestamp;
}

void DDESeatInterface::setTimestamp(quint32 time)
{
    if (d->timestamp == time) {
        return;
    }
    d->timestamp = time;
}

quint32 DDESeatInterface::touchtimestamp() const
{
    return d->touchtimestamp;
}

void DDESeatInterface::setTouchTimestamp(quint32 time)
{
    if (d->touchtimestamp == time) {
        return;
    }
    d->touchtimestamp = time;
}

void DDESeatInterface::setKeymap(int fd, quint32 size)
{
    if (!d->ddekeyboard) {
        return;
    }
    d->keys.keymap.xkbcommonCompatible = true;
    d->keys.keymap.fd = fd;
    d->keys.keymap.size = size;

    d->ddekeyboard->setKeymap(fd, size);
}

void DDESeatInterface::keyPressed(quint32 key)
{
    if (!d->ddekeyboard) {
        return;
    }
    d->keys.lastStateSerial = d->display->nextSerial();
    if (!d->updateKey(key, DDESeatInterfacePrivate::Keyboard::State::Pressed)) {
        return;
    }

    d->ddekeyboard->keyPressed(key, d->keys.lastStateSerial);
}

void DDESeatInterface::keyReleased(quint32 key)
{
    if (!d->ddekeyboard) {
        return;
    }
    d->keys.lastStateSerial = d->display->nextSerial();
    if (!d->updateKey(key, DDESeatInterfacePrivate::Keyboard::State::Released)) {
        return;
    }

    d->ddekeyboard->keyReleased(key, d->keys.lastStateSerial);
}

void DDESeatInterface::touchDown(qint32 id, const QPointF &pos)
{
    if (!d->ddetouch) {
        return;
    }
    d->ddetouch->touchDown(id, pos);
}

void DDESeatInterface::touchMotion(qint32 id, const QPointF &pos)
{
    if (!d->ddetouch) {
        return;
    }
    d->ddetouch->touchMotion(id, pos);
}

void DDESeatInterface::touchUp(qint32 id)
{
    if (!d->ddetouch) {
        return;
    }
    d->ddetouch->touchUp(id);
}

void DDESeatInterface::updateKeyboardModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group)
{
    if (!d->ddekeyboard) {
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

    d->ddekeyboard->updateModifiers(depressed, latched, locked, group, serial);
}

quint32 DDESeatInterface::depressedModifiers() const
{
    return d->keys.modifiers.depressed;
}

quint32 DDESeatInterface::groupModifiers() const
{
    return d->keys.modifiers.group;
}

quint32 DDESeatInterface::latchedModifiers() const
{
    return d->keys.modifiers.latched;
}

quint32 DDESeatInterface::lockedModifiers() const
{
    return d->keys.modifiers.locked;
}

quint32 DDESeatInterface::lastModifiersSerial() const
{
    return d->keys.modifiers.serial;
}

void DDESeatInterface::setHasKeyboard(bool has)
{
    Q_UNUSED(has)
}

void DDESeatInterface::setHasPointer(bool has)
{
    Q_UNUSED(has)
}

void DDESeatInterface::setHasTouch(bool has)
{
    Q_UNUSED(has)
}

/*********************************
 * DDEPointerInterface
 *********************************/
DDEPointerInterfacePrivate *DDEPointerInterfacePrivate::get(DDEPointerInterface *pointer)
{
    return pointer->d.data();
}

DDEPointerInterfacePrivate::DDEPointerInterfacePrivate(DDEPointerInterface *q, DDESeatInterface *seat, wl_resource *resource)
    : QtWaylandServer::dde_pointer(resource)
    , q(q)
    , ddeSeat(seat)
{
}

DDEPointerInterfacePrivate::~DDEPointerInterfacePrivate()
{
}

void DDEPointerInterfacePrivate::dde_pointer_get_motion(Resource *resource)
{
    Q_UNUSED(resource)

    const QPointF globalPos = ddeSeat->pointerPos();
    send_motion(wl_fixed_from_double(globalPos.x()), wl_fixed_from_double(globalPos.y()));
}

DDEPointerInterface::DDEPointerInterface(DDESeatInterface *seat, wl_resource *resource)
    : d(new DDEPointerInterfacePrivate(this, seat, resource))
{
}

DDEPointerInterface::~DDEPointerInterface() = default;

DDESeatInterface *DDEPointerInterface::ddeSeat() const
{
    return d->ddeSeat;
}

void DDEPointerInterface::buttonPressed(quint32 button)
{
    const QPointF globalPos = d->ddeSeat->pointerPos();
    d->send_button(wl_fixed_from_double(globalPos.x()), wl_fixed_from_double(globalPos.y()), button,
            QtWaylandServer::dde_pointer::button_state::button_state_pressed);
}

void DDEPointerInterface::buttonReleased(quint32 button)
{
    const QPointF globalPos = d->ddeSeat->pointerPos();
    d->send_button(wl_fixed_from_double(globalPos.x()), wl_fixed_from_double(globalPos.y()), button,
            QtWaylandServer::dde_pointer::button_state::button_state_released);
}

void DDEPointerInterface::axis(Qt::Orientation orientation, qint32 delta)
{
    d->send_axis(0, (orientation == Qt::Vertical) ? WL_POINTER_AXIS_VERTICAL_SCROLL : WL_POINTER_AXIS_HORIZONTAL_SCROLL,
            wl_fixed_from_int(delta));
}

void DDEPointerInterface::sendMotion(const QPointF &position)
{
    d->send_motion(wl_fixed_from_double(position.x()), wl_fixed_from_double(position.y()));
}

/*********************************
 * DDETouchInterface
 *********************************/
DDETouchInterfacePrivate *DDETouchInterfacePrivate::get(DDETouchInterface *touch)
{
    return touch->d.data();
}

DDETouchInterfacePrivate::DDETouchInterfacePrivate(DDETouchInterface *q, DDESeatInterface *seat, wl_resource *resource)
    : QtWaylandServer::dde_touch(resource)
    , q(q)
    , ddeSeat(seat)
{
}

DDETouchInterfacePrivate::~DDETouchInterfacePrivate() = default;

void DDETouchInterfacePrivate::dde_touch_release(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

DDETouchInterface::DDETouchInterface(DDESeatInterface *seat, wl_resource *resource)
    : d(new DDETouchInterfacePrivate(this, seat, resource))
{
}

DDETouchInterface *DDETouchInterface::get(wl_resource *native)
{
    if (DDETouchInterfacePrivate *touchPrivate = resource_cast<DDETouchInterfacePrivate *>(native)) {
        return touchPrivate->q;
    }
    return nullptr;
}

DDETouchInterface::~DDETouchInterface() = default;

DDESeatInterface *DDETouchInterface::ddeSeat() const
{
    return d->ddeSeat;
}

void DDETouchInterface::touchDown(qint32 id, const QPointF &pos)
{
    d->send_down(id, d->ddeSeat->touchtimestamp(), wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()));
}

void DDETouchInterface::touchMotion(qint32 id, const QPointF &pos)
{
    d->send_motion(id, d->ddeSeat->touchtimestamp(), wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()));
}

void DDETouchInterface::touchUp(qint32 id)
{
    d->send_up(id, d->ddeSeat->touchtimestamp());
}

}