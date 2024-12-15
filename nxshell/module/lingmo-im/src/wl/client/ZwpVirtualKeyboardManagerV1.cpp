// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ZwpVirtualKeyboardManagerV1.h"

#include "Seat.h"
#include "ZwpVirtualKeyboardV1.h"

using namespace wl::client;

ZwpVirtualKeyboardManagerV1::ZwpVirtualKeyboardManagerV1(zwp_virtual_keyboard_manager_v1 *val)
    : Type(val)
{
}

ZwpVirtualKeyboardManagerV1::~ZwpVirtualKeyboardManagerV1()
{
    zwp_virtual_keyboard_manager_v1_destroy(get());
}

zwp_virtual_keyboard_v1 *
ZwpVirtualKeyboardManagerV1::create_virtual_keyboard(const std::shared_ptr<Seat> &seat)
{
    return zwp_virtual_keyboard_manager_v1_create_virtual_keyboard(get(), seat->get());
}
