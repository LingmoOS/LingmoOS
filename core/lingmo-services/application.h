// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include "base/settings.h"
#include "base/signal.h"

class BatteryWarning;
class BrightnessManager;
class Clipboard;
class Connectivity;
class CrashManager;
class Language;
class NightLight;
class PermissionManager;
class PowerManager;
class PowerSupply;
class SoundService;
class ProxyService;
class ThemeManager;
class UpdateManager;
class WaylandIdle;

namespace Lingmo::base::dbus {
class Connection;
}

// The session's services, on the session bus as org.lingmo.Services.
class ServicesApplication
{
public:
    ServicesApplication();
    ~ServicesApplication();

    ServicesApplication(const ServicesApplication &) = delete;
    ServicesApplication &operator=(const ServicesApplication &) = delete;

    void initTrash();

private:
    // This process's dialogs follow the appearance it keeps itself.
    void applyTheme();

    Lingmo::Base::DBus::Connection *m_session;
    Lingmo::Base::DBus::Connection *m_system;
    // ~/.config/lingmo/services.conf, which every part shares.
    Lingmo::Base::Settings m_settings;
    std::unique_ptr<WaylandIdle> m_idle;
    std::unique_ptr<ThemeManager> m_themeManager;
    std::unique_ptr<BrightnessManager> m_brightnessManager;
    std::unique_ptr<PowerSupply> m_powerSupply;
    std::unique_ptr<BatteryWarning> m_batteryWarning;
    std::unique_ptr<Language> m_language;
    std::unique_ptr<Clipboard> m_clipboard;
    std::unique_ptr<PowerManager> m_powerManager;
    std::unique_ptr<NightLight> m_nightLight;
    std::unique_ptr<PermissionManager> m_permissionManager;
    std::unique_ptr<CrashManager> m_crashManager;
    std::unique_ptr<ProxyService> m_proxyService;
    std::unique_ptr<UpdateManager> m_updateManager;
    std::unique_ptr<SoundService> m_soundService;
    std::unique_ptr<Connectivity> m_connectivity;
    Lingmo::Base::Subscription m_themeChanged;
};

