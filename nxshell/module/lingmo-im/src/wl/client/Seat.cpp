// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Seat.h"

using namespace wl::client;

Seat::Seat(wl_seat *val)
    : Type(val)
{
}

Seat::~Seat()
{
    uint32_t version = wl_seat_get_version(get());
    if (version >= 5) {
        wl_seat_release(get());
    } else {
        wl_seat_destroy(get());
    }
}

struct wl_keyboard *Seat::get_keyboard() {
    return wl_seat_get_keyboard(get());
}
