// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processmanager.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <filesystem>
#include <unistd.h>

#include <systemd/sd-bus.h>

#include <QString>
#include <QStringList>

#include "base/application_registry.h"
#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "base/strings.h"
#include "common/runtime_client.h"

#include "autostartentries.h"
#include "compositorprocess.h"
#include "journalstream.h"
#include "logs.h"
#include "restartpolicy.h"
#include "shellconnection.h"
#include "shellprocess.h"
#include "userdirectories.h"
#include "xwaylandenvironment.h"

namespace base    = Lingmo::Base;
namespace strings = Lingmo::Base::strings;
namespace dbus    = Lingmo::Base::DBus;
namespace runtime = Lingmo::Core::runtime;
using namespace std::chrono_literals;
using Lingmo::I18n::format;
using Lingmo::I18n::tr;

// lingmo-aurora installs to CMAKE_INSTALL_BINDIR, and lingmo-dm starts its
// greeter from the same path. Anything else here makes the session exit before
// the compositor is up, which reads as a login that bounces back to the DM.
static constexpr char kAurora[] = "/usr/bin/lingmo-aurora";

namespace Lingmo {

struct Publisher
{
    base::Process process;
    base::Timer deadline;
};

static QString environmentValue(const char *name, const char *fallback = "")
{
    const char *value = std::getenv(name);
    return value && *value ? QString::fromLocal8Bit(value)
                           : QString::fromLocal8Bit(fallback);
}

// Split a desktop Exec= line at whitespace, with double-quote grouping.
// Three consecutive quotes stand for one literal quote character.
static QStringList splitCommand(const QString &command)
{
    QStringList arguments;
    QString current;
    int quoteCount = 0;
    bool inQuote = false;

    for (const QChar c : command) {
        if (c == QLatin1Char('"')) {
            ++quoteCount;
            if (quoteCount == 3) {
                quoteCount = 0;
                current += c;
            }
            continue;
        }
        if (quoteCount) {
            if (quoteCount == 1)
                inQuote = !inQuote;
            quoteCount = 0;
        }
        if (!inQuote && c.isSpace()) {
            if (!current.isEmpty()) {
                arguments.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.isEmpty())
        arguments.push_back(current);

    return arguments;
}

SessionProcessManager::SessionProcessManager()
    : m_compositorProcess(std::make_unique<CompositorProcess>())
    , m_shellConnection(std::make_unique<ShellConnection>())
    , m_shellProcess(std::make_unique<ShellProcess>())
    , m_shellRestart(std::make_unique<RestartPolicy>(
          QStringLiteral("lingmo-filemanager"), [this] { startShell(); }))
{
    m_shellProcess->onFinished = [this](int code, bool crashed) {
        onShellFinished(code, crashed);
    };
    m_autostartTimer.setSingleShot(true);
    m_autostartTimer.onTimeout = [this] {
        loadAutoStartProcess();
    };
    m_exitTimer.setSingleShot(true);
}

SessionProcessManager::~SessionProcessManager()
{
    stopShellAndDaemons();
    m_compositorProcess->stop();
}

void SessionProcessManager::start()
{
    if (m_started)
        return;
    m_started = true;

    m_waylandSocket = environmentValue("WAYLAND_DISPLAY", "lingmo-0");
    const QString runtimeDir = environmentValue("XDG_RUNTIME_DIR", "/tmp");
    m_controlSocket = runtimeDir + QLatin1Char('/') + m_waylandSocket
                    + QStringLiteral("-shell.control");

    unsetenv("DISPLAY");
    unsetenv("XAUTHORITY");
    m_xwaylandEnvironment = std::make_unique<XwaylandEnvironment>();
    m_xwaylandEnvironment->onResolved = [this](bool) {
        publishXwaylandEnvironment(true);
    };
    m_xwaylandEnvironment->onLost = [this] {
        publishXwaylandEnvironment(false);
    };
    m_compositorProcess->onReady = [this] {
        m_xwaylandEnvironment->waitForReady();
    };
    m_compositorProcess->onFailed = [this](const QString &reason) {
        LM_CRITICAL(sessionLog()) << reason;
        stopShellAndDaemons();
        base::EventLoop::current()->quit(1);
    };
    // A compositor that dies takes the graphical session with it.
    m_compositorProcess->onExited = [this](int code) {
        stopShellAndDaemons();
        base::EventLoop::current()->quit(code);
    };
    m_compositorProcess->start(kAurora, m_waylandSocket,
                               m_controlSocket, 15000,
                               m_xwaylandEnvironment->readinessFile());
}

void SessionProcessManager::publishXwaylandEnvironment(bool startDesktop)
{
    if (m_loggingOut)
        return;

    auto publisher = std::make_unique<Publisher>();
    Publisher *raw = publisher.get();
    const auto finish = [this, raw, startDesktop] {
        raw->deadline.stop();
        const auto found = std::find_if(
            m_publishers.begin(), m_publishers.end(),
            [raw](const auto &item) { return item.get() == raw; });
        if (found != m_publishers.end()) {
            std::shared_ptr<Publisher> done(std::move(*found));
            m_publishers.erase(found);
            base::EventLoop::current()->post([done] {});
        }
        if (startDesktop && !m_loggingOut)
            startAfterCompositorReady();
    };

    raw->process.setOutputForwarded(true);
    raw->process.onFinished = [finish](int code, bool) {
        if (code != 0)
            LM_WARNING(sessionLog())
                << "Could not publish Xwayland environment to activation services";
        finish();
    };
    raw->deadline.setSingleShot(true);
    raw->deadline.onTimeout = [raw] { raw->process.kill(); };

    m_publishers.push_back(std::move(publisher));
    if (!raw->process.start(
            "dbus-update-activation-environment",
            {"--systemd",
             "DISPLAY="    + environmentValue("DISPLAY").toStdString(),
             "XAUTHORITY=" + environmentValue("XAUTHORITY").toStdString()})) {
        LM_WARNING(sessionLog()) << "dbus-update-activation-environment is unavailable";
        finish();
        return;
    }
    raw->deadline.start(3000ms);
}

void SessionProcessManager::startAfterCompositorReady()
{
    if (m_compositorReady)
        return;
    m_compositorReady = true;

    // The persistent control channel makes shell restart possible without
    // restarting the compositor.
    if (!m_shellConnection->connectToCompositor(m_controlSocket)) {
        LM_WARNING(sessionLog())
            << "No compositor control channel; the shell will run without "
               "the private Lingmo protocols";
    }

    // Application autostart waits for the external services daemon.
    if (dbus::Connection *bus = dbus::Connection::session()) {
        m_servicesWatch = bus->watchName(
            "org.lingmo.Services", [this](const std::string &owner) {
                if (owner.empty())
                    return;
                m_servicesWatch = {};
                startDesktopProcess();
            });
    }

    startDaemonProcess();
}

void SessionProcessManager::startShell()
{
    if (m_loggingOut || m_shellProcess->isRunning())
        return;

    m_shellRestart->started();
    m_shellProcess->startForSession(
        QStringLiteral("lingmo-filemanager"),
        m_waylandSocket,
        m_shellConnection->isConnected() ? m_shellConnection.get() : nullptr);
}

void SessionProcessManager::onShellFinished(int code, bool crashed)
{
    if (m_loggingOut || !m_desktopStarted)
        return;

    // The compositor and every visible application carry on; only the
    // desktop's icons disappear until the shell is restarted.
    m_shellRestart->exited(code, crashed);
}

void SessionProcessManager::logout()
{
    endAfterApplications(Ending::Logout, [this] {
        leave();
        return true;
    });
}

void SessionProcessManager::endAfterApplications(Ending ending,
                                                   std::function<bool()> then)
{
    if (m_ending)
        return;
    m_ending = true;

    dbus::Connection *bus = dbus::Connection::session();
    if (!bus) {
        m_ending = then();
        return;
    }

    const dbus::Message quitAll = bus->newMethodCall(
        "org.lingmo.Applications1", "/org/lingmo/Applications1",
        "org.lingmo.Applications1", "QuitAll");

    // No timeout: the user may take as long as they like over an unsaved document.
    m_quitAllCall = bus->callAsync(
        quitAll,
        [this, ending, then](const dbus::Reply &reply) {
            std::string outcome;
            std::string applicationId;
            const bool answered = reply.read(outcome, applicationId);
            if (!answered || outcome == "quit") {
                m_ending = then();
                return;
            }
            m_ending = false;
            notifyEndingStopped(ending, QString::fromStdString(outcome),
                                QString::fromStdString(applicationId));
        },
        std::chrono::milliseconds(INT_MAX));
}

void SessionProcessManager::notifyEndingStopped(Ending ending,
                                                  const QString &outcome,
                                                  const QString &applicationId)
{
    base::ApplicationRegistry &registry = base::ApplicationRegistry::instance();
    if (!applicationId.isEmpty() && registry.isLoading() && registry.entries().empty()) {
        m_registryChanged = registry.changed.connect(
            [this, ending, outcome, applicationId] {
                m_registryChanged.reset();
                notifyEndingStopped(ending, outcome, applicationId);
            });
        return;
    }

    const auto entry = registry.byId(applicationId.toStdString());
    QString name = entry ? QString::fromStdString(entry->name) : QString();
    if (name.isEmpty())
        name = applicationId;

    QString summary;
    switch (ending) {
    case Ending::Logout:   summary = tr("Log out cancelled");  break;
    case Ending::Restart:  summary = tr("Restart cancelled");  break;
    case Ending::Shutdown: summary = tr("Shutdown cancelled"); break;
    }

    const bool cancelled = outcome == QLatin1String("cancelled");
    QString body;
    if (name.isEmpty())
        body = cancelled ? tr("An application cancelled it.")
                         : tr("An application did not quit.");
    else
        body = QString::fromStdString(
            format(cancelled ? tr("{name} cancelled it.")
                             : tr("{name} did not quit."),
                   {{"name", name.toStdString()}}));

    dbus::Connection *bus = dbus::Connection::session();
    if (!bus)
        return;

    dbus::Message notify = bus->newMethodCall(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications", "Notify");
    sd_bus_message_set_auto_start(notify.get(), 0);

    // TRANSLATORS: the sender of a notification from the session itself.
    notify.append(
        std::string(tr("System")), uint32_t(0),
        entry && !entry->icon.empty() ? entry->icon : std::string("system-log-out"),
        summary.toStdString(), body.toStdString(),
        std::vector<std::string>(),
        dbus::VariantMap{{"urgency", dbus::Variant(uint8_t(1))}},
        int32_t(-1));
    bus->send(notify);
}

void SessionProcessManager::leave()
{
    m_loggingOut = true;
    stopRestarts();

    // Under lingmo-dm the login screen comes up on its own VT first.
    bool switched = false;
    if (dbus::Connection *system = dbus::Connection::system()) {
        const dbus::Message switchToGreeter = system->newMethodCall(
            "org.lingmo.DisplayManager", "/org/lingmo/DisplayManager",
            "org.lingmo.DisplayManager", "SwitchToGreeter");
        const dbus::Reply reply = system->call(switchToGreeter, 15s);
        switched = !reply.isError();
        if (reply.isError()
            && reply.error().name != "org.freedesktop.DBus.Error.ServiceUnknown")
            LM_WARNING(sessionLog())
                << "The login screen did not come up before logout:"
                << reply.error().message;
    } else {
        LM_WARNING(sessionLog())
            << "The login screen did not come up before logout: no system bus";
    }

    endSession(switched);
}

void SessionProcessManager::endSession(bool behindLoginScreen)
{
    m_loggingOut = true;
    stopRestarts();

    if (!runtime::quitAll()) {
        for (const std::string &appId : runtime::runningApplications())
            runtime::quitApplication(appId);
    }

    m_shellProcess->stop();
    stopProcesses(m_coreProcesses);

    m_exitTimer.onTimeout = [this] {
        m_compositorProcess->stop();
        base::EventLoop::current()->quit(0);
    };
    m_exitTimer.start(behindLoginScreen ? 1000ms : 0ms);
}

void SessionProcessManager::stopShellAndDaemons()
{
    m_loggingOut = true;
    stopRestarts();
    m_shellProcess->stop();
    stopProcesses(m_coreProcesses);
}

void SessionProcessManager::stopProcesses(
    std::map<QString, std::unique_ptr<base::Process>> &processes)
{
    for (auto &[program, process] : processes) {
        if (!process->isRunning())
            continue;
        process->terminate();
        if (!process->waitForFinished(2000ms))
            process->kill();
    }
}

void SessionProcessManager::startDesktopProcess()
{
    if (!m_compositorReady || m_desktopStarted)
        return;
    m_desktopStarted = true;

    dbus::Proxy(dbus::Connection::session(), "org.lingmo.Services", "/org/lingmo/Services/Sound",
                "org.lingmo.Services.Sound").send("playEvent", std::string("service-login"));

    UserDirectories::ensure(
        QString::fromStdString(base::paths::home()),
        QString::fromStdString(base::paths::configHome()));

    startShell();
    startCoreProcess(QStringLiteral("lingmo-polkit-agent"));
    m_autostartTimer.start(100ms);
}

void SessionProcessManager::startDaemonProcess()
{
    startCoreProcess(QStringLiteral("lingmo-appruntime"));
    startCoreProcess(QStringLiteral("lingmo-services"));
}

void SessionProcessManager::startCoreProcess(const QString &program,
                                              const QStringList &arguments)
{
    if (m_loggingOut)
        return;

    auto &process = m_coreProcesses[program];
    auto &restart = m_restartPolicies[program];
    if (!process) {
        process = std::make_unique<base::Process>();
        process->setOutputForwarded(true);
        restart = std::make_unique<RestartPolicy>(
            program, [this, program, arguments] {
                startCoreProcess(program, arguments);
            });
        RestartPolicy *policy = restart.get();
        process->onFinished = [policy](int code, bool crashed) {
            policy->exited(code, crashed);
        };
    }
    if (process->isRunning())
        return;

    restart->started();
    if (!startWithOwnJournalStream(*process, program, arguments)) {
        LM_WARNING(sessionLog()) << "Failed to start" << program;
        restart->exited(-1, false);
    }
}

void SessionProcessManager::stopRestarts()
{
    m_shellRestart->stop();
    for (auto &[program, restart] : m_restartPolicies)
        restart->stop();
}

void SessionProcessManager::loadAutoStartProcess()
{
    QStringList configDirectories;
    configDirectories << QString::fromStdString(base::paths::configHome());
    for (const auto &dir : base::paths::configDirectories())
        configDirectories << QString::fromStdString(dir);

    QStringList dirs;
    for (const QString &directory : configDirectories) {
        const QString path = directory + QStringLiteral("/autostart");
        std::error_code error;
        if (std::filesystem::is_directory(path.toStdString(), error)
            && !dirs.contains(path))
            dirs << path;
    }

    const QStringList desktops = environmentValue("XDG_CURRENT_DESKTOP", "Lingmo")
                                    .split(QLatin1Char(':'), Qt::SkipEmptyParts);
    const QStringList execList = autostartEntries(dirs, desktops);

    for (const QString &exec : execList) {
        QStringList command = splitCommand(exec);

        // Field codes (e.g. %U, %f) expand to nothing for autostart entries.
        command.erase(
            std::remove_if(command.begin(), command.end(),
                           [](const QString &arg) {
                               return arg.size() == 2
                                   && arg.startsWith(QLatin1Char('%'));
                           }),
            command.end());

        if (command.isEmpty())
            continue;

        const QString exe = QString::fromStdString(
            std::filesystem::path(command.front().toStdString()).filename().string());
        if (isStartedBySession(exe))
            continue;

        runtime::launchCommand(toStdStrings(command));
    }
}

} // namespace Lingmo
