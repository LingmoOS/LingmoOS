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

#include <QObject>

#include "lingmo-kwin_export.h"

struct wl_resource;

namespace KWaylandServer
{
class OCEANSeatInterface;
class OCEANKeyboardInterfacePrivate;

/**
 * @brief Resource for the ocean_keyboard interface.
 *
 * OCEANKeyboardInterface gets created by OCEANSeatInterface.
 *
 * @since 5.4
 **/
class KWIN_EXPORT OCEANKeyboardInterface : public QObject
{
    Q_OBJECT
public:
    virtual ~OCEANKeyboardInterface();

    /**
     * @returns The OCEANSeatInterface which created this OCEANPointerInterface.
     **/
    OCEANSeatInterface *oceanSeat() const;

    void setKeymap(int fd, quint32 size);
    void updateModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial);
    void keyPressed(quint32 key, quint32 serial);
    void keyReleased(quint32 key, quint32 serial);
    void repeatInfo(qint32 charactersPerSecond, qint32 delay);

private:
    friend class OCEANSeatInterface;
    friend class OCEANSeatInterfacePrivate;
    friend class OCEANKeyboardInterfacePrivate;
    explicit OCEANKeyboardInterface(OCEANSeatInterface *oceanSeat);
    QScopedPointer<OCEANKeyboardInterfacePrivate> d;
};

}