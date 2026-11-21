// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <list>
#include <memory>
#include <unordered_map>
#include <utility>

namespace Lingmo::base {

// Owns up to a total cost of values; inserting past it drops the least
// recently used ones. Each value costs 1 unless given otherwise.
template<typename Key, typename Value, typename Hash = std::hash<Key>>
class LruCache
{
public:
    explicit LruCache(size_t maxCost = 100) : m_maxCost(maxCost) {}

    // False, and the value dropped, when it alone costs more than the cache
    // holds.
    bool insert(const Key& key, std::unique_ptr<Value> value, size_t cost = 1)
    {
        remove(key);
        if (cost > m_maxCost)
            return false;
        m_order.push_front(key);
        m_entries.emplace(key, Entry{std::move(value), cost, m_order.begin()});
        m_totalCost += cost;
        trim(m_maxCost);
        return true;
    }

    // Marks the entry as used; null when missing.
    Value* find(const Key& key)
    {
        const auto it = m_entries.find(key);
        if (it == m_entries.end())
            return nullptr;
        m_order.splice(m_order.begin(), m_order, it->second.position);
        return it->second.value.get();
    }
    bool contains(const Key& key) const { return m_entries.contains(key); }

    bool remove(const Key& key)
    {
        const auto it = m_entries.find(key);
        if (it == m_entries.end())
            return false;
        m_totalCost -= it->second.cost;
        m_order.erase(it->second.position);
        m_entries.erase(it);
        return true;
    }
    void clear()
    {
        m_entries.clear();
        m_order.clear();
        m_totalCost = 0;
    }

    size_t size() const { return m_entries.size(); }
    size_t totalCost() const { return m_totalCost; }
    size_t maxCost() const { return m_maxCost; }
    void setMaxCost(size_t maxCost)
    {
        m_maxCost = maxCost;
        trim(maxCost);
    }

private:
    struct Entry
    {
        std::unique_ptr<Value> value;
        size_t cost;
        typename std::list<Key>::iterator position;
    };

    void trim(size_t limit)
    {
        while (m_totalCost > limit && !m_order.empty())
            remove(m_order.back());
    }

    size_t m_maxCost;
    size_t m_totalCost = 0;
    std::list<Key> m_order;
    std::unordered_map<Key, Entry, Hash> m_entries;
};

} // namespace Lingmo::base
