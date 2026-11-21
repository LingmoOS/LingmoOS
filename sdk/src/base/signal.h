// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

// A host toolkit may define `emit` as an empty macro in an earlier header.
#pragma push_macro("emit")
#undef emit

namespace Lingmo::base {

// Keeps a callback connected to a Signal; destroying, reassigning or
// resetting it disconnects the callback.
class [[nodiscard]] Subscription
{
public:
    Subscription() = default;
    explicit Subscription(std::function<void()> disconnect) : m_disconnect(std::move(disconnect)) {}
    ~Subscription() { reset(); }

    Subscription(Subscription&& other) noexcept : m_disconnect(std::exchange(other.m_disconnect, nullptr)) {}
    Subscription& operator=(Subscription&& other) noexcept
    {
        if (this != &other) {
            reset();
            m_disconnect = std::exchange(other.m_disconnect, nullptr);
        }
        return *this;
    }

    void reset()
    {
        if (auto disconnect = std::exchange(m_disconnect, nullptr))
            disconnect();
    }

private:
    std::function<void()> m_disconnect;
};

// Callbacks to call when something happens. Callbacks may connect and
// disconnect others, or destroy the signal's owner, while it is emitted.
template<typename... Args>
class Signal
{
public:
    Signal() = default;
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    Subscription connect(std::function<void(Args...)> callback)
    {
        auto slot = std::make_shared<Slot>(Slot{std::move(callback), true});
        m_slots->push_back(slot);
        std::weak_ptr<std::vector<std::shared_ptr<Slot>>> weakSlots = m_slots;
        return Subscription([weakSlots, slot] {
            slot->connected = false;
            if (auto list = weakSlots.lock())
                std::erase(*list, slot);
        });
    }

    void emit(Args... args) const
    {
        // Copies, so changes during the emission apply to the next one.
        const auto snapshot = *m_slots;
        for (const auto& slot : snapshot) {
            if (slot->connected)
                slot->callback(args...);
        }
    }
    void operator()(Args... args) const { emit(std::forward<Args>(args)...); }

    bool isEmpty() const { return m_slots->empty(); }

private:
    struct Slot
    {
        std::function<void(Args...)> callback;
        bool connected;
    };

    std::shared_ptr<std::vector<std::shared_ptr<Slot>>> m_slots =
        std::make_shared<std::vector<std::shared_ptr<Slot>>>();
};

} // namespace Lingmo::base

#pragma pop_macro("emit")
