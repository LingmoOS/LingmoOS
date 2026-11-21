// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/shell_text_input.h"

#include <cmath>

#include "shell/shell.h"
#include "shell/shell_input.h"
#include "shell/shell_window.h"
#include "ui/embedded_window.h"

namespace Lingmo::Shell {

namespace {

// text-input-v3's change_cause and content_purpose values.
constexpr uint32_t kCauseInputMethod = 0;
constexpr uint32_t kCauseOther = 1;

uint32_t contentPurpose(ui::TextPurpose purpose)
{
    switch (purpose) {
    case ui::TextPurpose::Normal:
        return 0;
    case ui::TextPurpose::Number:
        return 3;
    case ui::TextPurpose::Url:
        return 5;
    case ui::TextPurpose::Email:
        return 6;
    case ui::TextPurpose::Password:
        return 8;
    }
    return 0;
}

} // namespace

ShellTextInput::ShellTextInput(wayland::InputMethodRelay *relay)
    : wayland::TextInput(relay)
{
    m_relay->add(this);
}

ShellTextInput::~ShellTextInput()
{
    m_relay->remove(this);
}

bool ShellTextInput::ownsSurface(const compositor::Surface *surface) const
{
    return m_window && m_window->surface() == surface;
}

void ShellTextInput::update(ShellWindow *window, const ui::TextInputState *state,
                            bool fromInputMethod)
{
    if (!state) {
        if (window == m_window) {
            m_relay->deactivate(this);
            m_window = nullptr;
            m_state = {};
        }
        return;
    }
    // Only the window with the keyboard has a text field the input method serves.
    if (!window || m_relay->focusedSurface() != window->surface())
        return;
    const bool restart = window != m_window || m_relay->active() != this;
    m_window = window;
    m_state.surrounding = state->surroundingText;
    m_state.cursor = int32_t(state->cursor);
    m_state.anchor = int32_t(state->anchor);
    m_state.cause = fromInputMethod ? kCauseInputMethod : kCauseOther;
    m_state.content = std::pair<uint32_t, uint32_t>(0, contentPurpose(state->purpose));
    const ui::RectF &caret = state->cursorRectangle;
    const int left = int(std::floor(caret.x));
    const int top = int(std::floor(caret.y));
    m_state.rectangle = base::Rect(left, top, std::max(1, int(std::ceil(caret.x + caret.width)) - left),
                                   std::max(1, int(std::ceil(caret.y + caret.height)) - top));
    if (restart)
        m_relay->activate(this, true);
    else
        m_relay->stateChanged(this, false);
}

void ShellTextInput::windowDestroying(ShellWindow *window)
{
    if (window == m_window) {
        m_relay->deactivate(this);
        m_window = nullptr;
        m_state = {};
    }
}

void ShellTextInput::sendEdit(const wayland::TextInputEdit &edit)
{
    if (!m_window || !m_window->window())
        return;
    ui::TextInputEvent event;
    if (edit.deletion) {
        event.deleteBefore = edit.deletion->first;
        event.deleteAfter = edit.deletion->second;
    }
    if (edit.commit)
        event.commit = *edit.commit;
    event.preedit = edit.preedit;
    event.preeditCursorBegin = edit.preedit.empty() ? -1 : edit.begin;
    event.preeditCursorEnd = edit.preedit.empty() ? -1 : edit.end;
    m_window->window()->textInput(event);
}

void ShellTextInput::detached()
{
    m_window = nullptr;
    m_state = {};
}

void ShellTextInput::focusChanged(compositor::Surface *, compositor::Surface *)
{
    // The window gaining the keyboard tells its own state when it has one.
    m_window = nullptr;
    m_state = {};
}

} // namespace Lingmo::Shell
