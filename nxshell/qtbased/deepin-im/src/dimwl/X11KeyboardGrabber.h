// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef X11KEYBOARDGRABBER_H
#define X11KEYBOARDGRABBER_H

#include "Xcb.h"

#include <wayland-server-core.h>
#include <xkbcommon/xkbcommon.h>

extern "C" {
#include <wlr/types/wlr_keyboard.h>
}

struct GrabberKeyEvent
{
    uint32_t keycode;
    bool isRelease;
};

class X11KeyboardGrabber : public Xcb
{
public:
    explicit X11KeyboardGrabber(wl_event_loop *loop);
    virtual ~X11KeyboardGrabber();

protected:
    void xcbEvent(const std::unique_ptr<xcb_generic_event_t> &event) override;

public:
    struct
    {
        struct wl_signal key;
        struct wl_signal modifiers;
    } events;

private:
    void updateModifiers(uint32_t keycode, bool isRelease);
    void initXinputExtension();

private:
    int xcbFd_;
    uint8_t xinput2OPCode_;

    std::unique_ptr<xkb_context, Deleter<xkb_context_unref>> xkbContext_;
    std::unique_ptr<xkb_keymap, Deleter<xkb_keymap_unref>> xkbKeymap_;
    std::unique_ptr<xkb_state, Deleter<xkb_state_unref>> xkbState_;
    wlr_keyboard_modifiers modifiers_;
};

#endif // !X11KEYBOARDGRABBER_H
