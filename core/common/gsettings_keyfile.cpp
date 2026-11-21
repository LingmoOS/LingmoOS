// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/gsettings_keyfile.h"

#include <cctype>
#include <cstdlib>

#include "base/file.h"
#include "base/paths.h"

namespace Lingmo::Core {

static std::string trimmed(std::string_view text)
{
    size_t begin = 0;
    size_t end = text.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(text[begin])))
        ++begin;
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])))
        --end;
    return std::string(text.substr(begin, end - begin));
}

static std::vector<std::string> readLines(const std::string& path)
{
    std::vector<std::string> lines;
    const std::optional<std::string> contents = Lingmo::Base::file::read(path);
    if (!contents)
        return lines;
    size_t start = 0;
    while (start <= contents->size()) {
        const size_t end = contents->find('\n', start);
        lines.push_back(contents->substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    while (!lines.empty() && lines.back().empty())
        lines.pop_back();
    return lines;
}

static bool isGroupHeader(const std::string& line)
{
    return !line.empty() && line[0] == '[';
}

namespace keyfile {

std::map<std::string, Group> read(const std::string& path)
{
    std::map<std::string, Group> groups;
    Group* current = nullptr;
    for (const std::string& rawLine : readLines(path)) {
        const std::string line = trimmed(rawLine);
        if (isGroupHeader(line)) {
            const size_t end = line.find(']');
            current = end != std::string::npos && end > 0 ? &groups[line.substr(1, end - 1)] : nullptr;
            continue;
        }
        if (!current || line.starts_with('#') || line.starts_with(';'))
            continue;

        const size_t eq = line.find('=');
        if (eq != std::string::npos && eq > 0)
            (*current)[trimmed(line.substr(0, eq))] = trimmed(line.substr(eq + 1));
    }
    return groups;
}

bool writeGroup(const std::string& path, const std::string& group, const Group& values)
{
    std::vector<std::string> lines = readLines(path);
    Group pending = values;
    const std::string header = "[" + group + "]";
    bool changed = false;
    long groupEnd = -1;
    bool inGroup = false;
    bool groupFound = false;
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string line = trimmed(lines[i]);
        if (isGroupHeader(line)) {
            if (inGroup) {
                groupEnd = long(i);
                break;
            }
            inGroup = line == header;
            groupFound = groupFound || inGroup;
            continue;
        }
        if (!inGroup)
            continue;

        const size_t eq = line.find('=');
        if (eq == std::string::npos || eq == 0)
            continue;
        const std::string key = trimmed(line.substr(0, eq));
        const auto found = pending.find(key);
        if (found == pending.end())
            continue;

        const std::string value = found->second;
        pending.erase(found);
        if (trimmed(line.substr(eq + 1)) != value) {
            lines[i] = key + "=" + value;
            changed = true;
        }
    }
    if (inGroup && groupEnd < 0)
        groupEnd = long(lines.size());

    std::vector<std::string> added;
    for (const auto& [key, value] : pending)
        added.push_back(key + "=" + value);

    if (!groupFound) {
        if (!lines.empty())
            lines.emplace_back();
        lines.push_back(header);
        lines.insert(lines.end(), added.begin(), added.end());
        changed = true;
    } else if (!added.empty()) {
        // Keep the blank separator before the next group after the inserted keys.
        long insertAt = groupEnd;
        while (insertAt > 0 && trimmed(lines[size_t(insertAt - 1)]).empty())
            --insertAt;
        lines.insert(lines.begin() + insertAt, added.begin(), added.end());
        changed = true;
    }

    if (!changed)
        return false;

    std::string contents;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i > 0)
            contents += '\n';
        contents += lines[i];
    }
    contents += '\n';
    // GLib and KConfig replace these files atomically too; a partial write would read as empty.
    return Lingmo::Base::file::writeAtomically(path, contents);
}

} // namespace keyfile

static void appendUtf8(std::string& out, char32_t c)
{
    if (c < 0x80) {
        out += char(c);
    } else if (c < 0x800) {
        out += char(0xc0 | (c >> 6));
        out += char(0x80 | (c & 0x3f));
    } else if (c < 0x10000) {
        out += char(0xe0 | (c >> 12));
        out += char(0x80 | ((c >> 6) & 0x3f));
        out += char(0x80 | (c & 0x3f));
    } else {
        out += char(0xf0 | (c >> 18));
        out += char(0x80 | ((c >> 12) & 0x3f));
        out += char(0x80 | ((c >> 6) & 0x3f));
        out += char(0x80 | (c & 0x3f));
    }
}

