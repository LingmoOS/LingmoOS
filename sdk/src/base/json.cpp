// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/json.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <limits>

namespace Lingmo::base::json {

// Deeper nesting fails to parse.
static constexpr int kMaximumDepth = 1024;

namespace {

class Parser
{
public:
    explicit Parser(std::string_view text) : m_text(text) {}

    std::optional<Value> parseDocument()
    {
        std::optional<Value> value = parseValue(0);
        skipSpace();
        if (!value || m_position != m_text.size())
            return std::nullopt;
        return value;
    }

private:
    void skipSpace()
    {
        while (m_position < m_text.size()
               && (m_text[m_position] == ' ' || m_text[m_position] == '\t' || m_text[m_position] == '\n'
                   || m_text[m_position] == '\r'))
            ++m_position;
    }

    bool consume(std::string_view word)
    {
        if (m_text.substr(m_position, word.size()) != word)
            return false;
        m_position += word.size();
        return true;
    }

    std::optional<Value> parseValue(int depth)
    {
        if (depth > kMaximumDepth)
            return std::nullopt;
        skipSpace();
        if (m_position >= m_text.size())
            return std::nullopt;
        const char c = m_text[m_position];
        if (c == '{')
            return parseObject(depth);
        if (c == '[')
            return parseArray(depth);
        if (c == '"') {
            std::optional<std::string> text = parseString();
            return text ? std::optional<Value>(Value(std::move(*text))) : std::nullopt;
        }
        if (consume("true"))
            return Value(true);
        if (consume("false"))
            return Value(false);
        if (consume("null"))
            return Value();
        return parseNumber();
    }

    std::optional<Value> parseObject(int depth)
    {
        ++m_position;
        Value object = Value::object();
        skipSpace();
        if (consume("}"))
            return object;
        while (true) {
            skipSpace();
            if (m_position >= m_text.size() || m_text[m_position] != '"')
                return std::nullopt;
            std::optional<std::string> key = parseString();
            skipSpace();
            if (!key || !consume(":"))
                return std::nullopt;
            std::optional<Value> value = parseValue(depth + 1);
            if (!value)
                return std::nullopt;
            object.insert(std::move(*key), std::move(*value));
            skipSpace();
            if (consume("}"))
                return object;
            if (!consume(","))
                return std::nullopt;
        }
    }

    std::optional<Value> parseArray(int depth)
    {
        ++m_position;
        Value::Array array;
        skipSpace();
        if (consume("]"))
            return Value(std::move(array));
        while (true) {
            std::optional<Value> value = parseValue(depth + 1);
            if (!value)
                return std::nullopt;
            array.push_back(std::move(*value));
            skipSpace();
            if (consume("]"))
                return Value(std::move(array));
            if (!consume(","))
                return std::nullopt;
        }
    }

