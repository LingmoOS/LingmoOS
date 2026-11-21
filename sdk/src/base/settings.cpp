// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/settings.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstring>

#include "base/file.h"

namespace Lingmo::base {

static bool startsWith(std::string_view text, std::string_view prefix)
{
    return text.substr(0, prefix.size()) == prefix;
}

static bool endsWith(std::string_view text, std::string_view suffix)
{
    return text.size() >= suffix.size() && text.substr(text.size() - suffix.size()) == suffix;
}

static bool equalsIgnoringCase(std::string_view a, std::string_view b)
{
    return a.size() == b.size()
        && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) { return std::tolower(x) == std::tolower(y); });
}

static void appendUtf8(std::string& out, char32_t c)
{
    if (c < 0x80) {
        out += char(c);
    } else if (c < 0x800) {
        out += char(0xC0 | (c >> 6));
        out += char(0x80 | (c & 0x3F));
    } else if (c < 0x10000) {
        out += char(0xE0 | (c >> 12));
        out += char(0x80 | ((c >> 6) & 0x3F));
        out += char(0x80 | (c & 0x3F));
    } else {
        out += char(0xF0 | (c >> 18));
        out += char(0x80 | ((c >> 12) & 0x3F));
        out += char(0x80 | ((c >> 6) & 0x3F));
        out += char(0x80 | (c & 0x3F));
    }
}

// Decodes one code point; malformed bytes stand for themselves.
static char32_t decodeUtf8(std::string_view text, size_t& i)
{
    const auto byte = [&text](size_t at) { return static_cast<unsigned char>(text[at]); };
    const unsigned char first = byte(i);
    int length = first < 0x80 ? 1 : (first >> 5) == 6 ? 2 : (first >> 4) == 14 ? 3 : (first >> 3) == 30 ? 4 : 0;
    if (length == 0 || i + size_t(length) > text.size()) {
        ++i;
        return first;
    }
    char32_t c = length == 1 ? first : first & (0xFF >> (length + 1));
    for (int k = 1; k < length; ++k) {
        if ((byte(i + size_t(k)) & 0xC0) != 0x80) {
            ++i;
            return first;
        }
        c = (c << 6) | (byte(i + size_t(k)) & 0x3F);
    }
    i += size_t(length);
    return c;
}

// UTF-16 units, the units keys and values are escaped by.
static std::vector<char16_t> utf16(std::string_view text)
{
    std::vector<char16_t> units;
    for (size_t i = 0; i < text.size();) {
        const char32_t c = decodeUtf8(text, i);
        if (c > 0xFFFF) {
            units.push_back(char16_t(0xD800 + ((c - 0x10000) >> 10)));
            units.push_back(char16_t(0xDC00 + ((c - 0x10000) & 0x3FF)));
        } else {
            units.push_back(char16_t(c));
        }
    }
    return units;
}

static std::string fromUtf16(const std::u16string& units)
{
    std::string out;
    for (size_t i = 0; i < units.size(); ++i) {
        char32_t c = units[i];
        if (c >= 0xD800 && c < 0xDC00 && i + 1 < units.size() && units[i + 1] >= 0xDC00 && units[i + 1] < 0xE000)
            c = 0x10000 + ((c - 0xD800) << 10) + (units[++i] - 0xDC00);
        appendUtf8(out, c);
    }
    return out;
}

static const char kHexUpper[] = "0123456789ABCDEF";

// Letters, digits, '_', '-' and '.' stay; '/' becomes '\', other units up to
// 0xFF %XX and the rest %UXXXX.
static std::string escapeKey(std::string_view key)
{
    std::string result;
    for (const char16_t unit : utf16(key)) {
        if (unit == '/') {
            result += '\\';
        } else if ((unit >= 'a' && unit <= 'z') || (unit >= 'A' && unit <= 'Z') || (unit >= '0' && unit <= '9')
                   || unit == '_' || unit == '-' || unit == '.') {
            result += char(unit);
        } else if (unit <= 0xFF) {
            result += '%';
            result += kHexUpper[unit / 16];
            result += kHexUpper[unit % 16];
        } else {
            result += "%U";
            for (int shift = 12; shift >= 0; shift -= 4)
                result += kHexUpper[(unit >> shift) & 0xF];
        }
    }
    return result;
}

