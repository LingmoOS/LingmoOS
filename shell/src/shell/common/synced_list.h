// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "shell/common/list_model.h"

namespace Lingmo::Shell {

// A list of rows that reaches a new list through removes, moves and inserts
// instead of a reset, so a view keeps the items (and loaded icons) of rows
// that stay. Rows are matched by a unique key; rows on the longest run
// already in the new order stay put, so a drag reorder is reported as the
// single move it was.
template<typename T>
class SyncedList
{
public:
    using Key = std::function<std::string(const T &)>;
    using SameContent = std::function<bool(const T &, const T &)>;

    SyncedList(ListModelSignals &signals, Key key, SameContent sameContent = {})
        : m_signals(signals)
        , m_key(std::move(key))
        , m_sameContent(std::move(sameContent))
    {
    }

    const std::vector<T> &rows() const { return m_rows; }

    void sync(const std::vector<T> &target)
    {
        std::unordered_map<std::string, size_t> targetIndex;
        for (size_t i = 0; i < target.size(); ++i)
            targetIndex.emplace(m_key(target[i]), i);

        removeMissing(targetIndex);
        reorderSurvivors(target, targetIndex);
        insertMissing(target);

        for (size_t i = 0; i < m_rows.size(); ++i) {
            // Without a comparison every row counts as unchanged.
            if (!m_sameContent || m_sameContent(m_rows[i], target[i]))
                continue;
            m_rows[i] = target[i];
            m_signals.dataChanged(int(i), int(i));
        }
    }

private:
    int indexOfKey(const std::string &key) const
    {
        for (size_t i = 0; i < m_rows.size(); ++i) {
            if (m_key(m_rows[i]) == key)
                return int(i);
        }
        return -1;
    }

    void removeMissing(const std::unordered_map<std::string, size_t> &targetIndex)
    {
        for (int last = int(m_rows.size()) - 1; last >= 0; --last) {
            if (targetIndex.count(m_key(m_rows[size_t(last)])))
                continue;
            int first = last;
            while (first > 0 && !targetIndex.count(m_key(m_rows[size_t(first - 1)])))
                --first;
            m_rows.erase(m_rows.begin() + first, m_rows.begin() + last + 1);
            m_signals.rowsRemoved(first, last - first + 1);
            last = first;
        }
    }

    void reorderSurvivors(const std::vector<T> &target, const std::unordered_map<std::string, size_t> &targetIndex)
    {
        const size_t n = m_rows.size();
        std::vector<size_t> order(n);
        for (size_t i = 0; i < n; ++i)
            order[i] = targetIndex.at(m_key(m_rows[i]));
        if (std::is_sorted(order.begin(), order.end()))
            return;

        // The longest increasing run of target positions.
        std::vector<size_t> tails;
        std::vector<int> previous(n, -1);
        for (size_t i = 0; i < n; ++i) {
            const auto tail = std::lower_bound(tails.begin(), tails.end(), order[i],
                                               [&order](size_t index, size_t value) { return order[index] < value; });
            const size_t length = size_t(tail - tails.begin());
            if (length > 0)
                previous[i] = int(tails[length - 1]);
            if (length == tails.size())
                tails.push_back(i);
            else
                tails[length] = i;
        }
        std::unordered_set<std::string> fixed;
        for (int i = int(tails.back()); i >= 0; i = previous[size_t(i)])
            fixed.insert(m_key(m_rows[size_t(i)]));

        std::unordered_set<std::string> survivors;
        for (const T &row : m_rows)
            survivors.insert(m_key(row));

        // Each moved row goes right after the survivor that precedes it in
        // the target, which is already in place by then.
        std::string precedingKey;
        bool hasPreceding = false;
        for (const T &row : target) {
            const std::string key = m_key(row);
            if (!survivors.count(key))
                continue;
            if (!fixed.count(key)) {
                const int from = indexOfKey(key);
                const int anchor = hasPreceding ? indexOfKey(precedingKey) : -1;
                const int to = from > anchor ? anchor + 1 : anchor;
                if (to != from) {
                    T moved = std::move(m_rows[size_t(from)]);
                    m_rows.erase(m_rows.begin() + from);
                    m_rows.insert(m_rows.begin() + to, std::move(moved));
                    m_signals.rowMoved(from, to);
                }
            }
            precedingKey = key;
            hasPreceding = true;
        }
    }

    void insertMissing(const std::vector<T> &target)
    {
        std::unordered_set<std::string> present;
        for (const T &row : m_rows)
            present.insert(m_key(row));

        for (size_t first = 0; first < target.size(); ++first) {
            if (present.count(m_key(target[first])))
                continue;
            size_t last = first;
            while (last + 1 < target.size() && !present.count(m_key(target[last + 1])))
                ++last;
            m_rows.insert(m_rows.begin() + ptrdiff_t(first), target.begin() + ptrdiff_t(first),
                          target.begin() + ptrdiff_t(last + 1));
            m_signals.rowsInserted(int(first), int(last - first + 1));
            first = last;
        }
    }

    ListModelSignals &m_signals;
    Key m_key;
    SameContent m_sameContent;
    std::vector<T> m_rows;
};

} // namespace Lingmo::Shell

