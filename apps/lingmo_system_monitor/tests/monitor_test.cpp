// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileutil.h"
#include "monitor.h"
#include "preferences.h"
#include "procreader.h"

#include "base/event_loop.h"
#include "base/timer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std::chrono_literals;
namespace base = Lingmo::base;
namespace fs = std::filesystem;

namespace {

std::string processStat(int pid, const std::string& name, uint64_t ticks, uint64_t start, uint32_t flags = 0)
{
    std::vector<std::string> fields(22, "0");
    fields[0] = "S";
    fields[1] = "42";
    fields[6] = std::to_string(flags);
    fields[11] = std::to_string(ticks);
    fields[16] = "5";
    fields[17] = "3";
    fields[19] = std::to_string(start);
    fields[21] = "7";
    std::string result = std::to_string(pid) + " (" + name + ")";
    for (const std::string& field : fields)
        result += ' ' + field;
    return result;
}

std::string commandLine(const std::vector<std::string>& arguments)
{
    std::string result;
    for (const std::string& argument : arguments)
        result += argument + '\0';
    return result;
}

bool writeFile(const std::string& path, const std::string& contents)
{
    fs::create_directories(fs::path(path).parent_path());
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << contents;
    return bool(file);
}

ProcessRecord record(int pid, const std::string& name, double cpu = 0)
{
    ProcessRecord process;
    process.pid = pid;
    process.uid = getuid();
    process.name = name;
    process.displayName = name;
    process.command = "/usr/bin/" + name + " --example";
    process.user = "test-user";
    process.startTicks = 1000;
    process.cpu = cpu;
    process.memory = uint64_t(pid) * 1024;
    return process;
}

class TemporaryDirectory
{
public:
    TemporaryDirectory()
    {
        std::string pattern = (fs::temp_directory_path() / "monitor-test-XXXXXX").string();
        m_path = mkdtemp(pattern.data());
    }
    ~TemporaryDirectory() { fs::remove_all(m_path); }
    const std::string& path() const { return m_path; }

private:
    std::string m_path;
};

// A child process, killed when it goes.
class Child
{
public:
    explicit Child(std::vector<std::string> arguments, bool pipeInput = false)
    {
        int input[2] = {-1, -1};
        if (pipeInput && pipe(input) != 0)
            return;
        m_pid = fork();
        if (m_pid == 0) {
            if (pipeInput) {
                dup2(input[0], STDIN_FILENO);
                close(input[0]);
                close(input[1]);
            }
            std::vector<char*> argv;
            for (std::string& argument : arguments)
                argv.push_back(argument.data());
            argv.push_back(nullptr);
            execv(argv[0], argv.data());
            _exit(127);
        }
        if (pipeInput) {
            close(input[0]);
            m_input = input[1];
        }
        // Until it runs the program, the child is a copy of the test.
        const std::string program = fs::canonical(arguments[0]).string();
        for (int i = 0; i < 300 && symLinkTarget("/proc/" + std::to_string(m_pid) + "/exe") != program; ++i)
            usleep(10000);
    }
    ~Child()
    {
        if (m_input >= 0)
            close(m_input);
        if (m_pid > 0 && !m_finished) {
            kill(m_pid, SIGKILL);
            waitpid(m_pid, nullptr, 0);
        }
    }

    int pid() const { return m_pid; }
    void closeInput()
    {
        close(m_input);
        m_input = -1;
    }
    bool running()
    {
        if (m_finished)
            return false;
        int status = 0;
        if (waitpid(m_pid, &status, WNOHANG) == m_pid)
            m_finished = true;
        return !m_finished;
    }
    bool waitForFinished(std::chrono::milliseconds timeout)
    {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while (running() && std::chrono::steady_clock::now() < deadline)
            usleep(5000);
        return m_finished;
    }

private:
    int m_pid = -1;
    int m_input = -1;
    bool m_finished = false;
};

std::string ownExecutable()
{
    return fs::canonical("/proc/self/exe").string();
}

// Runs the loop until the condition holds, or the time is up.
bool runUntil(const std::function<bool()>& condition, std::chrono::milliseconds timeout = 5000ms)
{
    if (condition())
        return true;
    base::EventLoop& loop = *base::EventLoop::current();
    base::Timer poll;
    poll.onTimeout = [&] {
        if (condition())
            loop.quit();
    };
    poll.start(5ms);
    base::Timer deadline;
    deadline.setSingleShot(true);
    deadline.onTimeout = [&] { loop.quit(); };
    deadline.start(timeout);
    loop.run();
    return condition();
}

class MonitorTest : public testing::Test
{
protected:
    base::EventLoop m_loop;
};

// Desktop entries as the session's registry would resolve them: by id first,
// then by the program the entry runs.
struct FixtureApplications
{
    struct Entry
    {
        DesktopApplication application;
        std::string program;
    };
    std::vector<Entry> entries;

    ApplicationLookup lookup() const
    {
        ApplicationLookup lookup;
        lookup.resolve = [this](const std::string& name, const std::string& program)
            -> std::optional<DesktopApplication> {
            const Entry* best = nullptr;
            int bestScore = 0;
            for (const Entry& entry : entries) {
                int score = 0;
                if (entry.application.id == name)
                    score = 100;
                else if (entry.program == program)
                    score = 85;
                else if (fileName(entry.program) == name)
                    score = 80;
                if (score > bestScore) {
                    best = &entry;
                    bestScore = score;
                }
            }
            return best ? std::optional(best->application) : std::nullopt;
        };
        lookup.byId = [this](const std::string& id) -> std::optional<DesktopApplication> {
            for (const Entry& entry : entries) {
                if (entry.application.id == id)
                    return entry.application;
            }
            return std::nullopt;
        };
        return lookup;
    }
};

} // namespace

