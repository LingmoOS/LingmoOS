#include "proxyservice.h"

#include <filesystem>

#include "common/gsettings_keyfile.h"
#include "common/proxy_settings.h"

using namespace std::chrono_literals;
using namespace Lingmo::Base;
namespace proxy = Lingmo::Core::proxy;

ProxyService::ProxyService(dbus::Connection *session)
    : m_session(session)
{
    // gsettings and atomic writers both replace the file by rename; the
    // directory is what is watched.
    m_reloadTimer.setSingleShot(true);
    m_reloadTimer.setInterval(100ms);
    m_reloadTimer.onTimeout = [this] {
        reload();
    };

    const std::string path = Lingmo::Core::gsettings::path();
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path(), error);
    m_watch = FileWatch::file(path, [this] {
        m_reloadTimer.start();
    });
    reload();
}

ProxyService::~ProxyService() = default;

void ProxyService::reload()
{
    std::map<std::string, std::string> variables =
            proxy::variables(Lingmo::Core::ProxySettings::load());
    // The first pass always publishes: systemd keeps what an earlier login exported.
    if (m_environmentPublished && variables == m_environment)
        return;
    m_environmentPublished = true;
    m_environment = std::move(variables);

    publishEnvironment();
}

void ProxyService::publishEnvironment()
{
    if (!m_session)
        return;

    std::vector<std::string> unset;
    std::vector<std::string> assignments;
    std::map<std::string, std::string> activation;
    for (const std::string &name : proxy::names()) {
        const auto it = m_environment.find(name);
        if (it != m_environment.end())
            assignments.push_back(name + '=' + it->second);
        else
            unset.push_back(name);
        // dbus-daemon cannot unset; an empty proxy variable means none to every reader.
        activation[name] = it != m_environment.end() ? it->second : std::string();
    }

    const dbus::Proxy systemd(m_session, "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
                              "org.freedesktop.systemd1.Manager");
    systemd.send("UnsetAndSetEnvironment", unset, assignments);

    const dbus::Proxy bus(m_session, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
    bus.send("UpdateActivationEnvironment", activation);
}
