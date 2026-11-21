// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/dock/dock.h"

#include "base/logging.h"
#include "shell/common/file_thread.h"
#include "shell/common/task_manager.h"

namespace Lingmo::Shell {

namespace dbus = base::dbus;

static const base::LogCategory lingmoDock("lingmo.shell.dock");

Dock::Dock(dbus::Connection *bus, TaskManager &tasks, ApplicationLauncher &launcher, std::string settingsPath,
           std::unique_ptr<TrashMonitor> trash)
    : m_bus(bus)
    , m_settings(settingsPath)
    , m_applications(tasks, launcher, settingsPath)
    , m_minimizedWindows(tasks.taskWindows(), m_applications, m_settings)
    , m_trash(trash ? std::move(trash) : std::make_unique<TrashMonitor>())
{
}

Dock::~Dock()
{
    if (m_object && m_bus)
        m_bus->releaseName("org.lingmo.Dock");
    waitForFileThread();
}

void Dock::notifyPrimaryGeometryChanged()
{
    primaryGeometryChanged();
}

bool Dock::registerService()
{
    if (m_object)
        return true;
    if (!m_bus)
        return false;

    auto object = std::make_unique<dbus::Object>(m_bus, "/Dock", "org.lingmo.Dock");
    object->addMethod("setEfficientMode", [this](bool enabled) { m_settings.setEfficientMode(enabled); });
    object->addProperty("efficientMode", [this] { return m_settings.efficientMode(); });
    object->addProperty("taskbarAlignment", [this] { return int32_t(m_settings.taskbarAlignment()); });
    object->addProperty("taskbarSearch", [this] { return int32_t(m_settings.taskbarSearch()); });
    m_subscriptions.push_back(m_settings.efficientModeChanged.connect([this] {
        if (m_object) m_object->emitPropertiesChanged({"efficientMode"});
    }));
    object->addMethod("setDirection",
                      [this](int32_t direction) { m_settings.setDirection(DockSettings::Direction(direction)); },
                      dbus::ArgumentNames{{"direction"}, {}});
    object->addMethod("setIconSize", [this](int32_t iconSize) { m_settings.setIconSize(iconSize); },
                      dbus::ArgumentNames{{"iconSize"}, {}});
    object->addMethod("setTaskbarAlignment", [this](int32_t value) {
        m_settings.setTaskbarAlignment(DockSettings::TaskbarAlignment(value));
    }, dbus::ArgumentNames{{"alignment"}, {}});
    object->addMethod("setTaskbarSearch", [this](int32_t value) {
        m_settings.setTaskbarSearch(DockSettings::TaskbarSearch(value));
    }, dbus::ArgumentNames{{"search"}, {}});
    object->addMethod("setVisibility",
                      [this](int32_t visibility) { m_settings.setVisibility(DockSettings::Visibility(visibility)); },
                      dbus::ArgumentNames{{"visibility"}, {}});
    object->addMethod("setGlassTint", [this](double tint) { m_settings.setGlassTint(tint); },
                      dbus::ArgumentNames{{"tint"}, {}});
    object->addMethod("setMinimizedArea", [this](bool enabled) { m_settings.setMinimizedArea(enabled); },
                      dbus::ArgumentNames{{"enabled"}, {}});
    object->addMethod("setMagnification", [this](bool enabled) { m_settings.setMagnification(enabled); },
                      dbus::ArgumentNames{{"enabled"}, {}});
    object->addMethod("setMagnificationFactor",
                      [this](double factor) { m_settings.setMagnificationFactor(factor); },
                      dbus::ArgumentNames{{"factor"}, {}});
    object->addMethod("setBounceOnLaunch",
                      [this](bool enabled) { m_settings.setBounceOnLaunch(enabled); },
                      dbus::ArgumentNames{{"enabled"}, {}});
    if (!object->publish() || !m_bus->requestName("org.lingmo.Dock", dbus::Connection::DoNotQueue)) {
        LM_WARNING(lingmoDock) << "Cannot own org.lingmo.Dock";
        return false;
    }
    m_object = std::move(object);
    return true;
}

} // namespace Lingmo::Shell