static int hexValue(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

// Reverses escapeKey().
static std::string unescapeKey(std::string_view key)
{
    std::u16string result;
    const std::vector<char16_t> units = utf16(key);
    for (size_t i = 0; i < units.size();) {
        const char16_t unit = units[i];
        if (unit == '\\') {
            result += u'/';
            ++i;
            continue;
        }
        if (unit != '%' || i == units.size() - 1) {
            result += unit;
            ++i;
            continue;
        }
        size_t first = i + 1;
        size_t digits = 2;
        if (units[i + 1] == 'U') {
            ++first;
            digits = 4;
        }
        if (first + digits > units.size()) {
            result += u'%';
            ++i;
            continue;
        }
        int value = 0;
        bool ok = true;
        for (size_t k = 0; k < digits; ++k) {
            const char16_t digit = units[first + k];
            const int v = digit < 0x80 ? hexValue(char(digit)) : -1;
            if (v < 0) {
                ok = false;
                break;
            }
            value = value * 16 + v;
        }
        if (!ok) {
            result += u'%';
            ++i;
            continue;
        }
        result += char16_t(value);
        i = first + digits;
    }
    return fromUtf16(result);
}

// C-style escapes, quoted when the text holds ';', ',' or '=', for text
// that is not @ByteArray or @Variant data: other characters above 0x1F go
// out as UTF-8.
static std::string escapeString(std::string_view text)
{
    std::string result;
    bool needsQuotes = false;
    bool escapeNextIfDigit = false;
    for (size_t i = 0; i < text.size();) {
        const size_t start = i;
        const char32_t c = decodeUtf8(text, i);
        if (c == ';' || c == ',' || c == '=')
            needsQuotes = true;
        if (escapeNextIfDigit && c < 0x80 && hexValue(char(c)) >= 0) {
            char buffer[8];
            const auto [end, error] = std::to_chars(buffer, buffer + sizeof(buffer), uint32_t(c), 16);
            result += "\\x";
            result.append(buffer, end);
            continue;
        }
        escapeNextIfDigit = false;
        switch (c) {
        case '\0':
            result += "\\0";
            escapeNextIfDigit = true;
            break;
        case '\a':
            result += "\\a";
            break;
        case '\b':
            result += "\\b";
            break;
        case '\f':
            result += "\\f";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        case '\v':
            result += "\\v";
            break;
        case '"':
            result += "\\\"";
            break;
        case '\\':
            result += "\\\\";
            break;
        default:
            if (c <= 0x1F) {
                char buffer[8];
                const auto [end, error] = std::to_chars(buffer, buffer + sizeof(buffer), uint32_t(c), 16);
                result += "\\x";
                result.append(buffer, end);
                escapeNextIfDigit = true;
            } else {
                result.append(text.substr(start, i - start));
            }
        }
    }
    if (needsQuotes || (!result.empty() && (result.front() == ' ' || result.back() == ' ')))
        result = '"' + result + '"';
    return result;
}

// Undoes escapeString(): the value as UTF-16 units, and
// whether commas made it a list.
static bool unescapeValue(std::string_view raw, std::u16string& string, std::vector<std::u16string>& list)
{
    static const char escapes[][2] = {{'a', '\a'}, {'b', '\b'}, {'f', '\f'}, {'n', '\n'}, {'r', '\r'}, {'t', '\t'},
                                      {'v', '\v'}, {'"', '"'},  {'?', '?'},  {'\'', '\''}, {'\\', '\\'}};

    bool isList = false;
    bool inQuotes = false;
    bool currentQuoted = false;
    size_t chopLimit = 0;
    size_t i = 0;

    const auto chopTrailingSpaces = [&string, &chopLimit] {
        while (string.size() > chopLimit && (string.back() == ' ' || string.back() == '\t'))
            string.pop_back();
    };
    const auto skipSpaces = [&] {
        while (i < raw.size() && (raw[i] == ' ' || raw[i] == '\t'))
            ++i;
        chopLimit = string.size();
    };

    skipSpaces();
    while (i < raw.size()) {
        const char c = raw[i];
        if (c == '\\') {
            ++i;
            if (i >= raw.size())
                break;
            const char escape = raw[i++];
            bool simple = false;
            for (const auto& pair : escapes) {
                if (escape == pair[0]) {
                    string += char16_t(pair[1]);
                    simple = true;
                    break;
                }
            }
            if (simple)
                continue;
            if (escape == 'x' || (escape >= '0' && escape <= '7')) {
                const bool hex = escape == 'x';
                char16_t value = hex ? 0 : char16_t(escape - '0');
                if (hex && (i >= raw.size() || hexValue(raw[i]) < 0))
                    continue;
                while (i < raw.size()) {
                    const int digit = hex ? hexValue(raw[i]) : (raw[i] >= '0' && raw[i] <= '7' ? raw[i] - '0' : -1);
                    if (digit < 0)
                        break;
                    value = char16_t(value * (hex ? 16 : 8) + digit);
                    ++i;
                }
                string += value;
            } else if (escape == '\n' || escape == '\r') {
                if (i < raw.size() && (raw[i] == '\n' || raw[i] == '\r') && raw[i] != escape)
                    ++i;
            }
            chopLimit = string.size();
            continue;
        }
        if (c == '"') {
            ++i;
            currentQuoted = true;
            inQuotes = !inQuotes;
            if (!inQuotes)
                skipSpaces();
            continue;
        }
        if (c == ',' && !inQuotes) {
            if (!currentQuoted)
                chopTrailingSpaces();
            if (!isList) {
                isList = true;
                list.clear();
            }
            list.push_back(string);
            string.clear();
            currentQuoted = false;
            ++i;
            skipSpaces();
            continue;
        }
        size_t j = i + 1;
        while (j < raw.size() && raw[j] != '\\' && raw[j] != '"' && raw[j] != ',')
            ++j;
        for (const char16_t unit : utf16(raw.substr(i, j - i)))
            string += unit;
        i = j;
    }
    if (!currentQuoted)
        chopTrailingSpaces();
    if (isList)
        list.push_back(string);
    return isList;
}

namespace {

// A raw value: a string, a comma-separated list, or an @-prefixed form.
struct Value
{
    enum class Kind {
        Missing,
        Invalid,
        String,
        List,
        Size,
        Point,
        Rect,
        Other,
    };

    Kind kind = Kind::Missing;
    std::string string;
    std::vector<std::string> list;
    int numbers[4] = {};
};

} // namespace

static bool parseNumbers(std::string_view text, int* numbers, int count)
{
    size_t i = 0;
    for (int k = 0; k < count; ++k) {
        while (i < text.size() && text[i] == ' ')
            ++i;
        const auto [end, error] = std::from_chars(text.data() + i, text.data() + text.size(), numbers[k]);
        if (error != std::errc())
            return false;
        i = size_t(end - text.data());
    }
    return true;
}

static Value parseString(const std::u16string& units)
{
    Value value;
    const std::string text = fromUtf16(units);
    if (!startsWith(text, "@")) {
        value.kind = Value::Kind::String;
        value.string = text;
        return value;
    }
    if (endsWith(text, ")")) {
        const auto inner = [&text](size_t prefix) {
            return std::string_view(text).substr(prefix, text.size() - prefix - 1);
        };
        if (startsWith(text, "@ByteArray(")) {
            // Latin-1 bytes.
            value.kind = Value::Kind::String;
            for (const char16_t unit : std::u16string_view(units).substr(11, units.size() - 12))
                value.string += char(unit);
            return value;
        }
        if (startsWith(text, "@String(")) {
            value.kind = Value::Kind::String;
            value.string = std::string(inner(8));
            return value;
        }
        if (text == "@Invalid()") {
            value.kind = Value::Kind::Invalid;
            return value;
        }
        if (startsWith(text, "@Size(") && parseNumbers(inner(6), value.numbers, 2)) {
            value.kind = Value::Kind::Size;
            return value;
        }
        if (startsWith(text, "@Point(") && parseNumbers(inner(7), value.numbers, 2)) {
            value.kind = Value::Kind::Point;
            return value;
        }
        if (startsWith(text, "@Rect(") && parseNumbers(inner(6), value.numbers, 4)) {
            value.kind = Value::Kind::Rect;
            return value;
        }
        if (startsWith(text, "@Variant(") || startsWith(text, "@DateTime(")) {
            value.kind = Value::Kind::Other;
            return value;
        }
    }
    value.kind = Value::Kind::String;
    value.string = startsWith(text, "@@") ? text.substr(1) : text;
    return value;
}

static Value parseRaw(const std::optional<std::string>& raw)
{
    if (!raw)
        return {};
    std::u16string string;
    std::vector<std::u16string> list;
    if (!unescapeValue(*raw, string, list))
        return parseString(string);

    Value value;
    value.kind = Value::Kind::List;
    for (const std::u16string& item : list) {
        const Value element = parseString(item);
        value.list.push_back(element.kind == Value::Kind::String ? element.string : fromUtf16(item));
    }
    return value;
}

static std::string trimmed(std::string_view text)
{
    const size_t start = text.find_first_not_of(" \t");
    if (start == std::string_view::npos)
        return {};
    const size_t end = text.find_last_not_of(" \t");
    return std::string(text.substr(start, end - start + 1));
}

static void parseFile(std::string_view contents, std::map<std::string, std::string>& values)
{
    std::string section;
    size_t position = 0;
    while (position < contents.size()) {
        size_t end = contents.find('\n', position);
        if (end == std::string_view::npos)
            end = contents.size();
        std::string line(contents.substr(position, end - position));
        position = end + 1;
        // A value continues on the next line after an escaped line break.
        while (!line.empty()) {
            const size_t end = line.size() - (line.back() == '\r' ? 1 : 0);
            size_t backslashes = 0;
            while (backslashes < end && line[end - 1 - backslashes] == '\\')
                ++backslashes;
            if (backslashes % 2 == 0 || position >= contents.size())
                break;
            size_t next = contents.find('\n', position);
            if (next == std::string_view::npos)
                next = contents.size();
            line += '\n';
            line.append(contents.substr(position, next - position));
            position = next + 1;
        }
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        const size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos || line[start] == ';' || line[start] == '#')
            continue;

        if (line[start] == '[') {
            const size_t close = line.find(']', start);
            const std::string name = trimmed(std::string_view(line).substr(
                start + 1, close == std::string::npos ? std::string::npos : close - start - 1));
            if (equalsIgnoringCase(name, "general"))
                section.clear();
            else if (equalsIgnoringCase(name, "%general"))
                section = name.substr(1) + "/";
            else
                section = unescapeKey(name) + "/";
            continue;
        }

        const size_t equals = line.find('=', start);
        if (equals == std::string::npos)
            continue;
        const std::string key = section + unescapeKey(trimmed(std::string_view(line).substr(start, equals - start)));
        values[key] = line.substr(equals + 1);
    }
}

