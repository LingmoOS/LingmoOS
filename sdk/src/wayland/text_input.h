// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "ui/event.h"

struct wl_seat;
struct zwp_text_input_manager_v3;
struct zwp_text_input_v3;

namespace Lingmo::ui::wayland {

class ShellSurface;
struct TextInputListeners;

// The seat's text-input-v3 object. It is enabled only while the window the
// compositor sent it to has a text view with focus, so the input method knows
// when typing is text and when it is a shortcut.
class TextInput
{
public:
    TextInput(zwp_text_input_manager_v3* manager, wl_seat* seat);
    ~TextInput();

    TextInput(const TextInput&) = delete;
    TextInput& operator=(const TextInput&) = delete;

    // The focused text view's state, or none to disable.
    void update(ShellSurface* shell, const TextInputState* state);
    void forget(ShellSurface* shell);

private:
    friend struct TextInputListeners;

    void apply();

    zwp_text_input_v3* m_input;
    ShellSurface* m_entered = nullptr;
    std::unordered_map<ShellSurface*, TextInputState> m_wanted;
    std::optional<TextInputState> m_sent;
    bool m_enabled = false;
    // The next state follows from what the input method sent.
    bool m_inputMethodCause = false;
    TextInputEvent m_pending;
};

} // namespace Lingmo::ui::wayland
