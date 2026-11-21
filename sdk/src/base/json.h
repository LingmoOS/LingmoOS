// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Lingmo::base::json {

// A JSON value, read from and written to RFC 8259 text. Numbers are doubles;
// objects keep their members in the order read or added.
class Value
{
public:
    enum class Type {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object,
    };

    using Array = std::vector<Value>;
    using Member = std::pair<std::string, Value>;
    using Object = std::vector<Member>;

    Value() = default;
    Value(std::nullptr_t) {}
    Value(bool value) : m_type(Type::Bool), m_bool(value) {}
    Value(int value) : m_type(Type::Number), m_number(value) {}
    Value(int64_t value) : m_type(Type::Number), m_number(double(value)) {}
    Value(double value) : m_type(Type::Number), m_number(value) {}
    Value(const char* value) : m_type(Type::String), m_string(value) {}
    Value(std::string value) : m_type(Type::String), m_string(std::move(value)) {}
    Value(std::string_view value) : m_type(Type::String), m_string(value) {}
    Value(Array value) : m_type(Type::Array), m_array(std::move(value)) {}
    static Value object(Object members = {});

    // Null when the text is not JSON; trailing white space is allowed.
    static std::optional<Value> parse(std::string_view text);
    // Compact, without white space; indented by four spaces otherwise.
    std::string serialize(bool compact = true) const;

    Type type() const { return m_type; }
    bool isNull() const { return m_type == Type::Null; }
    bool isBool() const { return m_type == Type::Bool; }
    bool isNumber() const { return m_type == Type::Number; }
    bool isString() const { return m_type == Type::String; }
    bool isArray() const { return m_type == Type::Array; }
    bool isObject() const { return m_type == Type::Object; }

    // The fallback when the value is of another type.
    bool toBool(bool fallback = false) const { return isBool() ? m_bool : fallback; }
    double toDouble(double fallback = 0) const { return isNumber() ? m_number : fallback; }
    // Rounded to the nearest integer, halfway away from zero, and clamped to
    // the range.
    int64_t toInt(int64_t fallback = 0) const;
    std::string toString(const std::string& fallback = {}) const { return isString() ? m_string : fallback; }
    const Array& toArray() const;
    const Object& toObject() const;

    // An object's member; a null value when there is none.
    const Value& operator[](std::string_view key) const;
    bool contains(std::string_view key) const;
    // Adds the member, or replaces its value; turns a null value into an object.
    void insert(std::string key, Value value);
    // An array's element; a null value past its end.
    const Value& operator[](size_t index) const;
    void append(Value value);
    // Elements of an array, members of an object.
    size_t size() const;

    bool operator==(const Value& other) const;

private:
    Type m_type = Type::Null;
    bool m_bool = false;
    double m_number = 0;
    std::string m_string;
    Array m_array;
    Object m_object;
};

} // namespace Lingmo::base::json
