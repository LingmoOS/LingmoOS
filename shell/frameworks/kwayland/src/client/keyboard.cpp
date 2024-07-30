/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "keyboard.h"
#include "surface.h"
#include "wayland_pointer_p.h"
#include <QPointer>
// wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN Keyboard::Private
{
public:
    Private(Keyboard *q);
    void setup(wl_keyboard *k);

    WaylandPointer<wl_keyboard, wl_keyboard_release> keyboard;
    QPointer<Surface> enteredSurface;

    struct {
        qint32 charactersPerSecond = 0;
        qint32 delay = 0;
    } repeatInfo;

private:
    void enter(uint32_t serial, wl_surface *surface, wl_array *keys);
    void leave(uint32_t serial);
    static void keymapCallback(void *data, wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size);
    static void enterCallback(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys);
    static void leaveCallback(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface);
    static void keyCallback(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    static void
    modifiersCallback(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t modsDepressed, uint32_t modsLatched, uint32_t modsLocked, uint32_t group);
    static void repeatInfoCallback(void *data, wl_keyboard *keyboard, int32_t charactersPerSecond, int32_t delay);
    Keyboard *q;
    static const wl_keyboard_listener s_listener;
};

Keyboard::Private::Private(Keyboard *q)
    : q(q)
{
}

void Keyboard::Private::setup(wl_keyboard *k)
{
    Q_ASSERT(k);
    Q_ASSERT(!keyboard);
    keyboard.setup(k);
    wl_keyboard_add_listener(keyboard, &s_listener, this);
}

const wl_keyboard_listener Keyboard::Private::s_listener = {keymapCallback, enterCallback, leaveCallback, keyCallback, modifiersCallback, repeatInfoCallback};

Keyboard::Keyboard(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Keyboard::~Keyboard()
{
    release();
}

void Keyboard::release()
{
    d->keyboard.release();
}

void Keyboard::destroy()
{
    d->keyboard.destroy();
}

void Keyboard::setup(wl_keyboard *keyboard)
{
    d->setup(keyboard);
}

void Keyboard::Private::enterCallback(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys)
{
    auto k = reinterpret_cast<Private *>(data);
    Q_ASSERT(k->keyboard == keyboard);
    k->enter(serial, surface, keys);
}

void Keyboard::Private::enter(uint32_t serial, wl_surface *surface, wl_array *keys)
{
    Q_UNUSED(keys)
    enteredSurface = Surface::get(surface);
    Q_EMIT q->entered(serial);
}

void Keyboard::Private::leaveCallback(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface)
{
    Q_UNUSED(surface)
    auto k = reinterpret_cast<Private *>(data);
    Q_ASSERT(k->keyboard == keyboard);
    k->leave(serial);
}

void Keyboard::Private::leave(uint32_t serial)
{
    enteredSurface.clear();
    Q_EMIT q->left(serial);
}

void Keyboard::Private::keyCallback(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    Q_UNUSED(serial)
    auto k = reinterpret_cast<Keyboard::Private *>(data);
    Q_ASSERT(k->keyboard == keyboard);
    auto toState = [state] {
        if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
            return KeyState::Released;
        } else {
            return KeyState::Pressed;
        }
    };
    Q_EMIT k->q->keyChanged(key, toState(), time);
}

void Keyboard::Private::keymapCallback(void *data, wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size)
{
    auto k = reinterpret_cast<Keyboard::Private *>(data);
    Q_ASSERT(k->keyboard == keyboard);
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        return;
    }
    Q_EMIT k->q->keymapChanged(fd, size);
}

void Keyboard::Private::modifiersCallback(void *data,
                                          wl_keyboard *keyboard,
                                          uint32_t serial,
                                          uint32_t modsDepressed,
                                          uint32_t modsLatched,
                                          uint32_t modsLocked,
                                          uint32_t group)
{
    Q_UNUSED(serial)
    auto k = reinterpret_cast<Keyboard::Private *>(data);
    Q_ASSERT(k->keyboard == keyboard);
    Q_EMIT k->q->modifiersChanged(modsDepressed, modsLatched, modsLocked, group);
}

void Keyboard::Private::repeatInfoCallback(void *data, wl_keyboard *keyboard, int32_t charactersPerSecond, int32_t delay)
{
    auto k = reinterpret_cast<Keyboard::Private *>(data);
    Q_ASSERT(k->keyboard == keyboard);
    k->repeatInfo.charactersPerSecond = qMax(charactersPerSecond, 0);
    k->repeatInfo.delay = qMax(delay, 0);
    Q_EMIT k->q->keyRepeatChanged();
}

Surface *Keyboard::enteredSurface()
{
    return d->enteredSurface.data();
}

Surface *Keyboard::enteredSurface() const
{
    return d->enteredSurface.data();
}

bool Keyboard::isValid() const
{
    return d->keyboard.isValid();
}

bool Keyboard::isKeyRepeatEnabled() const
{
    return d->repeatInfo.charactersPerSecond > 0;
}

qint32 Keyboard::keyRepeatDelay() const
{
    return d->repeatInfo.delay;
}

qint32 Keyboard::keyRepeatRate() const
{
    return d->repeatInfo.charactersPerSecond;
}

Keyboard::operator wl_keyboard *()
{
    return d->keyboard;
}

Keyboard::operator wl_keyboard *() const
{
    return d->keyboard;
}

}
}

#include "moc_keyboard.cpp"
