/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ISOCODES_P_H
#define ISOCODES_P_H

#include <QStringView>

#include <cstdint>

/**
 * Utilities for efficient storage of ISO codes.
 * The focus on constexpr here matters, as this is used for the compiled in data tables,
 * anything in there has to be constexpr in order to put the data tables into the shared read-only
 * data section.
 *
 * There's basically two formats in here:
 * - upper case + digit codes of up to 3 characters are stored as a 3 digit base 37 number, which
 *   fits into a 16bit value
 *   To simplify data table handling this is done in a way that lexicographical order is retained.
 * - two letter upper case codes like ISO 3166-1 alpha2: those are stored equivalent to a const char[2]
 *   technically the same approach as for 3 char codes could be used as well, but that doesn't give us
 *   any space advantage while considerably easing debugging (codes are directly readable).
 */
namespace IsoCodes
{
constexpr inline bool isAlpha(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

constexpr inline bool isAlpha(QChar c)
{
    return c.row() == 0 ? isAlpha(c.cell()) : false;
}

constexpr inline bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

constexpr inline bool isDigit(QChar c)
{
    return c.row() == 0 ? isDigit(c.cell()) : false;
}

constexpr inline uint8_t mapToUpper(char c)
{
    return c >= 'a' ? c - 32 : c;
}

constexpr inline uint8_t mapToUpper(QChar c)
{
    return mapToUpper(c.cell());
}

template<typename T>
constexpr inline uint16_t alpha2CodeToKey(T code, std::size_t size)
{
    return (size == 2 && isAlpha(code[0]) && isAlpha(code[1])) ? mapToUpper(code[0]) << 8 | mapToUpper(code[1]) : 0;
}

template<std::size_t N>
constexpr inline uint16_t alpha2CodeToKey(const char (&code)[N])
{
    return alpha2CodeToKey(code, N - 1);
}

constexpr inline uint16_t alpha2CodeToKey(QStringView code)
{
    return alpha2CodeToKey(code, code.size());
}

constexpr inline uint8_t mapToAlphaNumKey(char c)
{
    // this maps digits 0-9 to values 1-10, and letters to the numbers 11-36
    uint8_t key = c;
    if (key <= '9') {
        return key - '/';
    } else if (key >= 'a') {
        key -= 32;
    }
    return key - 'A' + 11;
}

constexpr inline uint8_t mapToAlphaNumKey(QChar c)
{
    return mapToAlphaNumKey(c.cell());
}

enum {
    AlphaNumKeyFactor = 37,
};

template<typename T>
constexpr inline char mapFromAlphaNumKey(T key)
{
    char c = key % IsoCodes::AlphaNumKeyFactor;
    if (c > 0 && c < 11) {
        return c + '/';
    }
    if (c >= 11) {
        return c + 'A' - 11;
    }
    return 0;
}

template<typename T>
constexpr inline uint16_t alphaNum3CodeToKey(T code, std::size_t size)
{
    if (size > 3 || size == 0) {
        return 0;
    }
    uint16_t key = 0;
    for (std::size_t i = 0; i < size; ++i) {
        if (!isAlpha(code[i]) && !isDigit(code[i])) {
            return 0;
        }
        key *= AlphaNumKeyFactor;
        key += mapToAlphaNumKey(code[i]);
    }
    for (std::size_t i = size; i < 3; ++i) { // "left align" to retain lexicographical order
        key *= AlphaNumKeyFactor;
    }
    return key;
}

constexpr inline uint16_t alphaNum3CodeToKey(QStringView code)
{
    return alphaNum3CodeToKey(code, code.size());
}

template<typename T>
constexpr inline uint16_t alpha3CodeToKey(T code, std::size_t size)
{
    return (size == 3 && isAlpha(code[0]) && isAlpha(code[1]) && isAlpha(code[2])) ? alphaNum3CodeToKey(code, 3) : 0;
}

template<std::size_t N>
constexpr inline uint16_t alpha3CodeToKey(const char (&code)[N])
{
    return alpha3CodeToKey(code, N - 1);
}

constexpr inline uint16_t alpha3CodeToKey(QStringView code)
{
    return alpha3CodeToKey(code, code.size());
}

constexpr inline uint32_t subdivisionCodeToKey(const char *code, std::size_t size)
{
    if (size < 4 || code[2] != '-') {
        return 0;
    }

    const auto countryKey = alpha2CodeToKey(code, 2);
    const auto subdivKey = alphaNum3CodeToKey(code + 3, size - 3);
    return countryKey > 0 && subdivKey > 0 ? ((uint32_t)(countryKey) << 16 | subdivKey) : 0;
}

constexpr inline uint32_t subdivisionCodeToKey(QStringView code)
{
    if (code.size() < 4 || code[2] != QLatin1Char('-')) {
        return 0;
    }

    const auto countryKey = alpha2CodeToKey(code.left(2));
    const auto subdivKey = alphaNum3CodeToKey(code.mid(3), code.size() - 3);
    return countryKey > 0 && subdivKey > 0 ? ((uint32_t)(countryKey) << 16 | subdivKey) : 0;
}

template<std::size_t N>
constexpr inline uint32_t subdivisionCodeToKey(const char (&code)[N])
{
    return subdivisionCodeToKey(code, N - 1);
}

/// Before v4.16 iso-codes used for parent code just the subdivision code, without the country
/// (only by error sometimes). Since that version the full code now is always used.
/// Handle both cases gracefully.
/// Does not check the country part for sanity, but just discards the info.
constexpr inline uint16_t parentCodeToKey(QStringView code)
{
    if (code.size() < 4) {
        return alphaNum3CodeToKey(code);
    }
    if (code[2] != QLatin1Char('-')) {
        return 0;
    }

    const auto countryKey = alpha2CodeToKey(code.left(2));
    const auto subdivKey = alphaNum3CodeToKey(code.mid(3), code.size() - 3);
    return countryKey > 0 ? subdivKey : 0;
}
}

#endif // ISOCODES_P_H
