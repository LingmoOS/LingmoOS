// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYBAORD_H
#define KEYBAORD_H

#include "Listener.h"
#include "common/common.h"

#include <wayland-server-core.h>
#include <xkbcommon/xkbcommon.h>

#include <memory>

struct wlr_seat;
struct wlr_input_device;
struct wlr_keyboard;
struct wlr_input_method_keyboard_grab_v2;

class Server;

class Keyboard
{
public:
    Keyboard(Server *server, wlr_input_device *device, wl_list *list);
    ~Keyboard();

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

    std::unique_ptr<struct xkb_context, Deleter<xkb_context_unref>> xkb_ctx_;
    std::unique_ptr<struct xkb_keymap, Deleter<xkb_keymap_unref>> xkb_keymap_;

    Listener<&Keyboard::keyNotify> key_;
    Listener<&Keyboard::modifiersNotify> modifiers_;
    Listener<&Keyboard::keymapNotify> keymap_;
    Listener<&Keyboard::repeatInfoNotify> repeat_info_;
    Listener<&Keyboard::destroyNotify> destroy_;
};

#endif // !KEYBAORD_H
