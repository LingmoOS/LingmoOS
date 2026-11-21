// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/networkmanager/variants.h"

#include <cstdlib>

namespace Lingmo::Networkmanager::variants {

using base::dbus::ObjectPath;
using base::dbus::Signature;

template<typename T>
static bool readAs(const Variant& value, T& out)
{
    if (const auto read = value.get<T>()) {
        out = *read;
        return true;
    }
    return false;
}

int64_t toInt64(const Variant& value, int64_t fallback)
{
    const std::string& signature = value.signature();
    if (signature.size() != 1)
        return fallback;
    switch (signature[0]) {
    case 'b':
        return value.value<bool>() ? 1 : 0;
    case 'y':
        return value.value<uint8_t>();
    case 'n':
        return value.value<int16_t>();
    case 'q':
        return value.value<uint16_t>();
    case 'i':
        return value.value<int32_t>();
    case 'u':
        return value.value<uint32_t>();
    case 'x':
        return value.value<int64_t>();
    case 't':
        return int64_t(value.value<uint64_t>());
    case 'd':
        return int64_t(value.value<double>());
    case 's': {
        const std::string text = value.value<std::string>();
        char* end = nullptr;
        const long long number = std::strtoll(text.c_str(), &end, 10);
        return !text.empty() && end && *end == '\0' ? int64_t(number) : fallback;
    }
    default:
        return fallback;
    }
}

uint64_t toUInt64(const Variant& value, uint64_t fallback)
{
    if (value.signature() == "t")
        return value.value<uint64_t>();
    if (value.signature() == "s") {
        const std::string text = value.value<std::string>();
        char* end = nullptr;
        const unsigned long long number = std::strtoull(text.c_str(), &end, 10);
        return !text.empty() && text[0] != '-' && end && *end == '\0' ? uint64_t(number) : fallback;
    }
    const std::string& signature = value.signature();
    if (signature.size() != 1 || std::string("bynqiuxd").find(signature[0]) == std::string::npos)
        return fallback;
    return uint64_t(toInt64(value, int64_t(fallback)));
}

bool toBool(const Variant& value, bool fallback)
{
    if (value.signature() == "b")
        return value.value<bool>();
    if (value.signature() == "s") {
        const std::string text = value.value<std::string>();
        return !text.empty() && text != "0" && text != "false";
    }
    if (value.signature().size() == 1 && std::string("ynqiuxtd").find(value.signature()[0]) != std::string::npos)
        return toInt64(value) != 0;
    return fallback;
}

std::string toString(const Variant& value)
{
    std::string text;
    if (readAs(value, text))
        return text;
    ObjectPath path;
    if (readAs(value, path))
        return path.path;
    Signature signature;
    if (readAs(value, signature))
        return signature.signature;
    return {};
}

std::vector<std::string> toStringList(const Variant& value)
{
    if (value.signature() != "as" && value.signature() != "ao")
        return {};
    std::vector<std::string> strings;
    for (const Variant& item : value.items())
        strings.push_back(toString(item));
    return strings;
}

std::string toBytes(const Variant& value)
{
    return value.signature() == "ay" ? value.bytes() : std::string();
}

std::vector<std::string> toBytesList(const Variant& value)
{
    if (value.signature() != "aay")
        return {};
    std::vector<std::string> list;
    for (const Variant& item : value.items())
        list.push_back(item.bytes());
    return list;
}

std::vector<uint32_t> toUIntList(const Variant& value)
{
    if (value.signature().size() != 2 || value.signature()[0] != 'a')
        return {};
    std::vector<uint32_t> list;
    for (const Variant& item : value.items())
        list.push_back(toUInt(item));
    return list;
}

VariantMap toMap(const Variant& value)
{
    return value.get<VariantMap>().value_or(VariantMap());
}

std::vector<VariantMap> toMapList(const Variant& value)
{
    return value.get<std::vector<VariantMap>>().value_or(std::vector<VariantMap>());
}

std::map<std::string, std::string> toStringMap(const Variant& value)
{
    if (const auto strings = value.get<std::map<std::string, std::string>>())
        return *strings;
    std::map<std::string, std::string> strings;
    for (const auto& [key, item] : toMap(value))
        strings[key] = toString(item);
    return strings;
}

std::string objectPath(const Variant& value)
{
    const std::string path = toString(value);
    return path == "/" ? std::string() : path;
}

Variant objectPathValue(const std::string& path)
{
    return Variant(ObjectPath{path.empty() ? std::string("/") : path});
}

Variant bytesValue(const std::string& bytes)
{
    return Variant(std::vector<uint8_t>(bytes.begin(), bytes.end()));
}

Variant stringListValue(const std::vector<std::string>& strings)
{
    return Variant(strings);
}

const Variant& value(const VariantMap& map, const std::string& key)
{
    static const Variant empty;
    const auto it = map.find(key);
    return it == map.end() ? empty : it->second;
}

bool contains(const VariantMap& map, const std::string& key)
{
    return map.find(key) != map.end();
}

} // namespace Lingmo::Networkmanager::variants
