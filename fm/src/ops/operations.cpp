// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ops/operations.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/url.h"

#include <algorithm>
#include <cerrno>

namespace fm {

using base::dbus::Variant;
using base::dbus::VariantMap;
using Lingmo::i18n::tr;

static constexpr char kService[] = "org.lingmo.FileOperations";
static constexpr char kManagerPath[] = "/org/lingmo/FileOperations";
static constexpr char kManagerInterface[] = "org.lingmo.FileOperations.Manager1";
static constexpr char kTaskInterface[] = "org.lingmo.FileOperations.Task1";

template<typename T>
static T value(const VariantMap& map, const std::string& key, T fallback = T())
{
    const auto found = map.find(key);
    if (found == map.end())
        return fallback;
    // Numbers come as whatever width the daemon chose.
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
        const Variant& v = found->second;
        if (auto i = v.get<int32_t>())
            return T(*i);
        if (auto u = v.get<uint32_t>())
            return T(*u);
        if (auto x = v.get<int64_t>())
            return T(*x);
        if (auto t = v.get<uint64_t>())
            return T(*t);
        return fallback;
    } else {
        return found->second.value<T>(fallback);
    }
}

static std::string operationName(Operation operation)
{
    switch (operation) {
    case Operation::Copy:
        return "copy";
    case Operation::Move:
        return "move";
    case Operation::Link:
        return "link";
    case Operation::Delete:
        return "delete";
    case Operation::Mkdir:
        return "mkdir";
    case Operation::Rename:
        return "rename";
    case Operation::Trash:
        return "trash";
    case Operation::Restore:
        return "restore";
    case Operation::EmptyTrash:
        return "emptyTrash";
    case Operation::Size:
        return "size";
    }
    return {};
}

// The last segment of a URL or path, or the path itself for "/".
static std::string urlFileName(const std::string& url)
{
    std::string path = url;
    if (const auto local = base::url::toLocalFile(url))
        path = *local;
    else if (const size_t scheme = url.find(":/"); scheme != std::string::npos)
        path = base::url::percentDecoded(url.substr(scheme + 1));
    while (path.size() > 1 && path.back() == '/')
        path.pop_back();
    const size_t slash = path.rfind('/');
    const std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
    return name.empty() ? path : name;
}

static VariantMap initialSnapshot(const std::string& operation, const std::vector<std::string>& sources,
                                  const std::string& destination)
{
    VariantMap progress {
        {"preparing", Variant(true)},
        {"totalsKnown", Variant(false)},
        {"processedBytes", Variant(uint64_t(0))},
        {"totalBytes", Variant(uint64_t(0))},
        {"processedItems", Variant(uint64_t(0))},
        {"totalItems", Variant(uint64_t(0))},
        {"elapsedMs", Variant(int64_t(0))},
        {"currentPath", Variant(std::string())},
    };
    return {
        {"operationName", Variant(operation)},
        {"sourceCount", Variant(int32_t(sources.size()))},
        {"sourceName", Variant(sources.empty() ? std::string() : urlFileName(sources.front()))},
        {"destinationName", Variant(destination.empty() ? std::string() : urlFileName(destination))},
        {"progress", Variant(progress)},
    };
}

OperationTask::OperationTask(VariantMap snapshot)
    : m_snapshot(std::move(snapshot))
{
}

OperationTask::~OperationTask() = default;

TaskProgress OperationTask::progress() const
{
    TaskProgress result;
    const VariantMap progress = value<VariantMap>(m_snapshot, "progress");
    result.preparing = value<bool>(progress, "preparing", true);
    result.totalsKnown = value<bool>(progress, "totalsKnown");
    result.processedBytes = value<uint64_t>(progress, "processedBytes");
    result.totalBytes = value<uint64_t>(progress, "totalBytes");
    result.processedItems = value<uint64_t>(progress, "processedItems");
    result.totalItems = value<uint64_t>(progress, "totalItems");
    result.elapsedMs = value<int64_t>(progress, "elapsedMs");
    result.currentPath = value<std::string>(progress, "currentPath");
    return result;
}

std::string OperationTask::operationName() const
{
    return value<std::string>(m_snapshot, "operationName");
}

int OperationTask::sourceCount() const
{
    return value<int>(m_snapshot, "sourceCount");
}

std::string OperationTask::sourceName() const
{
    return value<std::string>(m_snapshot, "sourceName");
}

std::string OperationTask::destinationName() const
{
    return value<std::string>(m_snapshot, "destinationName");
}

bool OperationTask::waitingForConflict() const
{
    return value<bool>(m_snapshot, "waitingForConflict");
}

uint32_t OperationTask::conflictId() const
{
    return value<uint32_t>(m_snapshot, "conflictId");
}

std::string OperationTask::conflictSource() const
{
    return value<std::string>(m_snapshot, "conflictSource");
}

