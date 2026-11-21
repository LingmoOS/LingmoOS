// Copyright 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "shell/shell.h"
#include "base/timer.h"
#include <memory>
namespace Lingmo::base::dbus {class Object;}
namespace Lingmo::Shell {
class ShellWindow;
class ScreenKeyboardView;
class ScreenKeyboard : public Component {
public:
    explicit ScreenKeyboard(Shell* shell);
    ~ScreenKeyboard() override;
    bool registerService();
    void show();
    void hide();
    void toggle();
    bool visible() const;
    ShellWindow* window() const {return m_window.get();}
private:
    bool allowed() const;
    bool send(uint32_t symbol,bool control,bool alt);
    void place();
    Shell* m_shell;
    std::unique_ptr<ShellWindow> m_window;
    ScreenKeyboardView* m_view=nullptr;
    std::unique_ptr<base::dbus::Object> m_service;
    base::Timer m_guard;
    const void* m_lastFocus=nullptr;
    int m_glassDark=-1;
    std::vector<base::Subscription> m_subscriptions;
};
}