TEST(ProcReaderTest, CpuAccounting)
{
    const auto cpu = ProcReader::parseCpu("cpu 100 20 30 800 40 5 3 2 50 10");
    EXPECT_EQ(cpu.total, 1000u);
    EXPECT_EQ(cpu.idle, 840u);
    EXPECT_EQ(ProcReader::cpuUsage({2000, 1700}, {1000, 850}), 15.0);
    EXPECT_EQ(ProcReader::cpuUsage({1000, 800}, {1000, 800}), 0.0);
    EXPECT_EQ(ProcReader::cpuUsage({10, 5}, {1000, 800}), 0.0);
    EXPECT_EQ(ProcReader::cpuUsage({1100, 790}, {1000, 800}), 100.0);
}

TEST(ProcReaderTest, MemoryAccounting)
{
    const auto memory = ProcReader::parseMemory("MemTotal: 1000 kB\nMemAvailable: 600 kB\n"
                                                "MemFree: 100 kB\nCached: 200 kB\nSReclaimable: 50 kB\n"
                                                "SwapTotal: 500 kB\nSwapFree: 300 kB\n");
    EXPECT_EQ(memory.used, 400u * 1024);
    EXPECT_EQ(memory.percent, 40.0);
    EXPECT_EQ(memory.swapUsed, 200u * 1024);
    const auto fallback = ProcReader::parseMemory("MemTotal: 1000 kB\nMemFree: 100 kB\nBuffers: 50 kB\n"
                                                  "Cached: 200 kB\nSReclaimable: 50 kB\nShmem: 30 kB\n");
    EXPECT_EQ(fallback.available, 370u * 1024);
    EXPECT_EQ(ProcReader::parseMemory({}).percent, 0.0);
    EXPECT_EQ(ProcReader::parseMemory("MemTotal: 100 kB\nMemAvailable: 200 kB\n").used, 0u);
}

TEST(ProcReaderTest, ProcessNamesAndFields)
{
    ProcessRecord process;
    ASSERT_TRUE(ProcReader::parseProcess(processStat(123, "worker (pool) 3", 25, 999), &process, 4096));
    EXPECT_EQ(process.pid, 123);
    EXPECT_EQ(process.parentPid, 42);
    EXPECT_EQ(process.name, "worker (pool) 3");
    EXPECT_EQ(process.cpuTicks, 25u);
    EXPECT_EQ(process.startTicks, 999u);
    EXPECT_EQ(process.rss, 7u * 4096);
    EXPECT_EQ(process.memory, process.rss);
    EXPECT_EQ(process.threads, 3);
    EXPECT_EQ(process.nice, 5);
    EXPECT_FALSE(process.kernelThread);
    ASSERT_TRUE(ProcReader::parseProcess(processStat(2, "kthreadd", 0, 1, 0x00208040), &process, 4096));
    EXPECT_TRUE(process.kernelThread);
    EXPECT_FALSE(ProcReader::parseProcess("123 (short) R 1", &process, 4096));
    EXPECT_FALSE(ProcReader::parseProcess("broken", &process, 4096));

    uint32_t uid = 0;
    EXPECT_TRUE(ProcReader::parseUid("Name:\tagent\nUmask:\t0077\nUid:\t1000\t1001\t1000\t1001\n", &uid));
    EXPECT_EQ(uid, 1001u);
    EXPECT_FALSE(ProcReader::parseUid("Name:\tagent\n", &uid));

    EXPECT_EQ(ProcReader::applicationProgram(commandLine({"/usr/lib/chromium/chromium", "--type=renderer"})),
              "/usr/lib/chromium/chromium");
    EXPECT_EQ(ProcReader::applicationProgram(commandLine({"/usr/bin/python3", "-u", "/usr/bin/fixture-app", "--flag"})),
              "/usr/bin/fixture-app");
    EXPECT_EQ(ProcReader::applicationProgram(commandLine({"sh", "-c", "fixture-app"})), "");

    const std::string browser = "/usr/lib/chromium/chromium";
    const std::string title = commandLine({"/usr/lib/chromium/chromium --type=renderer --lang=en-US"});
    EXPECT_EQ(ProcReader::commandArguments(title, browser),
              commandLine({"/usr/lib/chromium/chromium", "--type=renderer", "--lang=en-US"}));
    EXPECT_EQ(ProcReader::commandArguments(title, "/usr/bin/other"), title);
    const std::string separate = commandLine({"/usr/lib/chromium/chromium", "--user-data-dir=/tmp/a b"});
    EXPECT_EQ(ProcReader::commandArguments(separate, browser), separate);
    EXPECT_EQ(ProcReader::helperType(ProcReader::commandArguments(title, browser)), "renderer");

    EXPECT_EQ(ProcReader::helperType(commandLine({"/usr/lib/chromium/chromium"})), "");
    EXPECT_EQ(ProcReader::helperType(commandLine({"/usr/lib/chromium/chromium", "--type=gpu-process", "--lang=en-US"})),
              "gpu-process");
    EXPECT_EQ(ProcReader::helperType(commandLine({"/usr/lib/chromium/chromium", "--type=renderer", "--extension-process"})),
              "extension");
    EXPECT_EQ(ProcReader::helperType(commandLine({"/usr/lib/chromium/chromium", "--type=utility",
                                                  "--utility-sub-type=network.mojom.NetworkService"})),
              "network.mojom.NetworkService");
    EXPECT_EQ(ProcReader::helperType(commandLine({"/usr/bin/fixture-app", "--verbose", "--type=renderer"})), "");
    EXPECT_EQ(ProcReader::helperName("gpu-process"), "GPU");
    EXPECT_EQ(ProcReader::helperName("network.mojom.NetworkService"), "Network");
    EXPECT_EQ(ProcReader::helperName("unzip.mojom.Unzipper"), "Utility");
    EXPECT_EQ(ProcReader::helperName("broker"), "broker");

    ProcessRecord helper = record(11, "fixture");
    helper.program = "/opt/fixture/fixture-app";
    helper.helperType = "renderer";
    EXPECT_EQ(ProcessList::programName(helper), "fixture-app (Renderer)");
}

