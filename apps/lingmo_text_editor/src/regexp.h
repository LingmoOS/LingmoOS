// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REGEXP_H
#define REGEXP_H

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// A regular expression on PCRE2, compiled in UTF mode with the JIT, on UTF-8
// subjects with byte offsets. Subjects are valid UTF-8.
// Matching reuses one match block, so one object is used from one thread.
class RegExp
{
public:
    enum Option : unsigned {
        NoOption = 0,
        CaseInsensitive = 1,
        // \w, \d, \s and \b by Unicode properties, not ASCII.
        UnicodeProperties = 2,
    };

    struct Match
    {
        // Byte offsets per group, -1 for a group that did not take part.
        std::vector<std::ptrdiff_t> starts;
        std::vector<std::ptrdiff_t> ends;

        bool hasMatch() const { return !starts.empty(); }
        std::ptrdiff_t start(int group = 0) const { return group < int(starts.size()) ? starts[group] : -1; }
        std::ptrdiff_t end(int group = 0) const { return group < int(ends.size()) ? ends[group] : -1; }
        std::ptrdiff_t length(int group = 0) const { return start(group) < 0 ? 0 : end(group) - start(group); }
        std::string_view captured(std::string_view subject, int group = 0) const;
    };

    RegExp() = default;
    explicit RegExp(std::string_view pattern, unsigned options = NoOption);

    bool isValid() const { return m_code != nullptr; }
    const std::string &pattern() const { return m_pattern; }
    // PCRE2's message for a pattern that does not compile.
    const std::string &errorString() const { return m_error; }
    int groupNumber(std::string_view name) const;

    // The first match at or after `offset`; `anchored` only at it, and
    // `notEmpty` passes over an empty match there.
    Match match(std::string_view subject, size_t offset = 0, bool anchored = false, bool notEmpty = false) const;
    // Every match from `offset` on: after an empty match it tries a
    // non-empty one at the same place, then moves on by a character.
    // Returning false stops.
    void forEachMatch(std::string_view subject, size_t offset, const std::function<bool(const Match &)> &callback) const;

    // The pattern that matches `text` literally.
    static std::string escape(std::string_view text);

private:
    struct Code;
    Match matchWith(std::string_view subject, size_t offset, unsigned options) const;

    std::string m_pattern;
    std::string m_error;
    std::shared_ptr<Code> m_code;
};

#endif
