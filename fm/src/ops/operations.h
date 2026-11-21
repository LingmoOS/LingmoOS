// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "base/dbus.h"
#include "base/signal.h"

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fm {

// org.lingmo.FileOperations.Manager1's operation and conflict numbers.
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
    Size,
};

enum class ConflictPolicy {
    Fail,
    Skip,
    KeepBoth,
    Replace,
    Ask,
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

// A file operation the operations daemon runs: its state as the daemon last
// reported it. Copies of the pointer keep it alive past its end.
class OperationTask
{
public:
    explicit OperationTask(base::dbus::VariantMap snapshot);
    ~OperationTask();

    OperationTask(const OperationTask&) = delete;
    OperationTask& operator=(const OperationTask&) = delete;

    TaskProgress progress() const;
    // "copy", "move", "trash", "compress", …
    std::string operationName() const;
    int sourceCount() const;
    std::string sourceName() const;
    std::string destinationName() const;
    bool waitingForConflict() const;
    // The conflict waiting for an answer; 0 when none is.
    uint32_t conflictId() const;
    std::string conflictSource() const;
    std::string conflictPath() const;
    // An extraction waiting for a password, until the job accepts one.
    bool waitingForPassword() const;
    // The password request waiting for an answer; 0 when none is.
    uint32_t passwordId() const;
    // Whether the passwords given so far did not fit.
    bool passwordRetry() const;
    bool isFinished() const { return m_finished; }
    // The bus name of the process that submitted it.
    std::string owner() const;
    bool wasCanceled() const;
    int error() const;
    std::string errorString() const;
    std::vector<std::string> outputs() const;
    const std::string& path() const { return m_path; }

    void cancel();
    // Answers the conflict with this id; applyToAll answers every later
    // conflict. The daemon refuses an answer to a conflict no longer waiting.
    void resolveConflict(uint32_t conflict, ConflictPolicy policy, bool applyToAll = false);
    // Answers the password request with this id. A wrong password opens a
    // request of a new id; the right one ends the waiting.
    void answerPassword(uint32_t request, const std::string& password);

    base::Signal<> changed;
    // The conflict's id, source and destination.
    base::Signal<uint32_t, std::string, std::string> conflictRequested;
    base::Signal<> finished;

private:
    friend class OperationsClient;

    void attach(base::dbus::Connection* bus, const std::string& path);
    void apply(const base::dbus::VariantMap& snapshot);
    void finish(const base::dbus::VariantMap& snapshot);

    base::dbus::VariantMap m_snapshot;
    std::string m_path;
    base::dbus::Proxy m_proxy;
    base::dbus::Match m_changed;
    base::dbus::Match m_conflict;
    base::dbus::Match m_finishedSignal;
    base::dbus::PendingCall m_pending;
    bool m_finished = false;
    bool m_cancelRequested = false;
};

using OperationTaskPtr = std::shared_ptr<OperationTask>;

// The file manager's way to the operations daemon, org.lingmo.FileOperations.
// Every call returns at once; the daemon is started by the bus when needed.
class OperationsClient
{
public:
    static OperationsClient& instance();
    // On another connection, for tests.
    explicit OperationsClient(base::dbus::Connection* bus);
    ~OperationsClient();

    OperationTaskPtr submit(Operation operation, const std::vector<std::string>& sources,
                            const std::string& destination = {},
                            ConflictPolicy conflict = ConflictPolicy::Fail);
    OperationTaskPtr copy(const std::vector<std::string>& sources, const std::string& destination);
    OperationTaskPtr move(const std::vector<std::string>& sources, const std::string& destination);
    OperationTaskPtr link(const std::vector<std::string>& sources, const std::string& destination);
    OperationTaskPtr remove(const std::vector<std::string>& sources);
    OperationTaskPtr mkdir(const std::string& destination);
    OperationTaskPtr rename(const std::string& source, const std::string& destination);
    OperationTaskPtr trash(const std::vector<std::string>& sources);
    OperationTaskPtr restore(const std::vector<std::string>& sources);
    OperationTaskPtr emptyTrash();
    // operation 0 compresses the sources into the output, 1 extracts the one source.
    OperationTaskPtr archive(int operation, const std::vector<std::string>& sources, const std::string& outputPath);
    void undo();
    void redo();

    bool undoAvailable() const { return m_canUndo; }
    bool redoAvailable() const { return m_canRedo; }
    // Tasks this process presents that have not finished yet.
    int activeTaskCount() const { return int(m_active.size()); }

    // A task to present: one this process submitted, or one whose submitter left.
    base::Signal<OperationTaskPtr> taskAdded;
    base::Signal<> historyChanged;
    base::Signal<int> activeTaskCountChanged;

private:
    OperationTaskPtr attach(const std::string& path, OperationTaskPtr task, bool submitted);
    void announce(const OperationTaskPtr& task);
    void queryHistory();
    void setHistory(bool canUndo, bool canRedo);
    template<typename... Args>
    void call(const base::dbus::Proxy& proxy, const std::string& method,
              std::function<void(const base::dbus::Reply&)> done, const Args&... args);

    base::dbus::Connection* m_bus;
    base::dbus::Proxy m_manager;
    base::dbus::Match m_taskAdded;
    base::dbus::Match m_historyChanged;
    base::dbus::NameWatch m_watch;
    base::dbus::PendingCall m_historyCall;
    std::map<int, base::dbus::PendingCall> m_calls;
    int m_nextCall = 1;
    std::map<std::string, std::weak_ptr<OperationTask>> m_tasks;
    std::vector<OperationTaskPtr> m_active;
    bool m_canUndo = false;
    bool m_canRedo = false;
    bool m_daemonUp = false;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace fm
