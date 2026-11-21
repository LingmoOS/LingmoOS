// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <concepts>
#include <memory>

namespace Lingmo::base {

namespace internal {

struct WeakFlag
{
    bool valid = true;
};

} // namespace internal

// A pointer that turns null when its object goes, for objects that are not
// owned through shared_ptr. Not thread-safe: use and invalidate it on one
// thread.
template<typename T>
class WeakPtr
{
public:
    WeakPtr() = default;
    WeakPtr(std::nullptr_t) {}
    template<typename U>
        requires std::convertible_to<U*, T*>
    WeakPtr(const WeakPtr<U>& other) : m_flag(other.m_flag), m_pointer(other.m_pointer) {}

    T* get() const { return m_flag && m_flag->valid ? m_pointer : nullptr; }
    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }
    explicit operator bool() const { return get() != nullptr; }
    void reset()
    {
        m_flag.reset();
        m_pointer = nullptr;
    }

    // Equal when both point to the same live object, or both are null.
    friend bool operator==(const WeakPtr& a, const WeakPtr& b) { return a.get() == b.get(); }
    friend bool operator==(const WeakPtr& a, std::nullptr_t) { return !a.get(); }

private:
    template<typename U>
    friend class WeakPtr;
    template<typename U>
    friend class WeakPtrFactory;

    WeakPtr(std::shared_ptr<internal::WeakFlag> flag, T* pointer) : m_flag(std::move(flag)), m_pointer(pointer) {}

    std::shared_ptr<internal::WeakFlag> m_flag;
    T* m_pointer = nullptr;
};

// Hands out WeakPtrs to its owner and invalidates them when it goes. Make it
// the owner's last member, so the pointers die before the rest of the owner:
//
//   WeakPtrFactory<Window> m_weakFactory{this};
template<typename T>
class WeakPtrFactory
{
public:
    explicit WeakPtrFactory(T* owner) : m_owner(owner) {}
    ~WeakPtrFactory() { invalidateWeakPtrs(); }

    WeakPtrFactory(const WeakPtrFactory&) = delete;
    WeakPtrFactory& operator=(const WeakPtrFactory&) = delete;

    WeakPtr<T> getWeakPtr()
    {
        if (!m_flag)
            m_flag = std::make_shared<internal::WeakFlag>();
        return WeakPtr<T>(m_flag, m_owner);
    }

    // Every pointer handed out so far turns null; later ones work.
    void invalidateWeakPtrs()
    {
        if (m_flag)
            m_flag->valid = false;
        m_flag.reset();
    }

    bool hasWeakPtrs() const { return m_flag && m_flag.use_count() > 1; }

private:
    T* m_owner;
    std::shared_ptr<internal::WeakFlag> m_flag;
};

} // namespace Lingmo::base