TEST(ProcReaderTest, SamplingAndPidReuse)
{
    TemporaryDirectory directory;
    const std::string proc = directory.path() + "/proc";
    const std::string sys = directory.path() + "/sys";
    ASSERT_TRUE(writeFile(proc + "/stat", "cpu 100 0 50 850 0 0 0 0\ncpu0 100 0 50 850 0 0 0 0\n"));
    ASSERT_TRUE(writeFile(proc + "/meminfo", "MemTotal: 1000 kB\nMemAvailable: 600 kB\n"));
    ASSERT_TRUE(writeFile(proc + "/123/stat", processStat(123, "sample", 20, 1000)));
    fs::create_symlink("/usr/bin/fixture-tool (deleted)", proc + "/123/exe");
    ProcReader reader(proc, sys);
    const auto first = reader.sample();
    ASSERT_TRUE(first.available);
    EXPECT_TRUE(first.baseline);
    ASSERT_EQ(first.processes.size(), 1u);
    EXPECT_EQ(first.processes[0].uid, getuid());
    EXPECT_FALSE(first.processes[0].kernelThread);
    EXPECT_EQ(first.processes[0].cpu, 0.0);
    EXPECT_EQ(first.processes[0].displayName, "fixture-tool");
    EXPECT_FALSE(first.processes[0].ioAvailable);

    ASSERT_TRUE(writeFile(proc + "/stat", "cpu 200 0 100 1700 0 0 0 0\ncpu0 200 0 100 1700 0 0 0 0\n"));
    ASSERT_TRUE(writeFile(proc + "/123/stat", processStat(123, "sample", 70, 1000)));
    const auto second = reader.sample();
    EXPECT_FALSE(second.baseline);
    EXPECT_EQ(second.metrics.cpuPercent, 15.0);
    EXPECT_EQ(second.processes[0].cpu, 5.0);

    // An exec keeps the PID and start time; the new name reads the new program.
    fs::remove(proc + "/123/exe");
    fs::create_symlink("/usr/bin/other-tool", proc + "/123/exe");
    ASSERT_TRUE(writeFile(proc + "/stat", "cpu 250 0 125 2125 0 0 0 0\ncpu0 250 0 125 2125 0 0 0 0\n"));
    ASSERT_TRUE(writeFile(proc + "/123/stat", processStat(123, "other-tool", 95, 1000)));
    const auto execed = reader.sample();
    EXPECT_EQ(execed.processes[0].displayName, "other-tool");
    EXPECT_EQ(execed.processes[0].cpu, 5.0);

    ASSERT_TRUE(writeFile(proc + "/stat", "cpu 300 0 150 2550 0 0 0 0\n"));
    ASSERT_TRUE(writeFile(proc + "/123/stat", processStat(123, "replacement", 9999, 2000)));
    const auto reused = reader.sample();
    EXPECT_EQ(reused.processes[0].cpu, 0.0);
    EXPECT_EQ(reused.processes[0].name, "replacement");
}

TEST(ProcReaderTest, OfflineProcessors)
{
    TemporaryDirectory directory;
    const std::string proc = directory.path() + "/proc";
    ASSERT_TRUE(writeFile(proc + "/meminfo", "MemTotal: 1000 kB\n"));
    ASSERT_TRUE(writeFile(proc + "/stat", "cpu 0 0 0 200 0 0 0 0\ncpu0 0 0 0 100 0 0 0 0\n"
                                          "cpu2 0 0 0 100 0 0 0 0\n"));
    ProcReader reader(proc, directory.path() + "/sys");
    reader.sample();
    ASSERT_TRUE(writeFile(proc + "/stat", "cpu 100 0 0 300 0 0 0 0\ncpu0 0 0 0 200 0 0 0 0\n"
                                          "cpu2 100 0 0 100 0 0 0 0\n"));
    const auto sample = reader.sample();
    ASSERT_EQ(sample.metrics.cores.size(), 2u);
    EXPECT_EQ(sample.metrics.cores[0].number, 0);
    EXPECT_EQ(sample.metrics.cores[0].usage, 0.0);
    EXPECT_EQ(sample.metrics.cores[1].number, 2);
    EXPECT_EQ(sample.metrics.cores[1].usage, 100.0);
}

TEST(ProcReaderTest, DeviceAccounting)
{
    TemporaryDirectory directory;
    const std::string proc = directory.path() + "/proc";
    const std::string sys = directory.path() + "/sys";
    ASSERT_TRUE(writeFile(proc + "/stat", "cpu 1 0 1 98 0 0 0 0\n"));
    ASSERT_TRUE(writeFile(proc + "/meminfo", "MemTotal: 1000 kB\n"));
    fs::create_directories(sys + "/class/net/test0/device");
    fs::create_directories(sys + "/block/sda/device");
    const std::string network = "test0: 1000 0 0 0 0 0 0 0 2000 0 0 0 0 0 0 0\n"
                                "lo: 5000 0 0 0 0 0 0 0 5000 0 0 0 0 0 0 0\n"
                                "veth0: 9000 0 0 0 0 0 0 0 9000 0 0 0 0 0 0 0\n";
    ASSERT_TRUE(writeFile(proc + "/net/dev", network));
    ASSERT_TRUE(writeFile(proc + "/diskstats", "8 0 sda 0 0 100 0 0 0 200 0 0 0 0\n8 1 sda1 0 0 100 0 0 0 200 0 0 0 0\n"));
    ProcReader reader(proc, sys);
    const auto first = reader.sample();
    EXPECT_EQ(first.metrics.received, 1000u);
    EXPECT_EQ(first.metrics.interfaces.size(), 2u);
    EXPECT_EQ(first.metrics.disks.size(), 1u);
    EXPECT_EQ(first.metrics.diskRead, 0.0);
    usleep(20000);
    ASSERT_TRUE(writeFile(proc + "/net/dev", "test0: 3000 0 0 0 0 0 0 0 4000 0 0 0 0 0 0 0\n"));
    ASSERT_TRUE(writeFile(proc + "/diskstats", "8 0 sda 0 0 200 0 0 0 400 0 0 0 0\n"));
    const auto second = reader.sample();
    EXPECT_GT(second.metrics.download, 0);
    EXPECT_GT(second.metrics.diskRead, 0);
    ASSERT_TRUE(writeFile(proc + "/net/dev", "test0: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"));
    EXPECT_EQ(reader.sample().metrics.download, 0.0);
}

