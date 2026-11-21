// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>
#include <string>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/thread_pool.h"
#include "shell/statusbar/app_menu_registrar.h"
#include "shell/statusbar/battery.h"
#include "shell/statusbar/brightness.h"
#include "shell/statusbar/gtk_settings.h"
#include "shell/statusbar/mpris.h"
#include "shell/statusbar/power_actions.h"
#include "shell/statusbar/status_bar_settings.h"
#include "shell/statusbar/status_notifier_watcher.h"
#include "shell/statusbar/system_tray_model.h"

namespace Lingmo::powerprofiles {
class PowerProfiles;
} // namespace Lingmo::powerprofiles

namespace Lingmo::Shell {

class TaskManager;

// The status bar without its views: the tray and its watcher, the AppMenu
// registrar, the media player, battery, brightness and power profiles, the
// power actions, the clock's format, and org.lingmo.Statusbar at
// /Statusbar.
class StatusBar
{
public:
    // `settingsPath` is shell.conf; `tasks` may be null.
    StatusBar(base::dbus::Connection *sessionBus, base::dbus::Connection *systemBus, TaskManager *tasks,
              std::string settingsPath);
    ~StatusBar();

    StatusBar(const StatusBar &) = delete;
    StatusBar &operator=(const StatusBar &) = delete;

    // org.lingmo.Statusbar, org.kde.StatusNotifierWatcher and
    // com.canonical.AppMenu.Registrar; false when one is taken.
    bool registerServices();

    // The bar's [Statusbar] group: the blur behind it and its opacity.
    StatusBarSettings &statusSettings() { return m_statusSettings; }

    SystemTrayModel &systemTray() { return m_tray; }
    AppMenuRegistrar &appMenuRegistrar() { return m_registrar; }
    MprisManager &mpris() { return m_mpris; }
    Battery &battery() { return *m_battery; }
    Brightness &brightness() { return m_brightness; }
    powerprofiles::PowerProfiles &powerProfiles() { return *m_powerProfiles; }
    PowerActions &powerActions() { return m_powerActions; }
    base::dbus::Connection *sessionBus() const { return m_bus; }

    // The clock follows the primary language, not LC_TIME: the region the
    // system was installed with would otherwise pick its words.
    const std::string &timeLocale() const { return m_timeLocale; }
    // A format string for base::Locale(timeLocale()).toString().
    const std::string &timeFormat() const { return m_timeFormat; }
    bool twentyFourTime() const { return m_twentyFourTime; }
    void setTwentyFourTime(bool enabled);

    base::Signal<> timeFormatChanged;

private:
    void reloadLanguage();
    void updateTimeFormat();

    base::dbus::Connection *m_bus;
    std::string m_settingsPath;
    StatusBarSettings m_statusSettings;
    StatusNotifierWatcher m_watcher;
    SystemTrayModel m_tray;
    AppMenuRegistrar m_registrar;
    GtkSettings m_gtkSettings;
    MprisManager m_mpris;
    std::unique_ptr<Battery> m_battery;
    Brightness m_brightness;
    std::unique_ptr<powerprofiles::PowerProfiles> m_powerProfiles;
    PowerActions m_powerActions;
    std::unique_ptr<base::dbus::Object> m_object;
    base::dbus::Match m_languageChanged;
    base::Job m_load;
    base::Job m_language;
    bool m_twentyFourTime = false;
    bool m_twentyFourTimeSet = false;
    std::string m_timeLocale;
    std::string m_timeFormat;
};

} // namespace Lingmo::Shell

