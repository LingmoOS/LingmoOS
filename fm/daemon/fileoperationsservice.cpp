// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperationsservice.h"
#include "daemontask.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/timer.h"

#include "lingmo/files/task.h"

#include <algorithm>

using namespace Lingmo;
using base::dbus::ObjectPath;
using Lingmo::i18n::tr;

static void later(std::chrono::milliseconds delay, std::function<void()> action)
{
    auto timer = std::make_shared<base::Timer>();
    timer->setSingleShot(true);
    timer->onTimeout = [timer, action = std::move(action)]() mutable {
        action();
        // The timer goes on the next turn, not from inside its own callback.
        base::EventLoop::current()->post([timer] {});
        timer.reset();
    };
    timer->start(delay);
}

FileOperationsService::FileOperationsService(base::dbus::Connection *bus)
    : m_bus(bus)
    , m_object(bus, "/org/lingmo/FileOperations", "org.lingmo.FileOperations.Manager1")
{
    auto *controller = files::TaskController::instance();
    m_subscriptions.push_back(controller->historyChanged.connect([this, controller] {
        m_object.emitSignal("HistoryChanged", controller->isUndoAvailable(), controller->isRedoAvailable());
    }));

    m_object.addMethod("Submit", "iassi", "o", [this](base::dbus::MethodCall &call) { submit(call); },
                       {{"operation", "sources", "destination", "conflictPolicy"}, {"task"}});
    m_object.addMethod("SubmitArchive", "iass", "o", [this](base::dbus::MethodCall &call) { submitArchive(call); },
                       {{"operation", "sources", "outputPath"}, {"task"}});
    m_object.addMethod("Undo", "", "o", [this](base::dbus::MethodCall &call) { history(call, true); },
                       {{}, {"task"}});
    m_object.addMethod("Redo", "", "o", [this](base::dbus::MethodCall &call) { history(call, false); },
                       {{}, {"task"}});
    m_object.addMethod("CanUndo", [] { return files::TaskController::instance()->isUndoAvailable(); });
    m_object.addMethod("CanRedo", [] { return files::TaskController::instance()->isRedoAvailable(); });
    m_object.addMethod("TaskPaths", [this] { return m_taskPaths; });
    m_object.addSignal("TaskAdded", "s", {"path"});
    m_object.addSignal("HistoryChanged", "bb", {"canUndo", "canRedo"});
}

FileOperationsService::~FileOperationsService()
{
    *m_alive = false;
}

bool FileOperationsService::publish()
{
    return m_object.publish();
}

std::string FileOperationsService::nextPath()
{
    return "/org/lingmo/FileOperations/tasks/" + std::to_string(m_nextTaskId++);
}

void FileOperationsService::submit(base::dbus::MethodCall &call)
{
    int32_t operation = 0;
    std::vector<std::string> sources;
    std::string destination;
    int32_t conflict = 0;
    if (!call.read(operation, sources, destination, conflict) || operation < int(files::Operation::Copy)
        || operation > int(files::Operation::Size) || conflict < int(files::ConflictPolicy::Fail)
        || conflict > int(files::ConflictPolicy::Ask)) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
        return;
    }

    files::TaskRequest request;
    request.operation = files::Operation(operation);
    for (const std::string &source : sources)
        request.sources.push_back(files::Url(source));
    request.destination = files::Url(destination);
    request.conflict = files::ConflictPolicy(conflict);

    const std::string path = nextPath();
    auto task = std::make_unique<DaemonTask>(m_bus, path, files::TaskController::instance()->submit(request));
    if (!announce(std::move(task), call.message().sender())) {
        call.replyError("org.freedesktop.DBus.Error.Failed", tr("The task could not be published"));
        return;
    }
    call.reply(ObjectPath {path});
}

