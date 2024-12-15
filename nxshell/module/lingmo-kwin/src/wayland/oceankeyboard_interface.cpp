/********************************************************************
Copyright 2022  luochaojiang <luochaojiang@uniontech.com>

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

#include "oceanseat_interface.h"
#include "oceankeyboard_interface.h"
#include "display.h"

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

OCEANKeyboardInterfacePrivate::OCEANKeyboardInterfacePrivate(OCEANKeyboardInterface *q, OCEANSeatInterface *seat)
    : q(q)
    , oceanSeat(seat)
{
}

OCEANKeyboardInterfacePrivate::~OCEANKeyboardInterfacePrivate() = default;

void OCEANKeyboardInterfacePrivate::ocean_keyboard_release(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

OCEANKeyboardInterface::OCEANKeyboardInterface(OCEANSeatInterface *seat)
    : d(new OCEANKeyboardInterfacePrivate(this, seat))
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
    const auto keyboardResources = resourceMap();
    for (Resource *resource : keyboardResources) {
        send_keymap(resource->handle, keymap_format_xkb_v1, fd, size);
    }
}

void OCEANKeyboardInterfacePrivate::sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial)
{
    const auto keyboardResources = resourceMap();
    for (Resource *resource : keyboardResources) {
        send_modifiers(resource->handle, serial, depressed, latched, locked, group);
    }
}

void OCEANKeyboardInterfacePrivate::sendModifiers()
{
    sendModifiers(oceanSeat->depressedModifiers(),
                  oceanSeat->latchedModifiers(),
                  oceanSeat->lockedModifiers(),
                  oceanSeat->groupModifiers(),
                  oceanSeat->lastModifiersSerial());
}

void OCEANKeyboardInterface::keyPressed(quint32 key, quint32 serial)
{
    const auto keyboardResources = d->resourceMap();
    for (OCEANKeyboardInterfacePrivate::Resource *resource : keyboardResources) {
        d->send_key(resource->handle, serial, d->oceanSeat->timestamp(), key,
                    QtWaylandServer::ocean_keyboard::key_state::key_state_pressed);
    }
}

void OCEANKeyboardInterface::keyReleased(quint32 key, quint32 serial)
{
    const auto keyboardResources = d->resourceMap();
    for (OCEANKeyboardInterfacePrivate::Resource *resource : keyboardResources) {
        d->send_key(resource->handle, serial, d->oceanSeat->timestamp(), key,
                    QtWaylandServer::ocean_keyboard::key_state::key_state_released);
    }
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
    const auto keyboardResources = d->resourceMap();
    for (OCEANKeyboardInterfacePrivate::Resource *resource : keyboardResources) {
        d->send_repeat_info(resource->handle, charactersPerSecond, delay);
    }
}

}