    std::optional<uint32_t> parseHex4()
    {
        if (m_position + 4 > m_text.size())
            return std::nullopt;
        uint32_t value = 0;
        for (int i = 0; i < 4; ++i) {
            const char c = m_text[m_position++];
            value <<= 4;
            if (c >= '0' && c <= '9')
                value |= uint32_t(c - '0');
            else if (c >= 'a' && c <= 'f')
                value |= uint32_t(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F')
                value |= uint32_t(c - 'A' + 10);
            else
                return std::nullopt;
        }
        return value;
    }

    static void appendUtf8(std::string& out, uint32_t c)
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

    std::optional<std::string> parseString()
    {
        ++m_position;
        std::string text;
        while (m_position < m_text.size()) {
            const char c = m_text[m_position++];
            if (c == '"')
                return text;
            if (static_cast<unsigned char>(c) < 0x20)
                return std::nullopt;
            if (c != '\\') {
                text += c;
                continue;
            }
            if (m_position >= m_text.size())
                return std::nullopt;
            const char escape = m_text[m_position++];
            switch (escape) {
            case '"':
            case '\\':
            case '/':
                text += escape;
                break;
            case 'b':
                text += '\b';
                break;
            case 'f':
                text += '\f';
                break;
            case 'n':
                text += '\n';
                break;
            case 'r':
                text += '\r';
                break;
            case 't':
                text += '\t';
                break;
            case 'u': {
                std::optional<uint32_t> unit = parseHex4();
                if (!unit)
                    return std::nullopt;
                uint32_t code = *unit;
                if (code >= 0xd800 && code < 0xdc00) {
                    if (!consume("\\u"))
                        return std::nullopt;
                    std::optional<uint32_t> low = parseHex4();
                    if (!low || *low < 0xdc00 || *low >= 0xe000)
                        return std::nullopt;
                    code = 0x10000 + ((code - 0xd800) << 10) + (*low - 0xdc00);
                } else if (code >= 0xdc00 && code < 0xe000) {
                    return std::nullopt;
                }
                appendUtf8(text, code);
                break;
            }
            default:
                return std::nullopt;
            }
        }
        return std::nullopt;
    }

    std::optional<Value> parseNumber()
    {
        const size_t start = m_position;
        if (m_position < m_text.size() && m_text[m_position] == '-')
            ++m_position;
        const auto digits = [this] {
            const size_t from = m_position;
            while (m_position < m_text.size() && m_text[m_position] >= '0' && m_text[m_position] <= '9')
                ++m_position;
            return m_position - from;
        };
        const size_t integerStart = m_position;
        const size_t integerDigits = digits();
        if (integerDigits == 0 || (integerDigits > 1 && m_text[integerStart] == '0'))
            return std::nullopt;
        if (m_position < m_text.size() && m_text[m_position] == '.') {
            ++m_position;
            if (digits() == 0)
                return std::nullopt;
        }
        if (m_position < m_text.size() && (m_text[m_position] == 'e' || m_text[m_position] == 'E')) {
            ++m_position;
            if (m_position < m_text.size() && (m_text[m_position] == '+' || m_text[m_position] == '-'))
                ++m_position;
            if (digits() == 0)
                return std::nullopt;
        }
        double value = 0;
        const char* first = m_text.data() + start;
        const char* last = m_text.data() + m_position;
        const std::from_chars_result result = std::from_chars(first, last, value);
        if (result.ec == std::errc::result_out_of_range)
            return Value(outOfRange(std::string_view(first, size_t(last - first))));
        if (result.ec != std::errc())
            return std::nullopt;
        return Value(value);
    }

    // A number too large or too small for a double: infinite or zero with
    // its sign, as other readers take it.
    static double outOfRange(std::string_view number)
    {
        const bool negative = number.front() == '-';
        const auto isDigit = [](char c) { return c >= '0' && c <= '9'; };
        // The power of ten of the first digit that is not zero.
        int64_t order = 0;
        bool significant = false;
        size_t i = negative ? 1 : 0;
        const size_t integerStart = i;
        while (i < number.size() && isDigit(number[i]))
            ++i;
        for (size_t k = integerStart; k < i && !significant; ++k) {
            if (number[k] != '0') {
                order = int64_t(i - k) - 1;
                significant = true;
            }
        }
        if (i < number.size() && number[i] == '.') {
            const size_t point = i++;
            for (; i < number.size() && isDigit(number[i]); ++i) {
                if (!significant && number[i] != '0') {
                    order = -int64_t(i - point);
                    significant = true;
                }
            }
        }
        int64_t exponent = 0;
        if (i < number.size()) {
            ++i;
            const bool negativeExponent = number[i] == '-';
            if (number[i] == '+' || number[i] == '-')
                ++i;
            for (; i < number.size(); ++i)
                exponent = std::min<int64_t>(exponent * 10 + (number[i] - '0'), 1000000);
            if (negativeExponent)
                exponent = -exponent;
        }
        const double magnitude = significant && order + exponent > 0 ? HUGE_VAL : 0.0;
        return negative ? -magnitude : magnitude;
    }

    std::string_view m_text;
    size_t m_position = 0;
};

} // namespace

static const Value& nullValue()
{
    static const Value null;
    return null;
}

Value Value::object(Object members)
{
    Value value;
    value.m_type = Type::Object;
    value.m_object = std::move(members);
    return value;
}

std::optional<Value> Value::parse(std::string_view text)
{
    return Parser(text).parseDocument();
}

int64_t Value::toInt(int64_t fallback) const
{
    if (!isNumber() || !std::isfinite(m_number))
        return fallback;
    // Beyond the range, where llround() has no defined result.
    if (m_number >= 9223372036854775807.0)
        return std::numeric_limits<int64_t>::max();
    if (m_number <= -9223372036854775808.0)
        return std::numeric_limits<int64_t>::min();
    return int64_t(std::llround(m_number));
}

const Value::Array& Value::toArray() const
{
    static const Array empty;
    return isArray() ? m_array : empty;
}

const Value::Object& Value::toObject() const
{
    static const Object empty;
    return isObject() ? m_object : empty;
}

const Value& Value::operator[](std::string_view key) const
{
    if (isObject()) {
        for (const Member& member : m_object) {
            if (member.first == key)
                return member.second;
        }
    }
    return nullValue();
}

bool Value::contains(std::string_view key) const
{
    if (!isObject())
        return false;
    for (const Member& member : m_object) {
        if (member.first == key)
            return true;
    }
    return false;
}

void Value::insert(std::string key, Value value)
{
    if (isNull())
        m_type = Type::Object;
    if (!isObject())
        return;
    for (Member& member : m_object) {
        if (member.first == key) {
            member.second = std::move(value);
            return;
        }
    }
    m_object.emplace_back(std::move(key), std::move(value));
}

const Value& Value::operator[](size_t index) const
{
    return isArray() && index < m_array.size() ? m_array[index] : nullValue();
}

void Value::append(Value value)
{
    if (isNull())
        m_type = Type::Array;
    if (isArray())
        m_array.push_back(std::move(value));
}

size_t Value::size() const
{
    return isArray() ? m_array.size() : isObject() ? m_object.size() : 0;
}

bool Value::operator==(const Value& other) const
{
    if (m_type != other.m_type)
        return false;
    switch (m_type) {
    case Type::Null:
        return true;
    case Type::Bool:
        return m_bool == other.m_bool;
    case Type::Number:
        return m_number == other.m_number;
    case Type::String:
        return m_string == other.m_string;
    case Type::Array:
        return m_array == other.m_array;
    case Type::Object:
        return m_object == other.m_object;
    }
    return false;
}

static void writeString(std::string& out, const std::string& text)
{
    out += '"';
    for (const char c : text) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\b':
            out += "\\b";
            break;
        case '\f':
            out += "\\f";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                char escape[8];
                std::snprintf(escape, sizeof(escape), "\\u%04x", unsigned(c));
                out += escape;
            } else {
                out += c;
            }
        }
    }
    out += '"';
}

