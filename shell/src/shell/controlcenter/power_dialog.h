// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <memory>
#include <string>

namespace Lingmo::Shell {

class Shell;
class ShellWindow;

// FishUI's ConfirmDialog asking before logging
// out, restarting or shutting down, a toplevel kept above applications and
// centred by the window manager. Return confirms even while Cancel has the
// focus; Escape and the close button cancel. Asking again while it is up
// changes what it asks.
class PowerDialog
{
public:
    enum class Action {
        Logout,
        Reboot,
        Shutdown,
    };

    explicit PowerDialog(Shell *shell);
    ~PowerDialog();

    PowerDialog(const PowerDialog &) = delete;
    PowerDialog &operator=(const PowerDialog &) = delete;

    // Shows the dialog for `action`, or changes it while shown.
    void open(Action action);
    // Hides it without an answer.
    void close();
    bool isOpen() const;
    Action action() const { return m_action; }
    // Takes its text again after the session language changed, still asking
    // what it asked.
    void retranslate();

    ShellWindow *window() const { return m_window.get(); }

    std::function<void()> onAccepted;
    std::function<void()> onRejected;

    static std::string title(Action action);
    static std::string confirmText(Action action);

private:
    class Content;

    void finish(bool accepted);
    void resize();

    Shell *m_shell;
    std::unique_ptr<ShellWindow> m_window;
    Content *m_content = nullptr;
    Action m_action = Action::Logout;
};

} // namespace Lingmo::Shell