static std::string sectionOf(const std::string& key)
{
    const size_t slash = key.find('/');
    return slash == std::string::npos ? std::string() : key.substr(0, slash);
}

// [General] first, then the groups and the keys in each sorted.
static std::string writeFile(const std::map<std::string, std::string>& values)
{
    std::map<std::string, std::vector<std::pair<std::string, const std::string*>>> sections;
    for (const auto& [key, raw] : values) {
        const std::string section = sectionOf(key);
        sections[section].emplace_back(section.empty() ? key : key.substr(section.size() + 1), &raw);
    }

    std::string contents;
    for (auto& [section, keys] : sections) {
        if (!contents.empty())
            contents += '\n';
        const std::string escaped = escapeKey(section);
        if (escaped.empty())
            contents += "[General]\n";
        else if (equalsIgnoringCase(escaped, "general"))
            contents += "[%General]\n";
        else
            contents += "[" + escaped + "]\n";

        std::sort(keys.begin(), keys.end());
        for (const auto& [name, raw] : keys)
            contents += escapeKey(name) + "=" + *raw + "\n";
    }
    return contents;
}

Settings::Settings(std::string path)
    : m_path(std::move(path))
{
    reload();
}

Settings::~Settings()
{
    sync();
}

void Settings::reload()
{
    m_values.clear();
    if (const std::optional<std::string> contents = file::read(m_path))
        parseFile(*contents, m_values);
    for (const auto& [key, raw] : m_changes)
        apply(key, raw);
}

