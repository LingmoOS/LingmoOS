// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYBAORD_H
#define KEYBAORD_H

#include "Listener.h"
#include "common/common.h"

#include <wayland-server-core.h>
#include <xkbcommon/xkbcommon.h>

#include <functional>
#include <memory>

struct wlr_seat;
struct wlr_input_device;
struct wlr_keyboard;
struct wlr_input_method_keyboard_grab_v2;
struct wlr_keyboard_key_event;
struct wlr_keyboard_modifiers;

WL_ADDONS_BASE_BEGIN_NAMESPACE

class Server;

class Keyboard
{
public:
    Keyboard(Server *server, wlr_input_device *device, wl_list *list, bool isVirtual = false);
    ~Keyboard();

    void setKeyEventCallback(const std::function<void(wlr_keyboard_key_event *event)> &callback)
    {
        keyEventCallback_ = callback;
    }

private:
    void keyNotify(void *data);
    void modifiersNotify(void *data);
    void keymapNotify(void *data);
    void repeatInfoNotify(void *data);
    void destroyNotify(void *data);

    wlr_input_method_keyboard_grab_v2 *getKeyboardGrab();

private:
    Server *server_;
    wl_list link_;
    wlr_input_device *device_;
    wlr_keyboard *keyboard_;
    bool isVirtual_;

    std::unique_ptr<struct xkb_context, Deleter<xkb_context_unref>> xkb_ctx_;
    std::unique_ptr<struct xkb_keymap, Deleter<xkb_keymap_unref>> xkb_keymap_;

    Listener<&Keyboard::keyNotify> key_;
    Listener<&Keyboard::modifiersNotify> modifiers_;
    Listener<&Keyboard::keymapNotify> keymap_;
    Listener<&Keyboard::repeatInfoNotify> repeat_info_;
    Listener<&Keyboard::destroyNotify> destroy_;

    std::function<void(wlr_keyboard_key_event *event)> keyEventCallback_;
};

WL_ADDONS_BASE_END_NAMESPACE

#endif // !KEYBAORD_H
