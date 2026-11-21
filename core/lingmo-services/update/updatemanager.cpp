// SPDX-FileCopyrightText: 2026 Lingmo OS Team
// SPDX-License-Identifier: GPL-3.0-or-later

#include "updatemanager.h"

#include <sys/utsname.h>

#include <chrono>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <ctime>
#include <fstream>
#include <sstream>

#include "base/i18n.h"
#include "base/json.h"
#include "base/locale.h"
#include "log.h"
#include "notification.h"

using namespace std::chrono_literals;
using namespace Lingmo::Base;
using Lingmo::I18n::tr;
using Lingmo::I18n::format;

static int64_t nowSeconds()
{
    return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
}

// Seconds from now until the next local occurrence of hour:minute.
static std::chrono::seconds secondsUntilDailyCheck(int hour, int minute)
{
    const std::time_t now = std::time(nullptr);
    std::tm local {};
    localtime_r(&now, &local);
    local.tm_hour = std::clamp(hour, 0, 23);
    local.tm_min = std::clamp(minute, 0, 59);
    local.tm_sec = 0;

    std::time_t target = std::mktime(&local);
    if (target <= now) {
        local.tm_mday += 1;
        target = std::mktime(&local);
    }
    return std::chrono::seconds(std::max<int64_t>(1, int64_t(target - now)));
}

// The single 0-100 bar the notification shows across both apt phases.
static int overallProgress(int phase, int percent)
{
    if (percent < 0)
        return 0;
    if (phase == 1)
        return std::clamp(percent / 2, 0, 50);
    if (phase == 2)
        return std::clamp(50 + percent / 2, 50, 100);
    return 0;
}

static constexpr auto s_channelKey       = "Update/Channel";
static constexpr auto s_lastCheckTimeKey = "Update/LastCheckTime";
static constexpr auto s_autoCheckKey     = "Update/AutoCheck";
static constexpr auto s_checkModeKey     = "Update/CheckMode";
static constexpr auto s_checkIntervalKey = "Update/CheckIntervalMinutes";
static constexpr auto s_checkHourKey     = "Update/CheckHour";
static constexpr auto s_checkMinuteKey   = "Update/CheckMinute";
static constexpr auto s_apiBase          = "https://lingmo.org/api/v1/update/check";
static constexpr auto s_startDelay      = 30s;
static constexpr int  s_defaultIntervalMinutes = 360;

static const char *channelName(UpdateManager::Channel ch)
{
    switch (ch) {
    case UpdateManager::Beta:    return "beta";
    case UpdateManager::Nightly: return "nightly";
    default:                     return "stable";
    }
}

