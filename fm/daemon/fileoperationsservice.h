// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/dbus.h"
#include "base/signal.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::Files {
class Task;
}

class DaemonTask;

// org.lingmo.FileOperations.Manager1 at /org/lingmo/FileOperations: runs
// the file manager's operations so they outlive its windows, with one undo
// history for every client.
class FileOperationsService
{
public:
    explicit FileOperationsService(Lingmo::base::dbus::Connection *bus);
    ~FileOperationsService();

    bool publish();

private:
    void submit(Lingmo::base::dbus::MethodCall &call);
    void submitArchive(Lingmo::base::dbus::MethodCall &call);
    void history(Lingmo::base::dbus::MethodCall &call, bool undo);
    std::string nextPath();
    // Registers the task and announces it; false when it could not go on the bus.
    bool announce(std::unique_ptr<DaemonTask> task, const std::string &owner);
    void watchOwner(const std::string &owner);
    void ownerLost(const std::string &owner);
    void retire(DaemonTask *task);
    void remove(const std::string &path);

    Lingmo::base::dbus::Connection *m_bus;
    Lingmo::base::dbus::Object m_object;
    uint64_t m_nextTaskId = 1;
    std::vector<std::string> m_taskPaths;
    std::vector<std::unique_ptr<DaemonTask>> m_tasks;
    std::map<std::string, Lingmo::base::dbus::NameWatch> m_owners;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    std::map<DaemonTask *, Lingmo::base::Subscription> m_finishedSubscriptions;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};
