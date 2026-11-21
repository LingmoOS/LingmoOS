#include "settingsportal.h"

#include "base/logging.h"
#include "base/settings.h"
#include "base/strings.h"

#include <systemd/sd-bus.h>

#include <algorithm>
#include <chrono>
#include <map>
#include <vector>

using namespace Lingmo;
using base::dbus::MethodCall;
using base::dbus::Variant;
using base::dbus::VariantMap;

static const std::string AppearanceService = "org.lingmo.Services";
static const std::string AppearancePath = "/org/lingmo/Services/Appearance";
static const std::string AppearanceInterface = "org.lingmo.Services.Appearance";
static const std::string AppearanceNamespace = "org.freedesktop.appearance";
static const std::string ColorSchemeKey = "color-scheme";

static bool matchesNamespace(const std::string &pattern)
{
    return pattern.empty() || pattern == "*" || pattern == AppearanceNamespace
            || (base::strings::endsWith(pattern, ".*")
                && base::strings::startsWith(AppearanceNamespace, pattern.substr(0, pattern.size() - 1)));
}

SettingsPortal::SettingsPortal(base::dbus::Connection *bus, const std::string &servicesConfig)
    : m_bus(bus)
    , m_object(bus, "/org/freedesktop/portal/desktop", "org.freedesktop.impl.portal.Settings")
    , m_appearance(bus, AppearanceService, AppearancePath, AppearanceInterface)
{
    {
        const base::Settings settings(servicesConfig);
        m_colorScheme = settings.boolValue("Appearance/DarkMode", false) ? 1u : 2u;
    }

    m_object.addProperty("version", [this] { return version(); });
    m_object.addMethod("ReadAll", "as", "a{sa{sv}}", [this](MethodCall &call) { readAll(call); },
                       {{"namespaces"}, {"value"}});
    m_object.addMethod("Read", "ss", "v", [this](MethodCall &call) { read(call); }, {{"nameSpace", "key"}, {"value"}});
    m_object.addSignal("SettingChanged", "ssv", {"nameSpace", "key", "value"});

    m_darkModeChanged = m_appearance.onSignal("darkModeChanged", [this](const base::dbus::Message &message) {
        bool darkMode = false;
        if (message.read(darkMode))
            setDarkMode(darkMode);
    });

    // Called back at once with the current owner, then on every change.
    m_appearanceOwner = m_bus->watchName(AppearanceService, [this](const std::string &owner) {
        if (!owner.empty())
            refreshAppearance();
    });
}

bool SettingsPortal::publish()
{
    return m_object.publish();
}

void SettingsPortal::readAll(MethodCall &call) const
{
    std::vector<std::string> namespaces;
    call.read(namespaces);

    std::map<std::string, VariantMap> result;
    if (namespaces.empty() || std::any_of(namespaces.cbegin(), namespaces.cend(), matchesNamespace))
        result[AppearanceNamespace] = {{ColorSchemeKey, Variant(m_colorScheme)}};
    call.reply(result);
}

void SettingsPortal::read(MethodCall &call) const
{
    std::string nameSpace;
    std::string key;
    call.read(nameSpace, key);

    if (nameSpace != AppearanceNamespace || key != ColorSchemeKey) {
        call.replyError("org.freedesktop.portal.Error.NotFound", "Unknown setting: " + nameSpace + "/" + key);
        return;
    }
    call.reply(Variant(m_colorScheme));
}

void SettingsPortal::setDarkMode(bool darkMode)
{
    const uint32_t colorScheme = darkMode ? 1u : 2u;
    if (m_colorScheme == colorScheme)
        return;

    m_colorScheme = colorScheme;
    m_object.emitSignal("SettingChanged", AppearanceNamespace, ColorSchemeKey, Variant(m_colorScheme));
}

void SettingsPortal::refreshAppearance()
{
    base::dbus::Message message =
            m_bus->newMethodCall(AppearanceService, AppearancePath, "org.freedesktop.DBus.Properties", "Get");
    message.append(AppearanceInterface, std::string("isDarkMode"));
    sd_bus_message_set_auto_start(message.get(), 0);

    // The live property can be newer than the settings file when a change is pending on disk.
    m_refresh = m_bus->callAsync(
            message,
            [this](const base::dbus::Reply &reply) {
                const std::optional<Variant> value = reply.value<Variant>();
                if (value && value->signature() == "b")
                    setDarkMode(value->value<bool>());
            },
            std::chrono::milliseconds(1000));
}
