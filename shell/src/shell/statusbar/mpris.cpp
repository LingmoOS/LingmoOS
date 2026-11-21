// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/mpris.h"

#include <algorithm>

#include "base/strings.h"

namespace Lingmo::Shell {

namespace dbus = base::dbus;

static constexpr const char *kPrefix = "org.mpris.MediaPlayer2.";
static constexpr const char *kPath = "/org/mpris/MediaPlayer2";
static constexpr const char *kRootInterface = "org.mpris.MediaPlayer2";
static constexpr const char *kPlayerInterface = "org.mpris.MediaPlayer2.Player";

MprisPlayer::MprisPlayer(dbus::Connection *bus, std::string service)
    : m_bus(bus)
    , m_service(std::move(service))
    , m_root(bus, m_service, kPath, kRootInterface)
    , m_player(bus, m_service, kPath, kPlayerInterface)
    , m_sender(bus, m_service)
{
    if (m_bus) {
        dbus::MatchRule rule;
        rule.sender = m_service;
        rule.path = kPath;
        rule.interface = "org.freedesktop.DBus.Properties";
        rule.member = "PropertiesChanged";
        m_matches.push_back(m_bus->subscribe(rule, [this](const dbus::Message &message) {
            std::string interface;
            dbus::VariantMap changed;
            std::vector<std::string> invalidated;
            if (!m_sender.accepts(message) || !message.read(interface, changed, invalidated))
                return;
            if (interface != kRootInterface && interface != kPlayerInterface)
                return;
            apply(interface, changed);
            if (!invalidated.empty())
                load(interface);
        }));
    }
    load(kRootInterface);
    load(kPlayerInterface);
}

MprisPlayer::~MprisPlayer() = default;

void MprisPlayer::load(const std::string &interface)
{
    const dbus::Proxy &proxy = interface == kRootInterface ? m_root : m_player;
    m_calls.track([&](std::function<void()> finished) {
        return proxy.getAllAsync([this, interface, finished](const dbus::Reply &reply) {
            if (interface == kRootInterface)
                m_rootReady = true;
            else
                m_playerReady = true;
            apply(interface, reply.value<dbus::VariantMap>().value_or(dbus::VariantMap()));
            finished();
        });
    });
}

void MprisPlayer::apply(const std::string &interface, const dbus::VariantMap &properties)
{
    const bool root = interface == kRootInterface;
    dbus::VariantMap &stored = root ? m_rootProperties : m_playerProperties;
    const PlaybackStatus status = playbackStatus();
    for (const auto &[name, value] : properties) {
        stored[name] = value;
        if (!root && name == "Metadata")
            m_metadata = value.value<dbus::VariantMap>();
    }
    changed();
    if (playbackStatus() != status)
        playbackStatusChanged();
}

static std::string stringOf(const dbus::VariantMap &properties, const char *name)
{
    const auto it = properties.find(name);
    return it == properties.end() ? std::string() : it->second.value<std::string>();
}

static bool flagOf(const dbus::VariantMap &properties, const char *name, bool fallback)
{
    const auto it = properties.find(name);
    return it == properties.end() ? fallback : it->second.value<bool>(fallback);
}

std::string MprisPlayer::identity() const
{
    return stringOf(m_rootProperties, "Identity");
}

MprisPlayer::PlaybackStatus MprisPlayer::playbackStatus() const
{
    const std::string status = stringOf(m_playerProperties, "PlaybackStatus");
    if (status == "Playing")
        return PlaybackStatus::Playing;
    if (status == "Paused")
        return PlaybackStatus::Paused;
    return PlaybackStatus::Stopped;
}

bool MprisPlayer::playerFlag(const char *name, bool fallback) const
{
    return flagOf(m_playerProperties, name, fallback);
}

bool MprisPlayer::canControl() const
{
    return playerFlag("CanControl");
}

bool MprisPlayer::canGoNext() const
{
    return playerFlag("CanGoNext");
}

bool MprisPlayer::canGoPrevious() const
{
    return playerFlag("CanGoPrevious");
}

bool MprisPlayer::canPlay() const
{
    return playerFlag("CanPlay");
}

bool MprisPlayer::canPause() const
{
    return playerFlag("CanPause");
}

std::string MprisPlayer::title() const
{
    return stringOf(m_metadata, "xesam:title");
}

std::string MprisPlayer::artist() const
{
    const auto it = m_metadata.find("xesam:artist");
    if (it == m_metadata.end())
        return {};
    // A list by the spec; some players send one string.
    if (auto artists = it->second.get<std::vector<std::string>>())
        return base::strings::join(*artists, ", ");
    return it->second.value<std::string>();
}

std::string MprisPlayer::artUrl() const
{
    return stringOf(m_metadata, "mpris:artUrl");
}

void MprisPlayer::callPlayer(const char *method)
{
    if (canControl())
        m_player.send(method);
}

void MprisPlayer::play()
{
    callPlayer("Play");
}

void MprisPlayer::pause()
{
    callPlayer("Pause");
}

void MprisPlayer::playPause()
{
    callPlayer("PlayPause");
}

void MprisPlayer::next()
{
    callPlayer("Next");
}

void MprisPlayer::previous()
{
    callPlayer("Previous");
}

static bool playing(const MprisPlayer &player)
{
    return player.playbackStatus() == MprisPlayer::PlaybackStatus::Playing;
}

MprisManager::MprisManager(dbus::Connection *bus)
    : m_bus(bus)
{
    if (!m_bus)
        return;

    dbus::MatchRule rule;
    rule.sender = "org.freedesktop.DBus";
    rule.path = "/org/freedesktop/DBus";
    rule.interface = "org.freedesktop.DBus";
    rule.member = "NameOwnerChanged";
    m_nameOwnerChanged = m_bus->subscribe(rule, [this](const dbus::Message &message) {
        std::string name, oldOwner, newOwner;
        if (!message.read(name, oldOwner, newOwner) || !name.starts_with(kPrefix))
            return;
        if (oldOwner.empty())
            add(name);
        else if (newOwner.empty())
            remove(name);
    });

    const dbus::Proxy daemon(m_bus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
    m_calls.track([&](std::function<void()> finished) {
        return daemon.callAsync("ListNames", [this, finished](const dbus::Reply &reply) {
            for (const std::string &name : reply.value<std::vector<std::string>>().value_or(std::vector<std::string>())) {
                if (name.starts_with(kPrefix))
                    add(name);
            }
            finished();
        });
    });
}

MprisManager::~MprisManager() = default;

std::string MprisManager::currentService() const
{
    return m_current ? m_current->service() : std::string();
}

std::vector<std::string> MprisManager::availableServices() const
{
    std::vector<const Entry *> others;
    for (const Entry &entry : m_entries) {
        if (entry.player != m_current)
            others.push_back(&entry);
    }
    std::sort(others.begin(), others.end(), [](const Entry *a, const Entry *b) { return a->active > b->active; });

    std::vector<std::string> services;
    if (present(m_current))
        services.push_back(m_current->service());
    for (const Entry *entry : others)
        services.push_back(entry->player->service());
    return services;
}

MprisManager::Entry *MprisManager::entry(const std::string &service)
{
    const auto found = std::find_if(m_entries.begin(), m_entries.end(),
                                    [&service](const Entry &entry) { return entry.player->service() == service; });
    return found == m_entries.end() ? nullptr : &*found;
}

bool MprisManager::present(const std::shared_ptr<MprisPlayer> &player) const
{
    return player && std::any_of(m_entries.begin(), m_entries.end(),
                                 [&player](const Entry &entry) { return entry.player == player; });
}

void MprisManager::add(const std::string &service)
{
    if (Entry *known = entry(service)) {
        known->active = ++m_clock;
        return;
    }

    Entry added;
    added.player = std::make_shared<MprisPlayer>(m_bus, service);
    added.active = ++m_clock;
    added.statusChanged = added.player->playbackStatusChanged.connect([this, service] { statusChanged(service); });
    m_entries.push_back(std::move(added));

    // Nothing playing is interrupted by showing the newcomer.
    if (!m_current || !playing(*m_current))
        show(m_entries.back().player);
}

void MprisManager::remove(const std::string &service)
{
    std::erase_if(m_entries, [&service](const Entry &entry) { return entry.player->service() == service; });
    if (m_current && m_current->service() == service)
        show(choose());
}

void MprisManager::statusChanged(const std::string &service)
{
    Entry *changed = entry(service);
    if (!changed)
        return;

    if (playing(*changed->player))
        changed->active = ++m_clock;
    show(choose());
}

std::shared_ptr<MprisPlayer> MprisManager::choose() const
{
    if (present(m_current) && playing(*m_current))
        return m_current;

    const Entry *latestPlaying = nullptr;
    const Entry *latest = nullptr;
    for (const Entry &entry : m_entries) {
        if (playing(*entry.player) && (!latestPlaying || entry.active > latestPlaying->active))
            latestPlaying = &entry;
        if (!latest || entry.active > latest->active)
            latest = &entry;
    }

    if (latestPlaying)
        return latestPlaying->player;
    if (present(m_current))
        return m_current;
    return latest ? latest->player : nullptr;
}

void MprisManager::show(std::shared_ptr<MprisPlayer> player)
{
    if (player == m_current)
        return;

    m_current = std::move(player);
    m_currentSubscription = m_current ? m_current->changed.connect([this] { currentPlayerChanged(); })
                                      : base::Subscription();
    currentChanged();
}

} // namespace Lingmo::Shell
