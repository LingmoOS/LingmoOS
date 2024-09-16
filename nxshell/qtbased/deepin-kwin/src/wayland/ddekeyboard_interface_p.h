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
#pragma once

// KWayland
#include "ddekeyboard_interface.h"
// Qt
#include <QHash>
#include <QMap>
#include <QPointer>
#include <QPointF>

#include "qwayland-server-dde-seat.h"

namespace KWaylandServer
{
class DDEKeyboardInterfacePrivate : public QtWaylandServer::dde_keyboard
{
public:
    static DDEKeyboardInterfacePrivate *get(DDEKeyboardInterface *ddekeyboard);

    DDEKeyboardInterfacePrivate(DDEKeyboardInterface *q, DDESeatInterface *seat);
    ~DDEKeyboardInterfacePrivate() override;

    void sendKeymap(int fd, quint32 size);
    void sendModifiers();
    void sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial);

    DDEKeyboardInterface *q;
    DDESeatInterface *ddeSeat;
protected:
    void dde_keyboard_release(Resource *resource) override;
};

}
