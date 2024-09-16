/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

// KWayland
#include "ddeseat_interface.h"
// Qt
#include <QHash>
#include <QMap>
#include <QPointer>
#include <QPointF>

#include "qwayland-server-dde-seat.h"

namespace KWaylandServer
{
class DDESeatInterfacePrivate : public QtWaylandServer::dde_seat
{
public:
    DDESeatInterfacePrivate(DDESeatInterface *q, Display *d);

    static DDESeatInterfacePrivate *get(DDESeatInterface *seat);

    DDESeatInterface *q;
    QPointer<Display> display;

    QScopedPointer<DDEPointerInterface> ddepointer;
    QScopedPointer<DDEKeyboardInterface> ddekeyboard;
    QScopedPointer<DDETouchInterface> ddetouch;

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
    void dde_seat_get_dde_pointer(Resource *resource, uint32_t id) override;
    void dde_seat_get_dde_keyboard(Resource *resource, uint32_t id) override;
    void dde_seat_get_dde_touch(Resource *resource, uint32_t id) override;
};

class DDEPointerInterfacePrivate : public QtWaylandServer::dde_pointer
{
public:
    static DDEPointerInterfacePrivate *get(DDEPointerInterface *pointer);

    DDEPointerInterfacePrivate(DDEPointerInterface *q, DDESeatInterface *seat);
    ~DDEPointerInterfacePrivate() override;

    DDEPointerInterface *q;
    DDESeatInterface *ddeSeat;

protected:
    void dde_pointer_get_motion(Resource *resource) override;
};

class DDETouchInterfacePrivate : public QtWaylandServer::dde_touch
{
public:
    static DDETouchInterfacePrivate *get(DDETouchInterface *touch);
    DDETouchInterfacePrivate(DDETouchInterface *q, DDESeatInterface *seat);
    ~DDETouchInterfacePrivate() override;

    DDETouchInterface *q;
    DDESeatInterface *ddeSeat;

protected:
    void dde_touch_release(Resource *resource) override;
};

}
