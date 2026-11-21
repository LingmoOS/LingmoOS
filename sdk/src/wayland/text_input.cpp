// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayland/text_input.h"

#include <cmath>
#include <wayland-client.h>

#include <text-input-unstable-v3-client-protocol.h>

#include "wayland/shell_surface.h"

namespace Lingmo::ui::wayland {

static uint32_t wirePurpose(TextPurpose purpose)
{
    switch (purpose) {
    case TextPurpose::Password:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_PASSWORD;
    case TextPurpose::Number:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NUMBER;
    case TextPurpose::Email:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_EMAIL;
    case TextPurpose::Url:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_URL;
    case TextPurpose::Digits:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_DIGITS;
    case TextPurpose::Normal:
        break;
    }
    return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NORMAL;
}

struct TextInputListeners
{
    static ShellSurface* shellSurfaceFor(wl_surface* surface)
    {
        return surface ? static_cast<ShellSurface*>(wl_surface_get_user_data(surface)) : nullptr;
    }

    static void enter(void* data, zwp_text_input_v3*, wl_surface* surface)
    {
        auto* self = static_cast<TextInput*>(data);
        self->m_entered = shellSurfaceFor(surface);
        self->m_enabled = false;
        self->m_sent.reset();
        self->apply();
    }

    // Requests until the next enter are ignored, so it is disabled implicitly.
    static void leave(void* data, zwp_text_input_v3*, wl_surface*)
    {
        auto* self = static_cast<TextInput*>(data);
        self->m_entered = nullptr;
        self->m_enabled = false;
        self->m_sent.reset();
        self->m_pending = {};
    }

    static void preeditString(void* data, zwp_text_input_v3*, const char* text, int32_t begin,
                              int32_t end)
    {
        auto* self = static_cast<TextInput*>(data);
        self->m_pending.preedit = text ? text : "";
        self->m_pending.preeditCursorBegin = begin;
        self->m_pending.preeditCursorEnd = end;
    }

    static void commitString(void* data, zwp_text_input_v3*, const char* text)
    {
        auto* self = static_cast<TextInput*>(data);
        self->m_pending.commit = text ? text : "";
    }

    static void deleteSurroundingText(void* data, zwp_text_input_v3*, uint32_t before,
                                      uint32_t after)
    {
        auto* self = static_cast<TextInput*>(data);
        self->m_pending.deleteBefore = before;
        self->m_pending.deleteAfter = after;
    }

    static void done(void* data, zwp_text_input_v3*, uint32_t)
    {
        auto* self = static_cast<TextInput*>(data);
        const TextInputEvent event = self->m_pending;
        self->m_pending = {};
        if (!self->m_entered || !self->m_enabled)
            return;
        self->m_inputMethodCause = true;
        self->m_entered->delegate()->textInput(event);
    }

    static constexpr zwp_text_input_v3_listener listener = {
        enter, leave, preeditString, commitString, deleteSurroundingText, done,
    };
};

TextInput::TextInput(zwp_text_input_manager_v3* manager, wl_seat* seat)
    : m_input(zwp_text_input_manager_v3_get_text_input(manager, seat))
{
    zwp_text_input_v3_add_listener(m_input, &TextInputListeners::listener, this);
}

TextInput::~TextInput()
{
    zwp_text_input_v3_destroy(m_input);
}

void TextInput::update(ShellSurface* shell, const TextInputState* state)
{
    if (state)
        m_wanted[shell] = *state;
    else
        m_wanted.erase(shell);
    if (shell == m_entered)
        apply();
}

void TextInput::forget(ShellSurface* shell)
{
    m_wanted.erase(shell);
    if (m_entered == shell) {
        m_entered = nullptr;
        m_enabled = false;
        m_sent.reset();
    }
}

void TextInput::apply()
{
    if (!m_entered)
        return;
    const auto it = m_wanted.find(m_entered);
    if (it == m_wanted.end()) {
        if (m_enabled) {
            zwp_text_input_v3_disable(m_input);
            zwp_text_input_v3_commit(m_input);
            m_enabled = false;
            m_sent.reset();
        }
        return;
    }

    const TextInputState& state = it->second;
    if (!m_enabled) {
        zwp_text_input_v3_enable(m_input);
        m_enabled = true;
        m_sent.reset();
    }
    const bool fresh = !m_sent;
    const bool textChanged = fresh || m_sent->surroundingText != state.surroundingText
            || m_sent->cursor != state.cursor || m_sent->anchor != state.anchor;
    const bool purposeChanged = fresh || m_sent->purpose != state.purpose || m_sent->hints != state.hints;
    const bool rectangleChanged = fresh || !(m_sent->cursorRectangle == state.cursorRectangle);
    // Every commit is a new state for the input method to work on.
    if (!textChanged && !purposeChanged && !rectangleChanged)
        return;
    if (textChanged) {
        zwp_text_input_v3_set_surrounding_text(m_input, state.surroundingText.c_str(),
                                               int32_t(state.cursor), int32_t(state.anchor));
        zwp_text_input_v3_set_text_change_cause(
            m_input, m_inputMethodCause ? ZWP_TEXT_INPUT_V3_CHANGE_CAUSE_INPUT_METHOD
                                        : ZWP_TEXT_INPUT_V3_CHANGE_CAUSE_OTHER);
    }
    if (purposeChanged)
        zwp_text_input_v3_set_content_type(m_input, state.hints, wirePurpose(state.purpose));
    const RectF& rect = state.cursorRectangle;
    if (rectangleChanged)
        zwp_text_input_v3_set_cursor_rectangle(m_input, int32_t(std::floor(rect.x)),
                                               int32_t(std::floor(rect.y)),
                                               int32_t(std::ceil(rect.width)),
                                               int32_t(std::ceil(rect.height)));
    zwp_text_input_v3_commit(m_input);
    m_sent = state;
    m_inputMethodCause = false;
}

} // namespace Lingmo::ui::wayland
