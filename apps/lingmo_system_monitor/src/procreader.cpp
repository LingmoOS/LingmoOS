// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "procreader.h"
#include "fileutil.h"

#include "base/i18n.h"

#include <algorithm>
#include <climits>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

using Lingmo::i18n::tr;

// python, perl, ruby and lua with any version suffix, and a few more.
static bool isScriptInterpreter(std::string_view name)
{
    for (const std::string_view versioned : {"python", "perl", "ruby", "lua"}) {
        if (name.starts_with(versioned)
            && std::all_of(name.begin() + versioned.size(), name.end(),
                           [](char c) { return (c >= '0' && c <= '9') || c == '.'; }))
            return true;
    }
    for (const std::string_view exact : {"node", "nodejs", "gjs", "sh", "bash", "dash"}) {
        if (name == exact)
            return true;
    }
    return false;
}

static bool parseInt(std::string_view text, int* value)
{
    int64_t number = 0;
    if (!parseNumber(text, &number) || number < INT_MIN || number > INT_MAX)
        return false;
    *value = int(number);
    return true;
}

static int toInt(std::string_view text)
{
    int value = 0;
    return parseInt(text, &value) ? value : 0;
}

static std::string_view field(const std::vector<std::string_view>& fields, size_t index)
{
    return index < fields.size() ? fields[index] : std::string_view();
}

ProcReader::ProcReader(std::string procRoot, std::string sysRoot)
    : m_procRoot(std::move(procRoot))
    , m_networks(m_procRoot, sysRoot)
    , m_disks(m_procRoot, sysRoot)
    , m_gpus(m_procRoot, sysRoot)
{
    m_uid = getuid();
    const long pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize > 0)
        m_pageSize = pageSize;

    const std::string cpuInfo = readFile(m_procRoot + "/cpuinfo");
    for (const std::string_view line : split(cpuInfo, '\n')) {
        if (line.starts_with("model name") || line.starts_with("Hardware")) {
            const size_t colon = line.find(':');
            m_cpuName = validUtf8(trimmed(colon == std::string_view::npos ? line : line.substr(colon + 1)));
            break;
        }
    }
    if (m_cpuName.empty()) {
        utsname system;
        if (uname(&system) == 0)
            m_cpuName = std::string_view(system.machine) == "aarch64" ? "arm64" : system.machine;
    }
}

CpuCounters ProcReader::parseCpu(std::string_view line)
{
    const auto values = splitFields(line);
    CpuCounters result;
    if (values.size() < 5 || !values[0].starts_with("cpu"))
        return result;

    // Guest counters are already included in user and nice time.
    for (size_t i = 1; i < std::min<size_t>(values.size(), 9); ++i)
        result.total += toUnsigned(values[i]);
    result.idle = toUnsigned(values[4]);
    if (values.size() > 5)
        result.idle += toUnsigned(values[5]);
    return result;
}

double ProcReader::cpuUsage(const CpuCounters& current, const CpuCounters& previous)
{
    const uint64_t total = counterDelta(current.total, previous.total);
    const uint64_t idle = counterDelta(current.idle, previous.idle);
    return total > 0 ? 100.0 * (total - std::min(idle, total)) / total : 0;
}

