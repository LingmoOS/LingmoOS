// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitor.h"

#include "fileutil.h"
#include "preferences.h"
#include "procreader.h"

#include "base/i18n.h"
#include "base/locale.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <signal.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <unordered_set>

using namespace std::chrono_literals;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;

// Programs and application ids no process has named for this many samples are
// looked up again if they return; titles some programs set make the set unbounded.
static constexpr uint64_t kRetainedSamples = 60;

Monitor::Monitor(ApplicationLookup lookup, std::string preferences)
    : m_lookup(std::move(lookup))
    , m_preferences(std::move(preferences))
    , m_reader(std::make_shared<ProcReader>())
{
    m_metrics.memory = ProcReader::parseMemory({});
    const Preferences saved = m_preferences.empty() ? Preferences() : Preferences::load(m_preferences);
    m_timer.setInterval(std::chrono::milliseconds(saved.interval));
    m_scope = saved.scope;
    m_processes.setAllUsers(m_scope == 2);
    m_processes.setSorting(saved.sortColumn, saved.sortAscending);
    m_applications.setSorting(saved.sortColumn, saved.sortAscending);

    m_timer.onTimeout = [this] { sample(); };
    m_baselineTimer.setSingleShot(true);
    m_baselineTimer.onTimeout = [this] { sample(); };
    m_timer.start();
    sample();
}

Monitor::~Monitor() = default;

int Monitor::interval() const
{
    return int(std::chrono::duration_cast<std::chrono::milliseconds>(m_timer.interval()).count());
}

void Monitor::setInterval(int interval)
{
    if ((interval != 1000 && interval != 2000 && interval != 5000) || interval == this->interval())
        return;
    m_timer.start(std::chrono::milliseconds(interval));
    savePreferences();
    intervalChanged();
    updated();
}

void Monitor::sample()
{
    // A hung network mount blocks statfs; it must not stall the other measurements.
    if (!m_volumeJob.isPending())
        m_volumeJob = Lingmo::base::runInThread([] { return DiskReader::volumes(); },
                                                  [this](std::vector<Volume> volumes) { m_volumes = std::move(volumes); });

    if (m_sampleJob.isPending())
        return;
    const auto reader = m_reader;
    m_sampleJob = Lingmo::base::runInThread([reader] { return reader->sample(); },
                                              [this](Snapshot snapshot) { acceptSample(std::move(snapshot)); });
}

void Monitor::acceptSample(Snapshot snapshot)
{
    if (snapshot.baseline) {
        // Rates need a previous sample; publish from the next one.
        m_baselineTimer.start(250ms);
        return;
    }

    m_error = snapshot.error;
    if (snapshot.available) {
        m_ready = true;
        m_metrics = std::move(snapshot.metrics);
        m_metrics.volumes = m_volumes;
        ++m_samples;
        for (ProcessRecord& process : snapshot.processes)
            identify(&process);
        m_applications.apply(groupApplications(snapshot.processes));
        const auto stale = [this](const auto& entry) { return m_samples - entry.second.seen > kRetainedSamples; };
        std::erase_if(m_programs, stale);
        std::erase_if(m_applicationIds, stale);
        m_processes.apply(std::move(snapshot.processes));
        const int64_t timestamp
            = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_historyClock)
                  .count();
        m_history.timestamps.push_back(timestamp);
        m_history.cpuPercent.push_back(m_metrics.cpuPercent);
        m_history.memoryPercent.push_back(m_metrics.memory.percent);
        m_history.download.push_back(m_metrics.download);
        m_history.upload.push_back(m_metrics.upload);
        m_history.diskRead.push_back(m_metrics.diskRead);
        m_history.diskWrite.push_back(m_metrics.diskWrite);
        m_history.gpuPercent.push_back(m_metrics.gpuPercent);
        m_history.gpuTemperature.push_back(m_metrics.gpuTemperature);

        // Retain the sample before the visible window to interpolate its left edge.
        const int64_t cutoff = timestamp - historyDuration() * 1000;
        while (m_history.timestamps.size() > 2 && m_history.timestamps[1] < cutoff) {
            for (auto* series : {&m_history.cpuPercent, &m_history.memoryPercent, &m_history.download,
                                 &m_history.upload, &m_history.diskRead, &m_history.diskWrite,
                                 &m_history.gpuPercent, &m_history.gpuTemperature})
                series->erase(series->begin());
            m_history.timestamps.erase(m_history.timestamps.begin());
        }
    }
    updated();
}

void Monitor::applicationsChanged()
{
    m_programs.clear();
    m_applicationIds.clear();
}

Monitor::Application Monitor::describeEntry(const std::optional<DesktopApplication>& entry)
{
    // Hidden entries (portals, handlers) are no application, and their names
    // ("Portal") would disguise the process behind them.
    if (!entry || entry->noDisplay || entry->hidden)
        return {};
    return {entry->id, entry->name, entry->icon};
}