bool Settings::sync()
{
    if (m_changes.empty())
        return true;
    // Another program may have written the file since.
    auto changes = std::move(m_changes);
    m_changes.clear();
    m_values.clear();
    if (const std::optional<std::string> contents = file::read(m_path))
        parseFile(*contents, m_values);
    for (const auto& [key, raw] : changes)
        apply(key, raw);
    if (!file::writeAtomically(m_path, writeFile(m_values))) {
        m_changes = std::move(changes);
        return false;
    }
    return true;
}

void Settings::apply(const std::string& key, const std::optional<std::string>& raw)
{
    if (raw)
        m_values[key] = *raw;
    else
        m_values.erase(key);
}

void Settings::setRaw(const std::string& key, std::string raw)
{
    m_changes[key] = raw;
    apply(key, raw);
}

std::optional<std::string> Settings::rawValue(const std::string& key) const
{
    const auto it = m_values.find(key);
    return it == m_values.end() ? std::nullopt : std::optional<std::string>(it->second);
}

bool Settings::contains(const std::string& key) const
{
    return m_values.contains(key);
}

std::vector<std::string> Settings::childKeys(const std::string& group) const
{
    const std::string prefix = group.empty() ? std::string() : group + "/";
    std::vector<std::string> keys;
    for (const auto& [key, raw] : m_values) {
        if (startsWith(key, prefix) && key.find('/', prefix.size()) == std::string::npos)
            keys.push_back(key.substr(prefix.size()));
    }
    return keys;
}

