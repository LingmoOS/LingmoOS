// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "base/dbus.h"

// Reads NetworkManager's values the way its properties are documented, not
// strictly by their D-Bus type: a missing or mistyped value reads as empty or
// zero, as a property NetworkManager leaves out stands for its default.
namespace Lingmo::Networkmanager::variants {

using base::dbus::Variant;
using base::dbus::VariantMap;

int64_t toInt64(const Variant& value, int64_t fallback = 0);
uint64_t toUInt64(const Variant& value, uint64_t fallback = 0);
inline int toInt(const Variant& value, int fallback = 0)
{
    return int(toInt64(value, fallback));
}
inline uint32_t toUInt(const Variant& value, uint32_t fallback = 0)
{
    return uint32_t(toUInt64(value, fallback));
}
bool toBool(const Variant& value, bool fallback = false);
// A string, object path or signature.
std::string toString(const Variant& value);
// "as" and "ao".
std::vector<std::string> toStringList(const Variant& value);
// The bytes of an "ay".
std::string toBytes(const Variant& value);
// "aay".
std::vector<std::string> toBytesList(const Variant& value);
std::vector<uint32_t> toUIntList(const Variant& value);
VariantMap toMap(const Variant& value);
// "aa{sv}".
std::vector<VariantMap> toMapList(const Variant& value);
// a{ss}, and an a{sv} of strings.
std::map<std::string, std::string> toStringMap(const Variant& value);

// An object path property, empty for NetworkManager's "/".
std::string objectPath(const Variant& value);
// "/" for an empty path.
Variant objectPathValue(const std::string& path);
Variant bytesValue(const std::string& bytes);
Variant stringListValue(const std::vector<std::string>& strings);

const Variant& value(const VariantMap& map, const std::string& key);
bool contains(const VariantMap& map, const std::string& key);

} // namespace Lingmo::Networkmanager::variants
