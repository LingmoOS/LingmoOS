// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "InputMethodGrabV1.h"

#include "ZwpInputMethodContextV1.h"
#include "common/shm_open_anon.h"

#include <wayland-server-protocol.h>

#include <QDebug>

#include <sys/mman.h>
#include <unistd.h>

WL_ADDONS_BASE_USE_NAMESPACE

const struct wl_keyboard_interface InputMethodGrabV1::impl = {
    ResourceCallbackWrapper<&InputMethodGrabV1::zwp_input_method_keyboard_grab_release>::func,
};

uint32_t nextSerial()
{
    static uint32_t current = 1;

    return current++;
}

InputMethodGrabV1::InputMethodGrabV1(ZwpInputMethodContextV1 *context)
    : Type()
    , context_(context)
    , xkbContext_(xkb_context_new(XKB_CONTEXT_NO_FLAGS))
    , state_({})
{
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

std::pair<int, size_t> InputMethodGrabV1::genKeymapData(xkb_keymap *keymap) const
{
    std::unique_ptr<char[]> keymapStr(xkb_keymap_get_as_string(keymap, XKB_KEYMAP_FORMAT_TEXT_V1));
    if (!keymapStr) {
        return std::make_pair(0, 0);
    }
    auto size = strlen(keymapStr.get());
    int fd = shm_open_anon();
    if (ftruncate(fd, size) != 0) {
        qWarning() << "ftruncate failed";
    }

    void *tmp = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (tmp == MAP_FAILED) {
        qWarning() << "mmap failed";
        close(fd);
        return std::make_pair(0, 0);
    }
    memcpy(tmp, keymapStr.get(), size);
    munmap(tmp, size);

    return std::make_pair(fd, size);
}

InputMethodGrabV1::~InputMethodGrabV1()
{
    zwp_input_method_keyboard_grab_release(getResource().get());
}

void InputMethodGrabV1::sendKey(uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    if (this != context_->getKeyboardGrab().get()) {
        return;
    }

    auto isRelease = state ? XKB_KEY_UP : XKB_KEY_DOWN;

    if (updateState(key, isRelease)) {
        return;
    }

    if (getResource() && getResource()->handle) {
        wl_keyboard_send_key(getResource()->handle, serial, time, key, state);
    }
}

void InputMethodGrabV1::sendModifiers(uint32_t serial,
                                      uint32_t mods_depressed,
                                      uint32_t mods_latched,
                                      uint32_t mods_locked,
                                      uint32_t group)
{
    if (this != context_->getKeyboardGrab().get()) {
        return;
    }

    if (getResource() && getResource()->handle) {
        wl_keyboard_send_modifiers(getResource()->handle,
                                   serial,
                                   mods_depressed,
                                   mods_latched,
                                   mods_locked,
                                   group);
    }
}

void InputMethodGrabV1::resource_bind(Resource *resource)
{
    auto res = genKeymapData(xkbKeymap_.get());

    wl_keyboard_send_keymap(resource->handle, XKB_KEYMAP_FORMAT_TEXT_V1, res.first, res.second);

    close(res.first);
}

bool InputMethodGrabV1::updateState(uint32_t keycode, bool isRelease)
{
    xkb_state_update_key(xkbState_.get(), keycode, isRelease ? XKB_KEY_UP : XKB_KEY_DOWN);

    State state = {};
    state.modsDepressed = xkb_state_serialize_mods(xkbState_.get(), XKB_STATE_MODS_DEPRESSED);
    state.modsLatched = xkb_state_serialize_mods(xkbState_.get(), XKB_STATE_MODS_LATCHED);
    state.modsLocked = xkb_state_serialize_mods(xkbState_.get(), XKB_STATE_MODS_LOCKED);
    state.group = xkb_state_serialize_layout(xkbState_.get(), XKB_STATE_LAYOUT_EFFECTIVE);
    if (xkb_state_update_mask(xkbState_.get(),
                              state.modsDepressed,
                              state.modsLatched,
                              state.modsLocked,
                              0,
                              0,
                              state.group)
        == 0) {
        return false;
    }

    if (memcmp(&state_, &state, sizeof(state)) == 0) {
        return false;
    }

    state_ = state;

    sendModifiers(nextSerial(),
                  state.modsDepressed,
                  state.modsLatched,
                  state.modsLocked,
                  state.group);

    return true;
}

void InputMethodGrabV1::zwp_input_method_keyboard_grab_release(Resource *resource)
{
    resource->destroy();
}
