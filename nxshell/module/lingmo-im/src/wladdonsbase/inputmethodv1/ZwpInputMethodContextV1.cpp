// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ZwpInputMethodContextV1.h"

#include "../InputMethodV1.h"

WL_ADDONS_BASE_USE_NAMESPACE

const struct zwp_input_method_context_v1_interface ZwpInputMethodContextV1::impl = {
    ResourceCallbackWrapper<&ZwpInputMethodContextV1::zwp_input_method_context_v1_destroy>::func,
    ResourceCallbackWrapper<
        &ZwpInputMethodContextV1::zwp_input_method_context_v1_commit_string>::func,
    ResourceCallbackWrapper<
        &ZwpInputMethodContextV1::zwp_input_method_context_v1_preedit_string>::func,
    ResourceCallbackWrapper<
        &ZwpInputMethodContextV1::zwp_input_method_context_v1_preedit_styling>::func,
    ResourceCallbackWrapper<
        &ZwpInputMethodContextV1::zwp_input_method_context_v1_preedit_cursor>::func,
    ResourceCallbackWrapper<
        &ZwpInputMethodContextV1::zwp_input_method_context_v1_delete_surrounding_text>::func,
    ResourceCallbackWrapper<
        &ZwpInputMethodContextV1::zwp_input_method_context_v1_cursor_position>::func,
    ResourceCallbackWrapper<
        &ZwpInputMethodContextV1::zwp_input_method_context_v1_modifiers_map>::func,
    ResourceCallbackWrapper<&ZwpInputMethodContextV1::zwp_input_method_context_v1_keysym>::func,
    ResourceCallbackWrapper<
        &ZwpInputMethodContextV1::zwp_input_method_context_v1_grab_keyboard>::func,
    ResourceCallbackWrapper<&ZwpInputMethodContextV1::zwp_input_method_context_v1_key>::func,
    ResourceCallbackWrapper<&ZwpInputMethodContextV1::zwp_input_method_context_v1_modifiers>::func,
    ResourceCallbackWrapper<&ZwpInputMethodContextV1::zwp_input_method_context_v1_language>::func,
    ResourceCallbackWrapper<
        &ZwpInputMethodContextV1::zwp_input_method_context_v1_text_direction>::func,
};

ZwpInputMethodContextV1::ZwpInputMethodContextV1(InputMethodV1 *inputMethodV1)
    : Type()
    , inputMethodV1_(inputMethodV1)
{
}

ZwpInputMethodContextV1::~ZwpInputMethodContextV1() = default;

void ZwpInputMethodContextV1::send_surrounding_text(struct wl_resource *resource,
                                                    const char *text,
                                                    uint32_t cursor,
                                                    uint32_t anchor)
{
    zwp_input_method_context_v1_send_surrounding_text(resource, text, cursor, anchor);
}

void ZwpInputMethodContextV1::send_reset(struct wl_resource *resource)
{
    zwp_input_method_context_v1_send_reset(resource);
}

void ZwpInputMethodContextV1::send_content_type(struct wl_resource *resource,
                                                uint32_t hint,
                                                uint32_t purpose)
{
    zwp_input_method_context_v1_send_content_type(resource, hint, purpose);
}

void ZwpInputMethodContextV1::send_invoke_action(struct wl_resource *resource,
                                                 uint32_t button,
                                                 uint32_t index)
{
    zwp_input_method_context_v1_send_invoke_action(resource, button, index);
}

void ZwpInputMethodContextV1::send_commit_state(struct wl_resource *resource, uint32_t serial)
{
    zwp_input_method_context_v1_send_commit_state(resource, serial);
}

void ZwpInputMethodContextV1::send_preferred_language(struct wl_resource *resource,
                                                      const char *language)
{
    zwp_input_method_context_v1_send_preferred_language(resource, language);
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_destroy(Resource *resource) { }

void ZwpInputMethodContextV1::zwp_input_method_context_v1_commit_string(Resource *resource,
                                                                        uint32_t serial,
                                                                        const char *text)
{
    if (inputMethodV1_->inputMethodContextV1CommitCallback_) {
        const std::string str = text;
        if (!str.empty()) {
            inputMethodV1_->inputMethodContextV1CommitCallback_(serial, text);
        }
    }
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_preedit_string(Resource *resource,
                                                                         uint32_t serial,
                                                                         const char *text,
                                                                         const char *commit)
{
    if (inputMethodV1_->inputMethodContextV1PreeditCallback_) {
        const std::string str = text;

        if (!str.empty()) {
            inputMethodV1_->inputMethodContextV1PreeditCallback_(serial, text, commit);
        }
    }
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_preedit_styling(Resource *resource,
                                                                          uint32_t index,
                                                                          uint32_t length,
                                                                          uint32_t style)
{
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_preedit_cursor(Resource *resource,
                                                                         int32_t index)
{
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_delete_surrounding_text(
    Resource *resource, int32_t index, uint32_t length)
{
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_cursor_position(Resource *resource,
                                                                          int32_t index,
                                                                          int32_t anchor)
{
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_modifiers_map(Resource *resource,
                                                                        struct wl_array *map)
{
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_keysym(Resource *resource,
                                                                 uint32_t serial,
                                                                 uint32_t time,
                                                                 uint32_t sym,
                                                                 uint32_t state,
                                                                 uint32_t modifiers)
{
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_grab_keyboard(Resource *resource,
                                                                        uint32_t keyboard)
{
    keyboardGrabV1_.reset(new InputMethodGrabV1(this));
    keyboardGrabV1_->init(resource->client(), keyboard);
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_key(
    Resource *resource, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    if (inputMethodV1_->inputMethodContextV1ForwardKeyCallback_) {
        inputMethodV1_->inputMethodContextV1ForwardKeyCallback_(key, state);
    }
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_modifiers(Resource *resource,
                                                                    uint32_t serial,
                                                                    uint32_t mods_depressed,
                                                                    uint32_t mods_latched,
                                                                    uint32_t mods_locked,
                                                                    uint32_t group)
{
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_language(Resource *resource,
                                                                   uint32_t serial,
                                                                   const char *language)
{
}

void ZwpInputMethodContextV1::zwp_input_method_context_v1_text_direction(Resource *resource,
                                                                         uint32_t serial,
                                                                         uint32_t direction)
{
}

void ZwpInputMethodContextV1::resetKeyboardGrab()
{
    keyboardGrabV1_.reset();
}
