// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TextInputV3.h"

#include "InputMethodV2.h"
#include "Server.h"

TextInputV3::TextInputV3(Server *server, wlr_text_input_v3 *text_input, wl_list *list)
    : server_(server)
    , text_input_(text_input)
    , enable_(this)
    , commit_(this)
    , disable_(this)
    , destroy_(this)
{
    wl_signal_add(&text_input_->events.enable, enable_);
    wl_signal_add(&text_input_->events.commit, commit_);
    wl_signal_add(&text_input_->events.disable, disable_);
    wl_signal_add(&text_input_->events.destroy, destroy_);

    wl_signal_init(&events.cursorRectangle);

    wl_list_insert(list, &link_);
}

TextInputV3::~TextInputV3()
{
    wl_list_remove(&link_);
}

void TextInputV3::enableNotify(void *data)
{
    auto *inputMethod = server_->inputMethod();
    if (inputMethod == nullptr) {
        return;
    }

    wlr_input_method_v2_send_activate(inputMethod->input_method_);

    sendIMState(inputMethod);
}

void TextInputV3::commitNotify(void *data)
{
    if (!text_input_->current_enabled) {
        // todo: log
        return;
    }

    auto *inputMethod = server_->inputMethod();
    if (inputMethod == nullptr) {
        // todo: log
        return;
    }

    sendIMState(inputMethod);
}

void TextInputV3::disableNotify(void *data)
{
    if (text_input_->focused_surface == NULL) {
        return;
    }

    auto *inputMethod = server_->inputMethod();
    if (inputMethod == nullptr) {
        // todo: log
        return;
    }

    disableTextInput(inputMethod);
}

void TextInputV3::destroyNotify(void *data)
{
    delete this;
}

void TextInputV3::sendIMState(InputMethodV2 *inputMethod)
{
    // TODO: only send each of those if they were modified
    if (text_input_->active_features & WLR_TEXT_INPUT_V3_FEATURE_SURROUNDING_TEXT) {
        wlr_input_method_v2_send_surrounding_text(inputMethod->input_method_,
                                                  text_input_->current.surrounding.text,
                                                  text_input_->current.surrounding.cursor,
                                                  text_input_->current.surrounding.anchor);
    }
    wlr_input_method_v2_send_text_change_cause(inputMethod->input_method_,
                                               text_input_->current.text_change_cause);
    if (text_input_->active_features & WLR_TEXT_INPUT_V3_FEATURE_CONTENT_TYPE) {
        wlr_input_method_v2_send_content_type(inputMethod->input_method_,
                                              text_input_->current.content_type.hint,
                                              text_input_->current.content_type.purpose);
    }

    // TODO:here will be coredump, wait to fix
    // if (text_input_->active_features & WLR_TEXT_INPUT_V3_FEATURE_CURSOR_RECTANGLE) {
    //     wl_signal_emit(&events.cursorRectangle, &text_input_->current.cursor_rectangle);
    // }

    wlr_input_method_v2_send_done(inputMethod->input_method_);
}

void TextInputV3::disableTextInput(InputMethodV2 *inputMethod)
{
    wlr_input_method_v2_send_deactivate(inputMethod->input_method_);

    sendIMState(inputMethod);
}
