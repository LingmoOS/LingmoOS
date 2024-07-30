/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MAPENTRY_P_H
#define MAPENTRY_P_H

#include <algorithm>
#include <cstdint>
#include <iterator>

#pragma pack(push)
#pragma pack(1)

template<typename KeyType>
struct MapEntry {
    KeyType key;
    uint16_t value;
};

template<typename KeyType>
constexpr inline bool operator<(MapEntry<KeyType> lhs, MapEntry<KeyType> rhs)
{
    return lhs.key < rhs.key;
}

template<typename KeyType>
constexpr inline bool operator<(MapEntry<KeyType> lhs, KeyType rhs)
{
    return lhs.key < rhs;
}

template<typename KeyType>
constexpr inline bool operator<(KeyType lhs, MapEntry<KeyType> rhs)
{
    return lhs < rhs.key;
}

template<typename MapEntry, std::size_t N>
inline constexpr bool isSortedLookupTable(const MapEntry (&map)[N])
{
#if __cplusplus > 201703L && defined(__GNUC__) && __GNUC__ > 9 && !defined(__clang__)
    return std::is_sorted(std::begin(map), std::end(map));
#else
    (void)map;
    return true;
#endif
}

#pragma pack(pop)

#endif
