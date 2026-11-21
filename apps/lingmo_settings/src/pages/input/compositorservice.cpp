// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/input/compositorservice.h"

#include <algorithm>
#include <cstdio>

#include "base/event_loop.h"

using namespace Lingmo::base;

namespace settings {

static std::string pathFor(std::string name)
{
    std::replace(name.begin(), name.end(), '.', '/');
    return "/" + name;
}

CompositorService::CompositorService(std::string name)
    : m_proxy(dbus::Connection::session(), name, pathFor(name), name)
{
    dbus::Connection* bus = m_proxy.connection();
    if (!bus)
        return;
    if (const dbus::Reply reply = m_proxy.getAll(); !reply.isError()) {
        m_properties = reply.value<dbus::VariantMap>().value_or(dbus::VariantMap());
        m_available = true;
    }
    // Every signal of the interface tells that a property changed.
    m_signals = bus->subscribe({.sender = name, .path = m_proxy.path(), .interface = name},
                               [this](const dbus::Message&) { reload(); });
    m_watch = bus->watchName(name, [this](const std::string& owner) {
        if (!owner.empty()) {
            reload();
        } else if (m_available) {
            m_available = false;
            m_properties.clear();
            m_reload.reset();
            changed.emit();
        }
    });
}

CompositorService::~CompositorService() = default;

void CompositorService::reload()
{
    m_reload = m_proxy.getAllAsync([this](const dbus::Reply& reply) {
        if (reply.isError())
            return;
        m_properties = reply.value<dbus::VariantMap>().value_or(dbus::VariantMap());
        m_available = true;
        changed.emit();
    });
}

void CompositorService::finishCall(uint64_t id, const std::string& method, const dbus::Reply& reply)
{
    if (reply.isError()) {
        fprintf(stderr, "Cannot call %s.%s: %s\n", m_proxy.interface().c_str(), method.c_str(),
                reply.error().message.c_str());
        reload();
    }
    // Not from inside the call's own callback, which the pending call owns.
    EventLoop::current()->post([this, id, alive = std::weak_ptr<int>(m_alive)] {
        if (alive.lock())
            m_calls.erase(id);
    });
}

const dbus::Variant* CompositorService::find(const std::string& name) const
{
    const auto it = m_properties.find(name);
    return it == m_properties.end() ? nullptr : &it->second;
}

bool CompositorService::boolValue(const std::string& name, bool fallback) const
{
    const dbus::Variant* value = find(name);
    return value ? value->value<bool>(fallback) : fallback;
}

int CompositorService::intValue(const std::string& name, int fallback) const
{
    const dbus::Variant* value = find(name);
    if (!value)
        return fallback;
    if (const auto number = value->get<int32_t>())
        return *number;
    if (const auto number = value->get<uint32_t>())
        return int(*number);
    if (const auto number = value->get<double>())
        return int(*number);
    return fallback;
}

double CompositorService::doubleValue(const std::string& name, double fallback) const
{
    const dbus::Variant* value = find(name);
    if (!value)
        return fallback;
    if (const auto number = value->get<double>())
        return *number;
    if (const auto number = value->get<int32_t>())
        return *number;
    return fallback;
}

std::string CompositorService::stringValue(const std::string& name, const std::string& fallback) const
{
    const dbus::Variant* value = find(name);
    return value ? value->value<std::string>(fallback) : fallback;
}

std::vector<std::string> CompositorService::stringListValue(const std::string& name) const
{
    const dbus::Variant* value = find(name);
    return value ? value->value<std::vector<std::string>>({}) : std::vector<std::string>();
}

} // namespace settings
