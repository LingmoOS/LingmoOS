/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

// KWayland
#include "ddekvm_interface.h"
// Qt
#include <QHash>
#include <QMap>
#include <QPointer>
#include <QPointF>

#include "qwayland-server-dde-kvm.h"

namespace KWaylandServer
{
class DDEKvmInterfacePrivate : public QtWaylandServer::dde_kvm
{
public:
    DDEKvmInterfacePrivate(DDEKvmInterface *q, Display *d);

    static DDEKvmInterfacePrivate *get(DDEKvmInterface *kvm);

    DDEKvmInterface *q;
    QPointer<Display> display;

    QScopedPointer<DDEKvmPointerInterface> ddeKvmPointer;
    QScopedPointer<DDEKvmKeyboardInterface> ddeKvmKeyboard;

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
    void dde_kvm_get_dde_kvm_pointer(Resource *resource, uint32_t id) override;
    void dde_kvm_get_dde_kvm_keyboard(Resource *resource, uint32_t id) override;
};

class DDEKvmPointerInterfacePrivate : public QtWaylandServer::dde_kvm_pointer
{
public:
    static DDEKvmPointerInterfacePrivate *get(DDEKvmPointerInterface *pointer);

    DDEKvmPointerInterfacePrivate(DDEKvmPointerInterface *q, DDEKvmInterface *kvm);
    ~DDEKvmPointerInterfacePrivate() override;

    DDEKvmPointerInterface *q;
    DDEKvmInterface *ddeKvm;

protected:
    void dde_kvm_pointer_enable_pointer(Resource *resource, uint32_t is_enable) override;
    void dde_kvm_pointer_enable_cursor(Resource *resource, uint32_t is_enable) override;
    void dde_kvm_pointer_set_pos(Resource *resource, wl_fixed_t x, wl_fixed_t y) override;
};

class DDEKvmKeyboardInterfacePrivate : public QtWaylandServer::dde_kvm_keyboard
{
public:
    static DDEKvmKeyboardInterfacePrivate *get(DDEKvmKeyboardInterface *keyboard);
    DDEKvmKeyboardInterfacePrivate(DDEKvmKeyboardInterface *q, DDEKvmInterface *kvm);
    ~DDEKvmKeyboardInterfacePrivate() override;

    DDEKvmKeyboardInterface *q;
    DDEKvmInterface *ddeKvm;

protected:
    void dde_kvm_keyboard_enable_keyboard(Resource *resource, uint32_t is_enable) override;
};

}