std::string OperationTask::conflictPath() const
{
    return value<std::string>(m_snapshot, "conflictPath");
}

bool OperationTask::waitingForPassword() const
{
    return value<bool>(m_snapshot, "waitingForPassword");
}

uint32_t OperationTask::passwordId() const
{
    return value<uint32_t>(m_snapshot, "passwordId");
}

bool OperationTask::passwordRetry() const
{
    return value<bool>(m_snapshot, "passwordRetry");
}

std::string OperationTask::owner() const
{
    return value<std::string>(m_snapshot, "owner");
}

bool OperationTask::wasCanceled() const
{
    return value<bool>(m_snapshot, "canceled");
}

int OperationTask::error() const
{
    return value<int>(m_snapshot, "errorCode");
}

std::string OperationTask::errorString() const
{
    return value<std::string>(m_snapshot, "errorString");
}

std::vector<std::string> OperationTask::outputs() const
{
    return value<std::vector<std::string>>(m_snapshot, "outputs");
}

void OperationTask::cancel()
{
    if (m_finished)
        return;
    if (m_path.empty()) {
        // Not on the bus yet: cancelled as soon as it is.
        m_cancelRequested = true;
        return;
    }
    m_proxy.send("Cancel");
}

void OperationTask::resolveConflict(uint32_t conflict, ConflictPolicy policy, bool applyToAll)
{
    if (m_path.empty() || m_finished || conflict == 0)
        return;
    if (conflict == conflictId())
        apply({{"waitingForConflict", Variant(false)}, {"conflictId", Variant(uint32_t(0))}});
    m_proxy.send("ResolveConflict", conflict, int32_t(policy), applyToAll);
}

void OperationTask::answerPassword(uint32_t request, const std::string& password)
{
    if (m_path.empty() || m_finished || request == 0)
        return;
    m_proxy.send("AnswerPassword", request, password);
}

void OperationTask::attach(base::dbus::Connection* bus, const std::string& path)
{
    m_path = path;
    m_proxy = base::dbus::Proxy(bus, kService, path, kTaskInterface);
    m_changed = m_proxy.onSignal("Changed", [this](const base::dbus::Message& message) {
        if (auto snapshot = message.argument<VariantMap>())
            apply(*snapshot);
    });
    m_conflict = m_proxy.onSignal("ConflictRequested", [this](const base::dbus::Message& message) {
        uint32_t conflict = 0;
        std::string source;
        std::string destination;
        if (message.read(conflict, source, destination)) {
            apply({{"waitingForConflict", Variant(true)},
                   {"conflictId", Variant(conflict)},
                   {"conflictSource", Variant(source)},
                   {"conflictPath", Variant(destination)}});
            conflictRequested.emit(conflict, source, destination);
        }
    });
    m_finishedSignal = m_proxy.onSignal("Finished", [this](const base::dbus::Message& message) {
        if (auto snapshot = message.argument<VariantMap>())
            finish(*snapshot);
    });
    m_pending = m_proxy.callAsync("GetSnapshot", [this](const base::dbus::Reply& reply) {
        const auto snapshot = reply.value<VariantMap>();
        if (!snapshot)
            return;
        apply(*snapshot);
        if (value<bool>(*snapshot, "finished"))
            finish(*snapshot);
    });

    if (m_cancelRequested)
        m_proxy.send("Cancel");
}

void OperationTask::apply(const VariantMap& snapshot)
{
    for (const auto& [key, entry] : snapshot)
        m_snapshot[key] = entry;
    changed.emit();
}

void OperationTask::finish(const VariantMap& snapshot)
{
    if (m_finished)
        return;
    m_finished = true;
    for (const auto& [key, entry] : snapshot)
        m_snapshot[key] = entry;
    m_changed.reset();
    m_conflict.reset();
    m_finishedSignal.reset();
    changed.emit();
    finished.emit();
}

// Calls the method and keeps the call alive until its reply came.
template<typename... Args>
void OperationsClient::call(const base::dbus::Proxy& proxy, const std::string& method,
                            std::function<void(const base::dbus::Reply&)> done, const Args&... args)
{
    const int id = m_nextCall++;
    m_calls[id] = proxy.callAsync(
            method,
            [this, id, done = std::move(done)](const base::dbus::Reply& reply) {
                done(reply);
                // Not from inside the slot's own callback.
                base::EventLoop::current()->post([this, alive = m_alive, id] {
                    if (*alive)
                        m_calls.erase(id);
                });
            },
            args...);
}

OperationsClient& OperationsClient::instance()
{
    static OperationsClient client(base::dbus::Connection::session());
    return client;
}

