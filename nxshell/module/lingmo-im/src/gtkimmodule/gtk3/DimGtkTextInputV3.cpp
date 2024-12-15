// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DimGtkTextInputV3.h"

DimGtkTextInputV3::DimGtkTextInputV3(struct ::zwp_text_input_v3 *text_input,
                                     DimIMContextWaylandGlobal *global)
    : wl::client::ZwpTextInputV3(text_input)
    , global_(global)
{
}

void DimGtkTextInputV3::zwp_text_input_v3_enter(struct wl_surface *surface)
{
    if (global_->current) {
        m_surface = surface;
        DimIMContext *context = DIM_IM_CONTEXT(global_->current);
        ::enable(context, global_);
    }
}

void DimGtkTextInputV3::zwp_text_input_v3_leave(struct wl_surface *surface)
{
    if (global_->current) {
        if (m_surface != surface) {
            return;
        }

        m_surface = nullptr;
        DimIMContext *context = DIM_IM_CONTEXT(global_->current);
        ::disable(context, global_);
    }
}

void DimGtkTextInputV3::zwp_text_input_v3_preedit_string(const char *text,
                                                         int32_t cursor_begin,
                                                         int32_t cursor_end)
{
    if (!global_->current)
        return;

    DimIMContext *context = DIM_IM_CONTEXT(global_->current);

    g_free(context->pendingPreedit.text);
    context->pendingPreedit.text = g_strdup(text);
    context->pendingPreedit.cursorBegin = cursor_begin;
    context->pendingPreedit.cursorEnd = cursor_end;
}

void DimGtkTextInputV3::zwp_text_input_v3_commit_string(const char *text)
{
    if (!global_->current)
        return;

    DimIMContext *context = DIM_IM_CONTEXT(global_->current);

    g_free(context->pendingCommit);
    context->pendingCommit = g_strdup(text);
}

void DimGtkTextInputV3::zwp_text_input_v3_delete_surrounding_text(uint32_t before_length,
                                                                  uint32_t after_length)
{
    if (!global_->current)
        return;

    DimIMContext *context = DIM_IM_CONTEXT(global_->current);

    /* We already got byte lengths from text_input_v3, but GTK uses char lengths
     * for delete_surrounding, So convert it here.
     */
    char *cursor_pointer = context->surrounding.text + context->surrounding.cursorIdx;
    uint32_t char_before_length =
        g_utf8_pointer_to_offset(cursor_pointer - before_length, cursor_pointer);
    uint32_t char_after_length =
        g_utf8_pointer_to_offset(cursor_pointer, cursor_pointer + after_length);

    context->pendingSurroundingDelete.beforeLength = char_before_length;
    context->pendingSurroundingDelete.afterLength = char_after_length;
}

void DimGtkTextInputV3::zwp_text_input_v3_done(uint32_t serial)
{
    global_->doneSerial = serial;

    if (!global_->current)
        return;

    DimIMContext *context = DIM_IM_CONTEXT(global_->current);

    gboolean update_im =
        (context->pendingCommit != nullptr
         || g_strcmp0(context->pendingPreedit.text, context->currentPreedit.text) != 0);

    textInputDeleteSurroundingTextApply(global_);
    textInputCommitApply(global_);
    textInputPreeditApply(global_);

    if (update_im && global_->serial == serial)
        notifyImChange(context, ZWP_TEXT_INPUT_V3_CHANGE_CAUSE_INPUT_METHOD);
}

DimGtkTextInputV3::~DimGtkTextInputV3() { }