// Parse one line from the apt Status-Fd stream.
//
// When apt-get is run with -o APT::Status-Fd=<fd>, it writes colon-separated
// status lines to that file descriptor. The format is:
//
//   <type>:<package>:<percent>:<message>
//
// where <type> is one of:
//   dlstatus  — a package is being downloaded   (percent 0-100 per package)
//   pmstatus  — dpkg is installing / configuring (percent 0-100 overall)
//
// We report two separate phases to the UI:
//   phase 1 (downloading): dlstatus lines, percent 0-100
//   phase 2 (installing):  pmstatus lines, percent 0-100
//
// Returns false when the line is not a recognised status line.
static bool parseAptStatusFd(const std::string &line, int &outPhase, int &outPercent)
{
    // Expected: "dlstatus:pkg:30.5:Downloading"
    const auto c1 = line.find(':');
    if (c1 == std::string::npos) return false;
    const auto c2 = line.find(':', c1 + 1);
    if (c2 == std::string::npos) return false;
    const auto c3 = line.find(':', c2 + 1);
    if (c3 == std::string::npos) return false;

    const std::string type    = line.substr(0, c1);
    const std::string pctStr  = line.substr(c2 + 1, c3 - c2 - 1);

    double pct = 0;
    try { pct = std::stod(pctStr); } catch (...) { return false; }
    if (!std::isfinite(pct)) return false;
    pct = std::max(0.0, std::min(100.0, pct));

    if (type == "dlstatus") {
        outPhase   = 1;           // downloading
        outPercent = int(pct);
        return true;
    }
    if (type == "pmstatus") {
        outPhase   = 2;           // installing
        outPercent = int(pct);
        return true;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────

UpdateManager::UpdateManager(Settings &settings, dbus::Connection *session)
    : m_settings(settings)
    , m_session(session)
    , m_arch(machineArchitecture())
    , m_localBuildVersion(readLocalBuildVersion())
{
    if (m_localBuildVersion.empty()) {
        // The API needs a parseable build version; let a missing os-release
        // field ask for the update rather than fail the check.
        m_localBuildVersion = "0a0";
        LM_WARNING(services::log()) << "No BUILD_VERSION in os-release; using 0a0";
    }
    loadSettings();

    m_timer.setSingleShot(true);
    m_timer.onTimeout = [this] { startCheck(true); };
    scheduleNextCheck();
}

UpdateManager::~UpdateManager()
{
    if (m_notification)
        m_notification->close();
}

bool UpdateManager::publish()
{
    if (!m_session)
        return false;

    auto object = std::make_unique<dbus::Object>(m_session, "/org/lingmo/Services/Update",
                                                 "org.lingmo.Services.Update");

    object->addMethod("checkForUpdate", [this] { checkForUpdate(); });
    object->addMethod("startUpgrade",   [this] { startUpgrade(); });
    object->addMethod("setChannel", [this](int32_t ch) { setChannel(ch); },
                      {{"channel"}, {}});
    object->addMethod("setAutoCheck", [this](bool enabled) { setAutoCheck(enabled); },
                      {{"enabled"}, {}});
    object->addMethod("setCheckSchedule", [this](int32_t mode, int32_t minutes) {
        setCheckSchedule(mode, minutes);
    }, {{"mode", "minutes"}, {}});
    object->addMethod("setCheckTime", [this](int32_t hour, int32_t minute) {
        setCheckTime(hour, minute);
    }, {{"hour", "minute"}, {}});

    object->addProperty("state",               [this] { return int32_t(m_state); });
    object->addProperty("autoCheck",           [this] { return m_autoCheck; });
    object->addProperty("checkMode",           [this] { return int32_t(m_checkMode); });
    object->addProperty("checkInterval",       [this] { return int32_t(m_checkIntervalMinutes); });
    object->addProperty("checkHour",           [this] { return int32_t(m_checkHour); });
    object->addProperty("checkMinute",         [this] { return int32_t(m_checkMinute); });
    object->addProperty("channel",             [this] { return int32_t(m_channel); });
    object->addProperty("newVersion",          [this] { return m_newVersion; });
    object->addProperty("newBuild",            [this] { return m_newBuild; });
    object->addProperty("mandatory",           [this] { return m_mandatory; });
    object->addProperty("releaseNotes",        [this] { return m_releaseNotes; });
    object->addProperty("releaseDate",         [this] { return m_releaseDate; });
    object->addProperty("upgradableCount",     [this] { return int32_t(m_upgradableCount); });
    object->addProperty("upgradablePackages",  [this] { return m_upgradablePackages; });
    object->addProperty("upgradeProgress",     [this] { return int32_t(m_upgradeProgress); });
    // upgradePhase: 0 = not upgrading, 1 = downloading, 2 = installing.
    object->addProperty("upgradePhase",        [this] { return int32_t(m_upgradePhase); });
    // Unix timestamp of the last completed check (0 if none yet).
    object->addProperty("lastCheckTime",       [this] { return m_lastCheckTime; });
    // Changelog text (Markdown) for the available version; empty when up-to-date.
    object->addProperty("changelogText",       [this] { return m_changelogText; });
    // Error log from the last failed upgrade; empty unless state == Error.
    object->addProperty("errorLog",            [this] { return m_errorLog; });

    object->addSignal("stateChanged",    "i",  {"state"});
    // Fired whenever phase or progress changes.
    // phase: 1 = downloading, 2 = installing.  percent: 0–100.
    object->addSignal("upgradeProgress", "ii", {"phase", "percent"});

    if (!object->publish())
        return false;

    m_object = std::move(object);
    return true;
}

void UpdateManager::checkForUpdate()
{
    startCheck(false);
}

void UpdateManager::startCheck(bool automatic)
{
    // Once an upgrade has completed, only a restart clears the prompt; a
    // scheduled or manual check must not turn it back into "up to date".
    if (m_state == Checking || m_state == Upgrading || m_state == RestartRequired)
        return;

    m_automaticCheck = automatic;
    m_firstCheck = false;
    m_errorLog.clear();
    m_newVersion.clear(); m_newBuild = 0; m_mandatory = false;
    m_releaseNotes.clear(); m_releaseDate.clear(); m_changelogText.clear();
    m_upgradePhase = 0; m_upgradeProgress = -1;
    m_apiHasUpdate       = false;
    m_aptHasUpdate       = false;
    m_upgradableCount    = 0;
    m_upgradablePackages.clear();

    setState(Checking);
    fetchApi();
}

void UpdateManager::startUpgrade()
{
    if (m_state != SystemUpgrade && m_state != SoftwareUpdate)
        return;

    // A system upgrade may need new dependencies or removals; a regular
    // package update stays the conservative apt-get upgrade.
    m_systemUpgrade = m_state == SystemUpgrade;
    m_errorLog.clear();

    // Upgrade begins in the download phase; apt may emit dlstatus lines
    // first, then switch to pmstatus for the install phase.
    m_upgradePhase    = 1;
    m_upgradeProgress = 0;
    if (m_object)
        m_object->emitSignal("upgradeProgress", int32_t(m_upgradePhase),
                            int32_t(m_upgradeProgress));

    m_lastNotifiedKey.clear();
    m_notifiedPhase = m_upgradePhase;
    m_notifiedPercent = m_upgradeProgress;
    setState(Upgrading);
    notifyProgress();
    runAptUpgrade();
}

void UpdateManager::setChannel(int channel)
{
    if (m_state == Checking || m_state == Upgrading || m_state == RestartRequired)
        return;
    const Channel ch = (channel >= Stable && channel <= Nightly)
                           ? static_cast<Channel>(channel)
                           : Stable;
    if (ch == m_channel)
        return;
    m_channel = ch;
    saveChannel();
    if (m_object)
        m_object->emitPropertiesChanged({"channel"});

    checkForUpdate();
}

void UpdateManager::setAutoCheck(bool enabled)
{
    if (m_autoCheck == enabled)
        return;
    m_autoCheck = enabled;
    // Turning checks back on should not wait a whole interval.
    m_firstCheck = enabled;
    saveSchedule();
    if (m_object)
        m_object->emitPropertiesChanged({"autoCheck"});
    scheduleNextCheck();
}

void UpdateManager::setCheckSchedule(int mode, int minutes)
{
    const CheckMode newMode = mode == Daily ? Daily : Interval;
    const int newMinutes = std::clamp(minutes, 1, 24 * 60);
    if (newMode == m_checkMode && newMinutes == m_checkIntervalMinutes)
        return;
    m_checkMode = newMode;
    m_checkIntervalMinutes = newMinutes;
    saveSchedule();
    if (m_object)
        m_object->emitPropertiesChanged({"checkMode", "checkInterval"});
    scheduleNextCheck();
}

void UpdateManager::setCheckTime(int hour, int minute)
{
    hour = std::clamp(hour, 0, 23);
    minute = std::clamp(minute, 0, 59);
    if (hour == m_checkHour && minute == m_checkMinute)
        return;
    m_checkHour = hour;
    m_checkMinute = minute;
    saveSchedule();
    if (m_object)
        m_object->emitPropertiesChanged({"checkHour", "checkMinute"});
    scheduleNextCheck();
}

void UpdateManager::scheduleNextCheck()
{
    m_timer.stop();
    if (!m_autoCheck || m_state == RestartRequired || m_state == Upgrading)
        return;

    std::chrono::nanoseconds delay;
    if (m_checkMode == Daily) {
        delay = secondsUntilDailyCheck(m_checkHour, m_checkMinute);
    } else if (m_firstCheck) {
        delay = s_startDelay;
    } else {
        delay = std::chrono::minutes(std::max(1, m_checkIntervalMinutes));
    }
    m_timer.start(delay);
}

void UpdateManager::saveSchedule()
{
    m_settings.setValue(s_autoCheckKey, m_autoCheck);
    m_settings.setValue(s_checkModeKey, int64_t(m_checkMode));
    m_settings.setValue(s_checkIntervalKey, int64_t(m_checkIntervalMinutes));
    m_settings.setValue(s_checkHourKey, int64_t(m_checkHour));
    m_settings.setValue(s_checkMinuteKey, int64_t(m_checkMinute));
    m_settings.sync();
}

Notification &UpdateManager::updateNotification()
{
    if (!m_notification) {
        m_notification = std::make_unique<Notification>(m_session);
        m_notification->setAppName(tr("System Update"));
        m_notification->setIconName("system-software-update");
        m_notification->setDesktopEntry("lingmo-settings");
        m_notificationAction = m_notification->actionInvoked.connect(
                [this](uint32_t, const std::string &action) {
                    if (action == "restart")
                        requestRestart();
                });
    }
    return *m_notification;
}

void UpdateManager::notifyUpdatesAvailable()
{
    const std::string key = std::to_string(int(m_state)) + ':' + m_newVersion + ':'
                          + std::to_string(m_upgradableCount);
    if (key == m_lastNotifiedKey)
        return;
    m_lastNotifiedKey = key;

    Notification &notification = updateNotification();
    notification.setSummary(tr("Updates available"));
    if (m_state == SystemUpgrade && !m_newVersion.empty()) {
        notification.setBody(format(tr("System version {version} is available."),
                                    {{"version", m_newVersion}}));
    } else if (m_upgradableCount == 1) {
        notification.setBody(tr("1 package can be updated."));
    } else {
        notification.setBody(format(tr("{count} packages can be updated."),
                                    {{"count", std::to_string(m_upgradableCount)}}));
    }
    notification.setValue(-1);
    notification.setActions({});
    notification.setTimeout(-1);
    notification.setUrgency(Notification::Normal);
    m_notificationKind = NotificationKind::UpdatesAvailable;
    notification.show();
}

void UpdateManager::notifyProgress()
{
    Notification &notification = updateNotification();
    const std::string phase = m_upgradePhase == 1 ? tr("Downloading")
                            : m_upgradePhase == 2 ? tr("Installing")
                                                  : tr("Preparing");
    notification.setSummary(tr("Updating system"));
    notification.setBody(m_upgradeProgress >= 0
                                 ? phase + " " + std::to_string(m_upgradeProgress) + "%"
                                 : phase);
    notification.setValue(overallProgress(m_upgradePhase, m_upgradeProgress));
    notification.setActions({});
    notification.setTimeout(0);
    notification.setUrgency(Notification::Normal);
    m_notificationKind = NotificationKind::UpgradeProgress;
    notification.show();
}

void UpdateManager::notifyRestartRequired()
{
    Notification &notification = updateNotification();
    notification.setSummary(tr("Restart required"));
    notification.setBody(tr("The update has been installed. Restart the computer to finish."));
    notification.setValue(-1);
    notification.setActions({"restart", tr("Restart now")});
    notification.setTimeout(0);
    notification.setUrgency(Notification::Normal);
    m_notificationKind = NotificationKind::RestartNotice;
    notification.show();
}

void UpdateManager::notifyError()
{
    Notification &notification = updateNotification();
    notification.setSummary(tr("Update failed"));
    notification.setBody(m_errorLog.empty() ? tr("The update could not be installed.") : m_errorLog);
    notification.setValue(-1);
    notification.setActions({});
    notification.setTimeout(0);
    notification.setUrgency(Notification::Normal);
    m_notificationKind = NotificationKind::UpgradeError;
    notification.show();
}

void UpdateManager::clearUpdatesNotification()
{
    m_lastNotifiedKey.clear();
    if (m_notificationKind != NotificationKind::UpdatesAvailable)
        return;
    if (m_notification)
        m_notification->close();
    m_notificationKind = NotificationKind::None;
}

void UpdateManager::requestRestart()
{
    if (!m_session)
        return;
    dbus::Proxy session(m_session, "org.lingmo.Session", "/org/lingmo/Session",
                        "org.lingmo.Session");
    session.send("reboot");
}

// ── stage 1: API ─────────────────────────────────────────────────────────────

void UpdateManager::fetchApi()
{
    m_process = std::make_unique<Process>();
    m_process->setOomProtectionDropped(true);
    m_process->onFinished = [this](int exitCode, bool crashed) {
        const std::string output = m_process->standardOutput();
        const std::string error = m_process->standardError();
        m_process.reset();
        if (crashed || exitCode != 0) {
            LM_WARNING(services::log())
                    << "Update API request failed (curl exited" << exitCode << ")";
            failOperation(format(tr("Update API request failed (curl exit {code}): {error}"), {{"code", std::to_string(exitCode)}, {"error", error}}));
            return;
        }
        parseApiResponse(output);
    };

    if (!m_process->start("curl", {"--max-time", "15", "--location", "--fail", "--silent", "--show-error", apiUrl()})) {
        LM_WARNING(services::log()) << "Cannot start curl for update API check";
        m_process.reset();
        failOperation(tr("Cannot start curl. Install curl and check the update service PATH."));
    }
}

void UpdateManager::parseApiResponse(const std::string &text)
{
    const auto root = json::Value::parse(text);
    if (!root || !root->operator[]("success").toBool()) {
        LM_WARNING(services::log()) << "Update API returned an unexpected response";
        failOperation(tr("Update API returned an invalid response (expected success=true)."));
        return;
    }

    if (root->operator[]("updateAvailable").toBool()) {
        const auto &latest = root->operator[]("latest");
        if (!latest.isNull()) {
            m_apiHasUpdate  = true;
            m_newVersion    = latest["version"].toString();
            m_newBuild      = latest["buildCode"].toInt();
            m_mandatory     = latest["mandatory"].toBool();
            m_releaseDate   = latest["releaseDate"].toString();
            // HTML release notes for the Settings card and notifications.
            m_changelogText = root->operator[]("summary").toString();
            const std::string notes = root->operator[]("releaseNotesUrl").toString();
            if (!notes.empty())
                m_releaseNotes = notes.starts_with("http") ? notes : "https://lingmo.org" + notes;
        }
    }

    runAptUpdate();
}

// ── stage 2a: apt-get update ─────────────────────────────────────────────────

void UpdateManager::runAptUpdate()
{
    // Refresh the package lists from the mirrors. Needs root via pkexec.
    // -qq suppresses all output except errors.
    m_process = std::make_unique<Process>();
    m_process->setOomProtectionDropped(true);
    m_process->setEnvironment({"LANG=C", "LANGUAGE=C", "LC_ALL=C",
                               "DEBIAN_FRONTEND=noninteractive"});
    m_process->onFinished = [this](int exitCode, bool crashed) {
        const std::string error = m_process->standardError();
        m_process.reset();
        if (crashed || exitCode != 0) {
            failOperation(format(tr("Refreshing package lists failed (pkexec/apt-get exit {code}): {error}"), {{"code", std::to_string(exitCode)}, {"error", error}}));
            return;
        }
        runAptListUpgradable();
    };

    if (!m_process->start("pkexec", {"/usr/bin/apt-get", "update", "-qq", "-o", "APT::Update::Error-Mode=any"})) {
        m_process.reset();
        failOperation(tr("Cannot start pkexec to refresh package lists. Check polkit installation."));
    }
}

// ── stage 2b: apt list --upgradable ─────────────────────────────────────────

void UpdateManager::runAptListUpgradable()
{
    // Enumerate packages that have a newer version available.
    // Output format (one line per package):
    //   bash/bookworm 5.2.15 amd64 [upgradable from: 5.2.14]
    m_process = std::make_unique<Process>();
    m_process->setOomProtectionDropped(true);
    m_process->setEnvironment({"LANG=C", "LANGUAGE=C", "LC_ALL=C"});
    m_process->onFinished = [this](int exitCode, bool crashed) {
        const std::string output = m_process->standardOutput();
        const std::string error = m_process->standardError();
        m_process.reset();
        if (crashed || exitCode != 0) {
            failOperation(format(tr("Listing upgradable packages failed (apt exit {code}): {error}"), {{"code", std::to_string(exitCode)}, {"error", error}}));
            return;
        }
        parseUpgradableList(output);
    };

    if (!m_process->start("apt", {"list", "--upgradable"})) {
        m_process.reset();
        failOperation(tr("Cannot start apt to list upgradable packages."));
    }
}

void UpdateManager::parseUpgradableList(const std::string &text)
{
    // Each real package line contains "[upgradable from:".
    std::istringstream ss(text);
    std::string line;
    std::vector<std::string> names;

    while (std::getline(ss, line)) {
        if (line.find("[upgradable from:") == std::string::npos)
            continue;
        // Package name is the part before the first '/'.
        const auto slash = line.find('/');
        if (slash != std::string::npos)
            names.push_back(line.substr(0, slash));
    }

    m_upgradableCount = int(names.size());
    m_aptHasUpdate    = m_upgradableCount > 0;

    // Build a space-separated list for the UI.
    m_upgradablePackages.clear();
    for (const auto &n : names) {
        if (!m_upgradablePackages.empty())
            m_upgradablePackages += ' ';
        m_upgradablePackages += n;
    }

    // Publish completion metadata before announcing the resulting state.
    m_lastCheckTime = nowSeconds();
    saveLastCheckTime();

    // Apply the decision matrix.
    if (m_apiHasUpdate && m_aptHasUpdate) {
        setState(SystemUpgrade);
    } else if (!m_apiHasUpdate && m_aptHasUpdate) {
        // Regular update: show current version, generic changelog string.
        m_newVersion    = readCurrentVersion();
        m_changelogText = tr("Regular software update");
        m_releaseDate.clear();
        setState(SoftwareUpdate);
    } else {
        setState(UpToDate);
    }

    emitInfoChanged();

    if (m_state == UpToDate)
        clearUpdatesNotification();

    if (m_automaticCheck && m_autoCheck
        && (m_state == SystemUpgrade || m_state == SoftwareUpdate))
        notifyUpdatesAvailable();
    m_automaticCheck = false;
    scheduleNextCheck();
}

// ── upgrade ───────────────────────────────────────────────────────────────────

void UpdateManager::runAptUpgrade()
{
    // A system upgrade may bring new dependencies or require removals, so it
    // uses dist-upgrade; a regular package update stays with upgrade.
    const std::string action = m_systemUpgrade ? "dist-upgrade" : "upgrade";

    // Run the real apt-get action with root via pkexec.
    //
    // Progress reporting uses the APT Status-Fd protocol:
    //   -o APT::Status-Fd=1  — write status lines to stdout (fd 1)
    //   -o Dpkg::Progress=1  — enable dpkg progress reporting
    //
    // apt writes one line per event:
    //   dlstatus:<pkg>:<0-100>:<msg>   download phase
    //   pmstatus:<pkg>:<0-100>:<msg>   install/configure phase
    //
    // We map download→0-50 and install→50-100 so the UI sees a
    // continuous 0-100 range across both phases.
    m_process = std::make_unique<Process>();
    m_process->setOomProtectionDropped(true);
    m_process->setEnvironment({"LANG=C", "LANGUAGE=C", "LC_ALL=C",
                               "DEBIAN_FRONTEND=noninteractive"});

    // Accumulate partial lines across chunks.
    auto lineBuf = std::make_shared<std::string>();
    m_process->onStandardOutput = [this, lineBuf](std::string_view chunk) {
        lineBuf->append(chunk);
        std::string::size_type start = 0;
        std::string::size_type nl;
        while ((nl = lineBuf->find('\n', start)) != std::string::npos) {
            const std::string line = lineBuf->substr(start, nl - start);
            start = nl + 1;
            int phase = 0, percent = 0;
            if (parseAptStatusFd(line, phase, percent)) {
                if (phase != m_upgradePhase || percent != m_upgradeProgress) {
                    m_upgradePhase    = phase;
                    m_upgradeProgress = percent;
                    if (m_object)
                        m_object->emitSignal("upgradeProgress",
                                             int32_t(phase), int32_t(percent));

                    // The notification coalesces calls already; still update
                    // it only on a phase change or every few percent.
                    const bool phaseChanged = phase != m_notifiedPhase;
                    if (phaseChanged || percent < m_notifiedPercent
                        || percent >= m_notifiedPercent + 5 || percent == 100) {
                        m_notifiedPhase = phase;
                        m_notifiedPercent = percent;
                        notifyProgress();
                    }
                }
            }
        }
        lineBuf->erase(0, start);
    };

    m_process->onFinished = [this, action](int exitCode, bool crashed) {
        // Collect stderr for the error log before resetting the process.
        const std::string errOutput = m_process->standardError();
        m_process.reset();
        if (crashed || exitCode != 0) {
            LM_WARNING(services::log())
                    << "apt-get" << action << "failed (exit" << exitCode << ")";
            m_errorLog        = errOutput.empty()
                                    ? format(tr("apt-get {command} exited with code {code}"),
                                             {{"command", action}, {"code", std::to_string(exitCode)}})
                                    : errOutput;
            m_upgradePhase    = 0;
            m_upgradeProgress = -1;
            if (m_object)
                m_object->emitSignal("upgradeProgress",
                                     int32_t(m_upgradePhase), int32_t(m_upgradeProgress));
            failOperation(m_errorLog);
            notifyError();
        } else {
            m_errorLog.clear();
            // Finish the install phase at 100%.
            m_upgradePhase    = 2;
            m_upgradeProgress = 100;
            if (m_object)
                m_object->emitSignal("upgradeProgress",
                                     int32_t(m_upgradePhase), int32_t(m_upgradeProgress));
            m_upgradePhase    = 0;
            m_upgradeProgress = -1;
            // The new packages are on disk but the running session still uses
            // the old ones. Keep the prompt until the machine is restarted.
            m_upgradableCount = 0;
            m_upgradablePackages.clear();
            setState(RestartRequired);
            m_lastNotifiedKey.clear();
            notifyRestartRequired();
        }
    };

    const bool started = m_process->start(
            "pkexec",
            {"/usr/bin/apt-get", action, "-y",
             "-o", "APT::Status-Fd=1",
             "-o", "Dpkg::Progress=1"});

    if (!started) {
        LM_WARNING(services::log()) << "Cannot start pkexec apt-get" << action;
        m_process.reset();
        failOperation(tr("Cannot start pkexec to install updates. Check polkit installation."));
        notifyError();
    }
}

// ── helpers ───────────────────────────────────────────────────────────────────

void UpdateManager::loadSettings()
{
    const Channel detected = channelFromOsRelease();
    const bool hadChannel = m_settings.contains(s_channelKey);
    const int ch = int(m_settings.intValue(s_channelKey, int64_t(detected)));
    m_channel = (ch >= Stable && ch <= Nightly) ? static_cast<Channel>(ch) : detected;
    // No explicit choice yet, or one that this build no longer understands:
    // seed the channel from the installed OS type so services.conf shows it.
    if (!hadChannel || ch < Stable || ch > Nightly)
        saveChannel();

    m_lastCheckTime = m_settings.intValue(s_lastCheckTimeKey, 0);

    m_autoCheck = m_settings.boolValue(s_autoCheckKey, true);
    const int mode = int(m_settings.intValue(s_checkModeKey, int64_t(Interval)));
    m_checkMode = mode == Daily ? Daily : Interval;
    m_checkIntervalMinutes = std::clamp(
            int(m_settings.intValue(s_checkIntervalKey, int64_t(s_defaultIntervalMinutes))),
            1, 24 * 60);
    m_checkHour = std::clamp(int(m_settings.intValue(s_checkHourKey, 3)), 0, 23);
    m_checkMinute = std::clamp(int(m_settings.intValue(s_checkMinuteKey, 0)), 0, 59);
}

void UpdateManager::saveChannel()
{
    m_settings.setValue(s_channelKey, int64_t(m_channel));
    m_settings.sync();
}

void UpdateManager::saveLastCheckTime()
{
    m_settings.setValue(s_lastCheckTimeKey, m_lastCheckTime);
    m_settings.sync();
}

std::string UpdateManager::machineArchitecture()
{
    struct utsname buf {};
    if (::uname(&buf) != 0)
        return "x86_64";
    const std::string machine = buf.machine;
    if (machine == "aarch64")     return "arm64";
    if (machine == "loongarch64") return "loongarch";
    return "x86_64";
}

static std::string unquote(std::string value)
{
    if (value.size() >= 2
        && ((value.front() == '"' && value.back() == '"')
            || (value.front() == '\'' && value.back() == '\''))) {
        value = value.substr(1, value.size() - 2);
    }
    return value;
}

std::string UpdateManager::osReleaseValue(const char *key)
{
    // /etc/os-release is a symlink to /usr/lib/os-release on installed
    // systems; read both so an image without the link still works.
    const std::string prefix = std::string(key) + "=";
    for (const char *path : {"/etc/os-release", "/usr/lib/os-release"}) {
        std::ifstream file(path);
        if (!file.is_open())
            continue;
        std::string line;
        while (std::getline(file, line)) {
            if (line.rfind(prefix, 0) == 0)
                return unquote(line.substr(prefix.size()));
        }
    }
    return {};
}

UpdateManager::Channel UpdateManager::channelFromOsRelease()
{
    std::string type = osReleaseValue("VERSION_TYPE");
    std::transform(type.begin(), type.end(), type.begin(),
                   [](unsigned char c) { return char(std::tolower(c)); });
    if (type == "beta")
        return Beta;
    if (type == "alpha" || type == "nightly")
        return Nightly;
    if (type == "release" || type == "stable")
        return Stable;

    // Older images without VERSION_TYPE: derive it from the build stage.
    const std::string build = readLocalBuildVersion();
    if (build.find('b') != std::string::npos)
        return Beta;
    if (build.find('a') != std::string::npos)
        return Nightly;
    return Stable;
}

std::string UpdateManager::readLocalBuildVersion()
{
    return osReleaseValue("BUILD_VERSION");
}

std::string UpdateManager::readCurrentVersion()
{
    return osReleaseValue("VERSION");
}

std::string UpdateManager::apiUrl() const
{
    std::string locale = base::Locale::system().name();
    if (locale.empty() || locale == "C" || locale == "POSIX")
        locale = "en";
    std::replace(locale.begin(), locale.end(), '_', '-');

    return std::string(s_apiBase)
         + "?channel=" + channelName(m_channel)
         + "&arch="    + m_arch
         + "&build="   + m_localBuildVersion
         + "&locale="  + locale;
}

void UpdateManager::failOperation(const std::string &message)
{
    m_errorLog = message;
    m_upgradePhase = 0;
    m_upgradeProgress = -1;
    LM_WARNING(services::log()) << m_errorLog;
    setState(Error);
    emitInfoChanged();

    m_automaticCheck = false;
    scheduleNextCheck();
}

void UpdateManager::setState(State state)
{
    if (m_state == state)
        return;
    m_state = state;
    if (m_object)
        m_object->emitSignal("stateChanged", int32_t(state));
}

void UpdateManager::emitInfoChanged()
{
    if (!m_object)
        return;
    m_object->emitPropertiesChanged({"newVersion", "newBuild", "mandatory",
                                     "releaseNotes", "releaseDate",
                                     "changelogText", "upgradableCount",
                                     "upgradablePackages", "lastCheckTime",
                                     "errorLog"});
}
