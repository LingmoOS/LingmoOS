// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/collator.h"

#include <algorithm>
#include <cctype>
#include <mutex>
#include <vector>

#include <unicode/ucol.h>
#include <unicode/utf16.h>
#include <unicode/utf8.h>

namespace Lingmo::base {

// Guards the lazy open; compare() on an open collator needs no lock.
static std::mutex s_openMutex;

Collator::Collator(const Locale& locale)
    : m_id(locale.m_collation)
{
}

Collator::~Collator() = default;

Collator::Collator(const Collator& other)
    : m_id(other.m_id)
    , m_numeric(other.m_numeric)
    , m_caseSensitive(other.m_caseSensitive)
    , m_ignorePunctuation(other.m_ignorePunctuation)
{
    std::lock_guard lock(s_openMutex);
    m_collator = other.m_collator;
}

Collator& Collator::operator=(const Collator& other)
{
    if (this == &other)
        return *this;
    m_id = other.m_id;
    m_numeric = other.m_numeric;
    m_caseSensitive = other.m_caseSensitive;
    m_ignorePunctuation = other.m_ignorePunctuation;
    std::lock_guard lock(s_openMutex);
    m_collator = other.m_collator;
    return *this;
}

void Collator::setNumericMode(bool numeric)
{
    if (numeric == m_numeric)
        return;
    m_numeric = numeric;
    m_collator.reset();
}

void Collator::setCaseSensitive(bool sensitive)
{
    if (sensitive == m_caseSensitive)
        return;
    m_caseSensitive = sensitive;
    m_collator.reset();
}

void Collator::setIgnorePunctuation(bool ignore)
{
    if (ignore == m_ignorePunctuation)
        return;
    m_ignorePunctuation = ignore;
    m_collator.reset();
}

const UCollator* Collator::collator() const
{
    if (m_id.empty())
        return nullptr;
    std::lock_guard lock(s_openMutex);
    if (m_collator)
        return m_collator.get();
    UErrorCode status = U_ZERO_ERROR;
    UCollator* opened = ucol_open(m_id.c_str(), &status);
    if (U_FAILURE(status) || !opened) {
        if (opened)
            ucol_close(opened);
        return nullptr;
    }
    // Numeric collation for numeric mode, secondary strength for
    // case-insensitive, shifted alternates for ignoring punctuation.
    if (m_numeric)
        ucol_setAttribute(opened, UCOL_NUMERIC_COLLATION, UCOL_ON, &status);
    if (!m_caseSensitive)
        ucol_setAttribute(opened, UCOL_STRENGTH, UCOL_SECONDARY, &status);
    if (m_ignorePunctuation)
        ucol_setAttribute(opened, UCOL_ALTERNATE_HANDLING, UCOL_SHIFTED, &status);
    m_collator.reset(opened, ucol_close);
    return opened;
}

static int foldedCompare(std::string_view a, std::string_view b, bool caseSensitive)
{
    const size_t length = std::min(a.size(), b.size());
    for (size_t i = 0; i < length; ++i) {
        unsigned char x = static_cast<unsigned char>(a[i]);
        unsigned char y = static_cast<unsigned char>(b[i]);
        if (!caseSensitive) {
            x = static_cast<unsigned char>(std::tolower(x));
            y = static_cast<unsigned char>(std::tolower(y));
        }
        if (x != y)
            return x < y ? -1 : 1;
    }
    return a.size() < b.size() ? -1 : a.size() > b.size() ? 1 : 0;
}

int Collator::compare(std::string_view a, std::string_view b) const
{
    if (a.empty())
        return b.empty() ? 0 : -1;
    if (b.empty())
        return 1;
    const UCollator* ucollator = collator();
    if (!ucollator)
        return foldedCompare(a, b, m_caseSensitive);
    UErrorCode status = U_ZERO_ERROR;
    const UCollationResult order = ucol_strcollUTF8(ucollator, a.data(), int32_t(a.size()), b.data(),
                                                    int32_t(b.size()), &status);
    if (U_FAILURE(status))
        return foldedCompare(a, b, m_caseSensitive);
    return order == UCOL_LESS ? -1 : order == UCOL_GREATER ? 1 : 0;
}

std::string Collator::sortKey(std::string_view text) const
{
    const UCollator* ucollator = collator();
    if (!ucollator) {
        std::string key(text);
        if (!m_caseSensitive) {
            for (char& c : key)
                c = char(std::tolower(static_cast<unsigned char>(c)));
        }
        return key;
    }
    // ucol_getSortKey wants UTF-16.
    std::vector<UChar> utf16;
    utf16.reserve(text.size());
    const int32_t size = int32_t(text.size());
    for (int32_t i = 0; i < size;) {
        UChar32 c;
        U8_NEXT(text.data(), i, size, c);
        if (c < 0)
            c = 0xFFFD;
        if (c > 0xFFFF) {
            utf16.push_back(UChar(U16_LEAD(c)));
            utf16.push_back(UChar(U16_TRAIL(c)));
        } else {
            utf16.push_back(UChar(c));
        }
    }
    std::string key(64, '\0');
    int32_t length = ucol_getSortKey(ucollator, utf16.data(), int32_t(utf16.size()),
                                     reinterpret_cast<uint8_t*>(key.data()), int32_t(key.size()));
    if (length > int32_t(key.size())) {
        key.resize(size_t(length));
        length = ucol_getSortKey(ucollator, utf16.data(), int32_t(utf16.size()),
                                 reinterpret_cast<uint8_t*>(key.data()), int32_t(key.size()));
    }
    // Without the terminating zero, so keys compare as std::string.
    key.resize(length > 0 ? size_t(length - 1) : 0);
    return key;
}

} // namespace Lingmo::base
