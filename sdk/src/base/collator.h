// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "base/locale.h"

struct UCollator;

namespace Lingmo::base {

// Locale-aware string order through ICU, with options: numeric mode compares
// digit runs by value, so "Episode 2" sorts before "Episode 10";
// case-insensitive ignores case (ICU's secondary strength) and ignoring
// punctuation skips spaces and symbols. The C locale compares UTF-8 bytes,
// which orders by code point, case folded by ASCII when case-insensitive.
// Copies are cheap; a const collator may be used from several threads.
class Collator
{
public:
    // The locale's LC_COLLATE, the session's by default.
    explicit Collator(const Locale& locale = Locale::system());
    ~Collator();

    Collator(const Collator&);
    Collator& operator=(const Collator&);

    void setNumericMode(bool numeric);
    bool numericMode() const { return m_numeric; }
    void setCaseSensitive(bool sensitive);
    bool isCaseSensitive() const { return m_caseSensitive; }
    void setIgnorePunctuation(bool ignore);
    bool ignorePunctuation() const { return m_ignorePunctuation; }

    // < 0, 0 or > 0. An empty string sorts first.
    int compare(std::string_view a, std::string_view b) const;
    // For std::sort and ordered containers.
    bool operator()(std::string_view a, std::string_view b) const { return compare(a, b) < 0; }
    // A byte string whose plain order is compare()'s, for sorting many
    // strings or keeping keys.
    std::string sortKey(std::string_view text) const;

private:
    const UCollator* collator() const;

    std::string m_id;
    bool m_numeric = false;
    bool m_caseSensitive = true;
    bool m_ignorePunctuation = false;
    // Opened on first use with the options set then; shared by copies.
    mutable std::shared_ptr<UCollator> m_collator;
};

} // namespace Lingmo::base
