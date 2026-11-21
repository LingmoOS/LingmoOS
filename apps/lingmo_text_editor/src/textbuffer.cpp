// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textbuffer.h"
#include "unicode.h"

#include <algorithm>

TextBuffer::TextBuffer()
    : m_lines(1)
{
}

static std::vector<std::string> splitAtNewlines(std::string_view text)
{
    std::vector<std::string> lines;
    size_t start = 0;
    while (true) {
        const size_t newline = text.find('\n', start);
        if (newline == std::string_view::npos) {
            lines.emplace_back(text.substr(start));
            return lines;
        }
        lines.emplace_back(text.substr(start, newline - start));
        start = newline + 1;
    }
}

void TextBuffer::setText(std::string_view text)
{
    setLines(splitAtNewlines(text));
}

void TextBuffer::setLines(std::vector<std::string> lines)
{
    if (lines.empty())
        lines.emplace_back();
    m_lines = std::move(lines);
    m_length = int64_t(m_lines.size()) - 1;
    for (const std::string &line : m_lines)
        m_length += int64_t(line.size());
}

std::string TextBuffer::text() const
{
    std::string result;
    result.reserve(size_t(m_length));
    for (size_t i = 0; i < m_lines.size(); ++i) {
        if (i > 0)
            result += '\n';
        result += m_lines[i];
    }
    return result;
}

std::string TextBuffer::text(const TextRange &range) const
{
    const TextRange r{clamp(range.normalized().start), clamp(range.normalized().end)};
    if (r.start.line == r.end.line)
        return line(r.start.line).substr(size_t(r.start.column), size_t(r.end.column - r.start.column));

    std::string result = line(r.start.line).substr(size_t(r.start.column));
    for (int i = r.start.line + 1; i < r.end.line; ++i) {
        result += '\n';
        result += line(i);
    }
    result += '\n';
    result.append(line(r.end.line), 0, size_t(r.end.column));
    return result;
}

TextPosition TextBuffer::clamp(const TextPosition &position) const
{
    if (position.line < 0)
        return {0, 0};
    if (position.line >= lineCount())
        return end();
    const std::string &text = line(position.line);
    const size_t column = std::clamp<int64_t>(position.column, 0, int64_t(text.size()));
    return {position.line, int(Unicode::boundary(text, column))};
}

TextPosition TextBuffer::end() const
{
    return {lineCount() - 1, int(m_lines.back().size())};
}

TextPosition TextBuffer::endOf(const TextPosition &position, std::string_view text)
{
    const size_t last = text.rfind('\n');
    if (last == std::string_view::npos)
        return {position.line, position.column + int(text.size())};
    const int breaks = int(std::count(text.begin(), text.end(), '\n'));
    return {position.line + breaks, int(text.size() - last - 1)};
}

TextPosition TextBuffer::insert(const TextPosition &at, std::string_view text)
{
    const TextPosition position = clamp(at);
    if (text.empty())
        return position;

    m_length += int64_t(text.size());
    std::string &first = m_lines[size_t(position.line)];
    if (text.find('\n') == std::string_view::npos) {
        first.insert(size_t(position.column), text);
        return {position.line, position.column + int(text.size())};
    }

    std::vector<std::string> pieces = splitAtNewlines(text);
    std::string tail = first.substr(size_t(position.column));
    first.resize(size_t(position.column));
    first += pieces.front();
    const TextPosition result{position.line + int(pieces.size()) - 1, int(pieces.back().size())};
    pieces.back() += tail;
    m_lines.insert(m_lines.begin() + position.line + 1, std::make_move_iterator(pieces.begin() + 1),
                   std::make_move_iterator(pieces.end()));
    return result;
}

void TextBuffer::remove(const TextRange &range)
{
    const TextPosition start = clamp(range.normalized().start);
    const TextPosition end = clamp(range.normalized().end);
    if (start == end)
        return;

    if (start.line == end.line) {
        m_lines[size_t(start.line)].erase(size_t(start.column), size_t(end.column - start.column));
        m_length -= end.column - start.column;
        return;
    }

    int64_t removed = int64_t(line(start.line).size()) - start.column + end.column + (end.line - start.line);
    for (int i = start.line + 1; i < end.line; ++i)
        removed += int64_t(line(i).size());
    m_length -= removed;

    std::string &first = m_lines[size_t(start.line)];
    first.resize(size_t(start.column));
    first.append(line(end.line), size_t(end.column));
    m_lines.erase(m_lines.begin() + start.line + 1, m_lines.begin() + end.line + 1);
}

int64_t TextBuffer::offset(const TextPosition &position) const
{
    const TextPosition p = clamp(position);
    int64_t result = p.column;
    for (int i = 0; i < p.line; ++i)
        result += int64_t(line(i).size()) + 1;
    return result;
}

TextPosition TextBuffer::position(int64_t offset) const
{
    if (offset <= 0)
        return {0, 0};
    for (int i = 0; i < lineCount(); ++i) {
        const int64_t size = int64_t(line(i).size());
        if (offset <= size)
            return clamp({i, int(offset)});
        offset -= size + 1;
    }
    return end();
}
