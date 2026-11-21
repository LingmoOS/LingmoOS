
#include <map>
#include <string>
#pragma once

#include "base/dbus.h"
#include "base/file_watcher.h"
#include "base/timer.h"

// Follows org.gnome.system.proxy, which Lingmo Settings and third-party proxy clients write,
// and republishes it to the environment of D-Bus and systemd activated programs.
// Applications started by AppRuntime read the keyfile themselves at launch.
class ProxyService
{
public:
    explicit ProxyService(Lingmo::Base::DBus::Connection *session);
    ~ProxyService();

    ProxyService(const ProxyService &) = delete;
    ProxyService &operator=(const ProxyService &) = delete;

private:
    void reload();
    void publishEnvironment();

    Lingmo::Base::DBus::Connection *m_session;
    Lingmo::Base::FileWatch m_watch;
    Lingmo::Base::Timer m_reloadTimer;
    std::map<std::string, std::string> m_environment;
    bool m_environmentPublished = false;
};

