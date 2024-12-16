// Copyright 2021  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "oceanseat.h"
#include "oceankeyboard.h"
#include "wayland_pointer_p.h"
// wayland
#include "wayland-ocean-seat-client-protocol.h"
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN OCEANKeyboard::Private
{
public:
    Private(OCEANKeyboard *q);
    void setup(ocean_keyboard *k);

    WaylandPointer<ocean_keyboard, ocean_keyboard_release> oceanKeyboard;

    struct {
        qint32 charactersPerSecond = 0;
        qint32 delay = 0;
    } repeatInfo;
private:
    static void keymapCallback(void *data, ocean_keyboard *keyboard, uint32_t format, int fd, uint32_t size);
    static void enterCallback(void *data, ocean_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys);
    static void leaveCallback(void *data, ocean_keyboard *keyboard, uint32_t serial, wl_surface *surface);
    static void keyCallback(void *data, ocean_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    static void modifiersCallback(void *data, ocean_keyboard *keyboard, uint32_t serial, uint32_t modsDepressed,
                                  uint32_t modsLatched, uint32_t modsLocked, uint32_t group);
    static void repeatInfoCallback(void *data, ocean_keyboard *keyboard, int32_t charactersPerSecond, int32_t delay);
    OCEANKeyboard *q;
    static const ocean_keyboard_listener s_listener;
};

OCEANKeyboard::Private::Private(OCEANKeyboard *q)
    : q(q)
{
}

void OCEANKeyboard::Private::setup(ocean_keyboard *k)
{
    Q_ASSERT(k);
    Q_ASSERT(!oceanKeyboard);
    oceanKeyboard.setup(k);
    ocean_keyboard_add_listener(oceanKeyboard, &s_listener, this);
}

const ocean_keyboard_listener OCEANKeyboard::Private::s_listener = {
    keymapCallback,
    enterCallback,
    leaveCallback,
    keyCallback,
    modifiersCallback,
    repeatInfoCallback,
};

OCEANKeyboard::OCEANKeyboard(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

OCEANKeyboard::~OCEANKeyboard()
{
    release();
}

void OCEANKeyboard::release()
{
    d->oceanKeyboard.release();
}

void OCEANKeyboard::destroy()
{
    d->oceanKeyboard.destroy();
}

void OCEANKeyboard::setup(ocean_keyboard *keyboard)
{
    d->setup(keyboard);
}

bool OCEANKeyboard::isValid() const
{
    return d->oceanKeyboard.isValid();
}

OCEANKeyboard::operator ocean_keyboard*()
{
    return d->oceanKeyboard;
}

OCEANKeyboard::operator ocean_keyboard*() const
{
    return d->oceanKeyboard;
}

void OCEANKeyboard::Private::enterCallback(void *data, ocean_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys)
{
    Q_UNUSED(data)
    Q_UNUSED(keyboard)
    Q_UNUSED(serial)
    Q_UNUSED(surface)
    Q_UNUSED(keys)
}

void OCEANKeyboard::Private::leaveCallback(void *data, ocean_keyboard *keyboard, uint32_t serial, wl_surface *surface)
{
    Q_UNUSED(data)
    Q_UNUSED(keyboard)
    Q_UNUSED(serial)
    Q_UNUSED(surface)
}

void OCEANKeyboard::Private::keyCallback(void *data, ocean_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    Q_UNUSED(serial)
    auto k = reinterpret_cast<OCEANKeyboard::Private*>(data);
    Q_ASSERT(k->oceanKeyboard == keyboard);
    auto toState = [state] {
        if (state == OCEAN_KEYBOARD_KEY_STATE_RELEASED) {
            return KeyState::Released;
        } else {
            return KeyState::Pressed;
        }
    };
    Q_EMIT k->q->keyChanged(key, toState(), time);
}

void OCEANKeyboard::Private::keymapCallback(void *data, ocean_keyboard *keyboard, uint32_t format, int fd, uint32_t size)
{
    auto k = reinterpret_cast<OCEANKeyboard::Private*>(data);
    Q_ASSERT(k->oceanKeyboard == keyboard);
    if (format != OCEAN_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        return;
    }
    Q_EMIT k->q->keymapChanged(fd, size);
}

void OCEANKeyboard::Private::modifiersCallback(void *data, ocean_keyboard *keyboard, uint32_t serial, uint32_t modsDepressed,
                                 uint32_t modsLatched, uint32_t modsLocked, uint32_t group)
{
    Q_UNUSED(serial)
    auto k = reinterpret_cast<OCEANKeyboard::Private*>(data);
    Q_ASSERT(k->oceanKeyboard == keyboard);
    Q_EMIT k->q->modifiersChanged(modsDepressed, modsLatched, modsLocked, group);
}

void OCEANKeyboard::Private::repeatInfoCallback(void *data, ocean_keyboard *keyboard, int32_t charactersPerSecond, int32_t delay)
{
    auto k = reinterpret_cast<OCEANKeyboard::Private*>(data);
    Q_ASSERT(k->oceanKeyboard == keyboard);
    k->repeatInfo.charactersPerSecond = qMax(charactersPerSecond, 0);
    k->repeatInfo.delay = qMax(delay, 0);
    Q_EMIT k->q->keyRepeatChanged();
}

}
}