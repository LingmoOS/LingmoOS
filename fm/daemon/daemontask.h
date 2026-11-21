// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/dbus.h"
#include "base/signal.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::Files {
class Task;
struct TaskRequest;
}

class ArchiveJob;

// One task on the bus, org.lingmo.FileOperations.Task1: a file operation
// of the files library, or a compression or extraction.
class DaemonTask
{
public:
    DaemonTask(Lingmo::base::dbus::Connection *bus, std::string objectPath, Lingmo::Files::Task *task);
    // operation 0 compresses, 1 extracts.
    DaemonTask(Lingmo::base::dbus::Connection *bus, std::string objectPath, int operation,
               std::vector<std::string> sources, std::string outputPath);
    ~DaemonTask();

    DaemonTask(const DaemonTask &) = delete;
    DaemonTask &operator=(const DaemonTask &) = delete;

    bool publish();
    const std::string &objectPath() const { return m_objectPath; }
    bool isArchive() const { return m_archive != nullptr; }
    void setOwner(const std::string &owner);
    std::string owner() const;
    // The submitter left the bus. A task waiting at a conflict or for a
    // password is cancelled; one still running is cancelled at its next
    // conflict or password request unless a client adopts it first.
    void orphan();
    bool isFinished() const { return m_finished; }
    bool waitingForConflict() const;
    bool waitingForPassword() const;
    // Whether GetSnapshot was called since the task finished: the client saw
    // the outcome, so the object can leave the bus.
    bool outcomeRead() const { return m_outcomeRead; }
    void setOutcomeReadCallback(std::function<void()> callback) { m_outcomeReadCallback = std::move(callback); }

    void cancel();

    Lingmo::base::Signal<> finished;

private:
    void initialize();
    void addMembers();
    void changed(const Lingmo::base::dbus::VariantMap &update);
    void passwordSettled();
    Lingmo::base::dbus::VariantMap snapshot();

    std::string m_objectPath;
    Lingmo::Files::Task *m_task = nullptr;
    std::unique_ptr<ArchiveJob> m_archive;
    Lingmo::base::dbus::Object m_object;
    Lingmo::base::dbus::VariantMap m_snapshot;
    bool m_finished = false;
    bool m_outcomeRead = false;
    std::function<void()> m_outcomeReadCallback;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};