MemoryMetrics ProcReader::parseMemory(std::string_view contents)
{
    std::unordered_map<std::string, uint64_t> values;
    for (const std::string_view line : split(contents, '\n')) {
        const auto parts = splitFields(line);
        if (parts.size() >= 2)
            values[std::string(parts[0].substr(0, parts[0].size() - 1))] = toUnsigned(parts[1]) * 1024;
    }
    const auto value = [&values](const std::string& key, uint64_t fallback = 0) {
        const auto it = values.find(key);
        return it == values.end() ? fallback : it->second;
    };

    MemoryMetrics memory;
    memory.total = value("MemTotal");
    memory.cache = value("Cached") + value("SReclaimable");
    const uint64_t fallback = value("MemFree") + value("Buffers") + memory.cache
                              - std::min(memory.cache, value("Shmem"));
    memory.available = std::min(memory.total, value("MemAvailable", fallback));
    memory.used = memory.total - memory.available;
    memory.percent = memory.total > 0 ? 100.0 * (memory.total - memory.available) / memory.total : 0;
    memory.swapTotal = value("SwapTotal");
    memory.swapUsed = memory.swapTotal - std::min(memory.swapTotal, value("SwapFree"));
    memory.swapPercent = memory.swapTotal > 0 ? 100.0 * memory.swapUsed / memory.swapTotal : 0;
    return memory;
}

bool ProcReader::parseProcess(std::string_view contents, ProcessRecord* process, int64_t pageSize)
{
    const size_t opening = contents.find('(');
    const size_t closing = contents.rfind(')');
    if (opening == std::string_view::npos || opening < 1 || closing == std::string_view::npos
        || closing <= opening)
        return false;

    const auto values = splitFields(contents.substr(closing + 1));
    if (values.size() < 22)
        return false;

    const bool validPid = parseInt(trimmed(contents.substr(0, opening)), &process->pid);
    if (!validPid)
        process->pid = 0;
    process->name = validUtf8(contents.substr(opening + 1, closing - opening - 1));
    process->state = std::string(values[0]);
    process->parentPid = toInt(values[1]);
    uint64_t flags = 0;
    process->kernelThread = parseNumber(values[6], &flags) && flags <= UINT_MAX
                            && (flags & 0x00200000); // PF_KTHREAD
    process->cpuTicks = toUnsigned(values[11]) + toUnsigned(values[12]);
    process->nice = toInt(values[16]);
    process->threads = toInt(values[17]);
    const bool validStart = parseNumber(values[19], &process->startTicks);
    if (!validStart)
        process->startTicks = 0;
    process->rss = uint64_t(std::max<int64_t>(0, toSigned(values[21]))) * pageSize;
    process->memory = process->rss;
    return validPid && validStart && process->pid > 0;
}

std::string ProcReader::stateName(const std::string& state)
{
    if (state == "R")
        return tr("Running");
    if (state == "S" || state == "I")
        return tr("Sleeping");
    if (state == "D")
        return tr("Waiting for I/O");
    if (state == "T" || state == "t")
        return tr("Stopped");
    if (state == "Z")
        return tr("Zombie");
    return state;
}

bool ProcReader::parseUid(std::string_view status, uint32_t* uid)
{
    for (const std::string_view line : split(status, '\n')) {
        if (!line.starts_with("Uid:"))
            continue;
        // Real, effective, saved and filesystem IDs; ownership follows the effective one.
        uint64_t value = 0;
        if (!parseNumber(field(splitFields(line), 2), &value) || value > UINT_MAX)
            return false;
        *uid = uint32_t(value);
        return true;
    }
    return false;
}

std::string ProcReader::commandArguments(std::string_view commandLine, const std::string& executable)
{
    // Chromium rewrites its title as a single argument joined by spaces.
    const std::string prefix = executable + ' ';
    const size_t firstNull = commandLine.find('\0');
    const bool single = commandLine.empty() ? true : firstNull == commandLine.size() - 1;
    if (executable.empty() || !single || !commandLine.starts_with(prefix))
        return std::string(commandLine);

    std::string arguments(commandLine);
    std::replace(arguments.begin(), arguments.end(), ' ', '\0');
    return arguments;
}

std::string ProcReader::applicationProgram(std::string_view commandLine)
{
    const auto arguments = split(commandLine, '\0');
    std::string program(field(arguments, 0));

    // A script started through its shebang runs as the interpreter, which
    // would match every application written in that language. The script
    // names the application; inline code (-c, -e) or a module names none.
    if (isScriptInterpreter(fileName(program))) {
        program.clear();
        for (size_t i = 1; i < arguments.size(); ++i) {
            const std::string_view argument = arguments[i];
            if (argument.empty() || argument == "-c" || argument == "-e" || argument == "-m")
                break;
            if (argument.starts_with('-'))
                continue;

            program = argument;
            break;
        }
    }

    return validUtf8(program);
}

