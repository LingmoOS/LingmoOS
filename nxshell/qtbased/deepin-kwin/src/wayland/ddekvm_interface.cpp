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
#include "ddekvm_interface.h"

#include "display.h"
#include "utils.h"

#include <QPointF>
#include <QDebug>

#include "ddekvm_interface_p.h"

namespace KWaylandServer
{
static const int s_version = 1;
static const int s_ddeKvmPointerVersion = 1;
static const int s_ddeKvmKeyboardVersion = 1;

/*********************************
 * DDEKvmInterface
 *********************************/
DDEKvmInterfacePrivate::DDEKvmInterfacePrivate(DDEKvmInterface *q, Display *d)
    : QtWaylandServer::dde_kvm(*d, s_version)
    , q(q)
    , display(d)
{
    ddeKvmPointer.reset(new DDEKvmPointerInterface(q));
    ddeKvmKeyboard.reset(new DDEKvmKeyboardInterface(q));
}

DDEKvmInterfacePrivate *DDEKvmInterfacePrivate::get(DDEKvmInterface *ddekvm)
{
    return ddekvm->d.data();
}

void DDEKvmInterfacePrivate::dde_kvm_get_dde_kvm_pointer(Resource *resource, uint32_t id)
{
    DDEKvmPointerInterfacePrivate *kvmPointerPrivate = DDEKvmPointerInterfacePrivate::get(ddeKvmPointer.data());
    kvmPointerPrivate->add(resource->client(), id, resource->version());
}

void DDEKvmInterfacePrivate::dde_kvm_get_dde_kvm_keyboard(Resource *resource, uint32_t id)
{
    DDEKvmKeyboardInterfacePrivate *kvmKeyboardPrivate = DDEKvmKeyboardInterfacePrivate::get(ddeKvmKeyboard.data());
    kvmKeyboardPrivate->add(resource->client(), id, resource->version());
}

bool DDEKvmInterfacePrivate::updateKeyState(quint32 key, quint32 state)
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

DDEKvmInterface::DDEKvmInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new DDEKvmInterfacePrivate(this, display))
{
    connect(d->ddeKvmPointer.data(), &DDEKvmPointerInterface::kvmEnablePointerRequested, this, &DDEKvmInterface::kvmInterfaceEnablePointerRequested);
    connect(d->ddeKvmPointer.data(), &DDEKvmPointerInterface::kvmEnableCursorRequested, this, &DDEKvmInterface::kvmInterfaceEnableCursorRequested);
    connect(d->ddeKvmPointer.data(), &DDEKvmPointerInterface::kvmSetCursorPosRequested, this, &DDEKvmInterface::kvmInterfaceSetCursorPosRequested);
    connect(d->ddeKvmKeyboard.data(), &DDEKvmKeyboardInterface::kvmEnableKeyboardRequested, this, &DDEKvmInterface::kvmInterfaceEnableKeyboardRequested);
}

DDEKvmInterface::~DDEKvmInterface() = default;

DDEKvmInterface *DDEKvmInterface::get(wl_resource* native)
{
    if (DDEKvmInterfacePrivate *kvmPrivate = resource_cast<DDEKvmInterfacePrivate *>(native)) {
        return kvmPrivate->q;
    }
    return nullptr;
}

void DDEKvmInterface::pointerMotion(const QPointF &pos)
{
    if (!d->ddeKvmPointer) {
        return;
    }

    // TODO maybe need send pos as same
    // if (d->globalPos == pos) {
    //     return;
    // }
    d->globalPos = pos;
    d->ddeKvmPointer->sendMotion(pos);
}

QPointF DDEKvmInterface::pointerPos() const
{
    return d->globalPos;
}

void DDEKvmInterface::pointerButton(quint32 button, quint32 state, quint32 serial, const QPointF &position)
{
    if (!d->ddeKvmPointer) {
        return;
    }
    d->ddeKvmPointer->sendButton(button, state, serial, position);
}

void DDEKvmInterface::pointerAxis(Qt::Orientation orientation, qint32 delta)
{
    if (!d->ddeKvmPointer) {
        return;
    }
    d->ddeKvmPointer->sendAxis(orientation, delta);
}

void DDEKvmInterface::updateKey(quint32 key, quint32 serial, quint32 state)
{
    if (!d->ddeKvmKeyboard) {
        return;
    }

    d->keys.lastStateSerial = d->display->nextSerial();

    if (!d->updateKeyState(key, state)) {
        return;
    }

    d->ddeKvmKeyboard->sendKey(key, d->keys.lastStateSerial, state);
}