std::vector<std::string> Settings::childGroups(const std::string& group) const
{
    const std::string prefix = group.empty() ? std::string() : group + "/";
    std::vector<std::string> groups;
    for (const auto& [key, raw] : m_values) {
        if (!startsWith(key, prefix))
            continue;
        const size_t slash = key.find('/', prefix.size());
        if (slash == std::string::npos)
            continue;
        std::string name = key.substr(prefix.size(), slash - prefix.size());
        if (std::find(groups.begin(), groups.end(), name) == groups.end())
            groups.push_back(std::move(name));
    }
    return groups;
}

std::string Settings::stringValue(const std::string& key, const std::string& fallback) const
{
    const Value value = parseRaw(rawValue(key));
    if (value.kind == Value::Kind::String)
        return value.string;
    if (value.kind == Value::Kind::List && value.list.size() == 1)
        return value.list.front();
    return fallback;
}

bool Settings::boolValue(const std::string& key, bool fallback) const
{
    const Value value = parseRaw(rawValue(key));
    if (value.kind != Value::Kind::String)
        return fallback;
    // Any text but empty, "0" and "false" is true.
    return !(value.string.empty() || value.string == "0" || equalsIgnoringCase(value.string, "false"));
}

int64_t Settings::intValue(const std::string& key, int64_t fallback) const
{
    const Value value = parseRaw(rawValue(key));
    if (value.kind != Value::Kind::String)
        return fallback;
    const std::string text = trimmed(value.string);
    int64_t number = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), number);
    return error == std::errc() && end == text.data() + text.size() ? number : fallback;
}

