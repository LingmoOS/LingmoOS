// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "base/event_loop.h"
#include "base/signal.h"
#include "base/weak_ptr.h"
#include "lingmo/files/local_file_system.h"

namespace Lingmo::Files {

enum class Operation {
    Copy,
    Move,
    Link,
    Delete,
    Mkdir,
    Rename,
    Trash,
    Restore,
    EmptyTrash,
    Size
};

struct TaskRequest
{
    Operation operation = Operation::Copy;
    std::vector<Url> sources;
    Url destination;
    ConflictPolicy conflict = ConflictPolicy::Fail;
    // When set, Copy, Move and Link put each source at its own path instead of into destination.
    std::vector<Url> targets;
};

struct TaskResult
{
    Error error;
    std::vector<Url> outputs;
    // The source each output came from; empty for Mkdir.
    std::vector<Url> outputSources;
    std::vector<Url> completedSources;
    // An output is a folder merged with what was already there, so it cannot be taken back.
    bool merged = false;
    uint64_t bytes = 0;
    uint64_t files = 0;
};

struct TaskProgress
{
    bool preparing = true;
    bool totalsKnown = false;
    uint64_t processedBytes = 0;
    uint64_t totalBytes = 0;
    uint64_t processedItems = 0;
    uint64_t totalItems = 0;
    int64_t elapsedMs = 0;
    std::string currentPath;
};

// One operation on a thread of its own. Its signals arrive on the loop of the
// thread that made it; it may be destroyed at any time, which cancels it and
// waits for the thread.
class Task
{
public:
    enum class State {
        Queued,
        Running,
        Succeeded,
        Failed,
        Canceled
    };

    explicit Task(TaskRequest request);
    ~Task();

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    void start();
    void cancel();
    // Answers the conflict that conflictRequested announced with this id, with
    // a ConflictPolicy other than Ask; applyToAll answers every later conflict
    // of this task the same way. False, and nothing changes, when that conflict
    // is not the one waiting: already answered, or never asked.
    bool resolveConflict(uint32_t conflict, int policy, bool applyToAll = false);

    State state() const { return m_state; }
    const TaskProgress& progressInfo() const { return m_progressInfo; }

    // "copy", "move", "link", "delete", "mkdir", "rename", "trash", "restore",
    // "emptyTrash" or "size".
    std::string operationName() const;
    int sourceCount() const { return int(m_request.sources.size()); }
    std::string sourceName() const;
    std::string destinationName() const;
    // In powers of 1000 with one decimal, e.g. "1.5 MB".
    std::string formattedSize(uint64_t bytes) const;
    bool wasCanceled() const { return m_state == State::Canceled; }

    const TaskResult& outcome() const { return m_result; }
    const TaskRequest& request() const { return m_request; }
    int error() const { return m_result.error.code; }
    std::string errorString() const;

    // Calls back once with the finished task, for as long as the task lives.
    void whenFinished(std::function<void(Task*)> callback);

    base::Signal<uint64_t, uint64_t> progress;
    base::Signal<> progressInfoChanged;
    // The conflict's id, source and destination. Answer with resolveConflict();
    // without a listener every conflict fails.
    base::Signal<uint32_t, const std::string&, const std::string&> conflictRequested;
    base::Signal<State> stateChanged;
    base::Signal<Task*> result;

private:
    friend class TaskController;

    void finish(TaskResult result);

    TaskRequest m_request;
    TaskResult m_result;
    TaskProgress m_progressInfo;
    State m_state = State::Queued;
    std::shared_ptr<OperationContext> m_context;
    base::LoopHandle m_loop;
    std::thread m_thread;
    std::mutex m_conflictMutex;
    std::condition_variable m_conflictCondition;
    ConflictPolicy m_resolution = ConflictPolicy::Fail;
    std::optional<ConflictPolicy> m_resolutionForAll;
    bool m_resolved = false;
    uint32_t m_lastConflict = 0;
    uint32_t m_pendingConflict = 0;
    std::vector<base::Subscription> m_finishedCallbacks;
    base::WeakPtrFactory<Task> m_weakFactory{this};
};

// Runs tasks and keeps what undoes and redoes each one that changed something.
class TaskController
{
public:
    TaskController();
    ~TaskController();

    TaskController(const TaskController&) = delete;
    TaskController& operator=(const TaskController&) = delete;

    // The process's controller, on the thread that asks first.
    static TaskController* instance();

    // Started on the next loop iteration; the task is destroyed after
    // taskFinished.
    Task* submit(const TaskRequest& request);

    bool isUndoAvailable() const { return !m_undo.empty(); }
    bool isRedoAvailable() const { return !m_redo.empty(); }

    // Null when there is nothing to undo or redo.
    Task* undo();
    Task* redo();

    base::Signal<Task*> taskAdded;
    base::Signal<Task*> taskFinished;
    // isUndoAvailable() or isRedoAvailable() may have changed.
    base::Signal<> historyChanged;

private:
    enum class History {
        Fresh,
        Undo,
        Redo
    };

    Task* submit(const TaskRequest& request, History history);
    void record(const TaskRequest& request, const TaskResult& result, History history);

    // Each undoable operation adds an entry; a long session must not keep them all.
    static constexpr size_t kMaxHistory = 50;

    std::vector<TaskRequest> m_undo;
    std::vector<TaskRequest> m_redo;
    std::vector<std::unique_ptr<Task>> m_tasks;
    base::WeakPtrFactory<TaskController> m_weakFactory{this};
};

enum class DropAction {
    Copy,
    Move,
    Link
};

Task* copy(const std::vector<Url>& sources, const Url& destination);
Task* move(const std::vector<Url>& sources, const Url& destination);
Task* del(const std::vector<Url>& sources);
Task* mkdir(const Url& destination);
Task* trash(const std::vector<Url>& sources);
Task* restoreFromTrash(const std::vector<Url>& sources);
Task* emptyTrash();
// A paste of the clipboard's URLs: a move when they were cut, else a copy.
Task* paste(const std::vector<Url>& urls, bool cut, const Url& destination);
Task* drop(const std::vector<Url>& urls, const Url& destination, DropAction action);

} // namespace Lingmo::Files
