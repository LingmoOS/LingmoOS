// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/common/application_badges.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string_view>

namespace Lingmo::Shell {

namespace dbus = base::dbus;

static constexpr std::string_view kScheme = "application://";
static constexpr std::string_view kSuffix = ".desktop";

static std::string lowered(std::string text)
{
    for (char &c : text) {
        if (c >= 'A' && c <= 'Z')
            c = char(c - 'A' + 'a');
    }
    return text;
}

// The specification's count is an int64; some senders use another integer.
static std::optional<int64_t> integer(const dbus::Variant &value)
{
    if (const auto x = value.get<int64_t>())
        return *x;
    if (const auto i = value.get<int32_t>())
        return *i;
    if (const auto u = value.get<uint32_t>())
        return *u;
    if (const auto t = value.get<uint64_t>())
        return int64_t(std::min<uint64_t>(*t, INT64_MAX));
    return std::nullopt;
}

ApplicationBadges::ApplicationBadges(dbus::Connection *bus)
{
    if (!bus)
        return;

    dbus::MatchRule update;
    update.interface = "com.canonical.Unity.LauncherEntry";
    update.member = "Update";
    m_update = bus->subscribe(update, [this](const dbus::Message &message) {
        std::string uri;
        dbus::VariantMap properties;
        if (message.read(uri, properties))
            this->update(message.sender(), uri, properties);
    });

    dbus::MatchRule owner;
    owner.sender = "org.freedesktop.DBus";
    owner.path = "/org/freedesktop/DBus";
    owner.interface = "org.freedesktop.DBus";
    owner.member = "NameOwnerChanged";
    m_nameOwnerChanged = bus->subscribe(owner, [this](const dbus::Message &message) {
        std::string name, oldOwner, newOwner;
        if (message.read(name, oldOwner, newOwner) && name.starts_with(':') && newOwner.empty())
            senderVanished(name);
    });
}

ApplicationBadges::~ApplicationBadges() = default;

std::string ApplicationBadges::desktopId(const std::string &uri)
{
    if (!uri.starts_with(kScheme))
        return {};
    std::string id = uri.substr(kScheme.size());
    if (id.ends_with(kSuffix))
        id.resize(id.size() - kSuffix.size());
    return id;
}

int64_t ApplicationBadges::count(const std::string &desktopId) const
{
    const auto it = m_entries.find(lowered(desktopId));
    return it != m_entries.end() ? it->second.shown() : 0;
}

void ApplicationBadges::update(const std::string &sender, const std::string &uri,
                               const dbus::VariantMap &properties)
{
    const std::string id = desktopId(uri);
    if (id.empty())
        return;

    Entry &entry = m_entries[lowered(id)];
    const int64_t before = entry.shown();
    entry.sender = sender;
    // Each signal carries only the properties that changed.
    if (const auto it = properties.find("count"); it != properties.end()) {
        if (const auto count = integer(it->second))
            entry.count = *count;
    }
    if (const auto it = properties.find("count-visible"); it != properties.end()) {
        if (const auto visible = it->second.get<bool>())
            entry.visible = *visible;
    }
    if (entry.shown() != before)
        changed();
}

void ApplicationBadges::senderVanished(const std::string &sender)
{
    bool removed = false;
    for (auto it = m_entries.begin(); it != m_entries.end();) {
        if (it->second.sender != sender) {
            ++it;
            continue;
        }
        removed = removed || it->second.shown() > 0;
        it = m_entries.erase(it);
    }
    if (removed)
        changed();
}

} // namespace Lingmo::Shell