std::string ProcReader::helperType(std::string_view commandLine)
{
    // Chromium starts its helpers from the browser's own binary with --type=
    // first, so every helper would otherwise carry the browser's name.
    const auto arguments = split(commandLine, '\0');
    const std::string_view first = field(arguments, 1);
    if (!first.starts_with("--type="))
        return {};

    const std::string_view type = first.substr(7);
    for (const std::string_view argument : arguments) {
        if (type == "renderer" && argument == "--extension-process")
            return "extension";
        if (type == "utility" && argument.starts_with("--utility-sub-type="))
            return validUtf8(argument.substr(19));
    }
    return validUtf8(type);
}

std::string ProcReader::helperName(const std::string& type)
{
    if (type == "renderer")
        return tr("Renderer");
    if (type == "extension")
        return tr("Extension");
    if (type == "gpu-process")
        return tr("GPU");
    if (type == "zygote")
        return tr("Zygote");
    if (type == "network.mojom.NetworkService")
        return tr("Network");
    if (type == "audio.mojom.AudioService")
        return tr("Audio");
    if (type == "storage.mojom.StorageService")
        return tr("Storage");
    if (type == "video_capture.mojom.VideoCaptureService")
        return tr("Video Capture");
    if (type == "utility" || type.find(".mojom.") != std::string::npos)
        return tr("Utility");
    return type;
}

