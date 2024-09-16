// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ZwpInputMethodKeyboardGrabV2.h"

using namespace wl::client;

const zwp_input_method_keyboard_grab_v2_listener ZwpInputMethodKeyboardGrabV2::listener_ = {
    ListenerCallbackWrapper<&ZwpInputMethodKeyboardGrabV2::zwp_input_method_keyboard_grab_v2_keymap>::func,
    ListenerCallbackWrapper<&ZwpInputMethodKeyboardGrabV2::zwp_input_method_keyboard_grab_v2_key>::func,
    ListenerCallbackWrapper<&ZwpInputMethodKeyboardGrabV2::zwp_input_method_keyboard_grab_v2_modifiers>::func,
    ListenerCallbackWrapper<&ZwpInputMethodKeyboardGrabV2::zwp_input_method_keyboard_grab_v2_repeat_info>::func,
};

ZwpInputMethodKeyboardGrabV2::ZwpInputMethodKeyboardGrabV2(zwp_input_method_keyboard_grab_v2 *val)
    : Type(val)
{
    zwp_input_method_keyboard_grab_v2_add_listener(get(), &listener_, this);
}

ZwpInputMethodKeyboardGrabV2::~ZwpInputMethodKeyboardGrabV2()
{
    zwp_input_method_keyboard_grab_v2_release(get());
}
