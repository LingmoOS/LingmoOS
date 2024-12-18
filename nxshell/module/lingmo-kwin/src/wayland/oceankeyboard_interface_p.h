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
#include "oceankeyboard_interface.h"
// Qt
#include <QHash>
#include <QMap>
#include <QPointer>
#include <QPointF>

#include "qwayland-server-ocean-seat.h"

namespace KWaylandServer
{
class OCEANKeyboardInterfacePrivate : public QtWaylandServer::ocean_keyboard
{
public:
    static OCEANKeyboardInterfacePrivate *get(OCEANKeyboardInterface *oceankeyboard);

    OCEANKeyboardInterfacePrivate(OCEANKeyboardInterface *q, OCEANSeatInterface *seat);
    ~OCEANKeyboardInterfacePrivate() override;

    void sendKeymap(int fd, quint32 size);
    void sendModifiers();
    void sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial);

    OCEANKeyboardInterface *q;
    OCEANSeatInterface *oceanSeat;
protected:
    void ocean_keyboard_release(Resource *resource) override;
};

}
