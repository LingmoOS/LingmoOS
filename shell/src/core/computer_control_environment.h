// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_CORE_COMPUTER_CONTROL_ENVIRONMENT_H_
#define LINGMO_CORE_COMPUTER_CONTROL_ENVIRONMENT_H_

#include <functional>

#include "computer_control/controller.h"

namespace Lingmo {

namespace ComputerControl {
class PermissionConsent;
}

namespace Input {
class Seat;
}

namespace Session {
class SessionController;
}

namespace Wayland {
class InputMethodManager;
}

namespace WM {
class WindowManager;
}

namespace Core {

// What of the session an agent is given to drive: the windows, the seat, the
// input method and the clipboard, behind the controller's Environment.
struct ComputerControlHosts {
    wm::WindowManager *windowManager = nullptr;
    input::Seat *seat = nullptr;
    const session::SessionController *session = nullptr;
    computer_control::PermissionConsent *consent = nullptr;
    // Wayland comes up after the controller, so the input method is looked up late.
    std::function<wayland::InputMethodManager *()> inputMethod;
    // Whether the process `pid` shows system dialogs the agent must leave alone.
    std::function<bool(int64_t pid)> showsSystemDialogs;
    // The desktop entry of the application window `id`; empty for none.
    std::function<std::string(uint64_t id)> windowApplication;
};

computer_control::Environment computerControlEnvironment(ComputerControlHosts hosts);

} // namespace core
} // namespace Lingmo

#endif // LINGMO_CORE_COMPUTER_CONTROL_ENVIRONMENT_H_
