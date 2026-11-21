// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/display/brightness.h"

#include <algorithm>
#include <cstdint>

using namespace Lingmo::base;

namespace settings {

static constexpr const char* kService = "org.lingmo.Services";
static constexpr const char* kPath = "/org/lingmo/Services/Brightness";
static constexpr const char* kInterface = "org.lingmo.Services.Brightness";

Brightness::Brightness()
    : m_service(dbus::Connection::session(), kService, kPath, kInterface)
{
    m_matches.push_back(m_service.onSignal("displaysChanged", [this](const dbus::Message&) { refresh(); }));
    m_matches.push_back(m_service.onSignal("displayBrightnessChanged", [this](const dbus::Message& message) {
        std::string display;
        int32_t value = 0;
        if (message.read(display, value))
            handleValueChanged(display, value);
    }));
}

void Brightness::setDisplay(const std::string& display)
{
    if (m_display == display)
        return;
    m_display = display;
    refresh();
}

void Brightness::setValue(int value)
{
    if (m_available)
        m_service.send("setDisplayBrightness", m_display, int32_t(value));
}

// Read at once, as the row is shown or not from the page's first frame.
void Brightness::refresh()
{
    const auto displays = m_service.call("displays").value<std::vector<std::string>>();
    const bool available = !m_display.empty() && displays
                           && std::find(displays->begin(), displays->end(), m_display) != displays->end();
    if (available) {
        if (const auto value = m_service.call("displayBrightness", m_display).value<int32_t>())
            handleValueChanged(m_display, *value);
    }
    if (m_available != available) {
        m_available = available;
        changed.emit();
    }
}

void Brightness::handleValueChanged(const std::string& display, int value)
{
    if (display != m_display || value < 0 || m_value == value)
        return;
    m_value = value;
    changed.emit();
}

} // namespace settings