OperationsClient::OperationsClient(base::dbus::Connection* bus)
    : m_bus(bus)
    , m_manager(bus, kService, kManagerPath, kManagerInterface)
{
    if (!bus)
        return;

    // Long operations wait for the user at conflicts; the submit itself is quick.
    m_manager.setTimeout(std::chrono::seconds(25));

    m_taskAdded = m_manager.onSignal("TaskAdded", [this](const base::dbus::Message& message) {
        if (auto path = message.argument<std::string>()) {
            if (!m_tasks.contains(*path))
                attach(*path, nullptr, false);
        }
    });
    m_historyChanged = m_manager.onSignal("HistoryChanged", [this](const base::dbus::Message& message) {
        bool canUndo = false;
        bool canRedo = false;
        if (message.read(canUndo, canRedo))
            setHistory(canUndo, canRedo);
    });

    // The daemon's history is what counts while it runs; its stacks go with it.
    m_watch = bus->watchName(kService, [this](const std::string& owner) {
        const bool up = !owner.empty();
        if (up == m_daemonUp)
            return;
        m_daemonUp = up;
        if (!up) {
            setHistory(false, false);
            // A daemon that died mid-task never sends Finished.
            const auto active = m_active;
            for (const OperationTaskPtr& task : active)
                task->finish({{"finished", Variant(true)},
                              {"errorCode", Variant(int32_t(ECONNRESET))},
                              {"errorString", Variant(std::string(tr("The file operations service stopped.")))}});
            return;
        }
        queryHistory();
        call(m_manager, "TaskPaths", [this](const base::dbus::Reply& reply) {
            if (const auto paths = reply.value<std::vector<std::string>>()) {
                for (const std::string& path : *paths) {
                    if (!m_tasks.contains(path))
                        attach(path, nullptr, false);
                }
            }
        });
    });
}

OperationsClient::~OperationsClient()
{
    *m_alive = false;
}

void OperationsClient::queryHistory()
{
    m_historyCall = m_manager.callAsync("CanUndo", [this](const base::dbus::Reply& undoReply) {
        const bool canUndo = undoReply.value<bool>().value_or(false);
        m_historyCall = m_manager.callAsync("CanRedo", [this, canUndo](const base::dbus::Reply& redoReply) {
            setHistory(canUndo, redoReply.value<bool>().value_or(false));
        });
    });
}

void OperationsClient::setHistory(bool canUndo, bool canRedo)
{
    if (m_canUndo == canUndo && m_canRedo == canRedo)
        return;
    m_canUndo = canUndo;
    m_canRedo = canRedo;
    historyChanged.emit();
}

