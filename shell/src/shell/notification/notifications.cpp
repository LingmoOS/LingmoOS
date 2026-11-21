// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notification/notifications.h"

#include "base/logging.h"
#include "core/preferences.h"
#include "shell/common/file_thread.h"

namespace Lingmo::Shell {

namespace dbus = base::dbus;

static const base::LogCategory lingmoNotificationService("lingmo.shell.notifications.service");

Notifications::Files Notifications::Files::defaults()
{
    return {preferencesFile("shell"), NotificationHistory::defaultPath(), notification_images::defaultDirectory()};
}

Notifications::Notifications(dbus::Connection *bus, Applications &applications, TaskManager *tasks, Files files)
    : m_bus(bus)
    , m_settings(applications, files.settings)
    , m_server(bus, m_settings, applications, tasks, files.images)
    , m_history(files.history, files.images)
    , m_banners(m_server, m_settings, m_history)
{
    m_subscriptions.push_back(m_banners.soundRequested.connect([this](const std::string &event) {
        dbus::Proxy(m_bus, "org.lingmo.Services", "/org/lingmo/Services/Sound",
                    "org.lingmo.Services.Sound").send("playEvent", event);
    }));
    m_subscriptions.push_back(m_settings.doNotDisturbChanged.connect([this] {
        if (m_object)
            m_object->emitSignal("doNotDisturbChanged");
    }));
    m_subscriptions.push_back(m_settings.applicationsChanged.connect([this] {
        if (m_object)
            m_object->emitSignal("applicationsChanged");
    }));
}

Notifications::~Notifications()
{
    if (m_object && m_bus)
        m_bus->releaseName("org.lingmo.Notification");
    // The last history and settings writes reach the disk.
    waitForFileThread();
}

bool Notifications::registerServices()
{
    if (!m_server.registerService())
        return false;
    if (m_object)
        return true;

    auto object = std::make_unique<dbus::Object>(m_bus, "/Notification", "org.lingmo.Notification");
    object->addMethod("setDoNotDisturb", [this](bool enabled) { m_settings.setDoNotDisturb(enabled); },
                      dbus::ArgumentNames{{"enabled"}, {}});
    object->addMethod("applications", [this] {
        std::vector<dbus::VariantMap> list;
        for (const NotificationSettings::Application &application : m_settings.applications()) {
            list.push_back({{"key", dbus::Variant(application.key)},
                            {"name", dbus::Variant(application.name)},
                            {"icon", dbus::Variant(application.icon)},
                            {"mode", dbus::Variant(int32_t(application.mode))}});
        }
        return list;
    });
    object->addMethod("setApplicationMode",
                      [this](const std::string &key, int32_t mode) { m_settings.setApplicationMode(key, mode); },
                      dbus::ArgumentNames{{"key", "mode"}, {}});
    object->addSignal("doNotDisturbChanged", "");
    object->addSignal("applicationsChanged", "");
    object->addProperty("doNotDisturb", [this] { return m_settings.doNotDisturb(); });
    if (!object->publish()) {
        LM_WARNING(lingmoNotificationService) << "Cannot export /Notification";
        return false;
    }
    if (!m_bus->requestName("org.lingmo.Notification", dbus::Connection::DoNotQueue)) {
        LM_WARNING(lingmoNotificationService) << "Cannot own org.lingmo.Notification";
        return false;
    }
    m_object = std::move(object);
    return true;
}

} // namespace Lingmo::Shell
