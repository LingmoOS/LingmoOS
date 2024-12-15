/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

// KWayland
#include "oceankvm_interface.h"
// Qt
#include <QHash>
#include <QMap>
#include <QPointer>
#include <QPointF>

#include "qwayland-server-ocean-kvm.h"

namespace KWaylandServer
{
class OCEANKvmInterfacePrivate : public QtWaylandServer::ocean_kvm
{
public:
    OCEANKvmInterfacePrivate(OCEANKvmInterface *q, Display *d);

    static OCEANKvmInterfacePrivate *get(OCEANKvmInterface *kvm);

    OCEANKvmInterface *q;
    QPointer<Display> display;

    QScopedPointer<OCEANKvmPointerInterface> oceanKvmPointer;
    QScopedPointer<OCEANKvmKeyboardInterface> oceanKvmKeyboard;

    QPointF globalPos = QPointF(0, 0);
    quint32 kvmPointerTimestamp = 0;
    quint32 kvmKeyboardTimestamp = 0;

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
    bool updateKeyState(quint32 key, quint32 state);

protected:
    // interface
    void ocean_kvm_get_ocean_kvm_pointer(Resource *resource, uint32_t id) override;
    void ocean_kvm_get_ocean_kvm_keyboard(Resource *resource, uint32_t id) override;
};

class OCEANKvmPointerInterfacePrivate : public QtWaylandServer::ocean_kvm_pointer
{
public:
    static OCEANKvmPointerInterfacePrivate *get(OCEANKvmPointerInterface *pointer);

    OCEANKvmPointerInterfacePrivate(OCEANKvmPointerInterface *q, OCEANKvmInterface *kvm);
    ~OCEANKvmPointerInterfacePrivate() override;

    OCEANKvmPointerInterface *q;
    OCEANKvmInterface *oceanKvm;

protected:
    void ocean_kvm_pointer_enable_pointer(Resource *resource, uint32_t is_enable) override;
    void ocean_kvm_pointer_enable_cursor(Resource *resource, uint32_t is_enable) override;
    void ocean_kvm_pointer_set_pos(Resource *resource, wl_fixed_t x, wl_fixed_t y) override;
};

class OCEANKvmKeyboardInterfacePrivate : public QtWaylandServer::ocean_kvm_keyboard
{
public:
    static OCEANKvmKeyboardInterfacePrivate *get(OCEANKvmKeyboardInterface *keyboard);
    OCEANKvmKeyboardInterfacePrivate(OCEANKvmKeyboardInterface *q, OCEANKvmInterface *kvm);
    ~OCEANKvmKeyboardInterfacePrivate() override;

    OCEANKvmKeyboardInterface *q;
    OCEANKvmInterface *oceanKvm;

protected:
    void ocean_kvm_keyboard_enable_keyboard(Resource *resource, uint32_t is_enable) override;
};

}