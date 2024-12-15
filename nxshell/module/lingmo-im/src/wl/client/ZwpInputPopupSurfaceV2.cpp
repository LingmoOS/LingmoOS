// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ZwpInputPopupSurfaceV2.h"

using namespace wl::client;

const zwp_input_popup_surface_v2_listener ZwpInputPopupSurfaceV2::listener_ = {
    ListenerCallbackWrapper<&ZwpInputPopupSurfaceV2::zwp_input_popup_surface_v2_text_input_rectangle>::func,
};

ZwpInputPopupSurfaceV2::ZwpInputPopupSurfaceV2(zwp_input_popup_surface_v2 *val)
    : Type(val)
{
    zwp_input_popup_surface_v2_add_listener(get(), &listener_, this);
}

ZwpInputPopupSurfaceV2::~ZwpInputPopupSurfaceV2() {
    zwp_input_popup_surface_v2_destroy(get());
}