TEST(ProcReaderTest, UnavailableProc)
{
    TemporaryDirectory directory;
    ProcReader reader(directory.path(), directory.path());
    const auto snapshot = reader.sample();
    EXPECT_FALSE(snapshot.available);
    EXPECT_FALSE(snapshot.error.empty());
}

TEST(ProcReaderTest, Volumes)
{
    EXPECT_EQ(DiskReader::unescapePath("/media/My\\040Disk"), "/media/My Disk");
    const auto volumes = DiskReader::volumes();
    ASSERT_FALSE(volumes.empty());
    EXPECT_TRUE(std::any_of(volumes.begin(), volumes.end(), [](const Volume& volume) { return volume.mount == "/"; }));
    for (const Volume& volume : volumes) {
        EXPECT_NE(volume.type, "tmpfs");
        EXPECT_FALSE(volume.mount.starts_with("/proc") || volume.mount.starts_with("/sys")
                     || volume.mount.starts_with("/dev/"));
        EXPECT_GT(volume.total, 0u);
        EXPECT_LE(volume.used, volume.total);
    }
}

TEST(ProcessListTest, UpdatesKeepIdentity)
{
    ProcessList list;
    list.apply({record(10, "first"), record(20, "second")});
    list.apply({record(5, "new"), record(20, "second", 30)});
    EXPECT_EQ(list.processes().size(), 2u);
    EXPECT_EQ(list.find(10), nullptr);
    ASSERT_NE(list.find(20), nullptr);
    EXPECT_EQ(list.find(20)->name, "second");
    EXPECT_EQ(list.find(20)->cpu, 30.0);
    list.apply({});
    EXPECT_EQ(list.find(20), nullptr);
    EXPECT_EQ(list.count(), 0u);
}

TEST(ProcessListTest, FilteringAndSorting)
{
    ProcessList list;
    auto foreign = record(30, "foreign", 90);
    foreign.uid = getuid() + 1;
    list.apply({record(10, "Terminal", 10), record(20, "Browser", 30), foreign});
    EXPECT_EQ(list.count(), 2u);
    EXPECT_EQ(list.at(0)->pid, 20);
    list.setSearch("terminal");
    EXPECT_EQ(list.count(), 1u);
    list.setSearch("--example");
    EXPECT_EQ(list.count(), 2u);
    list.setSearch("20");
    EXPECT_EQ(list.count(), 1u);
    list.setSearch({});
    list.setAllUsers(true);
    EXPECT_EQ(list.count(), 3u);
    EXPECT_EQ(list.at(0)->pid, 30);
    list.sortBy("pid");
    EXPECT_EQ(list.at(0)->pid, 10);
    list.sortBy("pid");
    EXPECT_EQ(list.at(0)->pid, 30);
    EXPECT_EQ(list.indexOf(20), 1);
    EXPECT_EQ(list.at(100), nullptr);
    list.sortBy("name");
    EXPECT_EQ(list.at(0)->displayName, "Browser");
    EXPECT_TRUE(list.ascending());
}

TEST(ProcessListTest, SearchFoldsCase)
{
    ProcessList list;
    auto process = record(10, "Ärger");
    list.apply({process});
    list.setSearch("ärger");
    EXPECT_EQ(list.count(), 1u);
    list.setSearch("  ÄRGER ");
    EXPECT_EQ(list.count(), 1u);
}

class TerminateTest : public testing::TestWithParam<bool>
{
};

TEST_P(TerminateTest, TerminateChild)
{
    const bool force = GetParam();
    Child child({"/usr/bin/sleep", "30"});
    ASSERT_GT(child.pid(), 0);
    ProcessRecord process;
    ASSERT_TRUE(ProcReader::parseProcess(readFile("/proc/" + std::to_string(child.pid()) + "/stat"), &process, 1));
    EXPECT_EQ(Monitor::signalProcess(child.pid(), process.startTicks, force), "");
    EXPECT_TRUE(child.waitForFinished(3000ms));
}

INSTANTIATE_TEST_SUITE_P(Monitor, TerminateTest, testing::Values(false, true));

TEST(SignalTest, RejectReusedAndProtectedPid)
{
    EXPECT_FALSE(Monitor::signalProcess(1, 0, false).empty());
    EXPECT_FALSE(Monitor::signalProcess(getpid(), 0, false).empty());
    Child child({"/usr/bin/sleep", "30"});
    EXPECT_FALSE(Monitor::signalProcess(child.pid(), 0, true).empty());
    EXPECT_TRUE(child.running());
}

TEST(SignalTest, RejectForeignProcess)
{
    // kthreadd belongs to root; the kernel refuses the signal, not the monitor.
    const std::string stat = readFile("/proc/2/stat");
    if (stat.empty() || getuid() == 0)
        GTEST_SKIP() << "Needs an unprivileged user and a visible kthreadd";
    ProcessRecord process;
    ASSERT_TRUE(ProcReader::parseProcess(stat, &process, 1));
    EXPECT_EQ(Monitor::signalProcess(2, process.startTicks, false), "You can only end your own processes.");
}

