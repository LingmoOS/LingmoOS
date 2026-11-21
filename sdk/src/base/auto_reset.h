// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <utility>

namespace Lingmo::base {

// Restores a variable's value when the scope ends.
template<typename T>
class [[nodiscard]] AutoReset
{
public:
    // Saves the current value.
    explicit AutoReset(T& variable) : m_variable(variable), m_original(variable) {}
    // Saves the current value and sets `value`.
    template<typename U>
    AutoReset(T& variable, U&& value) : m_variable(variable), m_original(std::exchange(variable, std::forward<U>(value)))
    {
    }
    ~AutoReset() { m_variable = std::move(m_original); }

    AutoReset(const AutoReset&) = delete;
    AutoReset& operator=(const AutoReset&) = delete;

private:
    T& m_variable;
    T m_original;
};

// Runs a function when the scope ends, unless dismissed.
class [[nodiscard]] ScopeGuard
{
public:
    explicit ScopeGuard(std::function<void()> function) : m_function(std::move(function)) {}
    ~ScopeGuard()
    {
        if (m_function)
            m_function();
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    void dismiss() { m_function = nullptr; }

private:
    std::function<void()> m_function;
};

} // namespace Lingmo::base
