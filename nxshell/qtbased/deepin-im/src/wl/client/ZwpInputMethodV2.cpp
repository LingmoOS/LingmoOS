// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ZwpInputMethodV2.h"

#include "Surface.h"

using namespace wl::client;

const zwp_input_method_v2_listener ZwpInputMethodV2::listener_ = {
    ListenerCallbackWrapper<&ZwpInputMethodV2::zwp_input_method_v2_activate>::func,
    ListenerCallbackWrapper<&ZwpInputMethodV2::zwp_input_method_v2_deactivate>::func,
    ListenerCallbackWrapper<&ZwpInputMethodV2::zwp_input_method_v2_surrounding_text>::func,
    ListenerCallbackWrapper<&ZwpInputMethodV2::zwp_input_method_v2_text_change_cause>::func,
    ListenerCallbackWrapper<&ZwpInputMethodV2::zwp_input_method_v2_content_type>::func,
    ListenerCallbackWrapper<&ZwpInputMethodV2::zwp_input_method_v2_done>::func,
    ListenerCallbackWrapper<&ZwpInputMethodV2::zwp_input_method_v2_unavailable>::func,
};

ZwpInputMethodV2::ZwpInputMethodV2(zwp_input_method_v2 *val)
    : Type(val)
{
    zwp_input_method_v2_add_listener(get(), &listener_, this);
}

ZwpInputMethodV2::~ZwpInputMethodV2()
{
    zwp_input_method_v2_destroy(get());
}

zwp_input_method_keyboard_grab_v2 *ZwpInputMethodV2::grabKeyboard()
{
    return zwp_input_method_v2_grab_keyboard(get());
}

void ZwpInputMethodV2::commit_string(const char *text)
{
    zwp_input_method_v2_commit_string(get(), text);
}

void ZwpInputMethodV2::set_preedit_string(const char *text, int32_t cursorBegin, int32_t cursorEnd)
{
    zwp_input_method_v2_set_preedit_string(get(), text, cursorBegin, cursorEnd);
}

void ZwpInputMethodV2::commit(uint32_t serial)
{
    zwp_input_method_v2_commit(get(), serial);
}

zwp_input_popup_surface_v2 *ZwpInputMethodV2::get_input_popup_surface(wl_surface *surface)
{
    return zwp_input_method_v2_get_input_popup_surface(get(), surface);
}