TEST_F(MonitorTest, DesktopIdentity)
{
    const std::string program = ownExecutable();
    FixtureApplications fixture;
    fixture.entries.push_back({{"a-fixture-shortcut", "Fixture Shortcut", {}}, program});
    fixture.entries.push_back({{fileName(program), "Fixture Application", "fixture-icon"}, program});

    Monitor monitor(fixture.lookup());
    ASSERT_TRUE(runUntil([&] { return monitor.processes().indexOf(getpid()) >= 0; }));
    const ProcessRecord* self = monitor.processes().find(getpid());
    ASSERT_NE(self, nullptr);
    EXPECT_EQ(self->displayName, "Fixture Application");
    EXPECT_EQ(self->iconName, "fixture-icon");
}

TEST(ProcReaderTest, ScopesAndPss)
{
    EXPECT_EQ(ProcReader::scopeApplication("0::/user.slice/user-1000.slice/user@1000.service/app.slice/"
                                           "app-lingmo-lingmo\\x2dterminal-2091752.scope\n"),
              "lingmo-terminal");
    EXPECT_EQ(ProcReader::scopeApplication("0::/user.slice/app.slice/app-org.example.Viewer-12.scope"),
              "org.example.Viewer");
    EXPECT_EQ(ProcReader::scopeApplication("0::/app.slice/app-gnome-org.example.Editor@3a9f.service"),
              "org.example.Editor");
    EXPECT_EQ(ProcReader::scopeApplication("0::/app.slice/app-org.example.Player.service"), "org.example.Player");
    EXPECT_EQ(ProcReader::scopeApplication("0::/user.slice/user-1000.slice/session-580.scope"), "");
    EXPECT_EQ(ProcReader::scopeApplication("0::/app.slice/xdg-desktop-portal-lingmo.service"), "");
    EXPECT_EQ(ProcReader::scopeApplication("0::/app.slice/app-broken.scope"), "");
    EXPECT_EQ(ProcReader::scopeApplication({}), "");

    uint64_t pss = 0;
    EXPECT_TRUE(ProcReader::parsePss("55d0-7ffd ---p 00000000 00:00 0 [rollup]\nRss: 161924 kB\n"
                                     "Pss: 99783 kB\nPss_Dirty: 42360 kB\n",
                                     &pss));
    EXPECT_EQ(pss, 99783u * 1024);
    EXPECT_FALSE(ProcReader::parsePss("Rss: 10 kB\n", &pss));
}

TEST(ProcReaderTest, SampledScopesAndPss)
{
    TemporaryDirectory directory;
    const std::string proc = directory.path() + "/proc";
    ASSERT_TRUE(writeFile(proc + "/stat", "cpu 100 0 50 850 0 0 0 0\n"));
    ASSERT_TRUE(writeFile(proc + "/meminfo", "MemTotal: 1000 kB\nMemAvailable: 600 kB\n"));
    ASSERT_TRUE(writeFile(proc + "/100/stat", processStat(100, "viewer", 0, 1000)));
    ASSERT_TRUE(writeFile(proc + "/100/cgroup", "0::/user.slice/user-1000.slice/session-2.scope\n"));
    ASSERT_TRUE(writeFile(proc + "/100/smaps_rollup", "Pss: 300 kB\n"));
    ASSERT_TRUE(writeFile(proc + "/200/stat", processStat(200, "tool", 0, 1000)));
    ASSERT_TRUE(writeFile(proc + "/200/smaps_rollup", "Pss: 50 kB\n"));
    ASSERT_TRUE(writeFile(proc + "/400/stat", processStat(400, "locked", 0, 1000)));
    ProcReader reader(proc, directory.path() + "/sys");
    auto snapshot = reader.sample();
    ASSERT_EQ(snapshot.processes.size(), 3u);
    EXPECT_TRUE(snapshot.processes[0].scopeId.empty());

    // Every process of the user is measured; an unreadable one keeps its resident memory.
    EXPECT_TRUE(snapshot.processes[0].pssAvailable);
    EXPECT_EQ(snapshot.processes[0].memory, 300u * 1024);
    EXPECT_TRUE(snapshot.processes[1].pssAvailable);
    EXPECT_EQ(snapshot.processes[1].memory, 50u * 1024);
    EXPECT_FALSE(snapshot.processes[2].pssAvailable);
    EXPECT_GT(snapshot.processes[2].rss, 0u);
    EXPECT_EQ(snapshot.processes[2].memory, snapshot.processes[2].rss);

    // The launcher adopts the process into its scope after the first read.
    ASSERT_TRUE(writeFile(proc + "/100/cgroup", "0::/user.slice/app.slice/app-lingmo-org.example.Viewer-100.scope\n"));
    snapshot = reader.sample();
    EXPECT_EQ(snapshot.processes[0].scopeId, "org.example.Viewer");

    // Settled processes keep their scope and, for a few seconds, their PSS;
    // new processes are measured at once.
    ASSERT_TRUE(writeFile(proc + "/100/cgroup", "0::/elsewhere.scope\n"));
    ASSERT_TRUE(writeFile(proc + "/100/smaps_rollup", "Pss: 400 kB\n"));
    std::string child = processStat(300, "helper", 0, 1100);
    child.replace(child.find(" S 42 "), 6, " S 200 ");
    ASSERT_TRUE(writeFile(proc + "/300/stat", child));
    ASSERT_TRUE(writeFile(proc + "/300/smaps_rollup", "Pss: 20 kB\n"));
    snapshot = reader.sample();
    ASSERT_EQ(snapshot.processes.size(), 4u);
    EXPECT_EQ(snapshot.processes[0].scopeId, "org.example.Viewer");
    EXPECT_EQ(snapshot.processes[0].memory, 300u * 1024);
    EXPECT_EQ(snapshot.processes[1].memory, 50u * 1024);
    EXPECT_EQ(snapshot.processes[2].parentPid, 200);
    EXPECT_EQ(snapshot.processes[2].memory, 20u * 1024);
}

