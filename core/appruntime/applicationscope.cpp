// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applicationscope.h"

#include "base/dbus.h"
#include "base/logging.h"
#include "common/detached_call.h"

#include <cstdio>
#include <string>
#include <tuple>
#include <vector>

#include <signal.h>

namespace dbus = Lingmo::Base::DBus;

static const Lingmo::Base::LogCategory kLog("lingmo.appruntime");

using UnitProperty = std::tuple<std::string, dbus::Variant>;
using AuxiliaryUnit = std::tuple<std::string, std::vector<UnitProperty>>;

// systemd unit name escaping: [a-zA-Z0-9:_.] pass, a leading '.' and
// everything else become \xNN of the UTF-8 byte.
static std::string escape(const std::string &text)
{
    std::string result;
    bool first = true;
    for (const char byte : text) {
        const unsigned char c = static_cast<unsigned char>(byte);
        const bool plain = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9') || c == ':' || c == '_'
            || (c == '.' && !first);
        if (plain) {
            result += char(c);
        } else {
            char escaped[5];
            std::snprintf(escaped, sizeof(escaped), "\\x%02x", c);
            result += escaped;
        }
        first = false;
    }
    return result;
}

static dbus::Message managerCall(dbus::Connection *bus, const char *method)
{
    return bus->newMethodCall("org.freedesktop.systemd1",
                              "/org/freedesktop/systemd1",
                              "org.freedesktop.systemd1.Manager", method);
}

std::string ApplicationScope::unitName(const std::string &appId, uint32_t pid)
{
    return "app-lingmo-" + escape(appId) + "-" + std::to_string(pid) + ".scope";
}

void ApplicationScope::adopt(const std::string &appId, uint32_t pid)
{
    if (appId.empty() || pid == 0)
        return;
    dbus::Connection *bus = dbus::Connection::session();
    if (!bus)
        return;

    const std::vector<UnitProperty> properties{
        {"PIDs",        dbus::Variant(std::vector<uint32_t>{pid})},
        {"CollectMode", dbus::Variant(std::string("inactive-or-failed"))},
    };

    dbus::Message call = managerCall(bus, "StartTransientUnit");
    call.append(unitName(appId, pid), std::string("fail"),
                properties, std::vector<AuxiliaryUnit>());

    Lingmo::Core::callDetached(bus, call, [appId, pid](const dbus::Reply &reply) {
        if (reply.isError())
            LM_WARNING(kLog) << "AppRuntime: no systemd scope for"
                             << appId << pid << reply.error().message;
    });
}

void ApplicationScope::killAll(const std::string &unitName)
{
    dbus::Connection *bus = dbus::Connection::session();
    if (!bus)
        return;
    dbus::Message call = managerCall(bus, "KillUnit");
    call.append(unitName, std::string("all"), int32_t(SIGKILL));
    bus->send(call);
}
