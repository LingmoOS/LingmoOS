// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <mpv/client.h>

// A value of mpv's node API: nothing, a flag, an integer, a double, a string,
// a list or a map (whose keys keep mpv's order).
class MpvNode
{
public:
    using List = std::vector<MpvNode>;
    using Map = std::vector<std::pair<std::string, MpvNode>>;

    MpvNode() = default;
    MpvNode(bool value) : m_value(value) {}
    MpvNode(int value) : m_value(int64_t(value)) {}
    MpvNode(int64_t value) : m_value(value) {}
    MpvNode(double value) : m_value(value) {}
    MpvNode(const char* value) : m_value(std::string(value)) {}
    MpvNode(std::string value) : m_value(std::move(value)) {}
    MpvNode(List value) : m_value(std::move(value)) {}
    MpvNode(Map value) : m_value(std::move(value)) {}

    bool isNone() const { return std::holds_alternative<std::monostate>(m_value); }
    bool isFlag() const { return std::holds_alternative<bool>(m_value); }
    bool isInt() const { return std::holds_alternative<int64_t>(m_value); }
    bool isDouble() const { return std::holds_alternative<double>(m_value); }
    bool isString() const { return std::holds_alternative<std::string>(m_value); }
    bool isList() const { return std::holds_alternative<List>(m_value); }
    bool isMap() const { return std::holds_alternative<Map>(m_value); }

    // Numbers convert into one another; anything else gives the fallback.
    bool toBool(bool fallback = false) const;
    int64_t toInt(int64_t fallback = 0) const;
    double toDouble(double fallback = 0) const;
    std::string toString() const;
    const List& list() const;
    const Map& map() const;
    // A map's value, or a none node.
    const MpvNode& operator[](std::string_view key) const;

    bool operator==(const MpvNode&) const = default;

    // Fills node with copies of every string and list; free it with release(),
    // never with mpv_free_node_contents().
    void fill(mpv_node* node) const;
    static void release(mpv_node* node);
    static MpvNode from(const mpv_node* node);

private:
    std::variant<std::monostate, bool, int64_t, double, std::string, List, Map> m_value;
};