OperationTaskPtr OperationsClient::attach(const std::string& path, OperationTaskPtr task, bool submitted)
{
    // The daemon's TaskAdded can come before the Submit reply: the submitted
    // task then takes the place of the probe made for the announcement.
    if (const auto found = m_tasks.find(path); found != m_tasks.end()) {
        if (auto existing = found->second.lock(); existing && (!task || existing == task))
            return existing;
    }

    // Every task has a path of its own: the ones nobody holds any more would
    // otherwise stay for the session.
    std::erase_if(m_tasks, [](const auto& entry) { return entry.second.expired(); });

    if (!task) {
        // Announced by the daemon: presented here only when its submitter is
        // gone, so a task never runs unseen; the submitter presents its own.
        auto probe = std::make_shared<OperationTask>(VariantMap {});
        m_tasks[path] = probe;
        auto proxy = std::make_shared<base::dbus::Proxy>(m_bus, kService, path, kTaskInterface);
        call(*proxy, "GetSnapshot", [this, path, probe, proxy](const base::dbus::Reply& reply) {
            const auto snapshot = reply.value<VariantMap>();
            const std::string owner = snapshot ? value<std::string>(*snapshot, "owner") : std::string();
            const auto current = m_tasks.find(path);
            if (current == m_tasks.end() || current->second.lock() != probe)
                return;
            if (!snapshot || owner == m_bus->uniqueName()) {
                m_tasks.erase(path);
                return;
            }
            base::dbus::Proxy bus(m_bus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
            call(bus, "NameHasOwner", [this, path, probe, snapshot](const base::dbus::Reply& owned) {
                const auto current = m_tasks.find(path);
                if (current == m_tasks.end() || current->second.lock() != probe)
                    return;
                if (owned.value<bool>().value_or(false)) {
                    m_tasks.erase(path);
                    return;
                }
                probe->apply(*snapshot);
                probe->attach(m_bus, path);
                announce(probe);
            }, owner);
        });
        return probe;
    }

    m_tasks[path] = task;
    task->attach(m_bus, path);
    if (submitted)
        announce(task);
    return task;
}

void OperationsClient::announce(const OperationTaskPtr& task)
{
    // Counted before taskAdded, so whoever reacts to it never sees it missing.
    if (!task->isFinished() && std::find(m_active.begin(), m_active.end(), task) == m_active.end()) {
        m_active.push_back(task);
        std::weak_ptr<OperationTask> weak = task;
        auto subscription = std::make_shared<base::Subscription>();
        *subscription = task->finished.connect([this, weak, subscription] {
            const OperationTaskPtr finishedTask = weak.lock();
            std::erase(m_active, finishedTask);
            activeTaskCountChanged.emit(int(m_active.size()));
            // The subscription goes with the next loop turn, not inside its own call.
            base::EventLoop::current()->post([subscription] { subscription->reset(); });
        });
        activeTaskCountChanged.emit(int(m_active.size()));
    }
    taskAdded.emit(task);
}

OperationTaskPtr OperationsClient::submit(Operation operation, const std::vector<std::string>& sources,
                                          const std::string& destination, ConflictPolicy conflict)
{
    auto task = std::make_shared<OperationTask>(initialSnapshot(operationName(operation), sources, destination));
    if (!m_bus) {
        task->finish({{"finished", Variant(true)},
                      {"errorCode", Variant(int32_t(ENOTCONN))},
                      {"errorString", Variant(std::string(tr("The session bus cannot be reached.")))}});
        return task;
    }

    call(m_manager,
            "Submit",
            [this, task](const base::dbus::Reply& reply) {
                const auto path = reply.value<base::dbus::ObjectPath>();
                if (!path) {
                    task->finish({{"finished", Variant(true)},
                                  {"errorCode", Variant(int32_t(EIO))},
                                  {"errorString", Variant(reply.error().message)}});
                    announce(task);
                    return;
                }
                attach(path->path, task, true);
            },
            int32_t(operation), sources, destination, int32_t(conflict));
    return task;
}

OperationTaskPtr OperationsClient::copy(const std::vector<std::string>& sources, const std::string& destination)
{
    return submit(Operation::Copy, sources, destination, ConflictPolicy::Ask);
}

OperationTaskPtr OperationsClient::move(const std::vector<std::string>& sources, const std::string& destination)
{
    return submit(Operation::Move, sources, destination, ConflictPolicy::Ask);
}

OperationTaskPtr OperationsClient::link(const std::vector<std::string>& sources, const std::string& destination)
{
    return submit(Operation::Link, sources, destination, ConflictPolicy::Ask);
}

OperationTaskPtr OperationsClient::remove(const std::vector<std::string>& sources)
{
    return submit(Operation::Delete, sources);
}

OperationTaskPtr OperationsClient::mkdir(const std::string& destination)
{
    return submit(Operation::Mkdir, {}, destination);
}

OperationTaskPtr OperationsClient::rename(const std::string& source, const std::string& destination)
{
    return submit(Operation::Rename, {source}, destination, ConflictPolicy::Ask);
}

OperationTaskPtr OperationsClient::trash(const std::vector<std::string>& sources)
{
    return submit(Operation::Trash, sources);
}

OperationTaskPtr OperationsClient::restore(const std::vector<std::string>& sources)
{
    return submit(Operation::Restore, sources, {}, ConflictPolicy::Ask);
}

OperationTaskPtr OperationsClient::emptyTrash()
{
    return submit(Operation::EmptyTrash, {});
}

OperationTaskPtr OperationsClient::archive(int operation, const std::vector<std::string>& sources,
                                           const std::string& outputPath)
{
    VariantMap snapshot = initialSnapshot(operation == 0 ? "compress" : "extract", sources, outputPath);
    auto task = std::make_shared<OperationTask>(std::move(snapshot));
    if (!m_bus)
        return task;

    call(m_manager,
            "SubmitArchive",
            [this, task](const base::dbus::Reply& reply) {
                const auto path = reply.value<base::dbus::ObjectPath>();
                if (!path) {
                    task->finish({{"finished", Variant(true)},
                                  {"errorCode", Variant(int32_t(EIO))},
                                  {"errorString", Variant(reply.error().message)}});
                    announce(task);
                    return;
                }
                attach(path->path, task, true);
            },
            int32_t(operation), sources, outputPath);
    return task;
}

void OperationsClient::undo()
{
    if (!m_canUndo)
        return;
    call(m_manager, "Undo", [this](const base::dbus::Reply& reply) {
        const auto path = reply.value<base::dbus::ObjectPath>();
        // "/" means there was nothing to take back.
        if (path && path->path != "/")
            attach(path->path, std::make_shared<OperationTask>(VariantMap {}), true);
    });
}

void OperationsClient::redo()
{
    if (!m_canRedo)
        return;
    call(m_manager, "Redo", [this](const base::dbus::Reply& reply) {
        const auto path = reply.value<base::dbus::ObjectPath>();
        if (path && path->path != "/")
            attach(path->path, std::make_shared<OperationTask>(VariantMap {}), true);
    });
}

} // namespace fm
