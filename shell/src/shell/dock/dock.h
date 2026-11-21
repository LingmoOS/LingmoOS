// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <memory>
#include <string>

#include "base/dbus.h"
#include "base/geometry.h"
#include "base/signal.h"
#include "shell/dock/dock_model.h"
#include "shell/dock/dock_settings.h"
#include "shell/dock/minimized_windows.h"
#include "shell/dock/trash_monitor.h"

namespace Lingmo::Shell {

// The dock as the rest of the session sees it, without its view: its
// applications, minimized windows, trash, settings, and org.lingmo.Dock at
// /Dock, through which Settings changes the dock's settings.
class Dock
{
public:
    // `settingsPath` is shell.conf. Everything must outlive the dock.
    Dock(base::dbus::Connection *bus, TaskManager &tasks, ApplicationLauncher &launcher, std::string settingsPath,
         std::unique_ptr<TrashMonitor> trash = {});
    ~Dock();

    Dock(const Dock &) = delete;
    Dock &operator=(const Dock &) = delete;

    bool registerService();

    DockModel &applications() { return m_applications; }
    DockSettings &settings() { return m_settings; }
    MinimizedWindows &minimizedWindows() { return m_minimizedWindows; }
    TrashMonitor &trash() { return *m_trash; }

    // The view's frame on the primary output, for the primaryGeometry
    // property; empty without a view.
    std::function<base::Rect()> primaryGeometry;
    // The view moved or resized.
    void notifyPrimaryGeometryChanged();
    // For the launcher, which leaves the dock its room.
    base::Signal<> primaryGeometryChanged;

private:
    base::dbus::Connection *m_bus;
    DockSettings m_settings;
    DockModel m_applications;
    MinimizedWindows m_minimizedWindows;
    std::unique_ptr<TrashMonitor> m_trash;
    std::unique_ptr<base::dbus::Object> m_object;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

