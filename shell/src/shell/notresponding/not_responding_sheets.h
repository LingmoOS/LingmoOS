// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/signal.h"
#include "shell/shell.h"
#include "ui/view.h"
#include "wm/window_observer.h"

namespace Lingmo::ui {
class DialogButton;
class Label;
class ThemeIcon;
} // namespace Lingmo::UI

namespace Lingmo::WM {
class Window;
}

namespace Lingmo::Shell {

class ShellWindow;

// One sheet's content: the
// application's icon, "“Name” is not responding", the explanation, and
// Force Quit and Wait, on the dialog's opaque background.
class NotRespondingView : public ui::View
{
public:
    static constexpr int kWidth = 400;
    static constexpr int kPadding = 16;

    NotRespondingView(std::string name, std::string icon);

    const std::string &title() const;
    // Takes its text again after the session language changed.
    void retranslate();
    // The window's height for the layout at its width.
    double implicitHeight(double width) const;

    ui::DialogButton *forceQuitButton() const { return m_forceQuit; }
    ui::DialogButton *waitButton() const { return m_wait; }

    std::function<void()> onWait;
    std::function<void()> onForceQuit;

protected:
    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    void applyTheme();

    std::string m_name;
    ui::ThemeIcon *m_icon;
    ui::Label *m_title;
    ui::Label *m_body;
    ui::DialogButton *m_forceQuit;
    ui::DialogButton *m_wait;
    uint64_t m_themeGeneration = 0;
};

// Offers force quit for an application that stopped responding, once the
// user waits on one of its windows: a sheet hanging from that window, one
// per application, gone again as soon as the application answers. The
// window manager asks through setNotRespondingPrompt().
//
// Neither exo nor Ash handles hung clients; this is Lingmo's own design.
class NotRespondingSheets : public Component, public wm::WindowObserver
{
public:
    explicit NotRespondingSheets(Shell *shell);
    ~NotRespondingSheets() override;

    // A window whose process is unknown stands for an application of its own.
    static int64_t applicationOf(const wm::Window *window);

    // The window manager's prompt; always offered while the shell is up.
    bool prompt(wm::Window *window);

    // The sheet of an application, or null.
    ShellWindow *sheet(int64_t application) const;
    NotRespondingView *view(int64_t application) const;
    uint64_t parentWindow(int64_t application) const;
    size_t sheetCount() const { return m_sheets.size(); }
    bool isWaiting(int64_t application) const { return m_waiting.count(application) != 0; }

    // The user's answers.
    void wait(int64_t application);
    void forceQuit(int64_t application);

    // --- wm::WindowObserver ---

    void onWindowResponsiveChanged(wm::Window *window) override;
    void onWindowDestroying(wm::Window *window) override;

private:
    struct Sheet {
        std::unique_ptr<ShellWindow> window;
        NotRespondingView *view = nullptr;
        uint64_t parentWindow = 0;
    };

    void observe(wm::Window *window);
    void onWindowRemoved(wm::Window *window);
    void show(wm::Window *window, int64_t application);
    void hang(Sheet &sheet, wm::Window *parent);
    void close(int64_t application);
    void retranslate();

    Shell *m_shell;
    // By application: every window of a stalled client stops answering together.
    std::map<int64_t, Sheet> m_sheets;
    // Applications the user chose to wait for, until they answer again.
    std::set<int64_t> m_waiting;
    std::set<wm::Window *> m_observed;
    // Closed from inside their own buttons' handlers: destroyed on the next
    // loop turn.
    std::vector<std::unique_ptr<ShellWindow>> m_closed;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

