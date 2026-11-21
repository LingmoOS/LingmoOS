// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "options.h"

#include "base/strings.h"

#include <cmath>
#include <optional>

using namespace Lingmo;
using base::dbus::Variant;
using base::dbus::VariantMap;

// Whole numbers of any width, booleans as 0 or 1, and decimal text.
static std::optional<int64_t> integer(const Variant &value)
{
    const std::string &signature = value.signature();
    if (signature == "b")
        return value.value<bool>() ? 1 : 0;
    if (signature == "y")
        return value.value<uint8_t>();
    if (signature == "n")
        return value.value<int16_t>();
    if (signature == "q")
        return value.value<uint16_t>();
    if (signature == "i")
        return value.value<int32_t>();
    if (signature == "u")
        return value.value<uint32_t>();
    if (signature == "x")
        return value.value<int64_t>();
    if (signature == "t")
        return int64_t(value.value<uint64_t>());
    if (signature == "d")
        return std::llround(value.value<double>());
    if (signature == "s")
        return base::strings::toNumber<int64_t>(value.value<std::string>());
    return std::nullopt;
}

bool toBool(const Variant &value)
{
    if (value.signature() == "s") {
        const std::string text = value.value<std::string>();
        return !(text.empty() || text == "0" || base::strings::equalsIgnoreAsciiCase(text, "false"));
    }
    if (value.signature() == "d")
        return value.value<double>() != 0;
    return integer(value).value_or(0) != 0;
}

uint32_t toUInt(const Variant &value)
{
    return uint32_t(integer(value).value_or(0));
}

std::string toString(const Variant &value)
{
    if (value.signature() == "s")
        return value.value<std::string>();
    if (value.signature() == "o")
        return value.value<base::dbus::ObjectPath>().path;
    if (value.signature() == "ay")
        return value.bytes();
    return {};
}

std::vector<std::string> toStringList(const Variant &value)
{
    if (value.signature() == "s")
        return {value.value<std::string>()};
    return value.value<std::vector<std::string>>();
}

std::string toBytes(const Variant &value)
{
    if (value.signature() == "ay")
        return value.bytes();
    if (value.signature() == "s")
        return value.value<std::string>();
    return {};
}

bool optionBool(const VariantMap &options, const std::string &key, bool fallback)
{
    const auto it = options.find(key);
    return it == options.end() ? fallback : toBool(it->second);
}

uint32_t optionUInt(const VariantMap &options, const std::string &key, uint32_t fallback)
{
    const auto it = options.find(key);
    return it == options.end() ? fallback : toUInt(it->second);
}

std::string optionString(const VariantMap &options, const std::string &key)
{
    const auto it = options.find(key);
    return it == options.end() ? std::string() : toString(it->second);
}
