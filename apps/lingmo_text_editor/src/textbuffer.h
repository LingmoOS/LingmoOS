// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTBUFFER_H
#define TEXTBUFFER_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// A place in the text: a line and a UTF-8 byte offset into it, always on a
// character boundary.
struct TextPosition
{
    int line = 0;
    int column = 0;

    friend bool operator==(const TextPosition &a, const TextPosition &b)
    {
        return a.line == b.line && a.column == b.column;
    }
    friend bool operator!=(const TextPosition &a, const TextPosition &b) { return !(a == b); }
    friend bool operator<(const TextPosition &a, const TextPosition &b)
    {
        return a.line < b.line || (a.line == b.line && a.column < b.column);
    }
    friend bool operator<=(const TextPosition &a, const TextPosition &b) { return !(b < a); }
    friend bool operator>(const TextPosition &a, const TextPosition &b) { return b < a; }
    friend bool operator>=(const TextPosition &a, const TextPosition &b) { return !(a < b); }
};

struct TextRange
{
    TextPosition start;
    TextPosition end;

    bool isEmpty() const { return start == end; }
    TextRange normalized() const { return end < start ? TextRange{end, start} : *this; }
    friend bool operator==(const TextRange &a, const TextRange &b)
    {
        return a.start == b.start && a.end == b.end;
    }
};

// The text as a list of lines without their line breaks; there is always at
// least one line. Lines rather than one string keep an edit in the middle of
// a large file from moving the rest of it.
class TextBuffer
{
public:
    TextBuffer();

    void setText(std::string_view text);
    void setLines(std::vector<std::string> lines);

    int lineCount() const { return int(m_lines.size()); }
    const std::string &line(int index) const { return m_lines[size_t(index)]; }
    const std::vector<std::string> &lines() const { return m_lines; }

    std::string text() const;
    std::string text(const TextRange &range) const;
    // Bytes including one per line break.
    int64_t length() const { return m_length; }

    // Within the text and on a character boundary.
    TextPosition clamp(const TextPosition &position) const;
    TextPosition end() const;

    // Returns where the inserted text ends.
    TextPosition insert(const TextPosition &position, std::string_view text);
    void remove(const TextRange &range);

    int64_t offset(const TextPosition &position) const;
    TextPosition position(int64_t offset) const;

    // Where the given text ends when inserted at position.
    static TextPosition endOf(const TextPosition &position, std::string_view text);

private:
    std::vector<std::string> m_lines;
    int64_t m_length = 0;
};

#endif
