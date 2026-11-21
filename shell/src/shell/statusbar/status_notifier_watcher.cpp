// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/status_notifier_watcher.h"

#include <algorithm>

#include <unistd.h>

#include "base/logging.h"

namespace Lingmo::Shell {

namespace dbus = base::dbus;

static const base::LogCategory lingmoTray("lingmo.shell.tray");

static constexpr const char *kService = "org.kde.StatusNotifierWatcher";
static constexpr const char *kPath = "/StatusNotifierWatcher";

StatusNotifierWatcher::StatusNotifierWatcher(dbus::Connection *bus)
    : m_bus(bus)
{
}

StatusNotifierWatcher::~StatusNotifierWatcher()
{
    if (m_object && m_bus) {
        m_bus->releaseName(kService);
        m_bus->releaseName(m_hostName);
    }
}

bool StatusNotifierWatcher::registerService()
{
    if (m_object)
        return true;
    if (!m_bus)
        return false;

    auto object = std::make_unique<dbus::Object>(m_bus, kPath, kService);
    object->addMethod(
            "RegisterStatusNotifierItem", "s", "",
            [this](dbus::MethodCall &call) {
                std::string service;
                call.read(service);
                call.reply();
                registerItem(call.message().sender(), service);
            },
            dbus::ArgumentNames{{"service"}, {}});
    object->addMethod(
            "RegisterStatusNotifierHost", "s", "",
            [this](dbus::MethodCall &call) {
                std::string service;
                call.read(service);
                call.reply();
                registerHost(service);
            },
            dbus::ArgumentNames{{"service"}, {}});
    object->addProperty("RegisteredStatusNotifierItems", [this] { return m_items; });
    object->addProperty("IsStatusNotifierHostRegistered", [this] { return isHostRegistered(); });
    object->addProperty("ProtocolVersion", [] { return int32_t(0); });
    object->addSignal("StatusNotifierItemRegistered", "s");
    object->addSignal("StatusNotifierItemUnregistered", "s");
    object->addSignal("StatusNotifierHostRegistered", "");
    object->addSignal("StatusNotifierHostUnregistered", "");
    if (!object->publish() || !m_bus->requestName(kService, base::dbus::Connection::DoNotQueue)) {
        LM_WARNING(lingmoTray) << "Cannot serve" << kService;
        return false;
    }
    m_object = std::move(object);

    m_hostName = "org.kde.StatusNotifierHost-" + std::to_string(getpid());
    m_bus->requestName(m_hostName);
    m_internalHost = true;
    m_object->emitSignal("StatusNotifierHostRegistered");
    return true;
}

void StatusNotifierWatcher::watch(const std::string &service)
{
    if (m_watches.count(service))
        return;
    m_watches.emplace(service, m_bus->watchName(service, [this, service](const std::string &owner) {
        ownerChanged(service, owner);
    }));
}

void StatusNotifierWatcher::registerItem(const std::string &sender, const std::string &serviceOrPath)
{
    std::string service;
    std::string path;
    if (serviceOrPath.starts_with('/')) {
        service = sender;
        path = serviceOrPath;
    } else {
        service = serviceOrPath;
        path = "/StatusNotifierItem";
    }
    if (service.empty())
        return;
    const std::string id = service + path;
    if (std::find(m_items.begin(), m_items.end(), id) != m_items.end())
        return;

    // Registered only once its service is known to be on the bus.
    std::vector<std::string> &waiting = m_waitingItems[service];
    if (std::find(waiting.begin(), waiting.end(), id) == waiting.end())
        waiting.push_back(id);
    if (m_known.count(service))
        ownerChanged(service, "known");
    else
        watch(service);
}

void StatusNotifierWatcher::registerHost(const std::string &service)
{
    if (service.find("org.kde.StatusNotifierHost-") == std::string::npos || m_hosts.count(service))
        return;
    m_waitingHosts.insert(service);
    if (m_known.count(service))
        ownerChanged(service, "known");
    else
        watch(service);
}

void StatusNotifierWatcher::ownerChanged(const std::string &service, const std::string &owner)
{
    if (owner.empty()) {
        if (m_known.count(service)) {
            serviceUnregistered(service);
            return;
        }
        // Not on the bus: nothing it asked for is kept.
        m_waitingItems.erase(service);
        m_waitingHosts.erase(service);
        m_watches.erase(service);
        return;
    }

    m_known.insert(service);

    const std::vector<std::string> waiting = std::exchange(m_waitingItems[service], {});
    m_waitingItems.erase(service);
    for (const std::string &id : waiting) {
        if (std::find(m_items.begin(), m_items.end(), id) != m_items.end())
            continue;
        LM_DEBUG(lingmoTray) << "Registering" << id << "to the tray";
        m_items.push_back(id);
        if (m_object)
            m_object->emitSignal("StatusNotifierItemRegistered", id);
        itemRegistered(id);
    }

    if (m_waitingHosts.erase(service) && !m_hosts.count(service)) {
        m_hosts.insert(service);
        if (m_object)
            m_object->emitSignal("StatusNotifierHostRegistered");
    }
}

void StatusNotifierWatcher::serviceUnregistered(const std::string &service)
{
    m_known.erase(service);
    m_waitingItems.erase(service);
    m_waitingHosts.erase(service);

    const std::string prefix = service + '/';
    for (auto it = m_items.begin(); it != m_items.end();) {
        if (!it->starts_with(prefix)) {
            ++it;
            continue;
        }
        const std::string id = *it;
        it = m_items.erase(it);
        if (m_object)
            m_object->emitSignal("StatusNotifierItemUnregistered", id);
        itemUnregistered(id);
    }

    if (m_hosts.erase(service) && m_object)
        m_object->emitSignal("StatusNotifierHostUnregistered");

    // Last: the watch is what called.
    m_watches.erase(service);
}

} // namespace Lingmo::Shell
