// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/display_prefs.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: an atomic file write and stable monitor keys replace Chrome's
// preference service while preserving confirmed-only persistence.

#include "display/display_preferences.h"
#include "display/recommended_scale.h"

#include <sys/stat.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <functional>
#include <limits>
#include <map>
#include <set>
#include <system_error>

#include "base/file.h"
#include "base/json.h"
#include "base/strings.h"
#include "core/preferences.h"

namespace Lingmo::Display {
namespace {

using base::json::Value;

constexpr int kSchemaVersion = 2;
constexpr int64_t kMaximumFileSize = 1024 * 1024;

void fail(DisplayError *error, DisplayErrorCode code, const std::string &message)
{
    if (error)
        *error = {code, message, {}};
}

// A whole number in int's range, else the fallback, as the file has always been read.
int intValue(const Value &value, int fallback = 0)
{
    if (!value.isNumber())
        return fallback;
    const double number = value.toDouble();
    if (!std::isfinite(number) || number != std::trunc(number)
        || number < double(std::numeric_limits<int>::min())
        || number > double(std::numeric_limits<int>::max()))
        return fallback;
    return int(number);
}

// Keys sorted at every level, as display.json has always been written.
Value sorted(const Value &value)
{
    if (value.isArray()) {
        Value::Array result;
        for (const auto &item : value.toArray())
            result.push_back(sorted(item));
        return Value(std::move(result));
    }
    if (!value.isObject())
        return value;
    std::map<std::string, Value> members;
    for (const auto &[key, member] : value.toObject())
        members.insert_or_assign(key, sorted(member));
    Value::Object result;
    for (auto &[key, member] : members)
        result.emplace_back(key, std::move(member));
    return Value::object(std::move(result));
}

std::optional<std::string> readBounded(const std::string &path)
{
    struct stat status {};
    if (stat(path.c_str(), &status) != 0 || status.st_size > kMaximumFileSize)
        return std::nullopt;
    return base::file::read(path);
}

const DisplaySnapshot *snapshotForId(const HardwareSnapshot &hardware, const DisplayId &id)
{
    for (const auto &display : hardware.displays) {
        if (display.sessionId == id)
            return &display;
    }
    return nullptr;
}

const DisplaySnapshot *snapshotForPersistentKey(const HardwareSnapshot &hardware,
                                                const std::string &key)
{
    const DisplaySnapshot *found = nullptr;
    for (const auto &display : hardware.displays) {
        if (display.persistentKey != key)
            continue;
        if (found)
            return nullptr;
        found = &display;
    }
    return found;
}

Value modeToJson(const DisplayMode &mode)
{
    return Value::object({{"width", mode.pixelSize.width()},
                          {"height", mode.pixelSize.height()},
                          {"refreshMilliHz", mode.refreshMilliHz},
                          {"pixelClockKHz", mode.pixelClockKHz},
                          {"horizontalTotal", mode.horizontalTotal},
                          {"verticalTotal", mode.verticalTotal},
                          {"flags", int(mode.flags)},
                          {"name", mode.name}});
}

std::optional<DisplayMode> modeFromJson(const DisplaySnapshot &display, const Value &object)
{
    DisplayMode requested;
    requested.pixelSize = {intValue(object["width"]), intValue(object["height"])};
    requested.refreshMilliHz = intValue(object["refreshMilliHz"]);
    requested.pixelClockKHz = intValue(object["pixelClockKHz"]);
    requested.horizontalTotal = intValue(object["horizontalTotal"]);
    requested.verticalTotal = intValue(object["verticalTotal"]);
    requested.flags = uint32_t(intValue(object["flags"]));
    requested.name = object["name"].toString();
    for (const auto &mode : display.modes) {
        if (mode.stableKey() == requested.stableKey())
            return mode;
    }
    return std::nullopt;
}

std::string keyForId(const HardwareSnapshot &hardware, const DisplayId &id)
{
    const auto *display = snapshotForId(hardware, id);
    return display ? display->persistentKey : std::string();
}

std::string joined(std::vector<std::string> keys)
{
    std::sort(keys.begin(), keys.end());
    std::string result;
    for (size_t i = 0; i < keys.size(); ++i) {
        if (i > 0)
            result += '|';
        result += keys[i];
    }
    return result;
}

// The member as an object; an empty one when it is missing or of another type.
Value objectMember(const Value &object, std::string_view key)
{
    const Value &member = object[key];
    return member.isObject() ? member : Value::object();
}

} // namespace

DisplayPreferences::DisplayPreferences(std::string path)
    : m_path(path.empty() ? preferencesDirectory() + "/display.json" : std::move(path))
{
}

std::vector<std::string> DisplayPreferences::persistentKeys() const
{
    const auto text = readBounded(m_path);
    const auto root = text ? Value::parse(*text) : std::nullopt;
    if (!root || intValue((*root)["version"]) != kSchemaVersion)
        return {};
    std::set<std::string> keys;
    for (const auto &[key, value] : (*root)["monitors"].toObject())
        keys.insert(key);
    return {keys.begin(), keys.end()};
}

std::string DisplayPreferences::topologyKey(const HardwareSnapshot &hardware)
{
    std::vector<std::string> keys;
    for (const auto &display : hardware.displays)
        keys.push_back(display.persistentKey);
    return joined(std::move(keys));
}

bool DisplayPreferences::save(const HardwareSnapshot &hardware,
                              const DisplayConfiguration &configuration, DisplayError *error) const
{
    Value monitors = Value::object();
    Value enabled = Value::Array();
    for (const auto &request : configuration.displays) {
        const auto *display = snapshotForId(hardware, request.display);
        if (!display)
            continue;
        monitors.insert(display->persistentKey,
                        Value::object({{"preferredPort", display->portKey},
                                       {"mode", modeToJson(request.mode)},
                                       {"scale120", int(request.scale.numerator())},
                                       {"rotation", int(request.rotation)}}));
        if (request.enabled)
            enabled.append(display->persistentKey);
    }

    Value placements = Value::Array();
    for (const auto &placement : configuration.placements) {
        placements.append(Value::object({{"display", keyForId(hardware, placement.display)},
                                         {"parent", keyForId(hardware, placement.parent)},
                                         {"edge", int(placement.edge)},
                                         {"offset", placement.offset}}));
    }
    Value mirrorGroups = Value::Array();
    for (const auto &group : configuration.mirrorGroups) {
        Value stored = Value::Array();
        for (const auto &id : group)
            stored.append(keyForId(hardware, id));
        mirrorGroups.append(std::move(stored));
    }
    const Value profile = Value::object({{"primary", keyForId(hardware, configuration.primary)},
                                         {"enabled", enabled},
                                         {"placements", placements},
                                         {"mirrorGroups", mirrorGroups}});
    Value previous = Value::object();
    if (const auto text = readBounded(m_path)) {
        auto stored = Value::parse(*text);
        if (stored && stored->isObject() && intValue((*stored)["version"]) == kSchemaVersion)
            previous = std::move(*stored);
    }
    Value mergedMonitors = objectMember(previous, "monitors");
    for (const auto &[key, value] : monitors.toObject())
        mergedMonitors.insert(key, value);
    Value topologies = objectMember(previous, "topologies");
    topologies.insert(topologyKey(hardware), profile);
    const Value root = Value::object({{"version", kSchemaVersion},
                                      {"monitors", mergedMonitors},
                                      {"topologies", topologies}});

    const std::string &path = m_path;
    std::error_code directoryError;
    std::filesystem::create_directories(std::filesystem::absolute(path).parent_path(),
                                        directoryError);
    if (directoryError) {
        fail(error, DisplayErrorCode::PersistenceFailed,
             "Could not create the display preference directory");
        return false;
    }
    if (!base::file::writeAtomically(path, sorted(root).serialize(false))) {
        fail(error, DisplayErrorCode::PersistenceFailed,
             "Could not atomically save display preferences");
        return false;
    }
    return true;
}

std::optional<DisplayConfiguration> DisplayPreferences::restore(const HardwareSnapshot &hardware,
                                                                uint64_t appliedRevision,
                                                                DisplayError *error) const
{
    const std::string &path = m_path;
    std::error_code existsError;
    if (!std::filesystem::exists(path, existsError))
        return std::nullopt;
    const auto text = readBounded(path);
    if (!text) {
        fail(error, DisplayErrorCode::PersistenceFailed,
             "Display preferences cannot be read safely");
        return std::nullopt;
    }
    const auto document = Value::parse(*text);
    if (!document || intValue((*document)["version"]) != kSchemaVersion) {
        fail(error, DisplayErrorCode::PersistenceFailed,
             "Display preferences have an unsupported schema");
        return std::nullopt;
    }
    const Value &root = *document;
    const Value &topologies = root["topologies"];
    Value profile = objectMember(topologies, topologyKey(hardware));
    Value monitors = objectMember(root, "monitors");
    if (profile.size() == 0) {
        std::vector<std::string> oldTopology;
        std::map<std::string, std::string> aliases;
        for (const auto &display : hardware.displays) {
            const std::string oldPort = display.devicePath + '/'
                    + std::string(base::strings::section(display.portKey, '/', -1));
            std::string oldKey = display.persistentKey;
            if (oldKey.starts_with("port:"))
                oldKey = "port:" + oldPort;
            else if (oldKey.starts_with("virtual:"))
                oldKey = "virtual:" + oldPort;
            else if (oldKey.find('@') != std::string::npos)
                oldKey = oldKey.substr(0, oldKey.find('@')) + '@' + oldPort;
            aliases.insert_or_assign(oldKey, display.persistentKey);
            oldTopology.push_back(oldKey);
            if (monitors.contains(oldKey))
                monitors.insert(display.persistentKey, monitors[oldKey]);
        }
        profile = objectMember(topologies, joined(oldTopology));
        if (profile.size() == 0) {
            aliases.clear();
            oldTopology.clear();
            for (const auto &display : hardware.displays) {
                const std::string oldKey = "session:" + display.connectorName;
                aliases.insert_or_assign(oldKey, display.persistentKey);
                oldTopology.push_back(oldKey + "@port:" + display.connectorName);
                if (monitors.contains(oldKey))
                    monitors.insert(display.persistentKey, monitors[oldKey]);
            }
            profile = objectMember(topologies, joined(oldTopology));
        }
        std::function<Value(const Value &)> translate = [&](const Value &value) -> Value {
            if (value.isString()) {
                const auto alias = aliases.find(value.toString());
                return alias == aliases.end() ? value : Value(alias->second);
            }
            if (value.isArray()) {
                Value::Array result;
                for (const auto &item : value.toArray())
                    result.push_back(translate(item));
                return Value(std::move(result));
            }
            if (value.isObject()) {
                Value::Object result;
                for (const auto &[key, member] : value.toObject())
                    result.emplace_back(key, translate(member));
                return Value::object(std::move(result));
            }
            return value;
        };
        profile = translate(profile);
    }
    if (profile.size() == 0)
        return std::nullopt;

    DisplayConfiguration result;
    result.base = {hardware.compositorInstanceId, hardware.topologyGeneration, appliedRevision};
    std::set<std::string> enabled;
    for (const auto &value : profile["enabled"].toArray())
        enabled.insert(value.toString());
    for (const auto &display : hardware.displays) {
        const Value saved = objectMember(monitors, display.persistentKey);
        auto mode = modeFromJson(display, objectMember(saved, "mode"));
        if (!mode)
            mode = display.nativeMode ? display.nativeMode : display.currentMode;
        if (!mode)
            continue;
        const int scale120 =
                intValue(saved["scale120"], int(recommendedScale(display).numerator()));
        const auto scale = compositor::OutputScale::fromNumerator(scale120);
        if (!scale)
            continue;
        // A value this build does not know falls back rather than failing the profile.
        auto rotation = DisplayRotation(intValue(saved["rotation"]));
        if (!isValid(rotation))
            rotation = DisplayRotation::Normal;
        result.displays.push_back({display.sessionId, enabled.contains(display.persistentKey),
                                *mode, *scale, rotation});
    }
    const auto *primary = snapshotForPersistentKey(hardware, profile["primary"].toString());
    if (primary)
        result.primary = primary->sessionId;
    for (const auto &item : profile["placements"].toArray()) {
        const auto *display = snapshotForPersistentKey(hardware, item["display"].toString());
        const auto *parent = snapshotForPersistentKey(hardware, item["parent"].toString());
        const auto edge = LayoutEdge(intValue(item["edge"]));
        if (display && parent && isValid(edge))
            result.placements.push_back({display->sessionId, parent->sessionId, edge,
                                         intValue(item["offset"])});
    }
    for (const auto &value : profile["mirrorGroups"].toArray()) {
        std::vector<DisplayId> group;
        for (const auto &key : value.toArray()) {
            if (const auto *display = snapshotForPersistentKey(hardware, key.toString()))
                group.push_back(display->sessionId);
        }
        if (group.size() > 1)
            result.mirrorGroups.push_back(group);
    }
    return result;
}

} // namespace Lingmo::Display
