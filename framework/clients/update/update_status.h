// SPDX-FileCopyrightText: 2026 Lingmo OS Team
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::update {

// The state values of org.lingmo.Services.Update.
enum class State {
    Idle          = 0,
    Checking      = 1,
    SystemUpgrade = 2,
    SoftwareUpdate = 3,
    UpToDate      = 4,
    Upgrading     = 5,
    Error         = 6,
    // The upgrade completed; the running system needs a restart.
    RestartRequired = 7,
};

// The release channels.
enum class Channel { Stable = 0, Beta = 1, Nightly = 2 };

// Upgrade phase: 0 = not upgrading, 1 = downloading, 2 = installing.
enum class UpgradePhase { None = 0, Downloading = 1, Installing = 2 };

// The system update service, as org.lingmo.Services.Update reports it.
// Values are read when the service appears and kept up to date from its
// signals; writes go to the service and show here once it acknowledges them.
class UpdateStatus
{
public:
    // One instance for everyone on the calling thread's loop, while anyone holds it.
    static std::shared_ptr<UpdateStatus> shared();

    UpdateStatus();
    ~UpdateStatus();

    UpdateStatus(const UpdateStatus&) = delete;
    UpdateStatus& operator=(const UpdateStatus&) = delete;

    bool isAvailable() const { return m_available; }
    bool isRequestPending() const { return m_requestPending; }
    const std::string& requestError() const { return m_requestError; }

    State state() const;
    Channel channel() const;

    // Automatic check schedule.
    bool autoCheck() const;
    // 0 = every checkInterval minutes, 1 = daily at checkHour:checkMinute.
    int checkMode() const;
    int checkInterval() const;
    int checkHour() const;
    int checkMinute() const;

    // New OS version info (valid in SystemUpgrade state).
    std::string newVersion() const;
    int64_t newBuild() const;
    bool mandatory() const;
    std::string releaseNotes() const;
    std::string releaseDate() const;
    // HTML summary from the update API.
    std::string changelogText() const;

    // apt upgrade info.
    int upgradableCount() const;
    std::string upgradablePackages() const;

    // Upgrade progress: 0–100 in the current phase, -1 when not upgrading.
    int upgradeProgress() const { return m_upgradeProgress; }
    // 0 = not upgrading, 1 = downloading, 2 = installing.
    int upgradePhase() const { return m_upgradePhase; }

    // Unix timestamp of the last completed check; 0 if never.
    int64_t lastCheckTime() const;
    // apt stderr from the last failed upgrade; empty unless state == Error.
    std::string errorLog() const;

    // Trigger a check; ignored while one is already running.
    void checkForUpdate();
    // Start the upgrade; ignored unless state is SystemUpgrade or SoftwareUpdate.
    void startUpgrade();
    // Change the release channel.
    void setChannel(int channel);
    // Automatic checks: on or off, interval or daily, and the daily time.
    void setAutoCheck(bool enabled);
    void setCheckSchedule(int mode, int minutes);
    void setCheckTime(int hour, int minute);

    // Fired after any property changed, or the service came or went.
    base::Signal<> changed;
    // Fired when upgradePhase or upgradeProgress changes.
    base::Signal<> upgradeProgressChanged;

private:
    void reload();
    void beginRequest();
    void finishRequest(const base::dbus::Reply& reply);
    template<typename T>
    T value(const char* name, const T& fallback) const;

    base::dbus::Proxy m_service;
    base::dbus::NameWatch m_watch;
    std::vector<base::dbus::Match> m_matches;
    base::dbus::PendingCall m_reload;
    base::dbus::PendingCall m_request;
    bool m_requestPending = false;
    std::string m_requestError;
    base::dbus::VariantMap m_properties;
    bool m_available = false;
    // Kept separately so upgradeProgressChanged fires without a full reload.
    int m_upgradeProgress = -1;
    int m_upgradePhase    = 0;
};

} // namespace Lingmo::update
