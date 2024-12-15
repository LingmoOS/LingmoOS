// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ZwpVirtualKeyboardV1.h"

using namespace wl::client;

ZwpVirtualKeyboardV1::ZwpVirtualKeyboardV1(zwp_virtual_keyboard_v1 *val)
    : Type(val)
{
}

ZwpVirtualKeyboardV1::~ZwpVirtualKeyboardV1()
{
    auto version = zwp_virtual_keyboard_v1_get_version(get());
    if (version >= 1) {
        zwp_virtual_keyboard_v1_destroy(get());
    }
}

void ZwpVirtualKeyboardV1::keymap(uint32_t format, int32_t fd, uint32_t size)
{
    zwp_virtual_keyboard_v1_keymap(get(), format, fd, size);
}

void ZwpVirtualKeyboardV1::key(uint32_t time, uint32_t key, uint32_t state)
{
    zwp_virtual_keyboard_v1_key(get(), time, key, state);
}

void ZwpVirtualKeyboardV1::modifiers(uint32_t modsDepressed,
                                     uint32_t modsLatched,
                                     uint32_t modsLocked,
                                     uint32_t group)
{
    zwp_virtual_keyboard_v1_modifiers(get(), modsDepressed, modsLatched, modsLocked, group);
}