void Monitor::identify(ProcessRecord* process)
{
    if (process->program.empty())
        return;

    auto application = m_programs.find(process->program);
    if (application == m_programs.end()) {
        // A process has no app id; its own name stands in for one, so the
        // application's entry outranks web apps or shortcuts launching it.
        const std::string name = fileName(process->program);
        const auto entry = m_lookup.resolve ? m_lookup.resolve(name, process->program) : std::nullopt;
        application = m_programs.emplace(process->program, describeEntry(entry)).first;
    }
    application->second.seen = m_samples;
    process->appId = application->second.id;
    if (!application->second.name.empty())
        process->displayName = application->second.name;
    if (!application->second.icon.empty())
        process->iconName = application->second.icon;
    if (!process->helperType.empty())
        // TRANSLATORS: a browser's helper process, such as "Chromium (Renderer)".
        process->displayName = format(tr("{name} ({helper})"),
                                      {{"name", process->displayName}, {"helper", ProcReader::helperName(process->helperType)}});
}

std::vector<ProcessRecord> Monitor::groupApplications(const std::vector<ProcessRecord>& processes)
{
    const uint32_t uid = getuid();
    std::unordered_map<int, size_t> rows;
    for (size_t i = 0; i < processes.size(); ++i)
        rows.emplace(processes[i].pid, i);
    const auto row = [&rows](int pid) {
        const auto it = rows.find(pid);
        return it == rows.end() ? -1 : int(it->second);
    };

    // A process with a desktop entry of its own is that application; one
    // without joins its nearest such ancestor, then the application cgroup
    // a launcher put it in. A browser started from a terminal stays apart.
    std::unordered_map<std::string, std::vector<size_t>> groups;
    std::vector<std::string> order;
    for (size_t i = 0; i < processes.size(); ++i) {
        const ProcessRecord& process = processes[i];
        if (process.uid != uid || process.kernelThread)
            continue;

        std::string id = process.appId;
        int ancestor = row(process.parentPid);
        for (int depth = 0; id.empty() && ancestor >= 0 && depth < 64; ++depth) {
            const ProcessRecord& parent = processes[size_t(ancestor)];
            if (parent.uid != uid || parent.pid <= 1)
                break;
            id = parent.appId;
            ancestor = row(parent.parentPid);
        }
        if (id.empty() && !process.scopeId.empty()) {
            auto scope = m_applicationIds.find(process.scopeId);
            if (scope == m_applicationIds.end()) {
                const auto entry = m_lookup.byId ? m_lookup.byId(process.scopeId) : std::nullopt;
                scope = m_applicationIds.emplace(process.scopeId, describeEntry(entry)).first;
            }
            scope->second.seen = m_samples;
            id = scope->second.id;
        }
        if (id.empty())
            continue;
        auto& group = groups[id];
        if (group.empty())
            order.push_back(id);
        group.push_back(i);
    }

    std::vector<ProcessRecord> applications;
    for (const std::string& id : order) {
        const std::vector<size_t>& indexes = groups[id];
        std::unordered_set<int> pids;
        for (const size_t index : indexes)
            pids.insert(processes[index].pid);

        // The oldest process the application started itself stands for it.
        const ProcessRecord* main = nullptr;
        for (const size_t index : indexes) {
            const ProcessRecord& process = processes[index];
            if (pids.count(process.parentPid))
                continue;
            const bool own = process.appId == id;
            const bool mainOwn = main && main->appId == id;
            if (!main || (own && !mainOwn) || (own == mainOwn && process.startTicks < main->startTicks))
                main = &process;
        }
        // Reused pids can make every process the parent of another; take the oldest.
        if (!main) {
            for (const size_t index : indexes) {
                const ProcessRecord& process = processes[index];
                if (!main || process.startTicks < main->startTicks)
                    main = &process;
            }
        }

        auto application = m_applicationIds.find(id);
        if (application == m_applicationIds.end()) {
            const auto entry = m_lookup.byId ? m_lookup.byId(id) : std::nullopt;
            application = m_applicationIds
                              .emplace(id, entry ? Application{entry->id, entry->name, entry->icon}
                                                 : Application{id, main->displayName, main->iconName})
                              .first;
        }
        application->second.seen = m_samples;

        ProcessRecord group = *main;
        group.appId = id;
        group.displayName = application->second.name.empty() ? main->displayName : application->second.name;
        group.iconName = application->second.icon.empty() ? main->iconName : application->second.icon;
        group.cpu = 0;
        group.memory = 0;
        group.readRate = 0;
        group.writeRate = 0;
        group.threads = 0;
        group.ioAvailable = false;
        group.members.clear();
        for (const size_t index : indexes) {
            const ProcessRecord& process = processes[index];
            group.cpu += process.cpu;
            group.memory += process.memory;
            group.threads += process.threads;
            if (process.ioAvailable) {
                group.ioAvailable = true;
                group.readRate += process.readRate;
                group.writeRate += process.writeRate;
            }
            group.members.push_back(process);
        }
        group.cpu = std::min(100.0, group.cpu);
        applications.push_back(std::move(group));
    }

    std::sort(applications.begin(), applications.end(),
              [](const auto& left, const auto& right) { return left.pid < right.pid; });
    return applications;
}

