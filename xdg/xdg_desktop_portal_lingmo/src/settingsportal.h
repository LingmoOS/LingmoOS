#pragma once

#include "base/dbus.h"

#include <cstdint>
#include <string>

class SettingsPortal
{
public:
    // `servicesConfig` is lingmo-services' settings file, read once for
    // the dark mode until the live value arrives.
    SettingsPortal(Lingmo::base::dbus::Connection *bus, const std::string &servicesConfig);

    bool publish();

    uint32_t version() const { return 1; }

private:
    void readAll(Lingmo::base::dbus::MethodCall &call) const;
    void read(Lingmo::base::dbus::MethodCall &call) const;
    void setDarkMode(bool darkMode);
    void refreshAppearance();

    Lingmo::base::dbus::Connection *m_bus;
    Lingmo::base::dbus::Object m_object;
    Lingmo::base::dbus::Proxy m_appearance;
    Lingmo::base::dbus::Match m_darkModeChanged;
    Lingmo::base::dbus::NameWatch m_appearanceOwner;
    Lingmo::base::dbus::PendingCall m_refresh;
    uint32_t m_colorScheme = 2;
};
