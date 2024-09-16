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

#include "ddeseat_interface.h"
#include "ddekeyboard_interface.h"
#include "display.h"

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

DDEKeyboardInterfacePrivate::DDEKeyboardInterfacePrivate(DDEKeyboardInterface *q, DDESeatInterface *seat)
    : q(q)
    , ddeSeat(seat)
{
}

DDEKeyboardInterfacePrivate::~DDEKeyboardInterfacePrivate() = default;

void DDEKeyboardInterfacePrivate::dde_keyboard_release(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

DDEKeyboardInterface::DDEKeyboardInterface(DDESeatInterface *seat)
    : d(new DDEKeyboardInterfacePrivate(this, seat))
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
    const auto keyboardResources = resourceMap();
    for (Resource *resource : keyboardResources) {
        send_keymap(resource->handle, keymap_format_xkb_v1, fd, size);
    }
}

void DDEKeyboardInterfacePrivate::sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial)
{
    const auto keyboardResources = resourceMap();
    for (Resource *resource : keyboardResources) {
        send_modifiers(resource->handle, serial, depressed, latched, locked, group);
    }
}

void DDEKeyboardInterfacePrivate::sendModifiers()
{
    sendModifiers(ddeSeat->depressedModifiers(),
                  ddeSeat->latchedModifiers(),
                  ddeSeat->lockedModifiers(),
                  ddeSeat->groupModifiers(),
                  ddeSeat->lastModifiersSerial());
}

void DDEKeyboardInterface::keyPressed(quint32 key, quint32 serial)
{
    const auto keyboardResources = d->resourceMap();
    for (DDEKeyboardInterfacePrivate::Resource *resource : keyboardResources) {
        d->send_key(resource->handle, serial, d->ddeSeat->timestamp(), key,
                    QtWaylandServer::dde_keyboard::key_state::key_state_pressed);
    }
}

void DDEKeyboardInterface::keyReleased(quint32 key, quint32 serial)
{
    const auto keyboardResources = d->resourceMap();
    for (DDEKeyboardInterfacePrivate::Resource *resource : keyboardResources) {
        d->send_key(resource->handle, serial, d->ddeSeat->timestamp(), key,
                    QtWaylandServer::dde_keyboard::key_state::key_state_released);
    }
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
    const auto keyboardResources = d->resourceMap();
    for (DDEKeyboardInterfacePrivate::Resource *resource : keyboardResources) {
        d->send_repeat_info(resource->handle, charactersPerSecond, delay);
    }
}

}
