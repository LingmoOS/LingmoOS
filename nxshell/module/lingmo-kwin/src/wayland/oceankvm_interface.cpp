/********************************************************************
Copyright 2024  xinbo wang <wangxinbo@uniontech.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "oceankvm_interface.h"

#include "display.h"
#include "utils.h"

#include <QPointF>
#include <QDebug>

#include "oceankvm_interface_p.h"

namespace KWaylandServer
{
static const int s_version = 1;
static const int s_oceanKvmPointerVersion = 1;
static const int s_oceanKvmKeyboardVersion = 1;

/*********************************
 * OCEANKvmInterface
 *********************************/
OCEANKvmInterfacePrivate::OCEANKvmInterfacePrivate(OCEANKvmInterface *q, Display *d)
    : QtWaylandServer::ocean_kvm(*d, s_version)
    , q(q)
    , display(d)
{
    oceanKvmPointer.reset(new OCEANKvmPointerInterface(q));
    oceanKvmKeyboard.reset(new OCEANKvmKeyboardInterface(q));
}

OCEANKvmInterfacePrivate *OCEANKvmInterfacePrivate::get(OCEANKvmInterface *oceankvm)
{
    return oceankvm->d.data();
}

void OCEANKvmInterfacePrivate::ocean_kvm_get_ocean_kvm_pointer(Resource *resource, uint32_t id)
{
    OCEANKvmPointerInterfacePrivate *kvmPointerPrivate = OCEANKvmPointerInterfacePrivate::get(oceanKvmPointer.data());
    kvmPointerPrivate->add(resource->client(), id, resource->version());
}

void OCEANKvmInterfacePrivate::ocean_kvm_get_ocean_kvm_keyboard(Resource *resource, uint32_t id)
{
    OCEANKvmKeyboardInterfacePrivate *kvmKeyboardPrivate = OCEANKvmKeyboardInterfacePrivate::get(oceanKvmKeyboard.data());
    kvmKeyboardPrivate->add(resource->client(), id, resource->version());
}

bool OCEANKvmInterfacePrivate::updateKeyState(quint32 key, quint32 state)
{
    auto it = keys.states.find(key);
    if (it == keys.states.end()) {
        keys.states.insert(key, Keyboard::State(state));
        return true;
    }
    if (it.value() == Keyboard::State(state)) {
        return false;
    }
    it.value() = Keyboard::State(state);
    return true;
}

OCEANKvmInterface::OCEANKvmInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new OCEANKvmInterfacePrivate(this, display))
{
    connect(d->oceanKvmPointer.data(), &OCEANKvmPointerInterface::kvmEnablePointerRequested, this, &OCEANKvmInterface::kvmInterfaceEnablePointerRequested);
    connect(d->oceanKvmPointer.data(), &OCEANKvmPointerInterface::kvmEnableCursorRequested, this, &OCEANKvmInterface::kvmInterfaceEnableCursorRequested);
    connect(d->oceanKvmPointer.data(), &OCEANKvmPointerInterface::kvmSetCursorPosRequested, this, &OCEANKvmInterface::kvmInterfaceSetCursorPosRequested);
    connect(d->oceanKvmKeyboard.data(), &OCEANKvmKeyboardInterface::kvmEnableKeyboardRequested, this, &OCEANKvmInterface::kvmInterfaceEnableKeyboardRequested);
}

OCEANKvmInterface::~OCEANKvmInterface() = default;

OCEANKvmInterface *OCEANKvmInterface::get(wl_resource* native)
{
    if (OCEANKvmInterfacePrivate *kvmPrivate = resource_cast<OCEANKvmInterfacePrivate *>(native)) {
        return kvmPrivate->q;
    }
    return nullptr;
}

void OCEANKvmInterface::pointerMotion(const QPointF &pos)
{
    if (!d->oceanKvmPointer) {
        return;
    }

    // TODO maybe need send pos as same
    // if (d->globalPos == pos) {
    //     return;
    // }
    d->globalPos = pos;
    d->oceanKvmPointer->sendMotion(pos);
}