double Settings::doubleValue(const std::string& key, double fallback) const
{
    const Value value = parseRaw(rawValue(key));
    if (value.kind != Value::Kind::String)
        return fallback;
    const std::string text = trimmed(value.string);
    double number = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), number);
    return error == std::errc() && end == text.data() + text.size() ? number : fallback;
}

std::vector<std::string> Settings::stringListValue(const std::string& key,
                                                   const std::vector<std::string>& fallback) const
{
    const Value value = parseRaw(rawValue(key));
    switch (value.kind) {
    case Value::Kind::List:
        return value.list;
    case Value::Kind::String:
        return {value.string};
    case Value::Kind::Invalid:
        return {};
    default:
        return fallback;
    }
}

std::optional<Settings::Size> Settings::sizeValue(const std::string& key) const
{
    const Value value = parseRaw(rawValue(key));
    if (value.kind != Value::Kind::Size)
        return std::nullopt;
    return Size{value.numbers[0], value.numbers[1]};
}

std::optional<Settings::Point> Settings::pointValue(const std::string& key) const
{
    const Value value = parseRaw(rawValue(key));
    if (value.kind != Value::Kind::Point)
        return std::nullopt;
    return Point{value.numbers[0], value.numbers[1]};
}

std::optional<Settings::Rect> Settings::rectValue(const std::string& key) const
{
    const Value value = parseRaw(rawValue(key));
    if (value.kind != Value::Kind::Rect)
        return std::nullopt;
    return Rect{value.numbers[0], value.numbers[1], value.numbers[2], value.numbers[3]};
}

// A string value: a leading @ is doubled so it reads back as text.
static std::string escapeTextValue(std::string_view value)
{
    return escapeString(startsWith(value, "@") ? "@" + std::string(value) : std::string(value));
}

void Settings::setValue(const std::string& key, std::string_view value)
{
    setRaw(key, escapeTextValue(value));
}

void Settings::setValue(const std::string& key, const std::vector<std::string>& value)
{
    // An empty list and a list of one empty string differ only this way.
    if (value.empty()) {
        setRaw(key, "@Invalid()");
        return;
    }
    std::string raw;
    for (size_t i = 0; i < value.size(); ++i) {
        if (i != 0)
            raw += ", ";
        raw += escapeTextValue(value[i]);
    }
    setRaw(key, raw);
}

void Settings::setValue(const std::string& key, const Size& value)
{
    setRaw(key, "@Size(" + std::to_string(value.width) + " " + std::to_string(value.height) + ")");
}

void Settings::setValue(const std::string& key, const Point& value)
{
    setRaw(key, "@Point(" + std::to_string(value.x) + " " + std::to_string(value.y) + ")");
}

void Settings::setValue(const std::string& key, const Rect& value)
{
    setRaw(key, "@Rect(" + std::to_string(value.x) + " " + std::to_string(value.y) + " "
                    + std::to_string(value.width) + " " + std::to_string(value.height) + ")");
}

void Settings::remove(const std::string& key)
{
    if (key.empty())
        return;
    const std::string prefix = key + "/";
    std::vector<std::string> keys;
    for (const auto& [existing, raw] : m_values) {
        if (existing == key || startsWith(existing, prefix))
            keys.push_back(existing);
    }
    if (keys.empty())
        keys.push_back(key);
    for (const std::string& removed : keys) {
        m_changes[removed] = std::nullopt;
        apply(removed, std::nullopt);
    }
}

std::string Settings::formatDouble(double value)
{
    char buffer[64];
    const auto [end, error] = std::to_chars(buffer, buffer + sizeof(buffer), value);
    return std::string(buffer, end);
}

} // namespace Lingmo::base
