// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appruntime.h"
#include "applicationscope.h"
#include "chromiumswitches.h"

#include "base/application_registry.h"
#include "base/event_loop.h"
#include "base/file.h"
#include "base/logging.h"
#include "base/paths.h"
#include "base/strings.h"
#include "common/detached_process.h"
#include "common/language_environment.h"
#include "common/proxy_settings.h"

#include <algorithm>
#include <filesystem>

#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QString>
#include <QStringList>

namespace base    = Lingmo::Base;
namespace strings = Lingmo::Base::strings;
using namespace std::chrono_literals;

static const base::LogCategory kLog("lingmo.appruntime");
static constexpr auto kTerminateTimeout = 5000ms;
static const uint32_t kSessionPid = static_cast<uint32_t>(::getppid());

AppRuntime::AppRuntime()
{
    m_reaper.setInterval(2000ms);
    m_reaper.onTimeout = [this] { reap(); };
}

Lingmo::Core::Environment AppRuntime::applicationEnvironment()
{
    Lingmo::Core::Environment environment = Lingmo::Core::processEnvironment();
    Lingmo::Core::language::apply(environment);
    Lingmo::Core::proxy::apply(environment);
    environment.erase("XDG_ACTIVATION_TOKEN");
    return environment;
}

uint32_t AppRuntime::launchApplication(const QString &appId, const QStringList &arguments,
                                       const QString &activationToken)
{
    if (appId.isEmpty())
        return 0;

    base::ApplicationRegistry &registry = base::ApplicationRegistry::instance();
    auto entry = registry.byId(appId.toStdString());
    if (!entry && appId.contains(QLatin1Char('/'))) {
        entry = registry.byPath(appId.toStdString());
        if (!entry) {
            QString name = QString::fromStdString(
                std::filesystem::path(appId.toStdString()).filename().string());
            const int dot = name.lastIndexOf(QLatin1Char('.'));
            if (dot != -1)
                name.truncate(dot);
            entry = registry.byId(name.toStdString());
        }
    }

    if (!entry) {
        LM_WARNING(kLog) << "AppRuntime: no desktop entry for" << appId;
        return 0;
    }

    // Convert QStringList arguments to std::vector<std::string> for the registry call
    std::vector<std::string> stdArgs;
    for (const QString &a : arguments)
        stdArgs.push_back(a.toStdString());

    std::vector<std::string> command = entry->commandForArguments(stdArgs);
    if (command.empty())
        return 0;

    if (entry->terminal) {
        std::string terminal = base::paths::findExecutable("x-terminal-emulator");
        if (terminal.empty())
            terminal = base::paths::findExecutable("xterm");
        if (terminal.empty()) {
            LM_WARNING(kLog) << "AppRuntime: no terminal emulator for" << appId;
        } else {
            command.insert(command.begin(), {terminal, "-e"});
        }
    }

    QStringList cmd;
    for (const auto &s : command)
        cmd << QString::fromStdString(s);
    return startProcess(QString::fromStdString(entry->id), cmd,
                        QString::fromStdString(entry->workingDirectory), activationToken);
}

uint32_t AppRuntime::launchCommand(const QString &appId, const QStringList &command,
                                   const QString &workingDirectory, const QString &activationToken)
{
    if (command.isEmpty() || command.front().isEmpty())
        return 0;

    QString id = appId;
    if (id.isEmpty())
        id = QString::fromStdString(
            std::filesystem::path(command.front().toStdString()).filename().string());

    return startProcess(id, command, workingDirectory, activationToken);
}

uint32_t AppRuntime::startProcess(const QString &appId, const QStringList &command,
                                  const QString &workingDirectory, const QString &activationToken)
{
    Lingmo::Core::Environment environment = applicationEnvironment();
    const QStringList arguments = withChromiumSwitches(command);
    if (!activationToken.isEmpty())
        environment["XDG_ACTIVATION_TOKEN"] = activationToken.toStdString();

    std::vector<std::string> stdCommand;
    for (const QString &a : arguments)
        stdCommand.push_back(a.toStdString());

    const pid_t started = Lingmo::Core::startDetached(
        stdCommand, workingDirectory.toStdString(), environment);
    if (started <= 0) {
        LM_WARNING(kLog) << "AppRuntime: failed to start" << command.join(QLatin1Char(' '));
        return 0;
    }
    const uint32_t pid = static_cast<uint32_t>(started);

    if (const auto entry = base::ApplicationRegistry::instance().byId(appId.toStdString()))
        ApplicationScope::adopt(entry->id, pid);
    m_instances[pid] = Instance{appId, startTime(pid)};

    if (!m_reaper.isActive())
        m_reaper.start();

    applicationLaunched.emit(appId, pid);
    return pid;
}

