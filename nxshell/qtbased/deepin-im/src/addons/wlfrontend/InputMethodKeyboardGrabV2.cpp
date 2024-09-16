// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "InputMethodKeyboardGrabV2.h"

#include "wl/client/ZwpInputMethodKeyboardGrabV2.h"

#include <QDebug>

using namespace org::deepin::dim;

InputMethodKeyboardGrabV2::InputMethodKeyboardGrabV2(zwp_input_method_keyboard_grab_v2 *val)
    : wl::client::ZwpInputMethodKeyboardGrabV2(val)
    , qobject_(std::make_unique<InputMethodKeyboardGrabV2QObject>())
{
}

void InputMethodKeyboardGrabV2::zwp_input_method_keyboard_grab_v2_keymap(uint32_t format,
                                                                         int32_t fd,
                                                                         uint32_t size)
{
    emit qobject_->keymap(format, fd, size);
}

void InputMethodKeyboardGrabV2::zwp_input_method_keyboard_grab_v2_key(uint32_t serial,
                                                                      uint32_t time,
                                                                      uint32_t key,
                                                                      uint32_t state)
{
    emit qobject_->key(serial, time, key, state);
}

void InputMethodKeyboardGrabV2::zwp_input_method_keyboard_grab_v2_modifiers(uint32_t serial,
                                                                            uint32_t mods_depressed,
                                                                            uint32_t mods_latched,
                                                                            uint32_t mods_locked,
                                                                            uint32_t group)
{
    emit qobject_->modifiers(serial, mods_depressed, mods_latched, mods_locked, group);
}

void InputMethodKeyboardGrabV2::zwp_input_method_keyboard_grab_v2_repeat_info(int32_t rate,
                                                                              int32_t delay)
{
    emit qobject_->repeatInfo(rate, delay);
}
