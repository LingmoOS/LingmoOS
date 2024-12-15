// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ZwpTextInputManagerV3.h"

#include "Seat.h"
#include "ZwpTextInputV3.h"

using namespace wl::client;

ZwpTextInputManagerV3::ZwpTextInputManagerV3(zwp_text_input_manager_v3 *val)
    : Type(val)
{
}

ZwpTextInputManagerV3::~ZwpTextInputManagerV3()
{
    zwp_text_input_manager_v3_destroy(get());
}

struct zwp_text_input_v3 *ZwpTextInputManagerV3::get_text_input(const std::shared_ptr<Seat> &seat)
{
    return zwp_text_input_manager_v3_get_text_input(get(), seat->get());
}
