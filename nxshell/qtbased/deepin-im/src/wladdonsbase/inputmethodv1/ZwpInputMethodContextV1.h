// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_ADDONS_BASE_ZWPINPUTMETHODCONTEXTV1_H
#define WL_ADDONS_BASE_ZWPINPUTMETHODCONTEXTV1_H

#include "Type.h"
#include "wayland-input-method-unstable-v1-server-protocol.h"
#include "InputMethodGrabV1.h"

WL_ADDONS_BASE_BEGIN_NAMESPACE

class InputMethodV1;

class ZwpInputMethodContextV1 : public Type<ZwpInputMethodContextV1, zwp_input_method_context_v1>
{
    friend class Type;

public:
    explicit ZwpInputMethodContextV1(InputMethodV1 *inputMethodV1);
    ~ZwpInputMethodContextV1() override;

    const auto &getKeyboardGrab() const { return keyboardGrabV1_; }

    void resetKeyboardGrab();

    void send_surrounding_text(struct wl_resource *resource,
                               const char *text,
                               uint32_t cursor,
                               uint32_t anchor);
    void send_reset(struct wl_resource *resource);
    void send_content_type(struct wl_resource *resource, uint32_t hint, uint32_t purpose);
    void send_invoke_action(struct wl_resource *resource, uint32_t button, uint32_t index);
    void send_commit_state(struct wl_resource *resource, uint32_t serial);
    void send_preferred_language(struct wl_resource *resource, const char *language);
    void zwp_input_method_context_v1_destroy(Resource *resource);
    void zwp_input_method_context_v1_commit_string(Resource *resource,
                                                   uint32_t serial,
                                                   const char *text);
    void zwp_input_method_context_v1_preedit_string(Resource *resource,
                                                    uint32_t serial,
                                                    const char *text,
                                                    const char *commit);
    void zwp_input_method_context_v1_preedit_styling(Resource *resource,
                                                     uint32_t index,
                                                     uint32_t length,
                                                     uint32_t style);
    void zwp_input_method_context_v1_preedit_cursor(Resource *resource, int32_t index);
    void zwp_input_method_context_v1_delete_surrounding_text(Resource *resource,
                                                             int32_t index,
                                                             uint32_t length);
    void zwp_input_method_context_v1_cursor_position(Resource *resource,
                                                     int32_t index,
                                                     int32_t anchor);
    void zwp_input_method_context_v1_modifiers_map(Resource *resource, struct wl_array *map);
    void zwp_input_method_context_v1_keysym(Resource *resource,
                                            uint32_t serial,
                                            uint32_t time,
                                            uint32_t sym,
                                            uint32_t state,
                                            uint32_t modifiers);
    void zwp_input_method_context_v1_grab_keyboard(Resource *resource, uint32_t keyboard);
    void zwp_input_method_context_v1_key(
        Resource *resource, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void zwp_input_method_context_v1_modifiers(Resource *resource,
                                               uint32_t serial,
                                               uint32_t mods_depressed,
                                               uint32_t mods_latched,
                                               uint32_t mods_locked,
                                               uint32_t group);
    void zwp_input_method_context_v1_language(Resource *resource,
                                              uint32_t serial,
                                              const char *language);
    void zwp_input_method_context_v1_text_direction(Resource *resource,
                                                    uint32_t serial,
                                                    uint32_t direction);

private:
    static const struct zwp_input_method_context_v1_interface impl;
    std::unique_ptr<InputMethodGrabV1> keyboardGrabV1_ = nullptr;
    InputMethodV1 *inputMethodV1_ = nullptr;
};

WL_ADDONS_BASE_END_NAMESPACE

#endif // !WL_ADDONS_BASE_ZWPINPUTMETHODCONTEXTV1_H
