// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "daemontask.h"
#include "archivejob.h"

#include "base/event_loop.h"

#include "lingmo/files/task.h"

#include <algorithm>

using namespace Lingmo;
using base::dbus::Variant;
using base::dbus::VariantMap;

static constexpr char kInterface[] = "org.lingmo.FileOperations.Task1";

static VariantMap progressMap(const files::TaskProgress &progress)
{
    return {{"preparing", Variant(progress.preparing)},
            {"totalsKnown", Variant(progress.totalsKnown)},
            {"processedBytes", Variant(progress.processedBytes)},
            {"totalBytes", Variant(progress.totalBytes)},
            {"processedItems", Variant(progress.processedItems)},
            {"totalItems", Variant(progress.totalItems)},
            {"elapsedMs", Variant(progress.elapsedMs)},
            {"currentPath", Variant(progress.currentPath)}};
}

static std::string fileNameOf(const std::string &path)
{
    const size_t slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

DaemonTask::DaemonTask(base::dbus::Connection *bus, std::string objectPath, files::Task *task)
    : m_objectPath(std::move(objectPath))
    , m_task(task)
    , m_object(bus, m_objectPath, kInterface)
{
    initialize();
    addMembers();
}

DaemonTask::DaemonTask(base::dbus::Connection *bus, std::string objectPath, int operation,
                       std::vector<std::string> sources, std::string outputPath)
    : m_objectPath(std::move(objectPath))
    , m_object(bus, m_objectPath, kInterface)
{
    m_snapshot = {
        {"operationName", Variant(std::string(operation == 0 ? "compress" : "extract"))},
        {"sourceCount", Variant(int32_t(sources.size()))},
        {"sourceName", Variant(sources.empty() ? std::string() : fileNameOf(sources.front()))},
        {"destinationName", Variant(fileNameOf(outputPath))},
        {"progress", Variant(VariantMap {{"preparing", Variant(true)},
                                         {"totalsKnown", Variant(false)},
                                         {"processedItems", Variant(int32_t(0))},
                                         {"totalItems", Variant(int32_t(100))},
                                         {"currentPath", Variant(std::string())}})},
        {"finished", Variant(false)},
        {"waitingForConflict", Variant(false)},
        {"waitingForPassword", Variant(false)},
        {"passwordId", Variant(uint32_t(0))},
        {"passwordRetry", Variant(false)},
        {"canceled", Variant(false)},
        {"errorCode", Variant(int32_t(0))},
        {"errorString", Variant(std::string())},
        {"outputs", Variant(std::vector<std::string>())},
    };

    m_archive = std::make_unique<ArchiveJob>(operation == 0 ? ArchiveJob::Compress : ArchiveJob::Extract,
                                             std::move(sources), std::move(outputPath));
    const base::LoopHandle loop = base::EventLoop::current()->handle();
    const auto progress = [this](const std::function<void(VariantMap &)> &edit) {
        VariantMap info = m_snapshot["progress"].value<VariantMap>();
        edit(info);
        m_snapshot["progress"] = Variant(info);
        changed({{"progress", Variant(info)}});
    };
    m_archive->onProgress = [this, loop, progress, alive = m_alive](int value) {
        loop.post([alive, progress, value] {
            if (!*alive)
                return;
            progress([value](VariantMap &info) {
                info["preparing"] = Variant(value < 0);
                info["totalsKnown"] = Variant(value >= 0);
                info["processedItems"] = Variant(int32_t(std::max(0, value)));
            });
        });
    };
    m_archive->onCurrentFile = [loop, progress, alive = m_alive](const std::string &path) {
        loop.post([alive, progress, path] {
            if (*alive)
                progress([&path](VariantMap &info) { info["currentPath"] = Variant(path); });
        });
    };
    m_archive->onPasswordRequested = [this, loop, alive = m_alive](uint32_t request, bool retry) {
        loop.post([this, alive, request, retry] {
            if (!*alive)
                return;
            // Nobody is left to answer: the job must not wait for good.
            if (owner().empty()) {
                m_archive->cancel();
                return;
            }
            m_snapshot["waitingForPassword"] = Variant(true);
            m_snapshot["passwordId"] = Variant(request);
            m_snapshot["passwordRetry"] = Variant(retry);
            changed({{"waitingForPassword", Variant(true)},
                     {"passwordId", Variant(request)},
                     {"passwordRetry", Variant(retry)}});
        });
    };
    m_archive->onPasswordAccepted = [this, loop, alive = m_alive] {
        loop.post([this, alive] {
            if (*alive)
                passwordSettled();
        });
    };
    m_archive->onCompleted = [this, loop, alive = m_alive](bool success, bool canceled, const std::string &error,
                                                           const std::string &output) {
        loop.post([this, alive, success, canceled, error, output] {
            if (!*alive)
                return;
            m_snapshot["waitingForPassword"] = Variant(false);
            m_snapshot["passwordId"] = Variant(uint32_t(0));
            m_snapshot["finished"] = Variant(true);
            m_snapshot["canceled"] = Variant(canceled);
            m_snapshot["errorCode"] = Variant(int32_t(success || canceled ? 0 : 1));
            m_snapshot["errorString"] = Variant(error);
            m_snapshot["outputs"] = Variant(output.empty() ? std::vector<std::string>() : std::vector<std::string> {output});
            m_finished = true;
            m_object.emitSignal("Finished", m_snapshot);
            finished.emit();
        });
    };
    addMembers();
    m_archive->start();
}

DaemonTask::~DaemonTask()
{
    *m_alive = false;
    // The job's thread ends before its callbacks' captures go.
    m_archive.reset();
}

void DaemonTask::initialize()
{
    m_snapshot = {{"operationName", Variant(m_task->operationName())},
                  {"sourceCount", Variant(int32_t(m_task->sourceCount()))},
                  {"sourceName", Variant(m_task->sourceName())},
                  {"destinationName", Variant(m_task->destinationName())},
                  {"progress", Variant(progressMap(m_task->progressInfo()))},
                  {"finished", Variant(false)},
                  {"waitingForConflict", Variant(false)},
                  {"conflictId", Variant(uint32_t(0))},
                  {"conflictSource", Variant(std::string())},
                  {"conflictPath", Variant(std::string())},
                  {"canceled", Variant(false)},
                  {"errorCode", Variant(int32_t(0))},
                  {"errorString", Variant(std::string())}};

    m_subscriptions.push_back(m_task->progressInfoChanged.connect([this] {
        const Variant progress(progressMap(m_task->progressInfo()));
        m_snapshot["progress"] = progress;
        changed({{"progress", progress}});
    }));
    m_subscriptions.push_back(
            m_task->conflictRequested.connect([this](uint32_t conflict, const std::string &source,
                                                     const std::string &destination) {
                // Nobody is left to answer: a worker must not wait for good.
                if (owner().empty()) {
                    m_task->cancel();
                    return;
                }
                m_snapshot["waitingForConflict"] = Variant(true);
                m_snapshot["conflictId"] = Variant(conflict);
                m_snapshot["conflictSource"] = Variant(source);
                m_snapshot["conflictPath"] = Variant(destination);
                changed({{"waitingForConflict", Variant(true)},
                         {"conflictId", Variant(conflict)},
                         {"conflictSource", Variant(source)},
                         {"conflictPath", Variant(destination)}});
                m_object.emitSignal("ConflictRequested", conflict, source, destination);
            }));
    m_task->whenFinished([this, alive = m_alive](files::Task *task) {
        if (!*alive)
            return;
        m_snapshot["progress"] = Variant(progressMap(task->progressInfo()));
        m_snapshot["finished"] = Variant(true);
        m_snapshot["waitingForConflict"] = Variant(false);
        m_snapshot["conflictId"] = Variant(uint32_t(0));
        m_snapshot["canceled"] = Variant(task->wasCanceled());
        m_snapshot["errorCode"] = Variant(int32_t(task->outcome().error.code));
        m_snapshot["errorString"] = Variant(task->errorString());
        m_finished = true;
        m_subscriptions.clear();
        // The controller destroys the task once its result is out.
        m_task = nullptr;
        m_object.emitSignal("Finished", m_snapshot);
        finished.emit();
    });
}

void DaemonTask::addMembers()
{
    m_object.addMethod("GetSnapshot", [this] {
        return snapshot();
    });
    m_object.addMethod("Cancel", [this] {
        cancel();
    });
    m_object.addMethod("ResolveConflict", "uib", "", [this](base::dbus::MethodCall &call) {
        uint32_t conflict = 0;
        int32_t policy = 0;
        bool applyToAll = false;
        // Ask is no answer: the worker would ignore it and wait on unseen.
        if (!call.read(conflict, policy, applyToAll) || policy < int32_t(files::ConflictPolicy::Fail)
            || policy >= int32_t(files::ConflictPolicy::Ask)) {
            call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unknown conflict policy");
            return;
        }
        // A second answer, or one to a conflict another client answered, must
        // not reach the conflict asked next.
        if (!m_task || !m_task->resolveConflict(conflict, policy, applyToAll)) {
            call.replyError("org.freedesktop.DBus.Error.Failed", "No such conflict is waiting");
            return;
        }
        call.reply();
        m_snapshot["waitingForConflict"] = Variant(false);
        m_snapshot["conflictId"] = Variant(uint32_t(0));
        changed({{"waitingForConflict", Variant(false)}, {"conflictId", Variant(uint32_t(0))}});
    }, {{"conflict", "policy", "applyToAll"}, {}});
    m_object.addMethod("AnswerPassword", "us", "", [this](base::dbus::MethodCall &call) {
        uint32_t request = 0;
        std::string password;
        if (!call.read(request, password)) {
            call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
            return;
        }
        // A second answer, or one to a request already gone, must not reach
        // the request asked next.
        if (!m_archive || !m_archive->answerPassword(request, password)) {
            call.replyError("org.freedesktop.DBus.Error.Failed", "No such password request is waiting");
            return;
        }
        call.reply();
    }, {{"request", "password"}, {}});
    m_object.addSignal("Changed", "a{sv}", {"snapshot"});
    m_object.addSignal("ConflictRequested", "uss", {"conflict", "source", "destination"});
    m_object.addSignal("Finished", "a{sv}", {"snapshot"});
}

bool DaemonTask::publish()
{
    return m_object.publish();
}

void DaemonTask::changed(const VariantMap &update)
{
    m_object.emitSignal("Changed", update);
}

VariantMap DaemonTask::snapshot()
{
    if (m_finished && !m_outcomeRead) {
        m_outcomeRead = true;
        if (m_outcomeReadCallback)
            base::EventLoop::current()->post([alive = m_alive, callback = m_outcomeReadCallback] {
                if (*alive)
                    callback();
            });
    }
    return m_snapshot;
}

void DaemonTask::setOwner(const std::string &owner)
{
    m_snapshot["owner"] = Variant(owner);
}

std::string DaemonTask::owner() const
{
    const auto found = m_snapshot.find("owner");
    return found != m_snapshot.end() ? found->second.value<std::string>() : std::string();
}

bool DaemonTask::waitingForConflict() const
{
    const auto found = m_snapshot.find("waitingForConflict");
    return found != m_snapshot.end() && found->second.value<bool>();
}

bool DaemonTask::waitingForPassword() const
{
    const auto found = m_snapshot.find("waitingForPassword");
    return found != m_snapshot.end() && found->second.value<bool>();
}

void DaemonTask::passwordSettled()
{
    if (!waitingForPassword())
        return;
    m_snapshot["waitingForPassword"] = Variant(false);
    m_snapshot["passwordId"] = Variant(uint32_t(0));
    changed({{"waitingForPassword", Variant(false)}, {"passwordId", Variant(uint32_t(0))}});
}

void DaemonTask::orphan()
{
    setOwner({});
    if (m_task && waitingForConflict())
        m_task->cancel();
    if (m_archive && waitingForPassword())
        m_archive->cancel();
}

void DaemonTask::cancel()
{
    if (m_task)
        m_task->cancel();
    if (m_archive)
        m_archive->cancel();
}
