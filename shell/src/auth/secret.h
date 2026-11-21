// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Lingmo::Auth {

inline void wipeString(std::string &secret)
{
    if (!secret.empty())
        explicit_bzero(secret.data(), secret.size());
    secret.clear();
}

class Secret
{
public:
    Secret() = default;

    explicit Secret(std::string_view text)
    {
        // Reserved first: growing for the terminator would leave a copy behind.
        m_data.reserve(text.size() + 1);
        m_data.assign(text.begin(), text.end());
        m_data.push_back('\0');
    }

    Secret(const Secret &) = delete;
    Secret &operator=(const Secret &) = delete;

    Secret(Secret &&other) noexcept
        : m_data(std::move(other.m_data))
    {
        other.m_data.clear();
    }

    Secret &operator=(Secret &&other) noexcept
    {
        if (this != &other) {
            clear();
            m_data = std::move(other.m_data);
            other.m_data.clear();
        }
        return *this;
    }

    ~Secret() { clear(); }

    bool isEmpty() const { return m_data.size() <= 1; }

    // NUL-terminated, for the C APIs that take a credential. Never valid after
    // clear() and never copied into anything that outlives this object.
    const char *constData() const { return m_data.empty() ? "" : m_data.data(); }

    size_t size() const { return m_data.empty() ? 0 : m_data.size() - 1; }

    void clear()
    {
        if (!m_data.empty()) {
            // volatile so the write is not optimised away as dead.
            volatile char *data = m_data.data();
            for (size_t i = 0; i < m_data.size(); ++i)
                data[i] = '\0';
        }
        m_data.clear();
    }

private:
    std::vector<char> m_data;
};

} // namespace Lingmo::Auth