static int hexValue(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

std::string ProcReader::scopeApplication(std::string_view cgroup)
{
    std::string_view path;
    for (const std::string_view line : split(cgroup, '\n')) {
        if (line.starts_with("0::"))
            path = line.substr(3);
    }

    // Launchers name units app[-<launcher>]-<id>-<random>.scope or
    // app[-<launcher>]-<id>[@<random>].service, with '-' in the id escaped.
    const size_t slash = path.rfind('/');
    const std::string_view unit = slash == std::string_view::npos ? path : path.substr(slash + 1);
    if (!unit.starts_with("app-"))
        return {};
    std::string_view id;
    if (unit.ends_with(".scope")) {
        const auto parts = split(unit.substr(0, unit.size() - 6), '-');
        if (parts.size() < 3)
            return {};
        id = parts[parts.size() - 2];
    } else if (unit.ends_with(".service")) {
        const auto parts = split(unit.substr(0, unit.size() - 8), '-');
        if (parts.size() < 2)
            return {};
        id = parts.back();
        const size_t instance = id.find('@');
        if (instance != std::string_view::npos)
            id = id.substr(0, instance);
    } else {
        return {};
    }

    std::string unescaped;
    for (size_t i = 0; i < id.size(); ++i) {
        if (id[i] == '\\' && i + 3 < id.size() && id[i + 1] == 'x' && hexValue(id[i + 2]) >= 0
            && hexValue(id[i + 3]) >= 0) {
            unescaped += char(hexValue(id[i + 2]) * 16 + hexValue(id[i + 3]));
            i += 3;
        } else {
            unescaped += id[i];
        }
    }
    return validUtf8(unescaped);
}

bool ProcReader::parsePss(std::string_view rollup, uint64_t* pss)
{
    for (const std::string_view line : split(rollup, '\n')) {
        if (!line.starts_with("Pss:"))
            continue;
        uint64_t value = 0;
        const bool valid = parseNumber(field(splitFields(line), 1), &value);
        *pss = valid ? value * 1024 : 0;
        return valid;
    }
    return false;
}

void ProcReader::readIdentity(const std::string& path, ProcessRecord* process) const
{
    process->executable = validUtf8(symLinkTarget(path + "/exe"));
    std::string executable = process->executable;
    if (executable.ends_with(" (deleted)") && !fileExists(executable))
        executable.resize(executable.size() - 10);

    const std::string commandLine = readFile(path + "/cmdline");
    const std::string arguments = commandArguments(commandLine, executable);
    process->program = commandLine.empty() ? executable : applicationProgram(arguments);
    process->helperType = helperType(arguments);
    std::string command = commandLine;
    std::replace(command.begin(), command.end(), '\0', ' ');
    process->command = validUtf8(trimmed(command));

    const std::string executableName = fileName(executable);
    process->displayName = executableName.empty() ? process->name : executableName;
    process->iconName = "application-x-executable";
}

std::string ProcReader::userName(uint32_t uid)
{
    auto it = m_users.find(uid);
    if (it == m_users.end()) {
        std::vector<char> buffer(16384);
        passwd entry;
        passwd* result = nullptr;
        const int error = getpwuid_r(uid, &entry, buffer.data(), buffer.size(), &result);
        it = m_users.emplace(uid, error == 0 && result ? validUtf8(entry.pw_name) : std::to_string(uid)).first;
    }
    return it->second;
}

Snapshot ProcReader::sample()
{
    Snapshot snapshot;
    const std::string stat = readFile(m_procRoot + "/stat");
    const std::string memory = readFile(m_procRoot + "/meminfo");
    if (!stat.starts_with("cpu ") || memory.empty()) {
        snapshot.error = tr("System information is unavailable.");
        return snapshot;
    }

    const int64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            std::chrono::steady_clock::now() - m_clock)
                            .count();
    const double elapsed = m_hasPreviousSample ? (now - m_previousTime) / 1e9 : 0;
    Metrics& metrics = snapshot.metrics;
    metrics.memory = parseMemory(memory);
    std::unordered_map<std::string, CpuCounters> cpus;
    for (const std::string_view line : split(stat, '\n')) {
        if (!line.starts_with("cpu"))
            continue;
        const std::string name(splitFields(line).front());
        const CpuCounters counters = parseCpu(line);
        cpus[name] = counters;
        const auto previous = m_previousCpu.find(name);
        const double usage = previous != m_previousCpu.end() ? cpuUsage(counters, previous->second) : 0;
        if (name == "cpu")
            metrics.cpuPercent = usage;
        else
            metrics.cores.push_back({toInt(std::string_view(name).substr(3)), usage});
    }
    metrics.cpuName = m_cpuName;

    const auto counter = [](const std::unordered_map<std::string, CpuCounters>& counters) {
        const auto it = counters.find("cpu");
        return it == counters.end() ? uint64_t(0) : it->second.total;
    };
    const uint64_t totalDelta = counterDelta(counter(cpus), counter(m_previousCpu));
    std::unordered_map<int, ProcessRecord> processes;
    for (const std::string& directory : directoryEntries(m_procRoot)) {
        int number = 0;
        if (!parseInt(directory, &number))
            continue;

        const std::string path = m_procRoot + '/' + directory;
        ProcessRecord process;
        if (!parseProcess(readFile(path + "/stat"), &process, m_pageSize))
            continue;

        // A non-dumpable process's directory belongs to root, so status has to
        // name the owner of a root-owned one. It is costly to generate.
        struct stat info;
        process.uid = ::stat(path.c_str(), &info) == 0 ? info.st_uid : uint32_t(-2);
        if (process.uid == 0 && !process.kernelThread
            && !parseUid(readFile(path + "/status"), &process.uid))
            continue;
        process.user = userName(process.uid);

        const std::string io = readFile(path + "/io");
        bool hasRead = false;
        bool hasWrite = false;
        for (const std::string_view line : split(io, '\n')) {
            const auto values = splitFields(line);
            if (values.size() < 2)
                continue;
            if (values[0] == "read_bytes:") {
                process.readBytes = toUnsigned(values[1]);
                hasRead = true;
            } else if (values[0] == "write_bytes:") {
                process.writeBytes = toUnsigned(values[1]);
                hasWrite = true;
            }
        }
        process.ioAvailable = hasRead && hasWrite;

        // Identity is read once per program. A second stat read proves the
        // files above did not come from a process that reused the PID.
        const auto known = m_previousProcesses.find(process.pid);
        const ProcessRecord* previous = known != m_previousProcesses.end() ? &known->second : nullptr;
        const bool sameProcess = previous && previous->pid == process.pid
                                 && previous->startTicks == process.startTicks;
        // An exec keeps the PID and start time; the name it changes tells the new program.
        if (sameProcess && previous->name == process.name) {
            process.executable = previous->executable;
            process.program = previous->program;
            process.command = previous->command;
            process.helperType = previous->helperType;
            process.displayName = previous->displayName;
            process.iconName = previous->iconName;
            process.scopeId = previous->scopeId;
            process.pss = previous->pss;
            process.pssTime = previous->pssTime;
            process.pssAvailable = previous->pssAvailable;
            // The launcher moves a new process into its scope after starting
            // it, so the first read can predate that; the second one settles.
            process.scopeSettled = true;
            if (!previous->scopeSettled && process.uid == m_uid && !process.kernelThread)
                process.scopeId = scopeApplication(readFile(path + "/cgroup"));
        } else {
            readIdentity(path, &process);
            if (process.uid == m_uid && !process.kernelThread)
                process.scopeId = scopeApplication(readFile(path + "/cgroup"));
            ProcessRecord verified;
            if (!parseProcess(readFile(path + "/stat"), &verified, m_pageSize)
                || verified.startTicks != process.startTicks)
                continue;
        }
        if (sameProcess) {
            if (totalDelta > 0)
                process.cpu = std::min(100.0, 100.0 * counterDelta(process.cpuTicks, previous->cpuTicks)
                                                  / totalDelta);
            if (previous->ioAvailable && process.ioAvailable) {
                process.readRate = byteRate(process.readBytes, previous->readBytes, elapsed);
                process.writeRate = byteRate(process.writeBytes, previous->writeBytes, elapsed);
            }
        }
        processes[process.pid] = process;
        snapshot.processes.push_back(std::move(process));
    }
    std::sort(snapshot.processes.begin(), snapshot.processes.end(),
              [](const auto& left, const auto& right) { return left.pid < right.pid; });
    metrics.processCount = int(snapshot.processes.size());

    // PSS counts shared pages once across processes. Only the user's own
    // processes can be read, and the kernel walks every page table to report
    // it, so it is refreshed every few seconds rather than on every sample.
    for (ProcessRecord& process : snapshot.processes) {
        if (process.uid != m_uid || process.kernelThread) {
            process.pssAvailable = false;
            continue;
        }
        if (!process.pssAvailable || now - process.pssTime >= 5'000'000'000) {
            process.pssAvailable = parsePss(
                readFile(m_procRoot + '/' + std::to_string(process.pid) + "/smaps_rollup"), &process.pss);
            process.pssTime = now;
            ProcessRecord& retained = processes[process.pid];
            retained.pss = process.pss;
            retained.pssTime = process.pssTime;
            retained.pssAvailable = process.pssAvailable;
        }
        if (process.pssAvailable)
            process.memory = process.pss;
    }

    m_networks.sample(&metrics, elapsed);
    m_disks.sample(&metrics, elapsed);
    m_gpus.sample(&metrics, snapshot.processes, elapsed);

    m_previousCpu = std::move(cpus);
    m_previousProcesses = std::move(processes);
    m_previousTime = now;
    snapshot.baseline = !m_hasPreviousSample;
    m_hasPreviousSample = true;
    snapshot.available = true;
    return snapshot;
}
