// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/hotcorners/hotcorners.h"

#include <cstdint>
#include <cstdio>

#include "base/i18n.h"

using namespace Lingmo::base;
using Lingmo::i18n::trc;

namespace settings {

static constexpr const char* kService = "org.lingmo.HotCorners1";
static constexpr const char* kPath = "/org/lingmo/HotCorners1";

// The compositor's modifier bits, as in the shortcuts service.
static constexpr int kShiftDown = 1 << 0;
static constexpr int kControlDown = 1 << 1;
static constexpr int kAltDown = 1 << 2;
static constexpr int kMetaDown = 1 << 3;

// topLeft, topLeftModifiers and so on.
static std::string propertyName(const std::string& corner, const char* suffix = "")
{
    std::string name;
    bool upper = false;
    for (const char c : corner) {
        if (c == '-') {
            upper = true;
            continue;
        }
        name += upper ? char(c - 'a' + 'A') : c;
        upper = false;
    }
    return name + suffix;
}

HotCorners::HotCorners()
    : m_corners(dbus::Connection::session(), kService, kPath, kService)
{
    dbus::Connection* bus = m_corners.connection();
    if (!bus)
        return;
    // Read at once, so the page has the values for its first frame.
    if (const dbus::Reply reply = m_corners.getAll(); !reply.isError()) {
        m_properties = reply.value<dbus::VariantMap>().value_or(dbus::VariantMap());
        m_available = true;
    }
    const dbus::Proxy shortcuts(bus, "org.lingmo.Shortcuts1", "/org/lingmo/Shortcuts1", "org.lingmo.Shortcuts1");
    m_commandKeys = shortcuts.property<std::string>("shortcutConvention").value_or(std::string()) == "command";
    m_changedMatch = m_corners.onSignal("changed", [this](const dbus::Message&) { reload(); });
    m_watch = bus->watchName(kService, [this](const std::string& owner) {
        if (!owner.empty()) {
            reload();
        } else if (m_available) {
            m_available = false;
            m_properties.clear();
            changed.emit();
        }
    });
}

void HotCorners::reload()
{
    m_reload = m_corners.getAllAsync([this](const dbus::Reply& reply) {
        if (reply.isError())
            return;
        m_properties = reply.value<dbus::VariantMap>().value_or(dbus::VariantMap());
        m_available = true;
        changed.emit();
    });
}

std::string HotCorners::action(const std::string& corner) const
{
    const auto it = m_properties.find(propertyName(corner));
    return it == m_properties.end() ? std::string() : it->second.value<std::string>();
}

int HotCorners::modifiers(const std::string& corner) const
{
    const auto it = m_properties.find(propertyName(corner, "Modifiers"));
    return it == m_properties.end() ? 0 : it->second.value<int32_t>(0);
}

void HotCorners::setCorner(const std::string& corner, const std::string& action, int modifiers)
{
    if (!m_available)
        return;
    m_calls[corner] = m_corners.callAsync("setCorner", [this](const dbus::Reply& reply) {
        if (reply.isError()) {
            fprintf(stderr, "Cannot update hot corner: %s\n", reply.error().message.c_str());
            changed.emit();
        }
    }, corner, action, int32_t(modifiers));
}

int HotCorners::heldModifiers() const
{
    if (!m_available)
        return 0;
    return m_corners.call("heldModifiers").value<int32_t>().value_or(0);
}

std::string HotCorners::modifierText(int modifiers) const
{
    // Under the Command convention Meta is the Command key, Alt is Option.
    std::string text;
    const auto add = [&text](const char* part) {
        if (!text.empty())
            text += " + ";
        text += part;
    };
    if (modifiers & kControlDown)
        add(m_commandKeys ? trc("key", "Control") : trc("key", "Ctrl"));
    if (modifiers & kAltDown)
        add(m_commandKeys ? trc("key", "Option") : trc("key", "Alt"));
    if (modifiers & kShiftDown)
        add(trc("key", "Shift"));
    if (modifiers & kMetaDown)
        add(m_commandKeys ? trc("key", "Command") : trc("key", "Meta"));
    return text;
}

} // namespace settings
