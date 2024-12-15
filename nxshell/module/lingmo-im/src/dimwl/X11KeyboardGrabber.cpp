// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "X11KeyboardGrabber.h"

#include <xcb/xinput.h>

#include <stdexcept>

#include <string.h>

X11KeyboardGrabber::X11KeyboardGrabber(wl_event_loop *loop)
    : Xcb(loop)
    , xkbContext_(xkb_context_new(XKB_CONTEXT_NO_FLAGS))
    , modifiers_({})
{
    initXinputExtension();

    wl_signal_init(&events.key);
    wl_signal_init(&events.modifiers);

    xkb_rule_names rules = {
        .rules = getenv("XKB_DEFAULT_RULES"),
        .model = getenv("XKB_DEFAULT_MODEL"),
        .layout = getenv("XKB_DEFAULT_LAYOUT"),
        .variant = getenv("XKB_DEFAULT_VARIANT"),
        .options = getenv("XKB_DEFAULT_OPTIONS"),
    };
    xkbKeymap_.reset(
        xkb_keymap_new_from_names(xkbContext_.get(), &rules, XKB_KEYMAP_COMPILE_NO_FLAGS));
    xkbState_.reset(xkb_state_new(xkbKeymap_.get()));
}

X11KeyboardGrabber::~X11KeyboardGrabber() { }

void X11KeyboardGrabber::xcbEvent(const std::unique_ptr<xcb_generic_event_t> &event)
{
    auto responseType = XCB_EVENT_RESPONSE_TYPE(event);
    if (responseType != XCB_GE_GENERIC) {
        return;
    }

    auto *ge = reinterpret_cast<xcb_ge_generic_event_t *>(event.get());
    if (ge->extension != xinput2OPCode_) {
        return;
    }

    if (ge->event_type != XCB_INPUT_RAW_KEY_PRESS && ge->event_type != XCB_INPUT_RAW_KEY_RELEASE) {
        return;
    }

    bool isRelease = ge->event_type == XCB_INPUT_RAW_KEY_RELEASE;
    auto *ke = reinterpret_cast<xcb_input_raw_key_press_event_t *>(event.get());

    GrabberKeyEvent d{
        ke->detail - XKB_HISTORICAL_OFFSET,
        isRelease,
    };
    wl_signal_emit(&events.key, &d);
    updateModifiers(ke->detail, isRelease);
}

void X11KeyboardGrabber::updateModifiers(uint32_t keycode, bool isRelease)
{
    xkb_state_update_key(xkbState_.get(),
                         keycode,
                         isRelease ? XKB_KEY_UP : XKB_KEY_DOWN);

    wlr_keyboard_modifiers modifiers{
        xkb_state_serialize_mods(xkbState_.get(), XKB_STATE_MODS_DEPRESSED),
        xkb_state_serialize_mods(xkbState_.get(), XKB_STATE_MODS_LATCHED),
        xkb_state_serialize_mods(xkbState_.get(), XKB_STATE_MODS_LOCKED),
        xkb_state_serialize_layout(xkbState_.get(), XKB_STATE_LAYOUT_EFFECTIVE),
    };

    if (memcmp(&modifiers_, &modifiers, sizeof(modifiers)) == 0) {
        return;
    }

    modifiers_ = modifiers;

    wl_signal_emit(&events.modifiers, &modifiers_);
}

void X11KeyboardGrabber::initXinputExtension()
{
    {
        const char *extname = "XInputExtension";
        auto reply = XCB_REPLY(xcb_query_extension, xconn_.get(), strlen(extname), extname);
        if (!reply->present) {
            throw std::runtime_error("XInput extension is not available");
        }
        xinput2OPCode_ = reply->major_opcode;
    }

    {
        auto reply = XCB_REPLY(xcb_input_xi_query_version,
                               xconn_.get(),
                               XCB_INPUT_MAJOR_VERSION,
                               XCB_INPUT_MINOR_VERSION);
        if (!reply || reply->major_version != 2) {
            throw std::runtime_error("X server does not support XInput 2");
        }

        // qDebug() << "XInput version:" << reply->major_version << "." << reply->minor_version;
    }

    struct
    {
        xcb_input_event_mask_t head;
        xcb_input_xi_event_mask_t mask;
    } mask;

    mask.head.deviceid = XCB_INPUT_DEVICE_ALL_MASTER;
    mask.head.mask_len = sizeof(mask.mask) / sizeof(uint32_t);
    mask.mask = static_cast<xcb_input_xi_event_mask_t>(XCB_INPUT_XI_EVENT_MASK_RAW_KEY_PRESS
                                                       | XCB_INPUT_XI_EVENT_MASK_RAW_KEY_RELEASE);
    auto cookie = xcb_input_xi_select_events(xconn_.get(), screen()->root, 1, &mask.head);
    auto err = xcb_request_check(xconn_.get(), cookie);
    if (err) {
        throw std::runtime_error("xcb_input_xi_select_events failed");
    }
}
