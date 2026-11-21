// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "shell/common/pending_calls.h"
#include "shell/common/sender_filter.h"

namespace Lingmo::Shell {

// One media player on the session bus, org.mpris.MediaPlayer2.<name>: its
// MediaPlayer2 and MediaPlayer2.Player properties, read once and kept from
// PropertiesChanged, and the calls the control centre's media controls make.
class MprisPlayer
{
public:
    enum class PlaybackStatus {
        Stopped,
        Playing,
        Paused,
    };

    MprisPlayer(base::dbus::Connection *bus, std::string service);
    ~MprisPlayer();

    MprisPlayer(const MprisPlayer &) = delete;
    MprisPlayer &operator=(const MprisPlayer &) = delete;

    const std::string &service() const { return m_service; }
    // Both interfaces answered once.
    bool isReady() const { return m_rootReady && m_playerReady; }

    std::string identity() const;

    PlaybackStatus playbackStatus() const;
    bool canControl() const;
    bool canGoNext() const;
    bool canGoPrevious() const;
    bool canPlay() const;
    bool canPause() const;
    // xesam:title, xesam:artist joined by ", ", mpris:artUrl.
    std::string title() const;
    std::string artist() const;
    std::string artUrl() const;

    // Methods of a player that cannot be controlled are not sent.
    void play();
    void pause();
    void playPause();
    void next();
    void previous();

    // Any property changed.
    base::Signal<> changed;
    base::Signal<> playbackStatusChanged;

private:
    void load(const std::string &interface);
    void apply(const std::string &interface, const base::dbus::VariantMap &properties);
    bool playerFlag(const char *name, bool fallback = false) const;
    void callPlayer(const char *method);

    base::dbus::Connection *m_bus;
    std::string m_service;
    base::dbus::Proxy m_root;
    base::dbus::Proxy m_player;
    SenderFilter m_sender;
    std::vector<base::dbus::Match> m_matches;
    PendingCalls m_calls;
    base::dbus::VariantMap m_rootProperties;
    base::dbus::VariantMap m_playerProperties;
    base::dbus::VariantMap m_metadata;
    bool m_rootReady = false;
    bool m_playerReady = false;
};

// The media players on the bus and the one the control centre shows. A
// playing player stays shown; when it stops, the one that most recently
// started playing takes over. With none playing, a player that appears is
// shown, and when the shown one leaves, the most recently active one is.
class MprisManager
{
public:
    explicit MprisManager(base::dbus::Connection *bus);
    ~MprisManager();

    MprisManager(const MprisManager &) = delete;
    MprisManager &operator=(const MprisManager &) = delete;

    MprisPlayer *current() const { return m_current.get(); }
    std::string currentService() const;
    // The shown player first, then the most recently active.
    std::vector<std::string> availableServices() const;

    base::Signal<> currentChanged;
    // Something of the shown player changed.
    base::Signal<> currentPlayerChanged;

private:
    struct Entry
    {
        std::shared_ptr<MprisPlayer> player;
        // Raised when the player appears or starts playing.
        uint64_t active = 0;
        base::Subscription statusChanged;
    };

    void add(const std::string &service);
    void remove(const std::string &service);
    void statusChanged(const std::string &service);
    Entry *entry(const std::string &service);
    bool present(const std::shared_ptr<MprisPlayer> &player) const;
    std::shared_ptr<MprisPlayer> choose() const;
    void show(std::shared_ptr<MprisPlayer> player);

    base::dbus::Connection *m_bus;
    std::vector<Entry> m_entries;
    std::shared_ptr<MprisPlayer> m_current;
    uint64_t m_clock = 0;
    base::Subscription m_currentSubscription;
    base::dbus::Match m_nameOwnerChanged;
    PendingCalls m_calls;
};

} // namespace Lingmo::Shell

