// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_KEYBOARD_H
#define WL_CLIENT_KEYBOARD_H

#include "Type.h"

namespace wl::client {

class Keyboard : public Type<wl_keyboard>
{
public:
    Keyboard(wl_keyboard *val);
    ~Keyboard();

protected:
    /**
     * keyboard mapping
     *
     * This event provides a file descriptor to the client which can
     * be memory-mapped in read-only mode to provide a keyboard mapping
     * description.
     *
     * From version 7 onwards, the fd must be mapped with MAP_PRIVATE
     * by the recipient, as MAP_SHARED may fail.
     * @param format keymap format
     * @param fd keymap file descriptor
     * @param size keymap size, in bytes
     */
    virtual void wl_keyboard_keymap(uint32_t format, int32_t fd, uint32_t size) = 0;
    /**
     * enter event
     *
     * Notification that this seat's keyboard focus is on a certain
     * surface.
     *
     * The compositor must send the wl_keyboard.modifiers event after
     * this event.
     * @param serial serial number of the enter event
     * @param surface surface gaining keyboard focus
     * @param keys the currently pressed keys
     */
    virtual void wl_keyboard_enter(uint32_t serial,
                                   struct wl_surface *surface,
                                   struct wl_array *keys) = 0;
    /**
     * leave event
     *
     * Notification that this seat's keyboard focus is no longer on a
     * certain surface.
     *
     * The leave notification is sent before the enter notification for
     * the new focus.
     *
     * After this event client must assume that all keys, including
     * modifiers, are lifted and also it must stop key repeating if
     * there's some going on.
     * @param serial serial number of the leave event
     * @param surface surface that lost keyboard focus
     */
    virtual void wl_keyboard_leave(uint32_t serial, struct wl_surface *surface) = 0;
    /**
     * key event
     *
     * A key was pressed or released. The time argument is a
     * timestamp with millisecond granularity, with an undefined base.
     *
     * The key is a platform-specific key code that can be interpreted
     * by feeding it to the keyboard mapping (see the keymap event).
     *
     * If this event produces a change in modifiers, then the resulting
     * wl_keyboard.modifiers event must be sent after this event.
     * @param serial serial number of the key event
     * @param time timestamp with millisecond granularity
     * @param key key that produced the event
     * @param state physical state of the key
     */
    virtual void wl_keyboard_key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state) = 0;
    /**
     * modifier and group state
     *
     * Notifies clients that the modifier and/or group state has
     * changed, and it should update its local state.
     * @param serial serial number of the modifiers event
     * @param mods_depressed depressed modifiers
     * @param mods_latched latched modifiers
     * @param mods_locked locked modifiers
     * @param group keyboard layout
     */
    virtual void wl_keyboard_modifiers(uint32_t serial,
                                       uint32_t mods_depressed,
                                       uint32_t mods_latched,
                                       uint32_t mods_locked,
                                       uint32_t group) = 0;
    /**
     * repeat rate and delay
     *
     * Informs the client about the keyboard's repeat rate and delay.
     *
     * This event is sent as soon as the wl_keyboard object has been
     * created, and is guaranteed to be received by the client before
     * any key press event.
     *
     * Negative values for either rate or delay are illegal. A rate of
     * zero will disable any repeating (regardless of the value of
     * delay).
     *
     * This event can be sent later on as well with a new value if
     * necessary, so clients should continue listening for the event
     * past the creation of wl_keyboard.
     * @param rate the rate of repeating keys in characters per second
     * @param delay delay in milliseconds since key down until repeating starts
     * @since 4
     */
    virtual void wl_keyboard_repeat_info(int32_t rate, int32_t delay) = 0;

private:
    static const wl_keyboard_listener listener_;
};

} // namespace wl::client

#endif // !WL_CLIENT_KEYBOARD_H
