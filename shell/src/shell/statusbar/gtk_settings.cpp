// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/gtk_settings.h"

#include "base/logging.h"

namespace Lingmo::Shell {
namespace dbus = base::dbus;

namespace {
const base::LogCategory gtkSettingsLog("lingmo.shell.gtksettings");
constexpr const char *kService = "org.gtk.Settings";
constexpr const char *kPath = "/org/gtk/Settings";
}

GtkSettings::GtkSettings(dbus::Connection *bus)
    : m_bus(bus)
{
}

GtkSettings::~GtkSettings()
{
    if (m_object && m_bus)
        m_bus->releaseName(kService);
}

bool GtkSettings::registerService()
{
    if (m_object)
        return true;
    if (!m_bus)
        return false;
    auto object = std::make_unique<dbus::Object>(m_bus, kPath, kService);
    object->addProperty("Modules", [] { return std::string(kModules); });
    if (!object->publish() || !m_bus->requestName(kService, dbus::Connection::DoNotQueue)) {
        LM_WARNING(gtkSettingsLog) << "Cannot own" << kService;
        return false;
    }
    m_object = std::move(object);
    return true;
}

} // namespace Lingmo::Shell
