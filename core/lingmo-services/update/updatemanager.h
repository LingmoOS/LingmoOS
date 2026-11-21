// SPDX-FileCopyrightText: 2026 Lingmo OS Team
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/process.h"
#include "base/settings.h"
#include "base/signal.h"
#include "base/timer.h"

class Notification;

// Checks for system updates in two stages:
//
//   1. The lingmo.org API (GET /api/v1/update/check) — takes the local
//      BUILD_VERSION from os-release and tells whether a new OS version is
//      available, with its version, build code, release date, mandatory flag,
//      release notes URL and HTML summary.
//   2. apt — first "apt-get update" to refresh package lists, then
//      "apt list --upgradable" to enumerate what is pending.
//
// The two results are combined into one of three outcomes:
//
//   SystemUpgrade  — API says new version AND apt has upgrades.
//   SoftwareUpdate — API says up-to-date but apt has upgrades.
//   UpToDate       — Neither source has anything to install.
//
// After an upgrade succeeds the state becomes RestartRequired: the packages
// are installed, but the running session still uses the old ones. It stays
// there until the machine restarts, and checks are ignored meanwhile.
//
// The result is published on the session bus as org.lingmo.Services.Update
// at /org/lingmo/Services/Update. A D-Bus signal is sent whenever the
// state or the upgrade progress changes.
class UpdateManager
{
public:
    // The overall state of the update subsystem.
    enum State {
        Idle           = 0,  // No check has run yet since startup.
        Checking       = 1,  // A check is in progress.
        SystemUpgrade  = 2,  // New OS version available (API + apt both positive).
        SoftwareUpdate = 3,  // Regular package upgrades only (apt positive, API negative).
        UpToDate       = 4,  // Nothing to install.
        Upgrading      = 5,  // apt-get upgrade/dist-upgrade is running.
        Error          = 6,
        // The upgrade finished; the running system still uses the old
        // packages until it is restarted.
        RestartRequired = 7,
    };

    // The release channels the API serves.
    enum Channel { Stable = 0, Beta = 1, Nightly = 2 };

    // How the automatic check is scheduled.
    enum CheckMode {
        Interval = 0, // Every checkInterval minutes.
        Daily    = 1, // Once a day at checkHour:checkMinute.
    };

    // What the one update notification currently shows, so a later check
    // does not replace a restart prompt with "updates available".
    enum class NotificationKind {
        None,
        UpdatesAvailable,
        UpgradeProgress,
        RestartNotice,
        UpgradeError,
    };

    UpdateManager(Lingmo::Base::Settings &settings, Lingmo::Base::DBus::Connection *session);
    ~UpdateManager();

    UpdateManager(const UpdateManager &) = delete;
    UpdateManager &operator=(const UpdateManager &) = delete;

    // org.lingmo.Services.Update at /org/lingmo/Services/Update.
    bool publish();

    // Triggers a full two-stage check; ignored while one is already running,
    // while an upgrade is in progress, or while a restart is pending.
    void checkForUpdate();

    // Starts apt-get dist-upgrade for a system upgrade or upgrade for a
    // software update; ignored unless state is SystemUpgrade or
    // SoftwareUpdate. Progress is reported via the upgradeProgress signal.
    void startUpgrade();

    // Changes the release channel and re-runs the check.
    void setChannel(int channel);

    // Automatic checks, persisted in services.conf.
    void setAutoCheck(bool enabled);
    void setCheckSchedule(int mode, int minutes);
    void setCheckTime(int hour, int minute);

private:
    // Starts a check. Automatic checks also notify and schedule the next one.
    void startCheck(bool automatic);
    void scheduleNextCheck();
    void saveSchedule();

    // The one notification the service keeps, from "updates available"
    // through progress to the restart prompt.
    Notification &updateNotification();
    void notifyUpdatesAvailable();
    void notifyProgress();
    void notifyRestartRequired();
    void notifyError();
    void clearUpdatesNotification();
    void requestRestart();

    // --- stage 1: API check ---
    void fetchApi();
    void parseApiResponse(const std::string &text);

    // --- stage 2: apt check (two sub-steps) ---
    // 2a. pkexec apt-get update  — refreshes package lists from the mirrors.
    void runAptUpdate();
    // 2b. apt list --upgradable  — enumerates packages to upgrade.
    void runAptListUpgradable();
    void parseUpgradableList(const std::string &text);

    // --- upgrade ---
    void runAptUpgrade();

    // --- helpers ---
    void loadSettings();
    void saveChannel();
    void saveLastCheckTime();
    static std::string machineArchitecture();
    static std::string osReleaseValue(const char *key);
    static Channel channelFromOsRelease();
    static std::string readLocalBuildVersion();
    static std::string readCurrentVersion();
    std::string apiUrl() const;
    void setState(State state);
    void emitInfoChanged();
    void failOperation(const std::string &message);

    Lingmo::Base::Settings &m_settings;
    Lingmo::Base::DBus::Connection *m_session;
    std::unique_ptr<Lingmo::Base::DBus::Object> m_object;
    std::unique_ptr<Lingmo::Base::Process> m_process;
    Lingmo::Base::Timer m_timer;
    std::unique_ptr<Notification> m_notification;
    Lingmo::Base::Subscription m_notificationAction;

    Channel m_channel  = Stable;
    State   m_state    = Idle;
    std::string m_arch;
    std::string m_localBuildVersion;

    // Automatic check schedule.
    bool       m_autoCheck = true;
    CheckMode  m_checkMode = Interval;
    int        m_checkIntervalMinutes = 360;
    int        m_checkHour = 3;
    int        m_checkMinute = 0;
    bool       m_firstCheck = true;
    bool       m_automaticCheck = false;

    NotificationKind m_notificationKind = NotificationKind::None;
    // Avoids a banner every interval while the same update is still pending.
    std::string m_lastNotifiedKey;
    // Notification progress is throttled; these are the last values shown.
    int m_notifiedPhase = 0;
    int m_notifiedPercent = -1;

    // Set by the API stage.
    bool        m_apiHasUpdate = false;
    std::string m_newVersion;
    int64_t     m_newBuild     = 0;
    bool        m_mandatory    = false;
    std::string m_releaseNotes;  // full URL
    std::string m_releaseDate;
    std::string m_changelogText; // HTML summary from the update API

    // Set by the apt stage.
    bool m_aptHasUpdate    = false;
    int  m_upgradableCount = 0;   // number of packages apt would upgrade
    // The package names, space-separated, for the UI to display if desired.
    std::string m_upgradablePackages;

    // Failure details for every check/upgrade stage, cleared on retry.
    std::string m_errorLog;

    // Whether the running upgrade is a system upgrade (dist-upgrade) rather
    // than a regular package update (upgrade).
    bool m_systemUpgrade = false;

    // Upgrade phase and progress.
    // Phase: 0 = not upgrading, 1 = downloading, 2 = installing.
    // Progress: 0–100 within the current phase, -1 when not upgrading.
    int m_upgradePhase    = 0;
    int m_upgradeProgress = -1;

    // Unix timestamp (seconds since epoch) of the last completed check, 0 if
    // none has run yet this session. Persisted across restarts in Settings.
    int64_t m_lastCheckTime = 0;
};
