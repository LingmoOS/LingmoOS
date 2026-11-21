// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/task.h"

#include <algorithm>
#include <cerrno>
#include <linux/magic.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/vfs.h>

#include "base/elapsed_timer.h"
#include "base/locale.h"
#include "lingmo/files/file_item.h"
#include "framework_i18n.h"
#include "path_utils_p.h"
#include "posix_p.h"

namespace Lingmo::Files {

using namespace Lingmo::Base;
using Lingmo::framework::tr;
using Lingmo::i18n::format;

// A copy within a filesystem that shares extents may take no space at all, so it is not counted.
static bool takesSpace(const std::string& source, const std::string& directory, bool move)
{
    const std::string& sourcePath = internal::native(source);
    const std::string& directoryPath = internal::native(directory);

    struct stat from, to;
    struct statfs fromFs, toFs;
    if (::lstat(sourcePath.c_str(), &from) < 0 || ::stat(directoryPath.c_str(), &to) < 0
        || ::statfs(sourcePath.c_str(), &fromFs) < 0 || ::statfs(directoryPath.c_str(), &toFs) < 0)
        return false;

    if (move && from.st_dev == to.st_dev)
        return false;

    const auto type = toFs.f_type;
    return fromFs.f_type != type
        || (type != BTRFS_SUPER_MAGIC && type != XFS_SUPER_MAGIC && type != BCACHEFS_SUPER_MAGIC);
}

static TaskResult execute(const TaskRequest& request, OperationContext& ctx,
                          const std::function<void(const TaskProgress&, bool)>& report)
{
    TaskResult result;
    TaskProgress info;
    ElapsedTimer elapsed;
    int64_t pausedMs = 0;
    std::vector<std::pair<uint64_t, uint64_t>> weights;
    uint64_t creditedBytes = 0, creditedItems = 0;
    uint64_t sourceBytes = 0, sourceItems = 0;
    uint64_t required = 0;
    const auto legacyProgress = ctx.progress;
    const auto conflictResolver = ctx.conflict;

    // The folder a source goes into; Rename keeps its full destination path here.
    const auto destinationOf = [&](size_t index) {
        if (request.targets.empty())
            return request.destination;

        return index < request.targets.size() ? request.targets[index].folder() : Url();
    };

    auto update = [&](bool force = false) {
        info.elapsedMs = elapsed.isValid() ? elapsed.elapsedMs() - pausedMs : 0;
        report(info, force);
    };

    ctx.conflict = [&](const std::string& source, const std::string& destination) {
        const int64_t before = elapsed.isValid() ? elapsed.elapsedMs() : 0;
        const auto policy = conflictResolver ? conflictResolver(source, destination) : ConflictPolicy::Fail;
        if (elapsed.isValid())
            pausedMs += elapsed.elapsedMs() - before;

        return policy;
    };

    ctx.currentItem = [&](const std::string& path) {
        info.currentPath = path;
        update();
    };

    ctx.progress = [&](uint64_t bytes, uint64_t files) {
        info.processedBytes = info.totalsKnown ? creditedBytes + bytes - sourceBytes : bytes;
        info.processedItems = info.totalsKnown ? creditedItems + files - sourceItems : files;
        update();

        if (legacyProgress)
            legacyProgress(bytes, files);
    };

    try {
        ctx.checkpoint();

        if (request.operation == Operation::Move || request.operation == Operation::Rename
            || request.operation == Operation::Trash || request.operation == Operation::Delete) {
            for (size_t i = 0; i < request.sources.size(); ++i) {
                const auto& url = request.sources[i];
                const bool allowedHomeMove =
                    request.operation == Operation::Move && canMoveStandardFolderTo(url, destinationOf(i));
                if (isStandardFolder(url) && !allowedHomeMove)
                    throw Failure(EPERM, url.toLocalFile());
            }
        }

        auto sources = request.sources;
        if (request.operation == Operation::EmptyTrash) {
            for (const auto& item : LocalFileSystem::enumerate(Url("trash:/")))
                sources.push_back(item.url());
        }

        info.totalsKnown = request.operation != Operation::Size;

        if (request.operation != Operation::Size) {
            for (const auto& url : sources) {
                OperationContext scan;
                scan.cancelFlag = &ctx.canceled;
                scan.currentItem = ctx.currentItem;

                if (request.operation == Operation::Rename || request.operation == Operation::Link) {
                    scan.files = 1;
                } else {
                    try {
                        const auto source = FileSystemUrl(url).localPath();
                        LocalFileSystem::directorySize(source, scan);

                        // Items going to their own folders are not checked up front.
                        if ((request.operation == Operation::Copy || request.operation == Operation::Move)
                            && request.targets.empty()
                            && takesSpace(source, FileSystemUrl(request.destination).localPath(),
                                          request.operation == Operation::Move))
                            required += scan.allocated;
                    } catch (const Failure& failure) {
                        if (failure.error.code == ECANCELED)
                            throw;
                        info.totalsKnown = false;
                    }
                }

                weights.emplace_back(scan.bytes, scan.files);
                info.totalBytes += scan.bytes;
                info.totalItems += scan.files;
            }
        }

        if (info.totalsKnown && required) {
            // A lower bound: block rounding and metadata are not counted, so a copy can still
            // run out, but one that cannot fit fails before it writes anything.
            const auto destination = FileSystemUrl(request.destination).localPath();
            struct statvfs fs;
            if (::statvfs(internal::native(destination).c_str(), &fs) == 0
                && required > uint64_t(fs.f_bavail) * fs.f_frsize)
                throw Failure(ENOSPC, destination);
        }

        if (request.operation == Operation::Mkdir)
            info.totalItems = 1;

        info.preparing = false;
        elapsed.start();
        update(true);

        if (request.operation == Operation::Mkdir) {
            LocalFileSystem::mkdir(FileSystemUrl(request.destination).localPath());
            result.outputs.push_back(request.destination);
        } else if (request.operation == Operation::EmptyTrash) {
            LocalFileSystem::emptyTrash(ctx);
        } else {
            size_t sourceIndex = 0;

            for (const auto& url : request.sources) {
                sourceBytes = ctx.bytes;
                sourceItems = ctx.files;
                ctx.checkpoint(FileSystemUrl(url).localPath());

                const auto source = FileSystemUrl(url).localPath();
                Url output;

                switch (request.operation) {
                case Operation::Copy:
                case Operation::Move:
                case Operation::Link:
                case Operation::Rename: {
                    std::string target;
                    if (!request.targets.empty()) {
                        target = FileSystemUrl(sourceIndex < request.targets.size() ? request.targets[sourceIndex]
                                                                                    : Url())
                                     .localPath();
                    } else {
                        const auto destination = FileSystemUrl(request.destination).localPath();
                        target = request.operation == Operation::Rename ? destination
                                                                        : internal::joined(destination, url.fileName());
                    }

                    const auto path = request.operation == Operation::Link
                        ? LocalFileSystem::link(source, target, request.conflict, ctx)
                        : request.operation == Operation::Copy ? LocalFileSystem::copy(source, target, request.conflict, ctx)
                                                               : LocalFileSystem::move(source, target, request.conflict, ctx);
                    if (!path.empty())
                        output = Url::fromLocalFile(path);
                    break;
                }
                case Operation::Delete:
                    LocalFileSystem::remove(url, ctx);
                    break;
                case Operation::Trash:
                    output = LocalFileSystem::trash(url, ctx);
                    break;
                case Operation::Restore:
                    output = LocalFileSystem::restore(url, request.conflict, ctx);
                    break;
                case Operation::Size:
                    LocalFileSystem::directorySize(source, ctx);
                    break;
                default:
                    break;
                }

                result.completedSources.push_back(url);
                if (!output.isEmpty()) {
                    result.outputs.push_back(output);
                    result.outputSources.push_back(url);
                }

                if (info.totalsKnown && sourceIndex < weights.size()) {
                    creditedBytes += weights[sourceIndex].first;
                    creditedItems += weights[sourceIndex].second;
                    info.processedBytes = creditedBytes;
                    info.processedItems = creditedItems;
                }

                ++sourceIndex;
                update(true);
            }
        }
    } catch (const Failure& failure) {
        result.error = failure.error;
    } catch (const std::exception& exception) {
        result.error = {EIO, {}, exception.what()};
    }

    result.bytes = ctx.bytes;
    result.files = ctx.files;
    result.merged = ctx.merged;

    if (!result.error && info.totalsKnown) {
        info.processedBytes = info.totalBytes;
        info.processedItems = info.totalItems;
    }

    update(true);
    ctx.currentItem = {};
    ctx.progress = legacyProgress;
    ctx.conflict = conflictResolver;
    return result;
}

Task::Task(TaskRequest request)
    : m_request(std::move(request))
    , m_context(std::make_shared<OperationContext>())
    , m_loop(EventLoop::current()->handle())
{
}

Task::~Task()
{
    cancel();
    if (m_thread.joinable())
        m_thread.join();
}

void Task::cancel()
{
    {
        const std::lock_guard lock(m_conflictMutex);
        m_context->canceled.store(true);
    }

    m_conflictCondition.notify_all();
}

bool Task::resolveConflict(uint32_t conflict, int policy, bool applyToAll)
{
    if (policy < int(ConflictPolicy::Fail) || policy >= int(ConflictPolicy::Ask))
        return false;

    {
        const std::lock_guard lock(m_conflictMutex);
        if (conflict == 0 || conflict != m_pendingConflict)
            return false;

        m_pendingConflict = 0;
        m_resolution = ConflictPolicy(policy);
        m_resolved = true;
        if (applyToAll)
            m_resolutionForAll = m_resolution;
    }

    m_conflictCondition.notify_all();
    return true;
}

void Task::whenFinished(std::function<void(Task*)> callback)
{
    m_finishedCallbacks.push_back(result.connect(std::move(callback)));
}

void Task::finish(TaskResult outcome)
{
    if (m_thread.joinable())
        m_thread.join();

    m_result = std::move(outcome);
    m_state = m_result.error.code == ECANCELED ? State::Canceled : m_result.error ? State::Failed : State::Succeeded;

    const auto self = m_weakFactory.getWeakPtr();
    progress.emit(m_result.bytes, m_result.files);
    if (!self)
        return;
    stateChanged.emit(m_state);
    if (!self)
        return;
    result.emit(this);
}

void Task::start()
{
    if (m_state != State::Queued)
        return;

    m_state = State::Running;
    stateChanged.emit(m_state);

    const auto self = m_weakFactory.getWeakPtr();
    const LoopHandle loop = m_loop;

    m_context->progress = [self, loop, timer = ElapsedTimer()](uint64_t bytes, uint64_t files) mutable {
        if (timer.isValid() && timer.elapsedMs() < 40)
            return;

        timer.start();
        loop.post([self, bytes, files] {
            if (Task* task = self.get())
                task->progress.emit(bytes, files);
        });
    };

    m_context->conflict = [this, self, loop](const std::string& source, const std::string& destination) {
        std::unique_lock lock(m_conflictMutex);
        if (m_resolutionForAll)
            return *m_resolutionForAll;

        m_resolved = false;
        // A new id, so a late answer to an earlier conflict cannot resolve this one.
        const uint32_t conflict = ++m_lastConflict;
        m_pendingConflict = conflict;
        loop.post([self, conflict, source, destination] {
            Task* task = self.get();
            if (!task)
                return;

            if (!task->conflictRequested.isEmpty())
                task->conflictRequested.emit(conflict, source, destination);
            else
                task->resolveConflict(conflict, int(ConflictPolicy::Fail));
        });

        m_conflictCondition.wait(lock, [this] {
            return m_resolved || m_context->canceled.load();
        });

        m_pendingConflict = 0;

        m_context->checkpoint(destination);
        return m_resolution;
    };

    m_thread = std::thread([self, loop, request = m_request, ctx = m_context] {
        auto report = [self, loop, timer = ElapsedTimer()](const TaskProgress& info, bool force) mutable {
            if (!force && timer.isValid() && timer.elapsedMs() < 80)
                return;

            timer.start();
            loop.post([self, info] {
                if (Task* task = self.get()) {
                    task->m_progressInfo = info;
                    task->progressInfoChanged.emit();
                }
            });
        };

        TaskResult outcome = execute(request, *ctx, report);

        loop.post([self, outcome = std::move(outcome)]() mutable {
            if (Task* task = self.get())
                task->finish(std::move(outcome));
        });
    });
}

std::string Task::formattedSize(uint64_t bytes) const
{
    return Locale::system().formattedDataSize(int64_t(bytes), 1, Locale::DataSizeFormat::Si);
}

std::string Task::operationName() const
{
    switch (m_request.operation) {
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

std::string Task::sourceName() const
{
    return m_request.sources.empty() ? std::string() : m_request.sources.front().fileName();
}

std::string Task::destinationName() const
{
    const Url url = m_request.destination.strippedTrailingSlash();
    return url.fileName().empty() ? url.path() : url.fileName();
}

std::string Task::errorString() const
{
    if (m_result.error.path.empty())
        return m_result.error.message;
    // TRANSLATORS: the file an operation failed on, then why, such as "/tmp/a.txt: Permission denied".
    return format(tr("{path}: {error}"), {{"path", m_result.error.path}, {"error", m_result.error.message}});
}

TaskController::TaskController() = default;

TaskController::~TaskController()
{
    for (const auto& task : m_tasks)
        task->cancel();
}

TaskController* TaskController::instance()
{
    static TaskController* controller = new TaskController;
    return controller;
}

Task* TaskController::submit(const TaskRequest& request)
{
    return submit(request, History::Fresh);
}

// What puts back the part of an operation that was done, if anything can.
static std::optional<TaskRequest> inverse(const TaskRequest& request, const TaskResult& result)
{
    if (result.outputs.empty() || result.merged)
        return {};

    switch (request.operation) {
    case Operation::Copy:
    case Operation::Link:
    case Operation::Mkdir:
    case Operation::Restore:
        return TaskRequest{Operation::Trash, result.outputs, {}, ConflictPolicy::Fail, {}};
    case Operation::Trash:
        return TaskRequest{Operation::Restore, result.outputs, {}, ConflictPolicy::Ask, {}};
    case Operation::Move:
    case Operation::Rename: {
        // Out of a trash, the item would go back without its info file.
        for (const auto& source : result.outputSources) {
            if (!source.isLocalFile())
                return {};
        }

        if (result.outputSources.size() != result.outputs.size())
            return {};

        TaskRequest back{Operation::Move, result.outputs, {}, ConflictPolicy::Ask, {}};
        back.targets = result.outputSources;
        return back;
    }
    default:
        return {};
    }
}

// A fresh operation forks the history: what was undone before it can no
// longer be redone. Undo and redo themselves keep the other stack.
Task* TaskController::submit(const TaskRequest& request, History history)
{
    if (history == History::Fresh && !m_redo.empty()) {
        m_redo.clear();
        historyChanged.emit();
    }

    m_tasks.push_back(std::make_unique<Task>(request));
    Task* task = m_tasks.back().get();
    const auto self = m_weakFactory.getWeakPtr();

    task->whenFinished([self, request, history](Task* done) {
        TaskController* controller = self.get();
        if (!controller)
            return;

        controller->record(request, done->outcome(), history);
        controller->taskFinished.emit(done);

        // Destroyed once the signals of its end have returned.
        EventLoop::current()->post([self, done] {
            if (TaskController* controller = self.get()) {
                std::erase_if(controller->m_tasks, [done](const std::unique_ptr<Task>& owned) {
                    return owned.get() == done;
                });
            }
        });
    });

    taskAdded.emit(task);

    const auto weakTask = task->m_weakFactory.getWeakPtr();
    EventLoop::current()->post([weakTask] {
        if (Task* pending = weakTask.get())
            pending->start();
    });

    return task;
}

void TaskController::record(const TaskRequest& request, const TaskResult& result, History history)
{
    bool changed = false;

    if (const auto back = inverse(request, result)) {
        auto& stack = history == History::Undo ? m_redo : m_undo;
        stack.push_back(*back);
        while (stack.size() > kMaxHistory)
            stack.erase(stack.begin());
        changed = true;
    }

    // A cancelled undo or redo keeps what it did not reach; a failed one is dropped, as
    // retrying would fail the same way.
    if (history != History::Fresh && result.error.code == ECANCELED) {
        auto remaining = request;
        for (const auto& source : result.completedSources) {
            const auto found = std::find(remaining.sources.begin(), remaining.sources.end(), source);
            if (found == remaining.sources.end())
                continue;

            const size_t index = size_t(found - remaining.sources.begin());
            remaining.sources.erase(found);
            if (index < remaining.targets.size())
                remaining.targets.erase(remaining.targets.begin() + ptrdiff_t(index));
        }

        if (!remaining.sources.empty()) {
            (history == History::Undo ? m_undo : m_redo).push_back(remaining);
            changed = true;
        }
    }

    // Emptied trash items cannot come back.
    if (request.operation == Operation::EmptyTrash && !result.error) {
        const auto restores = [](const TaskRequest& entry) {
            return entry.operation == Operation::Restore;
        };
        changed |= std::erase_if(m_undo, restores) > 0;
        changed |= std::erase_if(m_redo, restores) > 0;
    }

    if (changed)
        historyChanged.emit();
}

Task* TaskController::undo()
{
    if (m_undo.empty())
        return nullptr;

    const auto request = m_undo.back();
    m_undo.pop_back();
    historyChanged.emit();
    return submit(request, History::Undo);
}

Task* TaskController::redo()
{
    if (m_redo.empty())
        return nullptr;

    const auto request = m_redo.back();
    m_redo.pop_back();
    historyChanged.emit();
    return submit(request, History::Redo);
}

Task* copy(const std::vector<Url>& sources, const Url& destination)
{
    return TaskController::instance()->submit({Operation::Copy, sources, destination, ConflictPolicy::Ask, {}});
}

Task* move(const std::vector<Url>& sources, const Url& destination)
{
    return TaskController::instance()->submit({Operation::Move, sources, destination, ConflictPolicy::Ask, {}});
}

Task* del(const std::vector<Url>& sources)
{
    return TaskController::instance()->submit({Operation::Delete, sources, {}, ConflictPolicy::Fail, {}});
}

Task* mkdir(const Url& destination)
{
    return TaskController::instance()->submit({Operation::Mkdir, {}, destination, ConflictPolicy::Fail, {}});
}

Task* trash(const std::vector<Url>& sources)
{
    return TaskController::instance()->submit({Operation::Trash, sources, {}, ConflictPolicy::Fail, {}});
}

Task* restoreFromTrash(const std::vector<Url>& sources)
{
    return TaskController::instance()->submit({Operation::Restore, sources, {}, ConflictPolicy::Ask, {}});
}

Task* emptyTrash()
{
    return TaskController::instance()->submit({Operation::EmptyTrash, {}, {}, ConflictPolicy::Fail, {}});
}

Task* paste(const std::vector<Url>& urls, bool cut, const Url& destination)
{
    return TaskController::instance()->submit(
        {cut ? Operation::Move : Operation::Copy, urls, destination, ConflictPolicy::Ask, {}});
}

Task* drop(const std::vector<Url>& urls, const Url& destination, DropAction action)
{
    return TaskController::instance()->submit({action == DropAction::Link   ? Operation::Link
                                               : action == DropAction::Move ? Operation::Move
                                                                            : Operation::Copy,
                                               urls, destination, ConflictPolicy::Ask, {}});
}

} // namespace Lingmo::Files
