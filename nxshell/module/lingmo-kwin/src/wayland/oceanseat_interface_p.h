/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

// KWayland
#include "oceanseat_interface.h"
// Qt
#include <QHash>
#include <QMap>
#include <QPointer>
#include <QPointF>

#include "qwayland-server-ocean-seat.h"

namespace KWaylandServer
{
class OCEANSeatInterfacePrivate : public QtWaylandServer::ocean_seat
{
public:
    OCEANSeatInterfacePrivate(OCEANSeatInterface *q, Display *d);

    static OCEANSeatInterfacePrivate *get(OCEANSeatInterface *seat);

    OCEANSeatInterface *q;
    QPointer<Display> display;

    QScopedPointer<OCEANPointerInterface> oceanpointer;
    QScopedPointer<OCEANKeyboardInterface> oceankeyboard;
    QScopedPointer<OCEANTouchInterface> oceantouch;

    QPointF globalPos = QPointF(0, 0);
    quint32 timestamp = 0;
    quint32 touchtimestamp = 0;

    // Keyboard related members
    struct Keyboard {
        enum class State {
            Released,
            Pressed
        };
        QHash<quint32, State> states;
        struct Keymap {
            int fd = -1;
            quint32 size = 0;
            bool xkbcommonCompatible = false;
        };
        Keymap keymap;
        struct Modifiers {
            quint32 depressed = 0;
            quint32 latched = 0;
            quint32 locked = 0;
            quint32 group = 0;
            quint32 serial = 0;
        };
        Modifiers modifiers;
        quint32 lastStateSerial = 0;
        struct {
            qint32 charactersPerSecond = 0;
            qint32 delay = 0;
        } keyRepeat;
    };
    Keyboard keys;
    bool updateKey(quint32 key, Keyboard::State state);

protected:
    // interface
    void ocean_seat_get_ocean_pointer(Resource *resource, uint32_t id) override;
    void ocean_seat_get_ocean_keyboard(Resource *resource, uint32_t id) override;
    void ocean_seat_get_ocean_touch(Resource *resource, uint32_t id) override;
};

class OCEANPointerInterfacePrivate : public QtWaylandServer::ocean_pointer
{
public:
    static OCEANPointerInterfacePrivate *get(OCEANPointerInterface *pointer);

    OCEANPointerInterfacePrivate(OCEANPointerInterface *q, OCEANSeatInterface *seat);
    ~OCEANPointerInterfacePrivate() override;

    OCEANPointerInterface *q;
    OCEANSeatInterface *oceanSeat;

protected:
    void ocean_pointer_get_motion(Resource *resource) override;
};

class OCEANTouchInterfacePrivate : public QtWaylandServer::ocean_touch
{
public:
    static OCEANTouchInterfacePrivate *get(OCEANTouchInterface *touch);
    OCEANTouchInterfacePrivate(OCEANTouchInterface *q, OCEANSeatInterface *seat);
    ~OCEANTouchInterfacePrivate() override;

    OCEANTouchInterface *q;
    OCEANSeatInterface *oceanSeat;

protected:
    void ocean_touch_release(Resource *resource) override;
};

}
