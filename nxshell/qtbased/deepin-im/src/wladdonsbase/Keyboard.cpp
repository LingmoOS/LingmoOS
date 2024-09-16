// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Keyboard.h"

#include "InputMethodV2.h"
#include "Server.h"

extern "C" {
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_virtual_keyboard_v1.h>
#include <wlr/util/log.h>
}

WL_ADDONS_BASE_USE_NAMESPACE

Keyboard::Keyboard(Server *server, wlr_input_device *device, wl_list *list, bool isVirtual)
    : server_(server)
    , device_(device)
    , keyboard_(wlr_keyboard_from_input_device(device))
    , isVirtual_(isVirtual)
    , key_(this)
    , modifiers_(this)
    , keymap_(this)
    , destroy_(this)
    , repeat_info_(this)
{
    /* We need to prepare an XKB keymap and assign it to the keyboard. This
     * assumes the defaults (e.g. layout = "us"). */
    xkb_ctx_.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));
    xkb_keymap_.reset(
        xkb_keymap_new_from_names(xkb_ctx_.get(), nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS));

    wlr_keyboard_set_keymap(keyboard_, xkb_keymap_.get());
    wlr_keyboard_set_repeat_info(keyboard_, 25, 600);

    /* Here we set up listeners for keyboard events. */
    wl_signal_add(&keyboard_->events.key, key_);
    wl_signal_add(&keyboard_->events.modifiers, modifiers_);
    wl_signal_add(&keyboard_->events.keymap, keymap_);
    wl_signal_add(&keyboard_->events.repeat_info, repeat_info_);
    wl_signal_add(&device->events.destroy, destroy_);

    wlr_seat_set_keyboard(server_->seat(), keyboard_);

    /* And add the keyboard to our list of keyboards */
    wl_list_insert(list, &link_);
}

Keyboard::~Keyboard()
{
    wl_list_remove(&link_);
}

void Keyboard::keyNotify(void *data)
{
    /* This event is raised when a key is pressed or released. */
    struct wlr_keyboard_key_event *event = static_cast<wlr_keyboard_key_event *>(data);
    struct wlr_seat *seat = server_->seat();

    bool handled = false;
    auto *keyboardGrab = getKeyboardGrab();
    if (keyboardGrab) {
        wlr_input_method_keyboard_grab_v2_set_keyboard(keyboardGrab, keyboard_);
        wlr_input_method_keyboard_grab_v2_send_key(keyboardGrab,
                                                   event->time_msec,
                                                   event->keycode,
                                                   event->state);
        handled = true;
    }

    if (!handled) {
        if (isVirtual_) {
            if (keyEventCallback_) {
                keyEventCallback_(event);
            }
        } else {
            /* Otherwise, we pass it along to the client. */
            wlr_seat_set_keyboard(seat, keyboard_);
            wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode, event->state);
        }
    }
}

void Keyboard::modifiersNotify(void *data)
{
    /* This event is raised when a modifier key, such as shift or alt, is
     * pressed. We simply communicate this to the client. */
    auto *keyboardGrab = getKeyboardGrab();
    if (keyboardGrab) {
        wlr_input_method_keyboard_grab_v2_set_keyboard(keyboardGrab, keyboard_);
        wlr_input_method_keyboard_grab_v2_send_modifiers(keyboardGrab, &keyboard_->modifiers);
    } else {
        /*
         * A seat can only have one keyboard, but this is a limitation of the
         * Wayland protocol - not wlroots. We assign all connected keyboards to the
         * same seat. You can swap out the underlying wlr_keyboard like this and
         * wlr_seat handles this transparently.
         */
        wlr_seat_set_keyboard(server_->seat(), keyboard_);
        /* Send modifiers to the client. */
        wlr_seat_keyboard_notify_modifiers(server_->seat(), &keyboard_->modifiers);
    }
}

void Keyboard::keymapNotify(void *data) { }

void Keyboard::repeatInfoNotify(void *data) { }

void Keyboard::destroyNotify(void *data)
{
    /* This event is raised by the keyboard base wlr_input_device to signal
     * the destruction of the wlr_keyboard. It will no longer receive events
     * and should be destroyed.
     */
    delete this;
}

wlr_input_method_keyboard_grab_v2 *Keyboard::getKeyboardGrab()
{
    auto &inputMethodV2s = server_->inputMethodV2s();
    if (inputMethodV2s.empty()) {
        return nullptr;
    }

    wlr_input_method_v2 *wlrInputMethod = nullptr;
    for (auto &[_, inputMethod] : inputMethodV2s) {
        wlrInputMethod = inputMethod->input_method_;
        if (wlrInputMethod->keyboard_grab == nullptr) {
            continue;
        }
    }
    if (wlrInputMethod == nullptr) {
        return nullptr;
    }

    auto *virtualKeyboard = wlr_input_device_get_virtual_keyboard(device_);
    if (virtualKeyboard
        && wl_resource_get_client(virtualKeyboard->resource)
            == wl_resource_get_client(wlrInputMethod->resource)) {
        return nullptr;
    }

    return wlrInputMethod->keyboard_grab;
}
