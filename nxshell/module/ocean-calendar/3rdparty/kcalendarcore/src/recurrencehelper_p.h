/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALCORE_RECURRENCEHELPER_P_H
#define KCALCORE_RECURRENCEHELPER_P_H

#include <algorithm>

namespace KCalendarCore {
template<typename T>
inline void sortAndRemoveDuplicates(T &container)
{
    std::sort(container.begin(), container.end());
    container.erase(std::unique(container.begin(), container.end()), container.end());
}

template<typename T>
inline void inplaceSetDifference(T &set1, const T &set2)
{
    auto beginIt = set1.begin();
    for (const auto &elem : set2) {
        const auto it = std::lower_bound(beginIt, set1.end(), elem);
        if (it != set1.end() && *it == elem) {
            beginIt = set1.erase(it);
        }
    }
}

template<typename Container, typename Value>
inline void setInsert(Container &c, const Value &v)
{
    const auto it = std::lower_bound(c.begin(), c.end(), v);
    if (it == c.end() || *it != v) {
        c.insert(it, v);
    }
}

template<typename It, typename Value>
inline It strictLowerBound(It begin, It end, const Value &v)
{
    const auto it = std::lower_bound(begin, end, v);
    if (it == end || (*it) >= v) {
        return it == begin ? end : (it - 1);
    }
    return it;
}

} // namespace KCalendarCore

#endif
