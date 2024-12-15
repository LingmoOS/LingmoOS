// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_SEAT_H
#define WL_CLIENT_SEAT_H

#include "Type.h"

namespace wl {
namespace client {

class Seat : public Type<wl_seat>
{
public:
    Seat(wl_seat *val);
    ~Seat();

    struct wl_keyboard *get_keyboard();
};

} // namespace client
} // namespace wl

#endif // !WL_CLIENT_SEAT_H
