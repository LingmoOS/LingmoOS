// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "base/geometry.h"
#include "base/timer.h"
#include "ui/event.h"

namespace Lingmo::Compositor {
class Surface;
}

namespace Lingmo::Input {
class Keyboard;
class Pointer;
class Touch;
} // namespace Lingmo::Input

namespace Lingmo::Shell {

class Shell;
class ShellWindow;

// The shell's windows as one more client of the seat: a pointer, a keyboard
// and a touch device whose delegates accept only the shell's surfaces and
// hand the events to their toolkit windows. Targeting, implicit grabs,
// focus, the session gate, popup grabs and the cursor therefore work as they
// do for a Wayland client's surfaces, as Ash's widgets and exo's surfaces
// share one aura event dispatcher.
class ShellInput
{
public:
    explicit ShellInput(Shell *shell);
    ShellInput(const ShellInput &) = delete;
    ShellInput &operator=(const ShellInput &) = delete;
    ~ShellInput();

    // The shell window owning `surface`, if any.
    ShellWindow *windowForSurface(const compositor::Surface *surface) const;
    // Where the pointer and the keyboard are, among the shell's windows.
    ShellWindow *pointerWindow() const { return m_pointerWindow; }
    ShellWindow *keyboardWindow() const { return m_keyboardWindow; }

    // The toolkit's modifier mask for the keys held now, as clients see them.
    uint32_t modifiers() const;

    // Called by ShellWindow; not part of the public API.
    void cursorShapeChanged(ShellWindow *window);
    void windowDestroying(ShellWindow *window);

private:
    class PointerDelegate;
    class KeyboardDelegate;
    class TouchDelegate;
    class GestureDelegate;
    friend class PointerDelegate;
    friend class KeyboardDelegate;
    friend class TouchDelegate;
    friend class GestureDelegate;

    void applyCursor();
    void sendKey(uint32_t code, bool pressed, bool repeat);
    void stopRepeat();

    Shell *m_shell;
    std::unique_ptr<PointerDelegate> m_pointerDelegate;
    std::unique_ptr<input::Pointer> m_pointer;
    std::unique_ptr<input::Keyboard> m_keyboard;
    std::unique_ptr<TouchDelegate> m_touchDelegate;
    std::unique_ptr<input::Touch> m_touch;
    std::vector<std::unique_ptr<GestureDelegate>> m_gestureDelegates;
    ShellWindow *m_pointerWindow = nullptr;
    ShellWindow *m_keyboardWindow = nullptr;
    // Touch points down on a shell window, by id.
    std::map<int32_t, ShellWindow *> m_touchPoints;
    // The window a touchpad gesture began on, which gets the rest of it.
    ShellWindow *m_gestureWindow = nullptr;
    base::Timer m_repeatTimer;
    uint32_t m_repeatCode = 0;
    bool m_repeatEnabled = true;
    std::chrono::milliseconds m_repeatDelay {500};
    std::chrono::milliseconds m_repeatInterval {40};
};

} // namespace Lingmo::Shell