TEST_F(MonitorTest, ApplicationGroups)
{
    const std::string program = ownExecutable();
    FixtureApplications fixture;
    fixture.entries.push_back({{fileName(program), "Fixture Application", "fixture-icon"}, program});
    // A program of its own: any other sleep of the user's would join the application.
    TemporaryDirectory directory;
    const std::string sleeper = directory.path() + "/fixture-sleeper";
    fs::copy_file("/usr/bin/sleep", sleeper);
    fs::permissions(sleeper, fs::perms::owner_all);
    fixture.entries.push_back({{"fixture-standalone", "Fixture Standalone", {}}, sleeper});
    DesktopApplication hidden{"cat", "Fixture Hidden", {}};
    hidden.noDisplay = true;
    fixture.entries.push_back({hidden, "/usr/bin/cat"});

    Child helper({"/bin/sh", "-c", "read line"}, true);
    Child separate({sleeper, "30"});
    Child handler({"/usr/bin/cat"}, true);

    Monitor monitor(fixture.lookup());
    ProcessList& applications = monitor.applications();
    ASSERT_TRUE(runUntil([&] {
        return applications.indexOf(getpid()) >= 0 && applications.indexOf(separate.pid()) >= 0;
    }));

    // The helper has no entry of its own and the handler only a hidden one,
    // so both join their parent; the second program has a listed entry and
    // stays apart even though this process started it.
    const ProcessRecord* self = applications.at(size_t(applications.indexOf(getpid())));
    ASSERT_NE(self, nullptr);
    EXPECT_EQ(self->displayName, "Fixture Application");
    EXPECT_EQ(self->iconName, "fixture-icon");
    EXPECT_EQ(self->members.size(), 3u);
    std::vector<int> pids;
    std::vector<std::string> names;
    double memory = 0;
    for (const ProcessRecord& member : self->members) {
        pids.push_back(member.pid);
        names.push_back(ProcessList::programName(member));
        memory += double(member.memory);
    }
    const auto contains = [&names](const std::string& name) {
        return std::find(names.begin(), names.end(), name) != names.end();
    };
    EXPECT_TRUE(contains(fileName(program)));
    // Inline shell code names no program; the executable behind /bin/sh does.
    EXPECT_TRUE(contains(fileName(canonicalPath("/bin/sh"))));
    EXPECT_TRUE(contains("cat"));
    std::vector<int> expected{int(getpid()), helper.pid(), handler.pid()};
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(pids, expected);
    EXPECT_EQ(double(self->memory), memory);
    EXPECT_GT(memory, 0);
    const ProcessRecord* led = monitor.application(getpid(), self->startTicks);
    ASSERT_NE(led, nullptr);
    EXPECT_FALSE(Monitor::canManage(*led));

    // A hidden entry claims no name or icon for its process.
    ProcessList& processes = monitor.processes();
    ASSERT_TRUE(runUntil([&] { return processes.indexOf(handler.pid()) >= 0; }));
    const ProcessRecord* handled = processes.find(handler.pid());
    ASSERT_NE(handled, nullptr);
    EXPECT_EQ(handled->displayName, "cat");
    EXPECT_EQ(handled->iconName, "application-x-executable");

    applications.setSearch(std::to_string(helper.pid()));
    EXPECT_EQ(applications.count(), 1u);
    applications.setSearch({});

    const ProcessRecord* other = applications.at(size_t(applications.indexOf(separate.pid())));
    ASSERT_NE(other, nullptr);
    EXPECT_EQ(other->displayName, "Fixture Standalone");
    EXPECT_EQ(other->members.size(), 1u);
    const uint64_t start = other->startTicks;
    const ProcessRecord* standalone = monitor.application(separate.pid(), start);
    ASSERT_NE(standalone, nullptr);
    EXPECT_TRUE(Monitor::canManage(*standalone));
    EXPECT_FALSE(monitor.endApplication(separate.pid(), 1, false).empty());
    EXPECT_TRUE(separate.running());
    EXPECT_EQ(monitor.endApplication(separate.pid(), start, false), "");
    EXPECT_TRUE(separate.waitForFinished(3000ms));

    helper.closeInput();
    handler.closeInput();
    EXPECT_TRUE(helper.waitForFinished(3000ms));
    EXPECT_TRUE(handler.waitForFinished(3000ms));
}