static void writeNumber(std::string& out, double value)
{
    // null for NaN or infinity; whole numbers below 2^53 without a fraction.
    if (!std::isfinite(value)) {
        out += "null";
        return;
    }
    char buffer[64];
    if (value == std::trunc(value) && std::fabs(value) < 9007199254740992.0) {
        std::snprintf(buffer, sizeof(buffer), "%lld", static_cast<long long>(value));
        out += buffer;
        return;
    }
    const std::to_chars_result result = std::to_chars(buffer, buffer + sizeof(buffer), value);
    out.append(buffer, result.ptr);
}

static void write(std::string& out, const Value& value, bool compact, int indent)
{
    const auto newline = [&out, compact](int level) {
        if (compact)
            return;
        out += '\n';
        out.append(size_t(level) * 4, ' ');
    };
    switch (value.type()) {
    case Value::Type::Null:
        out += "null";
        break;
    case Value::Type::Bool:
        out += value.toBool() ? "true" : "false";
        break;
    case Value::Type::Number:
        writeNumber(out, value.toDouble());
        break;
    case Value::Type::String:
        writeString(out, value.toString());
        break;
    case Value::Type::Array: {
        out += '[';
        const Value::Array& array = value.toArray();
        for (size_t i = 0; i < array.size(); ++i) {
            if (i > 0)
                out += ',';
            newline(indent + 1);
            write(out, array[i], compact, indent + 1);
        }
        // Indented, even an empty container closes on a line of its own.
        newline(indent);
        out += ']';
        break;
    }
    case Value::Type::Object: {
        out += '{';
        const Value::Object& object = value.toObject();
        for (size_t i = 0; i < object.size(); ++i) {
            if (i > 0)
                out += ',';
            newline(indent + 1);
            writeString(out, object[i].first);
            out += compact ? ":" : ": ";
            write(out, object[i].second, compact, indent + 1);
        }
        newline(indent);
        out += '}';
        break;
    }
    }
}

std::string Value::serialize(bool compact) const
{
    std::string out;
    write(out, *this, compact, 0);
    if (!compact)
        out += '\n';
    return out;
}

} // namespace Lingmo::base::json
