// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/status_bar.h"

#include "base/desktop_entry.h"
#include "base/logging.h"
#include "base/settings.h"
#include "lingmo/powerprofiles/power_profiles.h"
#include "lingmo/upower/battery_status.h"
#include "shell/common/file_thread.h"
#include "shell/statusbar/clock_format.h"

namespace Lingmo::Shell {

namespace dbus = base::dbus;

static const base::LogCategory lingmoStatusBar("lingmo.shell.statusbar");

StatusBar::StatusBar(dbus::Connection *sessionBus, dbus::Connection *systemBus, TaskManager *tasks,
                     std::string settingsPath)
    : m_bus(sessionBus)
    , m_settingsPath(std::move(settingsPath))
    , m_statusSettings(m_settingsPath)
    , m_watcher(sessionBus)
    , m_tray(m_watcher, sessionBus, tasks)
    , m_registrar(sessionBus)
    , m_gtkSettings(sessionBus)
    , m_mpris(sessionBus)
    , m_battery(std::make_unique<Battery>(std::make_shared<upower::BatteryStatus>(sessionBus, systemBus),
                                          m_settingsPath))
    , m_brightness(sessionBus)
    , m_powerProfiles(std::make_unique<powerprofiles::PowerProfiles>(systemBus))
    , m_powerActions(sessionBus)
{
    m_battery->followService(sessionBus, [sessionBus, systemBus] {
        return std::make_shared<upower::BatteryStatus>(sessionBus, systemBus);
    });
    m_timeFormat = ClockFormat::pattern(m_timeLocale, m_twentyFourTime);

    m_load = runOnFileThread(
            [path = m_settingsPath] { return base::Settings(path).boolValue("Statusbar/TwentyFourTime", false); },
            [this](bool enabled) {
                // A choice made while the file was read stays.
                if (m_twentyFourTimeSet || enabled == m_twentyFourTime)
                    return;
                m_twentyFourTime = enabled;
                updateTimeFormat();
            });

    if (m_bus) {
        dbus::MatchRule rule;
        rule.sender = "org.lingmo.Services";
        rule.path = "/org/lingmo/Services/Language";
        rule.interface = "org.lingmo.Services.Language";
        rule.member = "languageChanged";
        m_languageChanged = m_bus->subscribe(rule, [this](const dbus::Message &) { reloadLanguage(); });
    }
    reloadLanguage();
}

StatusBar::~StatusBar()
{
    if (m_object && m_bus)
        m_bus->releaseName("org.lingmo.Statusbar");
}

bool StatusBar::registerServices()
{
    if (!m_object) {
        if (!m_bus)
            return false;
        auto object = std::make_unique<dbus::Object>(m_bus, "/Statusbar", "org.lingmo.Statusbar");
        object->addMethod("setBatteryPercentage", [this](bool enabled) { m_battery->setShowPercentage(enabled); },
                          dbus::ArgumentNames{{"batteryPercentage"}, {}});
        object->addMethod("setTwentyFourTime", [this](bool enabled) { setTwentyFourTime(enabled); },
                          dbus::ArgumentNames{{"t"}, {}});
        object->addMethod("setBlurSigma",
                          [this](double sigma) { m_statusSettings.setBlurSigma(sigma); },
                          dbus::ArgumentNames{{"sigma"}, {}});
        object->addMethod("setOpacity",
                          [this](double opacity) { m_statusSettings.setOpacity(opacity); },
                          dbus::ArgumentNames{{"opacity"}, {}});
        // The same as the menu's: the user is asked first. The session's
        // own methods act at once.
        object->addMethod("requestLogout", [this] { m_powerActions.request(PowerActions::Logout); });
        object->addMethod("requestReboot", [this] { m_powerActions.request(PowerActions::Reboot); });
        object->addMethod("requestShutdown", [this] { m_powerActions.request(PowerActions::Shutdown); });
        if (!object->publish() || !m_bus->requestName("org.lingmo.Statusbar", dbus::Connection::DoNotQueue)) {
            LM_WARNING(lingmoStatusBar) << "Cannot own org.lingmo.Statusbar";
            return false;
        }
        m_object = std::move(object);
    }
    const bool watcher = m_watcher.registerService();
    const bool registrar = m_registrar.registerService();
    const bool gtkSettings = m_gtkSettings.registerService();
    return watcher && registrar && gtkSettings;
}

void StatusBar::setTwentyFourTime(bool enabled)
{
    m_twentyFourTimeSet = true;
    if (m_twentyFourTime == enabled)
        return;
    m_twentyFourTime = enabled;
    writeSettings(m_settingsPath, [enabled](base::Settings &settings) {
        settings.setValue("Statusbar/TwentyFourTime", enabled);
    });
    updateTimeFormat();
}

void StatusBar::reloadLanguage()
{
    // The session language lives in lingmo-services' settings.
    m_language = runOnFileThread([] { return base::DesktopEntry::displayLanguage(); },
                                 [this](std::string language) {
                                     if (language == m_timeLocale)
                                         return;
                                     m_timeLocale = language;
                                     updateTimeFormat();
                                 });
}

void StatusBar::updateTimeFormat()
{
    m_timeFormat = ClockFormat::pattern(m_timeLocale, m_twentyFourTime);
    timeFormatChanged();
}

} // namespace Lingmo::Shell