TEST(GpuTest, GpuAccounting)
{
    TemporaryDirectory directory;
    const std::string proc = directory.path() + "/proc";
    const std::string sys = directory.path() + "/sys";
    ASSERT_TRUE(writeFile(proc + "/stat", "cpu 1 0 1 98 0 0 0 0\n"));
    ASSERT_TRUE(writeFile(proc + "/meminfo", "MemTotal: 1000 kB\n"));
    {
        ProcReader reader(proc, sys);
        EXPECT_FALSE(reader.sample().metrics.gpuAvailable);
    }

    const std::string first = sys + "/class/drm/card0/device";
    ASSERT_TRUE(writeFile(first + "/gpu_busy_percent", "37\n"));
    ASSERT_TRUE(writeFile(first + "/mem_info_vram_used", "1024\n"));
    ASSERT_TRUE(writeFile(first + "/mem_info_vram_total", "4096\n"));
    ASSERT_TRUE(writeFile(first + "/product_name", "Fixture Graphics\n"));
    ASSERT_TRUE(writeFile(first + "/power/runtime_status", "active\n"));
    ASSERT_TRUE(writeFile(first + "/hwmon/hwmon4/temp1_input", "41500\n"));
    ASSERT_TRUE(writeFile(first + "/hwmon/hwmon4/freq1_input", "800000000\n"));
    fs::create_directories(sys + "/bus/pci/drivers/fixturegpu");
    fs::create_symlink(sys + "/bus/pci/drivers/fixturegpu", first + "/driver");
    const std::string second = sys + "/class/drm/card1/device";
    ASSERT_TRUE(writeFile(second + "/boot_vga", "1\n"));
    ASSERT_TRUE(writeFile(second + "/power/runtime_status", "suspended\n"));
    ASSERT_TRUE(writeFile(second + "/hwmon/hwmon5/temp1_input", "60000\n"));
    fs::create_directories(sys + "/class/drm/card0-eDP-1/device/hwmon/hwmon6");
    ASSERT_TRUE(writeFile(sys + "/class/drm/card0-eDP-1/device/gpu_busy_percent", "99\n"));
    ASSERT_TRUE(writeFile(sys + "/class/drm/card2/device/vendor", "0x1234\n"));

    ProcReader reader(proc, sys);
    const auto metrics = reader.sample().metrics;
    ASSERT_EQ(metrics.gpus.size(), 2u);

    // The boot display GPU is listed first; it is suspended and is not read.
    const Gpu& suspended = metrics.gpus[0];
    EXPECT_FALSE(suspended.active);
    EXPECT_TRUE(suspended.hasTemperature);
    EXPECT_EQ(suspended.temperature, -1.0);
    EXPECT_FALSE(suspended.hasBusy);

    const Gpu& active = metrics.gpus[1];
    EXPECT_EQ(active.name, "Fixture Graphics");
    EXPECT_EQ(active.driver, "fixturegpu");
    EXPECT_EQ(active.busy, 37);
    EXPECT_EQ(active.temperature, 41.5);
    EXPECT_EQ(active.frequency, 800.0);
    EXPECT_EQ(active.vramUsed, 1024u);
    EXPECT_EQ(active.vramTotal, 4096u);

    // The overview prefers a GPU with an activity counter.
    EXPECT_TRUE(metrics.gpuAvailable);
    EXPECT_TRUE(metrics.gpuHasBusy);
    EXPECT_EQ(metrics.gpuName, "Fixture Graphics");
    EXPECT_EQ(metrics.gpuPercent, 37.0);
    EXPECT_EQ(metrics.gpuTemperature, 41.5);
}

TEST(GpuTest, DrmClientAccounting)
{
    const DrmClient render = GpuReader::parseDrmFdinfo("pos:\t0\ndrm-driver:\ti915\ndrm-client-id:\t12\n"
                                                       "drm-engine-render:\t1000 ns\ndrm-engine-video:\t400 ns\n"
                                                       "drm-engine-capacity-video:\t2\n");
    EXPECT_EQ(render.id, "12");
    EXPECT_EQ(render.engines.size(), 2u);
    EXPECT_EQ(render.engines.at("render").busyTime, 1000u);
    EXPECT_EQ(render.engines.at("video").capacity, 2);
    const DrmClient cycles = GpuReader::parseDrmFdinfo("drm-client-id:\t3\ndrm-cycles-rcs:\t50\n"
                                                       "drm-total-cycles-rcs:\t200\n");
    EXPECT_TRUE(cycles.engines.at("rcs").hasCycles);
    EXPECT_EQ(cycles.engines.at("rcs").totalCycles, 200u);
    EXPECT_TRUE(GpuReader::parseDrmFdinfo("pos:\t0\nflags:\t02100002\n").engines.empty());

    // Clients add up per engine; the busiest engine, over its capacity, is the activity.
    auto client = [](const std::string& id, uint64_t render, uint64_t video) {
        DrmClient result;
        result.id = id;
        result.engines["render"] = {render, 0, 0, 1, true, false};
        result.engines["video"] = {video, 0, 0, 2, true, false};
        return result;
    };
    const DrmClients before{{"1", client("1", 0, 0)}, {"2", client("2", 0, 0)}};
    const DrmClients after{{"1", client("1", 200, 900)}, {"2", client("2", 100, 900)}, {"3", client("3", 1000, 0)}};
    EXPECT_EQ(GpuReader::drmBusy(after, before, 1000), 90.0);
    EXPECT_EQ(GpuReader::drmBusy(after, {}, 1000), 0.0);
    EXPECT_EQ(GpuReader::drmBusy({{"1", client("1", 5000, 0)}}, before, 1000), 100.0);

    TemporaryDirectory directory;
    const std::string proc = directory.path() + "/proc";
    const std::string sys = directory.path() + "/sys";
    ASSERT_TRUE(writeFile(proc + "/stat", "cpu 1 0 1 98 0 0 0 0\n"));
    ASSERT_TRUE(writeFile(proc + "/meminfo", "MemTotal: 1000 kB\n"));
    ASSERT_TRUE(writeFile(proc + "/200/stat", processStat(200, "client", 0, 1000)));
    fs::create_directories(proc + "/200/fd");
    fs::create_symlink("/dev/dri/renderD130", proc + "/200/fd/7");
    fs::create_symlink("/dev/dri/renderD130", proc + "/200/fd/8");
    fs::create_symlink("/dev/null", proc + "/200/fd/9");
    const std::string device = sys + "/class/drm/card3/device";
    fs::create_directories(device + "/drm/card3");
    fs::create_directories(device + "/drm/renderD130");
    ASSERT_TRUE(writeFile(device + "/product_name", "Client Graphics\n"));

    // A card without a counter or a temperature stays hidden until a client reports busyness.
    ASSERT_TRUE(writeFile(proc + "/200/fdinfo/7", "pos:\t0\n"));
    ASSERT_TRUE(writeFile(proc + "/200/fdinfo/8", "pos:\t0\n"));
    ProcReader reader(proc, sys);
    EXPECT_FALSE(reader.sample().metrics.gpuAvailable);

    // A descriptor duplicated by the client counts once.
    const std::string firstInfo = "drm-client-id:\t5\ndrm-cycles-rcs:\t100\ndrm-total-cycles-rcs:\t1000\n";
    ASSERT_TRUE(writeFile(proc + "/200/fdinfo/7", firstInfo));
    ASSERT_TRUE(writeFile(proc + "/200/fdinfo/8", firstInfo));
    auto metrics = reader.sample().metrics;
    EXPECT_TRUE(metrics.gpuHasBusy);
    EXPECT_EQ(metrics.gpuName, "Client Graphics");
    EXPECT_EQ(metrics.gpuPercent, 0.0);

    const std::string secondInfo = "drm-client-id:\t5\ndrm-cycles-rcs:\t400\ndrm-total-cycles-rcs:\t2000\n";
    ASSERT_TRUE(writeFile(proc + "/200/fdinfo/7", secondInfo));
    ASSERT_TRUE(writeFile(proc + "/200/fdinfo/8", secondInfo));
    metrics = reader.sample().metrics;
    EXPECT_EQ(metrics.gpuPercent, 30.0);

    // A descriptor a running process opens later is found within five samples.
    fs::create_symlink("/dev/dri/renderD130", proc + "/200/fd/10");
    double busy = 0;
    for (int sample = 1; sample <= 7 && busy == 0; ++sample) {
        ASSERT_TRUE(writeFile(proc + "/200/fdinfo/10", "drm-client-id:\t6\ndrm-cycles-rcs:\t" + std::to_string(sample * 500)
                                                           + "\ndrm-total-cycles-rcs:\t" + std::to_string(sample * 1000) + "\n"));
        busy = reader.sample().metrics.gpuPercent;
    }
    EXPECT_EQ(busy, 50.0);

    // A suspended card's clients are not read.
    ASSERT_TRUE(writeFile(device + "/power/runtime_status", "suspended\n"));
    metrics = reader.sample().metrics;
    EXPECT_TRUE(metrics.gpuHasBusy);
    EXPECT_EQ(metrics.gpuPercent, 0.0);
}

