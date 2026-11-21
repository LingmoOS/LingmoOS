// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/runtime_client.h"

#include "common/detached_process.h"
#include "common/language_environment.h"
#include "common/proxy_settings.h"

#include "base/dbus.h"

#include <chrono>

namespace Lingmo::Core::runtime {

static constexpr char kService[] = "org.lingmo.AppRuntime";
static constexpr char kPath[] = "/AppRuntime";

static base::dbus::Proxy appRuntime()
{
    base::dbus::Proxy proxy(base::dbus::Connection::session(), kService, kPath, kService);
    // A runtime that stops answering must not hold its caller for the bus's half minute.
    proxy.setTimeout(std::chrono::seconds(5));
    return proxy;
}

Environment applicationEnvironment(const std::string& activationToken)
{
    // A WAYLAND_SOCKET this process inherited is its own connection, and may
    // be a privileged one; nothing it launches may share it.
    Environment environment = processEnvironment();
    environment.erase("WAYLAND_SOCKET");
    environment.erase("XDG_ACTIVATION_TOKEN");
    if (!activationToken.empty())
        environment["XDG_ACTIVATION_TOKEN"] = activationToken;
    language::apply(environment);
    proxy::apply(environment);
    return environment;
}

bool launchCommand(const std::vector<std::string>& command, const std::string& workingDirectory,
                   const std::string& appId)
{
    if (command.empty() || command.front().empty())
        return false;

    const base::dbus::Reply reply = appRuntime().call("launchCommand", appId, command, workingDirectory, std::string());
    if (reply.value<uint32_t>().value_or(0) != 0)
        return true;

    return startDetached(command, workingDirectory, applicationEnvironment()) > 0;
}

bool launchApplication(const std::string& appId, const std::vector<std::string>& arguments)
{
    if (appId.empty())
        return false;

    const base::dbus::Reply reply = appRuntime().call("launchApplication", appId, arguments, std::string());
    return reply.value<uint32_t>().value_or(0) != 0;
}

bool quitApplication(const std::string& appId)
{
    if (appId.empty())
        return false;

    return appRuntime().call("quitApplication", appId).value<bool>().value_or(false);
}

bool quitAll()
{
    return appRuntime().call("quitAll").value<bool>().value_or(false);
}

std::vector<std::string> runningApplications()
{
    return appRuntime().call("runningApplications").value<std::vector<std::string>>().value_or(std::vector<std::string>());
}

} // namespace Lingmo::Core::runtime
