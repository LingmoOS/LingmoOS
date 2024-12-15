// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_ZWPINPUTMETHODKEYBOARDGRABV2_H
#define WL_CLIENT_ZWPINPUTMETHODKEYBOARDGRABV2_H

#include "Type.h"

namespace wl {
namespace client {

class ZwpInputMethodKeyboardGrabV2 : public Type<zwp_input_method_keyboard_grab_v2>
{
public:
    ZwpInputMethodKeyboardGrabV2(zwp_input_method_keyboard_grab_v2 *val);
    virtual ~ZwpInputMethodKeyboardGrabV2();

protected:
    virtual void zwp_input_method_keyboard_grab_v2_keymap(uint32_t format, int32_t fd, uint32_t size) = 0;
    virtual void zwp_input_method_keyboard_grab_v2_key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state) = 0;
    virtual void zwp_input_method_keyboard_grab_v2_modifiers(uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) = 0;
    virtual void zwp_input_method_keyboard_grab_v2_repeat_info(int32_t rate, int32_t delay) = 0;

private:
    static const zwp_input_method_keyboard_grab_v2_listener listener_;
};

} // namespace client
}; // namespace wl

#endif // !WL_CLIENT_ZWPINPUTMETHODKEYBOARDGRABV2_H