// Parses one quoted GVariant string starting at text[pos] and moves pos past it.
static bool parseString(const std::string& text, size_t& pos, std::string& result)
{
    if (pos >= text.size())
        return false;
    const char quote = text[pos];
    if (quote != '\'' && quote != '"')
        return false;

    result.clear();
    for (++pos; pos < text.size(); ++pos) {
        const char c = text[pos];
        if (c == quote) {
            ++pos;
            return true;
        }
        if (c != '\\') {
            result += c;
            continue;
        }
        if (++pos >= text.size())
            return false;

        const char escaped = text[pos];
        switch (escaped) {
        case 'n': result += '\n'; break;
        case 't': result += '\t'; break;
        case 'r': result += '\r'; break;
        case 'b': result += '\b'; break;
        case 'f': result += '\f'; break;
        case 'v': result += '\v'; break;
        case 'a': result += '\a'; break;
        case 'u':
        case 'U': {
            const size_t digits = escaped == 'u' ? 4 : 8;
            const std::string hex = text.substr(pos + 1, digits);
            char* end = nullptr;
            const unsigned long code = std::strtoul(hex.c_str(), &end, 16);
            if (hex.size() != digits || !end || *end != '\0')
                return false;
            appendUtf8(result, char32_t(code));
            pos += digits;
            break;
        }
        default:
            result += escaped;
        }
    }
    return false;
}

static void skipSpaces(const std::string& text, size_t& pos)
{
    while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos])))
        ++pos;
}

namespace gsettings {

std::string path()
{
    return Lingmo::Base::paths::configHome() + "/glib-2.0/settings/keyfile";
}

std::string toString(const std::string& text, const std::string& fallback)
{
    size_t pos = 0;
    std::string result;
    if (!parseString(text, pos, result))
        return fallback;
    return result;
}

int toInt(const std::string& text, int fallback)
{
    // Other writers may annotate the type, as in "uint32 8080".
    const size_t space = text.rfind(' ');
    const std::string number = space == std::string::npos ? text : text.substr(space + 1);
    char* end = nullptr;
    const long value = std::strtol(number.c_str(), &end, 10);
    return !number.empty() && end && *end == '\0' ? int(value) : fallback;
}

bool toBool(const std::string& text, bool fallback)
{
    if (text == "true")
        return true;
    if (text == "false")
        return false;
    return fallback;
}

std::vector<std::string> toStringList(const std::string& text, const std::vector<std::string>& fallback)
{
    std::string list = text;
    if (list.starts_with("@as"))
        list = trimmed(list.substr(3));
    if (!list.starts_with('['))
        return fallback;

    std::vector<std::string> result;
    size_t pos = 1;
    skipSpaces(list, pos);
    if (pos < list.size() && list[pos] == ']')
        return result;

    while (pos < list.size()) {
        std::string item;
        if (!parseString(list, pos, item))
            return fallback;
        result.push_back(item);

        skipSpaces(list, pos);
        if (pos >= list.size())
            return fallback;
        if (list[pos] == ']')
            return result;
        if (list[pos] != ',')
            return fallback;
        ++pos;
        skipSpaces(list, pos);
    }
    return fallback;
}

std::string stringValue(const std::string& value)
{
    std::string escaped = "'";
    for (const char c : value) {
        if (c == '\\')
            escaped += "\\\\";
        else if (c == '\'')
            escaped += "\\'";
        else if (c == '\n')
            escaped += "\\n";
        else
            escaped += c;
    }
    return escaped + "'";
}

std::string intValue(int value)
{
    return std::to_string(value);
}

std::string boolValue(bool value)
{
    return value ? "true" : "false";
}

std::string stringListValue(const std::vector<std::string>& values)
{
    std::string text = "[";
    for (const std::string& value : values) {
        if (text.size() > 1)
            text += ", ";
        text += stringValue(value);
    }
    return text + "]";
}

std::map<std::string, keyfile::Group> read()
{
    return keyfile::read(path());
}

bool writeGroup(const std::string& group, const std::map<std::string, Value>& values)
{
    keyfile::Group text;
    for (const auto& [key, value] : values) {
        if (const bool* flag = std::get_if<bool>(&value))
            text[key] = boolValue(*flag);
        else if (const int* number = std::get_if<int>(&value))
            text[key] = intValue(*number);
        else if (const std::string* string = std::get_if<std::string>(&value))
            text[key] = stringValue(*string);
        else
            text[key] = stringListValue(std::get<std::vector<std::string>>(value));
    }
    return keyfile::writeGroup(path(), group, text);
}

} // namespace gsettings

} // namespace Lingmo::Core