QPointF OCEANKvmInterface::pointerPos() const
{
    return d->globalPos;
}

void OCEANKvmInterface::pointerButton(quint32 button, quint32 state, quint32 serial, const QPointF &position)
{
    if (!d->oceanKvmPointer) {
        return;
    }
    d->oceanKvmPointer->sendButton(button, state, serial, position);
}

void OCEANKvmInterface::pointerAxis(Qt::Orientation orientation, qint32 delta)
{
    if (!d->oceanKvmPointer) {
        return;
    }
    d->oceanKvmPointer->sendAxis(orientation, delta);
}

void OCEANKvmInterface::updateKey(quint32 key, quint32 serial, quint32 state)
{
    if (!d->oceanKvmKeyboard) {
        return;
    }

    d->keys.lastStateSerial = d->display->nextSerial();

    if (!d->updateKeyState(key, state)) {
        return;
    }

    d->oceanKvmKeyboard->sendKey(key, d->keys.lastStateSerial, state);
}

void OCEANKvmInterface::updateModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial)
{
    if (!d->oceanKvmKeyboard) {
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
#undef UPDATE
    if (!changed) {
        return;
    }
    const quint32 nextSerial = d->display->nextSerial();
    d->keys.modifiers.serial = nextSerial;

    d->oceanKvmKeyboard->sendModifiers(depressed, latched, locked, group, nextSerial);
}

void OCEANKvmInterface::setKvmPointerTimestamp(quint32 time)
{
    if (d->kvmPointerTimestamp == time) {
        return;
    }
    d->kvmPointerTimestamp = time;
}

void OCEANKvmInterface::setkvmKeyboardTimestamp(quint32 time)
{
    if (d->kvmKeyboardTimestamp == time) {
        return;
    }
    d->kvmKeyboardTimestamp = time;
}

quint32 OCEANKvmInterface::kvmPointerTimestamp() const
{
    return d->kvmPointerTimestamp;
}

quint32 OCEANKvmInterface::kvmKeyboardTimestamp() const
{
    return d->kvmKeyboardTimestamp;
}

/*********************************
 * OCEANKvmPointerInterface
 *********************************/
OCEANKvmPointerInterfacePrivate *OCEANKvmPointerInterfacePrivate::get(OCEANKvmPointerInterface *pointer)
{
    return pointer->d.data();
}

OCEANKvmPointerInterfacePrivate::OCEANKvmPointerInterfacePrivate(OCEANKvmPointerInterface *q, OCEANKvmInterface *kvm)
    : q(q)
    , oceanKvm(kvm)
{
}

OCEANKvmPointerInterfacePrivate::~OCEANKvmPointerInterfacePrivate()
{
}

void OCEANKvmPointerInterfacePrivate::ocean_kvm_pointer_enable_pointer(Resource *resource, uint32_t is_enable)
{
    Q_UNUSED(resource)

    Q_EMIT q->kvmEnablePointerRequested(static_cast<quint32>(is_enable));
}

void OCEANKvmPointerInterfacePrivate::ocean_kvm_pointer_enable_cursor(Resource *resource, uint32_t is_enable)
{
    Q_UNUSED(resource)

    Q_EMIT q->kvmEnableCursorRequested(static_cast<quint32>(is_enable));
}

void OCEANKvmPointerInterfacePrivate::ocean_kvm_pointer_set_pos(Resource *resource, wl_fixed_t x, wl_fixed_t y)
{
    Q_UNUSED(resource)

    Q_EMIT q->kvmSetCursorPosRequested((double)x, (double)y);
}

OCEANKvmPointerInterface::OCEANKvmPointerInterface(OCEANKvmInterface *kvm)
    : d(new OCEANKvmPointerInterfacePrivate(this, kvm))
{
}

OCEANKvmPointerInterface::~OCEANKvmPointerInterface() = default;

OCEANKvmInterface *OCEANKvmPointerInterface::oceanKvm() const
{
    return d->oceanKvm;
}

void OCEANKvmPointerInterface::sendButton(quint32 button, quint32 state, quint32 serial, const QPointF &position)
{
    const QPointF globalPos = d->oceanKvm->pointerPos();
    const auto kvmPointerResources = d->resourceMap();
    for (OCEANKvmPointerInterfacePrivate::Resource *resource : kvmPointerResources) {
        d->send_button(resource->handle, serial, d->oceanKvm->kvmPointerTimestamp(), state);
    }
}

void OCEANKvmPointerInterface::sendMotion(const QPointF &position)
{
    const auto kvmPointerResources = d->resourceMap();
    for (OCEANKvmPointerInterfacePrivate::Resource *resource : kvmPointerResources) {
        d->send_motion(resource->handle, d->oceanKvm->kvmPointerTimestamp(), position.x(), position.y());
    }
}

void OCEANKvmPointerInterface::sendAxis(Qt::Orientation orientation, qreal delta)
{
    const auto kvmPointerResources = d->resourceMap();
    for (OCEANKvmPointerInterfacePrivate::Resource *resource : kvmPointerResources) {
        d->send_axis(resource->handle, d->oceanKvm->kvmPointerTimestamp(),
                     (orientation == Qt::Vertical) ? WL_POINTER_AXIS_VERTICAL_SCROLL : WL_POINTER_AXIS_HORIZONTAL_SCROLL,
                     delta);
    }
}

/*********************************
 * OCEANKvmKeyboardInterface
 *********************************/
OCEANKvmKeyboardInterfacePrivate *OCEANKvmKeyboardInterfacePrivate::get(OCEANKvmKeyboardInterface *kvmKeyboard)
{
    return kvmKeyboard->d.data();
}

OCEANKvmKeyboardInterfacePrivate::OCEANKvmKeyboardInterfacePrivate(OCEANKvmKeyboardInterface *q, OCEANKvmInterface *kvm)
    : q(q)
    , oceanKvm(kvm)
{
}

OCEANKvmKeyboardInterfacePrivate::~OCEANKvmKeyboardInterfacePrivate() = default;

void OCEANKvmKeyboardInterfacePrivate::ocean_kvm_keyboard_enable_keyboard(Resource *resource, uint32_t is_enable)
{
    Q_UNUSED(resource)

    Q_EMIT q->kvmEnableKeyboardRequested(static_cast<quint32>(is_enable));
}

OCEANKvmKeyboardInterface::OCEANKvmKeyboardInterface(OCEANKvmInterface *kvm)
    : d(new OCEANKvmKeyboardInterfacePrivate(this, kvm))
{
}

OCEANKvmKeyboardInterface *OCEANKvmKeyboardInterface::get(wl_resource *native)
{
    if (OCEANKvmKeyboardInterfacePrivate *kvmKeyboardPrivate = resource_cast<OCEANKvmKeyboardInterfacePrivate *>(native)) {
        return kvmKeyboardPrivate->q;
    }
    return nullptr;
}

OCEANKvmKeyboardInterface::~OCEANKvmKeyboardInterface() = default;

OCEANKvmInterface *OCEANKvmKeyboardInterface::oceanKvm() const
{
    return d->oceanKvm;
}

void OCEANKvmKeyboardInterface::sendKey(quint32 key, quint32 serial, quint32 state)
{
    const auto keyboardResources = d->resourceMap();
    for (OCEANKvmKeyboardInterfacePrivate::Resource *resource : keyboardResources) {
        d->send_key(resource->handle, serial, d->oceanKvm->kvmKeyboardTimestamp(), key,
                    state);
    }
}

void OCEANKvmKeyboardInterface::sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial)
{
    const auto keyboardResources = d->resourceMap();
    for (OCEANKvmKeyboardInterfacePrivate::Resource *resource : keyboardResources) {
        d->send_modifiers(resource->handle, serial, depressed, latched, locked, group);
    }
}

}
