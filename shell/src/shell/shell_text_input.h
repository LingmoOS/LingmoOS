// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include "ui/event.h"
#include "wayland/protocols/text_input.h"

namespace Lingmo::Shell {

class ShellWindow;

// The shell's text fields on the input-method relay: one more adapter next
// to text-input-v3 and v1, as exo's TextInput and Ash's views share one
// ui::InputMethod. The keyboard-focused shell window's focused text view
// enables it; the input method's edits go back to that window.
class ShellTextInput final : public wayland::TextInput
{
public:
    explicit ShellTextInput(wayland::InputMethodRelay *relay);
    ~ShellTextInput() override;

    // The toolkit's textInputChanged(); null disables.
    void update(ShellWindow *window, const ui::TextInputState *state, bool fromInputMethod);

    void windowDestroying(ShellWindow *window);

    // wayland::TextInput:
    bool attached() const override { return m_window != nullptr; }
    bool ownsSurface(const compositor::Surface *surface) const override;
    void sendEdit(const wayland::TextInputEdit &edit) override;
    void detached() override;
    void focusChanged(compositor::Surface *gained, compositor::Surface *lost) override;

private:
    ShellWindow *m_window = nullptr;
};

} // namespace Lingmo::Shell

