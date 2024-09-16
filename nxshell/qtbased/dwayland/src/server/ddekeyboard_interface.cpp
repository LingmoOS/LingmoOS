// Copyright 2022  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "ddeseat_interface.h"
#include "ddekeyboard_interface.h"
#include "display.h"
#include "logging.h"

#include "qwayland-server-dde-seat.h"

#include "ddekeyboard_interface_p.h"

namespace KWaylandServer
{
/*********************************
 * DDEKeyboardInterface
 *********************************/
DDEKeyboardInterfacePrivate *DDEKeyboardInterfacePrivate::get(DDEKeyboardInterface *keyboard)
{
    return keyboard->d.data();
}

DDEKeyboardInterfacePrivate::DDEKeyboardInterfacePrivate(DDEKeyboardInterface *q, DDESeatInterface *seat, wl_resource *resource)
    : QtWaylandServer::dde_keyboard(resource)
    , q(q)
    , ddeSeat(seat)
{
}

DDEKeyboardInterfacePrivate::~DDEKeyboardInterfacePrivate() = default;

void DDEKeyboardInterfacePrivate::dde_keyboard_release(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

DDEKeyboardInterface::DDEKeyboardInterface(DDESeatInterface *seat, wl_resource *resource)
    : d(new DDEKeyboardInterfacePrivate(this, seat, resource))
{
}

DDEKeyboardInterface::~DDEKeyboardInterface() = default;

DDESeatInterface *DDEKeyboardInterface::ddeSeat() const
{
    return d->ddeSeat;
}

void DDEKeyboardInterface::setKeymap(int fd, quint32 size)
{
    d->sendKeymap(fd, size);
}

void DDEKeyboardInterfacePrivate::sendKeymap(int fd, quint32 size)
{
    send_keymap(keymap_format_xkb_v1, fd, size);
}

void DDEKeyboardInterfacePrivate::sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial)
{
    send_modifiers(serial, depressed, latched, locked, group);
}

void DDEKeyboardInterfacePrivate::sendModifiers()
{
    sendModifiers(ddeSeat->depressedModifiers(), ddeSeat->latchedModifiers(), ddeSeat->lockedModifiers(), ddeSeat->groupModifiers(), ddeSeat->lastModifiersSerial());
}

void DDEKeyboardInterface::keyPressed(quint32 key, quint32 serial)
{
    d->send_key(serial, d->ddeSeat->timestamp(), key, QtWaylandServer::dde_keyboard::key_state::key_state_pressed);
}

void DDEKeyboardInterface::keyReleased(quint32 key, quint32 serial)
{
    d->send_key(serial, d->ddeSeat->timestamp(), key, QtWaylandServer::dde_keyboard::key_state::key_state_released);
}

void DDEKeyboardInterface::updateModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial)
{
    d->sendModifiers(depressed, latched, locked, group, serial);
}

void DDEKeyboardInterface::repeatInfo(qint32 charactersPerSecond, qint32 delay)
{
    if (d->resource()->version() < WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION) {
        // only supported since version 4
        return;
    }
    d->send_repeat_info(charactersPerSecond, delay);
}

}