TEST_F(MonitorTest, FirstPublishedSample)
{
    Monitor monitor;
    int firstHistory = -1;
    const auto subscription = monitor.updated.connect([&] {
        if (firstHistory < 0 && monitor.ready())
            firstHistory = int(monitor.history().timestamps.size());
    });
    ASSERT_TRUE(runUntil([&] { return monitor.ready(); }));
    // The baseline sample only primes the counters and never reaches the history.
    EXPECT_EQ(firstHistory, 1);
    EXPECT_GT(monitor.metrics().memory.total, 0u);
}

TEST_F(MonitorTest, IntervalPreservesHistory)
{
    Monitor monitor;
    ASSERT_TRUE(runUntil([&] { return monitor.ready(); }));
    const History history = monitor.history();
    EXPECT_FALSE(history.cpuPercent.empty());
    for (const int interval : {2000, 5000, 1000, 5000}) {
        monitor.setInterval(interval);
        EXPECT_EQ(monitor.interval(), interval);
        EXPECT_EQ(monitor.historyDuration(), 60);
    }
    monitor.setInterval(42);
    EXPECT_EQ(monitor.interval(), 5000);
    const auto& timestamps = monitor.history().timestamps;
    EXPECT_GE(timestamps.size(), history.timestamps.size());
    EXPECT_EQ(timestamps.front(), history.timestamps.front());
}

TEST_F(MonitorTest, RememberedPreferences)
{
    TemporaryDirectory directory;
    const std::string file = directory.path() + "/lingmo/system-monitor.conf";
    ASSERT_TRUE(writeFile(file, "[WindowSize]\nMain=@Size(840 600)\n"));
    {
        Monitor monitor({}, file);
        EXPECT_EQ(monitor.interval(), 1000);
        EXPECT_EQ(monitor.scope(), 1);
        EXPECT_FALSE(monitor.processes().allUsers());
        EXPECT_EQ(monitor.processes().sortColumn(), "cpu");
        EXPECT_FALSE(monitor.processes().ascending());
        monitor.setScope(2);
        monitor.setInterval(5000);
        monitor.sortBy("memory");
        monitor.sortBy("memory");
    }

    Monitor monitor({}, file);
    EXPECT_EQ(monitor.interval(), 5000);
    EXPECT_EQ(monitor.scope(), 2);
    EXPECT_TRUE(monitor.processes().allUsers());
    for (ProcessList* list : {&monitor.processes(), &monitor.applications()}) {
        EXPECT_EQ(list->sortColumn(), "memory");
        EXPECT_TRUE(list->ascending());
    }
    // The window's own keys stay.
    EXPECT_NE(readFile(file).find("Main=@Size(840 600)"), std::string::npos);
}

TEST(FormatTest, NumbersAsTheLocaleWritesThem)
{
    EXPECT_EQ(Monitor::formatBytes(0), "0 B");
    EXPECT_EQ(Monitor::formatBytes(1023), "1,023 B");
    EXPECT_EQ(Monitor::formatBytes(1024), "1.0 KiB");
    EXPECT_EQ(Monitor::formatBytes(1024 * 1023.95), "1,024.0 KiB");
    EXPECT_EQ(Monitor::formatRate(3.5 * 1024 * 1024), "3.5 MiB/s");
    // Halfway values round away from zero, as the binary value is exactly half.
    EXPECT_EQ(Monitor::formatPercent(12.25), "12.3%");
    EXPECT_EQ(Monitor::formatPercent(0.15), "0.1%");
    EXPECT_EQ(Monitor::formatPercent(99.96), "100.0%");
    EXPECT_EQ(Monitor::formatTemperature(41.5), "42 °C");
    EXPECT_EQ(Monitor::formatBytes(1023.5), "1,024 B");
}
