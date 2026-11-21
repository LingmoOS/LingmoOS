// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REGEXP_H
#define REGEXP_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Konsole
{

// QRegularExpression on UTF-16 text: the same PCRE2 options and matching,
// so offsets and matches are what the Qt build found.
class RegExp
{
public:
    enum Option
    {
        NoOption = 0,
        CaseInsensitive = 1,
    };

    struct Match
    {
        bool hasMatch = false;
        // Start and end of each group, -1 for a group that took no part.
        std::vector<long> offsets;

        int lastCapturedIndex() const { return int(offsets.size() / 2) - 1; }
        long capturedStart(int group = 0) const { return group <= lastCapturedIndex() ? offsets[group * 2] : -1; }
        long capturedEnd(int group = 0) const { return group <= lastCapturedIndex() ? offsets[group * 2 + 1] : -1; }
        long capturedLength(int group = 0) const { return capturedEnd(group) - capturedStart(group); }
        std::u16string captured(const std::u16string& subject, int group = 0) const;
    };

    RegExp();
    explicit RegExp(const std::u16string& pattern, int options = NoOption);
    ~RegExp();

    RegExp(const RegExp& other);
    RegExp& operator=(const RegExp& other);

    bool isValid() const;
    const std::u16string& pattern() const { return m_pattern; }
    int options() const { return m_options; }

    // QRegularExpression::match() at `offset`; anchored is AnchorAtOffsetMatchOption.
    Match match(const std::u16string& subject, long offset = 0, bool anchored = false) const;

    // The match after `previous` in the same subject, as
    // QRegularExpressionMatchIterator::next() finds it.
    Match nextMatch(const std::u16string& subject, const Match& previous) const;

    // QString::lastIndexOf(regExp, from, &match): the last match starting
    // at or before `from`, found by matching through the subject.
    long lastIndexIn(const std::u16string& subject, long from, Match* match) const;

    // QRegularExpression::escape().
    static std::u16string escape(const std::u16string& text);

private:
    struct Compiled;

    Match doMatch(const std::u16string& subject, long offset, uint32_t flags) const;

    std::u16string m_pattern;
    int m_options = NoOption;
    std::shared_ptr<Compiled> m_compiled;
};

}

#endif // REGEXP_H
