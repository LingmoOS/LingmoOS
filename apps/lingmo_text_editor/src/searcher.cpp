// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searcher.h"

static const TextRange notFound{{-1, -1}, {-1, -1}};

void Searcher::setPattern(const std::string &pattern, const Options &options)
{
    m_pattern = pattern;
    m_options = options;

    std::string source = options.regularExpression ? pattern : RegExp::escape(pattern);
    if (options.wholeWords)
        source = "(?<!\\w)(?:" + source + ")(?!\\w)";
    unsigned flags = RegExp::UnicodeProperties;
    if (!options.caseSensitive)
        flags |= RegExp::CaseInsensitive;
    m_expression = RegExp(source, flags);
}

std::string Searcher::errorString() const
{
    return m_expression.isValid() ? std::string() : m_expression.errorString();
}

std::vector<TextRange> Searcher::matchesInLine(const std::string &text, int line, int from, int to) const
{
    std::vector<TextRange> result;
    if (!isValid())
        return result;
    if (to < 0)
        to = int(text.size());

    // Plain text needs no regular expression.
    if (m_options.caseSensitive && !m_options.regularExpression && !m_options.wholeWords) {
        const int size = int(m_pattern.size());
        size_t at = text.find(m_pattern, size_t(from));
        while (at != std::string::npos && int(at) + size <= to) {
            result.push_back(TextRange{{line, int(at)}, {line, int(at) + size}});
            at = text.find(m_pattern, at + size_t(size));
        }
        return result;
    }

    // Plain text without case is matched from `from`; patterns from the
    // start of the line, so look-behinds see it.
    const bool plain = !m_options.regularExpression && !m_options.wholeWords;
    m_expression.forEachMatch(text, plain ? size_t(from) : 0, [&](const RegExp::Match &match) {
        const int start = int(match.start());
        const int end = int(match.end());
        if (end > to)
            return false;
        if (start < from || start == end)
            return true;
        result.push_back(TextRange{{line, start}, {line, end}});
        return true;
    });
    return result;
}

TextRange Searcher::find(const TextBuffer &buffer, const TextPosition &position, bool backwards,
                         const TextRange &scopeRange, bool *wrapped) const
{
    if (wrapped)
        *wrapped = false;
    if (!isValid())
        return notFound;

    const TextRange scope = scopeRange.isEmpty() ? TextRange{{0, 0}, buffer.end()} : scopeRange.normalized();
    TextPosition from = buffer.clamp(position);
    if (from < scope.start || from > scope.end)
        from = backwards ? scope.end : scope.start;

    auto lineMatches = [&](int line) {
        const std::string &text = buffer.line(line);
        const int lower = line == scope.start.line ? scope.start.column : 0;
        const int upper = line == scope.end.line ? scope.end.column : int(text.size());
        return matchesInLine(text, line, lower, upper);
    };

    if (!backwards) {
        for (int pass = 0; pass < 2; ++pass) {
            const int first = pass == 0 ? from.line : scope.start.line;
            const int last = pass == 0 ? scope.end.line : from.line;
            for (int line = first; line <= last; ++line) {
                for (const TextRange &match : lineMatches(line)) {
                    if (pass == 0 && match.start < from)
                        continue;
                    if (pass == 1 && match.start >= from)
                        return notFound;
                    if (wrapped)
                        *wrapped = pass == 1;
                    return match;
                }
            }
        }
        return notFound;
    }

    for (int pass = 0; pass < 2; ++pass) {
        const int first = pass == 0 ? from.line : scope.end.line;
        const int last = pass == 0 ? scope.start.line : from.line;
        for (int line = first; line >= last; --line) {
            const std::vector<TextRange> matches = lineMatches(line);
            for (auto it = matches.crbegin(); it != matches.crend(); ++it) {
                if (pass == 0 && it->end > from)
                    continue;
                if (pass == 1 && it->end <= from)
                    return notFound;
                if (wrapped)
                    *wrapped = pass == 1;
                return *it;
            }
        }
    }
    return notFound;
}

int Searcher::count(const TextBuffer &buffer, const TextRange &scopeRange, int limit) const
{
    if (!isValid())
        return 0;
    const TextRange scope = scopeRange.isEmpty() ? TextRange{{0, 0}, buffer.end()} : scopeRange.normalized();
    int total = 0;
    for (int line = scope.start.line; line <= scope.end.line && total < limit; ++line) {
        const std::string &text = buffer.line(line);
        const int lower = line == scope.start.line ? scope.start.column : 0;
        const int upper = line == scope.end.line ? scope.end.column : int(text.size());
        total += int(matchesInLine(text, line, lower, upper).size());
    }
    return std::min(total, limit);
}

std::string Searcher::replacement(const TextBuffer &buffer, const TextRange &range, const std::string &replace) const
{
    if (!m_options.regularExpression)
        return replace;

    const std::string &text = buffer.line(range.start.line);
    // Matches are never empty; one found past an empty match at its start is the non-empty one.
    const RegExp::Match match = m_expression.match(text, size_t(range.start.column), true, true);
    auto captured = [&](char digit) { return match.captured(text, digit - '0'); };
    std::string result;
    for (size_t i = 0; i < replace.size(); ++i) {
        const char c = replace[i];
        const char next = i + 1 < replace.size() ? replace[i + 1] : '\0';
        const bool digit = next >= '0' && next <= '9';
        if (c == '\\' && next != '\0') {
            ++i;
            if (digit)
                result += captured(next);
            else if (next == 'n')
                result += '\n';
            else if (next == 't')
                result += '\t';
            else
                result += next;
        } else if (c == '$' && digit) {
            ++i;
            result += captured(next);
        } else if (c == '$' && next == '$') {
            ++i;
            result += c;
        } else {
            result += c;
        }
    }
    return result;
}