bool Monitor::canManage(const ProcessRecord& process)
{
    if (process.members.empty())
        return process.uid == getuid() && process.pid > 1 && process.pid != getpid();
    return std::all_of(process.members.cbegin(), process.members.cend(),
                       [](const ProcessRecord& member) { return canManage(member); });
}

const ProcessRecord* Monitor::process(int pid, uint64_t startTicks) const
{
    const ProcessRecord* record = m_processes.find(pid);
    return record && record->startTicks == startTicks ? record : nullptr;
}

const ProcessRecord* Monitor::application(int pid, uint64_t startTicks) const
{
    const ProcessRecord* record = m_applications.find(pid);
    return record && record->startTicks == startTicks ? record : nullptr;
}

static const char* exitedText()
{
    return tr("The process has already exited.");
}

std::string Monitor::endApplication(int pid, uint64_t startTicks, bool force)
{
    const ProcessRecord* record = application(pid, startTicks);
    if (!record)
        return tr("The application has already quit.");
    if (!canManage(*record))
        return tr("This application cannot be quit here.");

    // Asking only the processes the application started lets it close its
    // own children; forcing it stops every one of them.
    std::unordered_set<int> pids;
    for (const ProcessRecord& member : record->members)
        pids.insert(member.pid);
    std::string error;
    bool signalled = false;
    for (const ProcessRecord& member : record->members) {
        if (!force && pids.count(member.parentPid))
            continue;
        const std::string result = signalProcess(member.pid, member.startTicks, force);
        if (result.empty())
            signalled = true;
        else if (result != exitedText() && error.empty())
            error = result;
    }
    if (!error.empty())
        return error;
    if (!signalled)
        return tr("The application has already quit.");
    sample();
    return {};
}

void Monitor::setScope(int scope)
{
    scope = std::clamp(scope, 0, 2);
    if (m_scope == scope)
        return;
    m_scope = scope;
    m_processes.setAllUsers(m_scope == 2);
    savePreferences();
    scopeChanged();
}

void Monitor::sortBy(const std::string& column)
{
    m_processes.sortBy(column);
    m_applications.sortBy(column);
    savePreferences();
}

void Monitor::savePreferences() const
{
    if (m_preferences.empty())
        return;
    Preferences preferences;
    preferences.interval = interval();
    preferences.scope = m_scope;
    preferences.sortColumn = m_processes.sortColumn();
    preferences.sortAscending = m_processes.ascending();
    preferences.save(m_preferences);
}

std::string Monitor::endProcess(int pid, uint64_t startTicks, bool force)
{
    const std::string result = signalProcess(pid, startTicks, force);
    if (result.empty())
        sample();
    return result;
}

std::string Monitor::signalProcess(int pid, uint64_t startTicks, bool force)
{
    if (pid <= 1 || pid == getpid())
        return tr("This process cannot be ended here.");

    // A pidfd binds the signal to this process even if its PID is reused.
    const int descriptor = int(syscall(SYS_pidfd_open, pid, 0));
    if (descriptor < 0)
        return errno == ESRCH ? exitedText() : format(tr("Unable to open the process: {error}"), {{"error", std::strerror(errno)}});

    ProcessRecord record;
    std::string error;
    if (!ProcReader::parseProcess(readFile("/proc/" + std::to_string(pid) + "/stat"), &record, 1)
        || record.startTicks != startTicks) {
        error = exitedText();
    } else if (syscall(SYS_pidfd_send_signal, descriptor, force ? SIGKILL : SIGTERM, nullptr, 0) != 0) {
        const int failure = errno;
        if (failure == ESRCH)
            error = exitedText();
        else if (failure == EPERM)
            error = tr("You can only end your own processes.");
        else
            error = format(tr("Unable to end the process: {error}"), {{"error", std::strerror(failure)}});
    }
    close(descriptor);
    return error;
}

std::string Monitor::formatBytes(double bytes)
{
    static const char* const units[] = {trNoop("B"), trNoop("KiB"), trNoop("MiB"), trNoop("GiB"), trNoop("TiB")};
    int unit = 0;
    bytes = std::max(0.0, bytes);
    while (bytes >= 1024 && unit < 4) {
        bytes /= 1024;
        ++unit;
    }
    // TRANSLATORS: a size, such as "1.5 MiB".
    return format(tr("{value} {unit}"),
                  {{"value", Lingmo::base::Locale::system().toString(bytes, unit == 0 ? 0 : 1)}, {"unit", tr(units[unit])}});
}

std::string Monitor::formatRate(double bytes)
{
    return format(tr("{size}/s"), {{"size", formatBytes(bytes)}});
}

std::string Monitor::formatPercent(double percent)
{
    return Lingmo::base::Locale::system().toString(percent, 1) + '%';
}

std::string Monitor::formatTemperature(double celsius)
{
    return Lingmo::base::Locale::system().toString(celsius, 0) + " °C";
}
