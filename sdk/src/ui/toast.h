// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <functional>
#include <string>

namespace Lingmo::ui {

class Window;

// A toast: short messages at the bottom centre of a window, the
// newest on top, each sliding and fading in and out over 400 ms. A message
// goes after its time unless the pointer rests on it, or when clicked.
class Toast
{
public:
    explicit Toast(Window* window);
    ~Toast();

    Toast(const Toast&) = delete;
    Toast& operator=(const Toast&) = delete;

    // As showNotification's "short", default and "long".
    static constexpr std::chrono::milliseconds Short{4000};
    static constexpr std::chrono::milliseconds Normal{7000};
    static constexpr std::chrono::milliseconds Long{12000};

    // With action text, a button that closes the message and calls the action.
    void show(std::string message, std::chrono::milliseconds timeout = Normal, std::string actionText = {},
              std::function<void()> action = {});

private:
    class Stack;

    Window* m_window;
    Stack* m_stack = nullptr;
};

} // namespace Lingmo::ui