bool AppRuntime::quitApplication(const QString &appId)
{
    if (appId.isEmpty())
        return false;
    bool result = false;
    for (const uint32_t pid : pidsForApplication(appId))
        result |= terminate(pid);
    return result;
}

bool AppRuntime::quitAll()
{
    QList<uint32_t> pids;
    for (const auto &[pid, instance] : m_instances)
        pids.push_back(pid);
    bool result = false;
    for (const uint32_t pid : pids)
        result |= terminate(pid);
    return result;
}

bool AppRuntime::quitByPid(uint32_t pid)  { return terminate(pid); }

bool AppRuntime::forceQuitByPid(uint32_t pid)
{
    const auto it = m_instances.find(pid);
    if (it == m_instances.end())
        return false;
    const auto entry = base::ApplicationRegistry::instance().byId(it->second.appId.toStdString());
    if (!entry)
        return false;
    ApplicationScope::killAll(ApplicationScope::unitName(entry->id, pid));
    return true;
}

bool AppRuntime::terminate(uint32_t pid)
{
    if (!isSafeTarget(pid) || !isAlive(pid) || !isOwnedByUser(pid))
        return false;
    const uint64_t started = startTime(pid);
    ::kill(static_cast<pid_t>(pid), SIGTERM);
    base::EventLoop::current()->startTimer(kTerminateTimeout, false, [pid, started] {
        if (isAlive(pid) && isOwnedByUser(pid) && (started == 0 || startTime(pid) == started))
            ::kill(static_cast<pid_t>(pid), SIGKILL);
    });
    if (!m_reaper.isActive())
        m_reaper.start();
    return true;
}

bool AppRuntime::isRunning(const QString &appId) const
{
    return !pidsForApplication(appId).isEmpty();
}

QStringList AppRuntime::runningApplications() const
{
    QStringList result;
    for (const auto &[pid, instance] : m_instances) {
        if (!result.contains(instance.appId))
            result << instance.appId;
    }
    return result;
}

QList<uint32_t> AppRuntime::pidsForApplication(const QString &appId) const
{
    QList<uint32_t> result;
    for (const auto &[pid, instance] : m_instances) {
        if (instance.appId == appId)
            result << pid;
    }
    return result;
}

void AppRuntime::reap()
{
    QList<uint32_t> pids;
    for (const auto &[pid, instance] : m_instances)
        pids << pid;

    for (const uint32_t pid : pids) {
        const Instance instance = m_instances[pid];
        if (isAlive(pid) && (instance.startTime == 0 || startTime(pid) == instance.startTime))
            continue;
        m_instances.erase(pid);
        applicationQuit.emit(instance.appId, pid);
    }
    if (m_instances.empty())
        m_reaper.stop();
}

bool AppRuntime::isAlive(uint32_t pid)
{
    if (pid == 0)
        return false;
    return ::kill(static_cast<pid_t>(pid), 0) == 0 || errno == EPERM;
}

bool AppRuntime::isSafeTarget(uint32_t pid)
{
    if (pid <= 1)
        return false;
    return pid != static_cast<uint32_t>(::getpid())
        && pid != static_cast<uint32_t>(::getpgrp())
        && pid != static_cast<uint32_t>(::getsid(0))
        && parentPid(pid) != kSessionPid;
}

static QList<QString> statFields(uint32_t pid)
{
    const auto stat = Lingmo::Base::file::read(
        "/proc/" + std::to_string(pid) + "/stat");
    if (!stat)
        return {};
    const std::string line = stat->substr(0, stat->find('\n'));
    const size_t commEnd = line.rfind(')');
    if (commEnd == std::string::npos || commEnd + 2 > line.size())
        return {};
    QList<QString> result;
    for (const auto &s : Lingmo::Base::strings::splitWhitespace(line.substr(commEnd + 2)))
        result << QString::fromStdString(s);
    return result;
}

uint32_t AppRuntime::parentPid(uint32_t pid)
{
    const QList<QString> fields = statFields(pid);
    return fields.size() < 2 ? 0
        : Lingmo::Base::strings::toNumber<uint32_t>(fields.at(1).toStdString()).value_or(0);
}

uint64_t AppRuntime::startTime(uint32_t pid)
{
    const QList<QString> fields = statFields(pid);
    if (fields.size() < 20)
        return 0;
    return Lingmo::Base::strings::toNumber<uint64_t>(fields.at(19).toStdString()).value_or(0);
}

bool AppRuntime::isOwnedByUser(uint32_t pid)
{
    struct stat info {};
    return ::stat(("/proc/" + std::to_string(pid)).c_str(), &info) == 0
        && info.st_uid == ::getuid();
}
