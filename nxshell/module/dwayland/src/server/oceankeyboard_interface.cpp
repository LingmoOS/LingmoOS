// Copyright 2022  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "oceanseat_interface.h"
#include "oceankeyboard_interface.h"
#include "display.h"
#include "logging.h"

#include "qwayland-server-ocean-seat.h"

#include "oceankeyboard_interface_p.h"

namespace KWaylandServer
{
/*********************************
 * OCEANKeyboardInterface
 *********************************/
OCEANKeyboardInterfacePrivate *OCEANKeyboardInterfacePrivate::get(OCEANKeyboardInterface *keyboard)
{
    return keyboard->d.data();
}

OCEANKeyboardInterfacePrivate::OCEANKeyboardInterfacePrivate(OCEANKeyboardInterface *q, OCEANSeatInterface *seat, wl_resource *resource)
    : QtWaylandServer::ocean_keyboard(resource)
    , q(q)
    , oceanSeat(seat)
{
}

OCEANKeyboardInterfacePrivate::~OCEANKeyboardInterfacePrivate() = default;

void OCEANKeyboardInterfacePrivate::ocean_keyboard_release(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

OCEANKeyboardInterface::OCEANKeyboardInterface(OCEANSeatInterface *seat, wl_resource *resource)
    : d(new OCEANKeyboardInterfacePrivate(this, seat, resource))
{
}

OCEANKeyboardInterface::~OCEANKeyboardInterface() = default;

OCEANSeatInterface *OCEANKeyboardInterface::oceanSeat() const
{
    return d->oceanSeat;
}

void OCEANKeyboardInterface::setKeymap(int fd, quint32 size)
{
    d->sendKeymap(fd, size);
}

void OCEANKeyboardInterfacePrivate::sendKeymap(int fd, quint32 size)
{
    send_keymap(keymap_format_xkb_v1, fd, size);
}

void OCEANKeyboardInterfacePrivate::sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial)
{
    send_modifiers(serial, depressed, latched, locked, group);
}

void OCEANKeyboardInterfacePrivate::sendModifiers()
{
    sendModifiers(oceanSeat->depressedModifiers(), oceanSeat->latchedModifiers(), oceanSeat->lockedModifiers(), oceanSeat->groupModifiers(), oceanSeat->lastModifiersSerial());
}

void OCEANKeyboardInterface::keyPressed(quint32 key, quint32 serial)
{
    d->send_key(serial, d->oceanSeat->timestamp(), key, QtWaylandServer::ocean_keyboard::key_state::key_state_pressed);
}

void OCEANKeyboardInterface::keyReleased(quint32 key, quint32 serial)
{
    d->send_key(serial, d->oceanSeat->timestamp(), key, QtWaylandServer::ocean_keyboard::key_state::key_state_released);
}

void OCEANKeyboardInterface::updateModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial)
{
    d->sendModifiers(depressed, latched, locked, group, serial);
}

void OCEANKeyboardInterface::repeatInfo(qint32 charactersPerSecond, qint32 delay)
{
    if (d->resource()->version() < WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION) {
        // only supported since version 4
        return;
    }
    d->send_repeat_info(charactersPerSecond, delay);
}

}