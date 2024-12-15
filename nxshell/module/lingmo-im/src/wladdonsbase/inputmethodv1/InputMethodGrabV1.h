// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_ADDONS_BASE_INPUTMETHOD_GRAB_H
#define WL_ADDONS_BASE_INPUTMETHOD_GRAB_H

#include "Type.h"

#include <wayland-server-protocol.h>
#include <xkbcommon/xkbcommon.h>

WL_ADDONS_BASE_BEGIN_NAMESPACE
class ZwpInputMethodContextV1;

class InputMethodGrabV1 : public Type<InputMethodGrabV1, wl_keyboard>
{
    friend class Type;

public:
    struct State
    {
        uint32_t modsDepressed;
        uint32_t modsLatched;
        uint32_t modsLocked;
        uint32_t group;
    };

    explicit InputMethodGrabV1(ZwpInputMethodContextV1 *context);
    ~InputMethodGrabV1() override;

    void sendKey(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void sendModifiers(uint32_t serial,
                       uint32_t mods_depressed,
                       uint32_t mods_latched,
                       uint32_t mods_locked,
                       uint32_t group);

protected:
    void resource_bind(Resource *resource) override;

private:
    bool updateState(uint32_t keycode, bool isRelease);
    std::pair<int, size_t> genKeymapData(xkb_keymap *keymap) const;
    void zwp_input_method_keyboard_grab_release(Resource *resource);

private:
    ZwpInputMethodContextV1 *context_;
    std::unique_ptr<xkb_context, Deleter<xkb_context_unref>> xkbContext_;
    std::unique_ptr<xkb_keymap, Deleter<xkb_keymap_unref>> xkbKeymap_;
    std::unique_ptr<xkb_state, Deleter<xkb_state_unref>> xkbState_;
    State state_;

    static const struct wl_keyboard_interface impl;
};

WL_ADDONS_BASE_END_NAMESPACE

#endif // !WL_ADDONS_BASE_INPUTMETHOD_GRAB_H
