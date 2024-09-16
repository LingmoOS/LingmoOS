// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ZwpInputMethodV1.h"

WL_ADDONS_BASE_USE_NAMESPACE

const struct wl_interface ZwpInputMethodV1::impl = {

};

ZwpInputMethodV1::ZwpInputMethodV1()
    : Type()
{
}

ZwpInputMethodV1::~ZwpInputMethodV1() = default;

void ZwpInputMethodV1::send_activate(struct wl_resource *resource, struct wl_resource *id)
{
    zwp_input_method_v1_send_activate(resource, id);
}

void ZwpInputMethodV1::send_deactivate(struct wl_resource *resource, struct wl_resource *context)
{
    zwp_input_method_v1_send_deactivate(resource, context);
}
