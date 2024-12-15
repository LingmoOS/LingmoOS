// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "InputMethodV2.h"

#include "Output.h"
#include "Server.h"

extern "C" {
#include <wlr/types/wlr_compositor.h>
}

WL_ADDONS_BASE_USE_NAMESPACE

InputMethodV2::InputMethodV2(Server *server, wlr_input_method_v2 *input_method)
    : server_(server)
    , input_method_(input_method)
    , commit_(this)
    , new_popup_surface_(this)
    , grab_keyboard_(this)
    , destroy_(this)
    , popup_destroy_(this)
    , popup_surface_commit_(this)
    , keyboard_grab_destroy_(this)
{
    wl_signal_add(&input_method_->events.commit, commit_);
    wl_signal_add(&input_method_->events.new_popup_surface, new_popup_surface_);
    wl_signal_add(&input_method_->events.grab_keyboard, grab_keyboard_);
    wl_signal_add(&input_method_->events.destroy, destroy_);
}

InputMethodV2::~InputMethodV2() { }

void InputMethodV2::sendActivate()
{
    wlr_input_method_v2_send_activate(input_method_);
}

void InputMethodV2::sendDeactivate()
{
    wlr_input_method_v2_send_deactivate(input_method_);
}

void InputMethodV2::sendContentType(uint32_t hint, uint32_t purpose)
{
    wlr_input_method_v2_send_content_type(input_method_, hint, purpose);
}

void InputMethodV2::sendSurroundingText(const char *text, uint32_t cursor, uint32_t anchor)
{
    wlr_input_method_v2_send_surrounding_text(input_method_, text, cursor, anchor);
}

void InputMethodV2::sendDone()
{
    wlr_input_method_v2_send_done(input_method_);
}

void InputMethodV2::setCursorRectangle(int x, int y, int width, int height)
{
    if (!popup_) {
        return;
    }

    wlr_box rectangle{ x, y, width, height };
    wlr_input_popup_surface_v2_send_text_input_rectangle(popup_, &rectangle);
}

void InputMethodV2::sendKey(uint32_t keycode, bool isRelease)
{
    if (!input_method_->keyboard_grab) {
        return;
    }

    uint32_t ts = getTimestamp();

    wlr_input_method_keyboard_grab_v2_send_key(input_method_->keyboard_grab,
                                               ts,
                                               keycode,
                                               isRelease ? WL_KEYBOARD_KEY_STATE_RELEASED
                                                         : WL_KEYBOARD_KEY_STATE_PRESSED);
}

void InputMethodV2::commitNotify(void *data)
{
    if (commitCallback_) {
        commitCallback_();
    }
}

void InputMethodV2::newPopupSurfaceNotify(void *data)
{
    popup_ = static_cast<wlr_input_popup_surface_v2 *>(data);
    wl_signal_add(&popup_->events.destroy, popup_destroy_);
    wl_signal_add(&popup_->surface->events.commit, popup_surface_commit_);

    if (popupCreateCallback_) {
        popupCreateCallback_();
    }
}

void InputMethodV2::grabKeyboardNotify(void *data)
{
    auto *keyboard_grab = static_cast<wlr_input_method_keyboard_grab_v2 *>(data);

    struct wlr_keyboard *active_keyboard = wlr_seat_get_keyboard(input_method_->seat);

    // send modifier state to grab
    wlr_input_method_keyboard_grab_v2_set_keyboard(keyboard_grab, active_keyboard);

    wl_signal_add(&keyboard_grab->events.destroy, keyboard_grab_destroy_);
}

void InputMethodV2::destroyNotify(void *data)
{
    delete this;
}

void InputMethodV2::popupDestroyNotify(void *data)
{
    popup_ = nullptr;

    if (popupDestroyCallback_) {
        popupDestroyCallback_();
    }
}

void InputMethodV2::popupSurfaceCommitNotify(void *data)
{
    auto &state = popup_->surface->current;
    server_->output()->setSize(state.width, state.height);
}

void InputMethodV2::keyboardGrabDestroyNotify(void *data)
{
    auto *keyboard_grab = static_cast<wlr_input_method_keyboard_grab_v2 *>(data);
    // wl_list_remove(&input_method_keyboard_grab_destroy_.link);

    if (keyboard_grab->keyboard) {
        // send modifier state to original client
        wlr_seat_keyboard_notify_modifiers(keyboard_grab->input_method->seat,
                                           &keyboard_grab->keyboard->modifiers);
    }
}

void InputMethodV2::grabberModifiersNotify(void *data)
{
    auto *modifiers = static_cast<wlr_keyboard_modifiers *>(data);

    wlr_input_method_keyboard_grab_v2_send_modifiers(input_method_->keyboard_grab, modifiers);
}
