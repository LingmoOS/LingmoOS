// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHER_H
#define SEARCHER_H

#include "regexp.h"
#include "textbuffer.h"

// Finds a pattern line by line: plain text, whole words or a regular
// expression. A match never spans a line break.
class Searcher
{
public:
    struct Options
    {
        bool caseSensitive = false;
        bool wholeWords = false;
        bool regularExpression = false;
    };

    void setPattern(const std::string &pattern, const Options &options);
    const std::string &pattern() const { return m_pattern; }
    const Options &options() const { return m_options; }
    bool isEmpty() const { return m_pattern.empty(); }
    bool isValid() const { return !m_pattern.empty() && m_expression.isValid(); }
    std::string errorString() const;

    // Matches within one line, in order, limited to [from, to).
    std::vector<TextRange> matchesInLine(const std::string &text, int line, int from = 0, int to = -1) const;

    // The next match at or after `from` (forwards) or ending at or before it
    // (backwards), inside `scope` when it is not empty. Wraps around the
    // scope; `wrapped` tells whether it did. Returns an empty range at {-1,-1}
    // when there is none.
    TextRange find(const TextBuffer &buffer, const TextPosition &from, bool backwards,
                   const TextRange &scope = {}, bool *wrapped = nullptr) const;

    // Matches in the whole buffer or the scope; stops counting at limit.
    int count(const TextBuffer &buffer, const TextRange &scope = {}, int limit = 100000) const;

    // What the match at `range` is replaced with: captures expand for
    // regular expressions (\1 or $1, \n and \t too).
    std::string replacement(const TextBuffer &buffer, const TextRange &range, const std::string &replace) const;

    static bool isNotFound(const TextRange &range) { return range.start.line < 0; }

private:
    std::string m_pattern;
    Options m_options;
    RegExp m_expression;
};

#endif