void FileOperationsService::submitArchive(base::dbus::MethodCall &call)
{
    int32_t operation = 0;
    std::vector<std::string> sources;
    std::string outputPath;
    if (!call.read(operation, sources, outputPath) || (operation != 0 && operation != 1) || sources.empty()
        || outputPath.empty()) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
        return;
    }

    const std::string path = nextPath();
    auto task = std::make_unique<DaemonTask>(m_bus, path, operation, sources, outputPath);
    if (!announce(std::move(task), call.message().sender())) {
        call.replyError("org.freedesktop.DBus.Error.Failed", tr("The task could not be published"));
        return;
    }
    call.reply(ObjectPath {path});
}

void FileOperationsService::history(base::dbus::MethodCall &call, bool undo)
{
    files::Task *task = undo ? files::TaskController::instance()->undo() : files::TaskController::instance()->redo();
    // An empty path is no valid object path, so the reply would never be sent.
    if (!task) {
        call.reply(ObjectPath {"/"});
        return;
    }

    const std::string path = nextPath();
    if (!announce(std::make_unique<DaemonTask>(m_bus, path, task), call.message().sender())) {
        call.replyError("org.freedesktop.DBus.Error.Failed", tr("The task could not be published"));
        return;
    }
    call.reply(ObjectPath {path});
}

bool FileOperationsService::announce(std::unique_ptr<DaemonTask> task, const std::string &owner)
{
    // The bus name of the client that asked for the task. Clients show
    // progress only for their own tasks, so one paste does not raise a
    // dialog in every process attached to the daemon.
    task->setOwner(owner);
    if (!task->publish()) {
        task->cancel();
        DaemonTask *raw = task.get();
        m_tasks.push_back(std::move(task));
        // The archive thread ends with the task, once it stopped.
        base::EventLoop::current()->post([this, alive = m_alive, path = raw->objectPath()] {
            if (*alive)
                remove(path);
        });
        return false;
    }

    DaemonTask *raw = task.get();
    m_tasks.push_back(std::move(task));
    m_taskPaths.push_back(raw->objectPath());
    watchOwner(owner);
    m_finishedSubscriptions[raw] = raw->finished.connect([this, raw] {
        retire(raw);
    });
    m_object.emitSignal("TaskAdded", raw->objectPath());
    return true;
}

void FileOperationsService::watchOwner(const std::string &owner)
{
    if (owner.empty() || m_owners.contains(owner))
        return;
    // Called at once with the current owner: a submitter that left between
    // its call and now is seen gone right away.
    m_owners[owner] = m_bus->watchName(owner, [this, alive = m_alive, owner](const std::string &current) {
        if (!current.empty())
            return;
        base::EventLoop::current()->post([this, alive, owner] {
            if (*alive)
                ownerLost(owner);
        });
    });
}

// Live clients leave a task to its submitter, so a submitter that leaves the
// bus mid-question would keep the worker waiting for good; the task stops.
void FileOperationsService::ownerLost(const std::string &owner)
{
    m_owners.erase(owner);
    for (const auto &task : m_tasks) {
        if (task->owner() == owner)
            task->orphan();
    }
}

// A client reads the snapshot of a task right after it is announced, so a
// finished task stays on the bus until its outcome was read, and a while
// after that for late readers. One nobody reads still goes eventually.
void FileOperationsService::retire(DaemonTask *task)
{
    std::erase(m_taskPaths, task->objectPath());
    // By path, which no later task reuses: whichever of the two timers comes
    // second finds the task gone.
    const auto unregister = [this, alive = m_alive, path = task->objectPath()] {
        if (*alive)
            remove(path);
    };
    const auto linger = [unregister] { later(std::chrono::seconds(10), unregister); };
    if (task->isArchive() || task->outcomeRead() || task->owner().empty()) {
        linger();
        return;
    }
    task->setOutcomeReadCallback(linger);
    later(std::chrono::minutes(5), unregister);
}

void FileOperationsService::remove(const std::string &path)
{
    const auto found = std::find_if(m_tasks.begin(), m_tasks.end(), [&path](const std::unique_ptr<DaemonTask> &entry) {
        return entry->objectPath() == path;
    });
    if (found == m_tasks.end())
        return;
    m_finishedSubscriptions.erase(found->get());
    m_tasks.erase(found);
}
