// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <map>
#include <memory>
#include <vector>

#include "ui/embedded_host.h"

namespace Lingmo::Wayland {
class InputMethodRelay;
}

namespace Lingmo::Shell {

class Shell;
class ShellDrag;
class ShellTextInput;
class ShellWindow;

// What the toolkit's embedded windows ask of the compositor, answered as the
// compositor answers a Wayland client: popups become popup ShellWindows
// placed by the xdg positioner, activation goes to the window manager, text
// input to the input-method relay, the blur to the window's surface and
// drags to the seat's drag controller.
class ShellHost final : public ui::EmbeddedHost
{
public:
    // `inputMethod` may be null, which leaves the shell without one.
    ShellHost(Shell *shell, wayland::InputMethodRelay *inputMethod);
    ShellHost(const ShellHost &) = delete;
    ShellHost &operator=(const ShellHost &) = delete;
    ~ShellHost() override;

    // The shell window hosting `window`, a toplevel's or a popup's.
    ShellWindow *windowFor(const ui::EmbeddedWindow *window) const;
    ShellDrag *drag() const { return m_drag.get(); }

    // Called by ShellWindow; not part of the public API.
    void windowDestroying(ShellWindow *window);
    // Called by ShellWindow before it renders a frame; not part of the public API.
    void frameStarting(ShellWindow *window);

    // ui::EmbeddedHost:
    void popupShown(ui::EmbeddedWindow *popup) override;
    void popupRepositioned(ui::EmbeddedWindow *popup) override;
    void popupHidden(ui::EmbeddedWindow *popup) override;
    void activationRequested(ui::EmbeddedWindow *window) override;
    void textInputChanged(ui::EmbeddedWindow *window, const ui::TextInputState *state,
                          bool fromInputMethod) override;
    void blurChanged(ui::EmbeddedWindow *window) override;
    bool startDrag(ui::EmbeddedWindow *window, ui::DragSource source) override;

private:
    Shell *m_shell;
    std::map<const ui::EmbeddedWindow *, std::unique_ptr<ShellWindow>> m_popups;
    // Popups let go of, destroyed on the next loop turn: the toolkit hides a
    // popup from inside input the seat is still dispatching to its surface.
    std::vector<std::unique_ptr<ShellWindow>> m_released;
    // The dark mode each window's glass was tinted for.
    std::map<const ui::EmbeddedWindow *, bool> m_glassDark;
    std::unique_ptr<ShellDrag> m_drag;
    std::unique_ptr<ShellTextInput> m_textInput;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace Lingmo::Shell

