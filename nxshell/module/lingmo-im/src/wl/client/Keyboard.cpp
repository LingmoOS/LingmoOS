// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Keyboard.h"

using namespace wl::client;

const wl_keyboard_listener Keyboard::listener_{
    ListenerCallbackWrapper<&Keyboard::wl_keyboard_keymap>::func,
    ListenerCallbackWrapper<&Keyboard::wl_keyboard_enter>::func,
    ListenerCallbackWrapper<&Keyboard::wl_keyboard_leave>::func,
    ListenerCallbackWrapper<&Keyboard::wl_keyboard_key>::func,
    ListenerCallbackWrapper<&Keyboard::wl_keyboard_modifiers>::func,
    ListenerCallbackWrapper<&Keyboard::wl_keyboard_repeat_info>::func,
};

Keyboard::Keyboard(wl_keyboard *val)
    : Type(val)
{
    wl_keyboard_add_listener(get(), &listener_, this);
}

Keyboard::~Keyboard() { }