void DDEKvmInterface::updateModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial)
{
    if (!d->ddeKvmKeyboard) {
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

    d->ddeKvmKeyboard->sendModifiers(depressed, latched, locked, group, nextSerial);
}

void DDEKvmInterface::setKvmPointerTimestamp(quint32 time)
{
    if (d->kvmPointerTimestamp == time) {
        return;
    }
    d->kvmPointerTimestamp = time;
}

void DDEKvmInterface::setkvmKeyboardTimestamp(quint32 time)
{
    if (d->kvmKeyboardTimestamp == time) {
        return;
    }
    d->kvmKeyboardTimestamp = time;
}

quint32 DDEKvmInterface::kvmPointerTimestamp() const
{
    return d->kvmPointerTimestamp;
}

quint32 DDEKvmInterface::kvmKeyboardTimestamp() const
{
    return d->kvmKeyboardTimestamp;
}

/*********************************
 * DDEKvmPointerInterface
 *********************************/
DDEKvmPointerInterfacePrivate *DDEKvmPointerInterfacePrivate::get(DDEKvmPointerInterface *pointer)
{
    return pointer->d.data();
}

DDEKvmPointerInterfacePrivate::DDEKvmPointerInterfacePrivate(DDEKvmPointerInterface *q, DDEKvmInterface *kvm)
    : q(q)
    , ddeKvm(kvm)
{
}

DDEKvmPointerInterfacePrivate::~DDEKvmPointerInterfacePrivate()
{
}

void DDEKvmPointerInterfacePrivate::dde_kvm_pointer_enable_pointer(Resource *resource, uint32_t is_enable)
{
    Q_UNUSED(resource)

    Q_EMIT q->kvmEnablePointerRequested(static_cast<quint32>(is_enable));
}

void DDEKvmPointerInterfacePrivate::dde_kvm_pointer_enable_cursor(Resource *resource, uint32_t is_enable)
{
    Q_UNUSED(resource)

    Q_EMIT q->kvmEnableCursorRequested(static_cast<quint32>(is_enable));
}

void DDEKvmPointerInterfacePrivate::dde_kvm_pointer_set_pos(Resource *resource, wl_fixed_t x, wl_fixed_t y)
{
    Q_UNUSED(resource)

    Q_EMIT q->kvmSetCursorPosRequested((double)x, (double)y);
}

DDEKvmPointerInterface::DDEKvmPointerInterface(DDEKvmInterface *kvm)
    : d(new DDEKvmPointerInterfacePrivate(this, kvm))
{
}

DDEKvmPointerInterface::~DDEKvmPointerInterface() = default;

DDEKvmInterface *DDEKvmPointerInterface::ddeKvm() const
{
    return d->ddeKvm;
}

void DDEKvmPointerInterface::sendButton(quint32 button, quint32 state, quint32 serial, const QPointF &position)
{
    const QPointF globalPos = d->ddeKvm->pointerPos();
    const auto kvmPointerResources = d->resourceMap();
    for (DDEKvmPointerInterfacePrivate::Resource *resource : kvmPointerResources) {
        d->send_button(resource->handle, serial, d->ddeKvm->kvmPointerTimestamp(), state);
    }
}

void DDEKvmPointerInterface::sendMotion(const QPointF &position)
{
    const auto kvmPointerResources = d->resourceMap();
    for (DDEKvmPointerInterfacePrivate::Resource *resource : kvmPointerResources) {
        d->send_motion(resource->handle, d->ddeKvm->kvmPointerTimestamp(), position.x(), position.y());
    }
}

void DDEKvmPointerInterface::sendAxis(Qt::Orientation orientation, qreal delta)
{
    const auto kvmPointerResources = d->resourceMap();
    for (DDEKvmPointerInterfacePrivate::Resource *resource : kvmPointerResources) {
        d->send_axis(resource->handle, d->ddeKvm->kvmPointerTimestamp(),
                     (orientation == Qt::Vertical) ? WL_POINTER_AXIS_VERTICAL_SCROLL : WL_POINTER_AXIS_HORIZONTAL_SCROLL,
                     delta);
    }
}

/*********************************
 * DDEKvmKeyboardInterface
 *********************************/
DDEKvmKeyboardInterfacePrivate *DDEKvmKeyboardInterfacePrivate::get(DDEKvmKeyboardInterface *kvmKeyboard)
{
    return kvmKeyboard->d.data();
}

DDEKvmKeyboardInterfacePrivate::DDEKvmKeyboardInterfacePrivate(DDEKvmKeyboardInterface *q, DDEKvmInterface *kvm)
    : q(q)
    , ddeKvm(kvm)
{
}

DDEKvmKeyboardInterfacePrivate::~DDEKvmKeyboardInterfacePrivate() = default;

void DDEKvmKeyboardInterfacePrivate::dde_kvm_keyboard_enable_keyboard(Resource *resource, uint32_t is_enable)
{
    Q_UNUSED(resource)

    Q_EMIT q->kvmEnableKeyboardRequested(static_cast<quint32>(is_enable));
}

DDEKvmKeyboardInterface::DDEKvmKeyboardInterface(DDEKvmInterface *kvm)
    : d(new DDEKvmKeyboardInterfacePrivate(this, kvm))
{
}

DDEKvmKeyboardInterface *DDEKvmKeyboardInterface::get(wl_resource *native)
{
    if (DDEKvmKeyboardInterfacePrivate *kvmKeyboardPrivate = resource_cast<DDEKvmKeyboardInterfacePrivate *>(native)) {
        return kvmKeyboardPrivate->q;
    }
    return nullptr;
}

DDEKvmKeyboardInterface::~DDEKvmKeyboardInterface() = default;

DDEKvmInterface *DDEKvmKeyboardInterface::ddeKvm() const
{
    return d->ddeKvm;
}

void DDEKvmKeyboardInterface::sendKey(quint32 key, quint32 serial, quint32 state)
{
    const auto keyboardResources = d->resourceMap();
    for (DDEKvmKeyboardInterfacePrivate::Resource *resource : keyboardResources) {
        d->send_key(resource->handle, serial, d->ddeKvm->kvmKeyboardTimestamp(), key,
                    state);
    }
}

void DDEKvmKeyboardInterface::sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial)
{
    const auto keyboardResources = d->resourceMap();
    for (DDEKvmKeyboardInterfacePrivate::Resource *resource : keyboardResources) {
        d->send_modifiers(resource->handle, serial, depressed, latched, locked, group);
    }
}

}
