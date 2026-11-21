// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"

#include "base/command_line.h"
#include "base/i18n.h"
#include "base/locale.h"
#include "base/paths.h"
#include "base/process.h"
#include "base/session_language.h"
#include "base/strings.h"
#include "common/environment.h"
#include "common/language_environment.h"
#include "logs.h"
#include "power.h"

#include <filesystem>
#include <optional>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace base = Lingmo::Base;
namespace dbus = Lingmo::Base::DBus;
using namespace std::chrono_literals;

static std::optional<QStringList> getSystemdEnvironment()
{
    dbus::Proxy manager(dbus::Connection::session(),
                        "org.freedesktop.systemd1",
                        "/org/freedesktop/systemd1",
                        "org.freedesktop.systemd1.Manager");
    const auto environment = manager.property<std::vector<std::string>>("Environment");
    if (!environment)
        return std::nullopt;
    return toQStringList(*environment);
}

static bool isShellVariable(const QString &name)
{
    return name == QLatin1String("_") || name.startsWith(QLatin1String("SHLVL"));
}

static bool isSessionVariable(const QString &name)
{
    return name == QLatin1String("DISPLAY")
        || name == QLatin1String("XAUTHORITY")
        || name == QLatin1String("WAYLAND_DISPLAY")
        || name == QLatin1String("WAYLAND_SOCKET")
        || name.startsWith(QLatin1String("XDG_"));
}

static bool isInputMethodModuleVariable(const QString &name)
{
    return name == QLatin1String("GTK_IM_MODULE")
        || name == QLatin1String("QT_IM_MODULE");
}

static bool isEmptyVariable(const char *name)
{
    const char *value = std::getenv(name);
    return !value || !*value;
}

static void setEnvironmentVariable(const QString &name, const QString &value)
{
    const char *current = std::getenv(name.toLocal8Bit().constData());
    if (!current || value != QLatin1String(current))
        setenv(name.toLocal8Bit().constData(), value.toLocal8Bit().constData(), 1);
}

static int s_terminationSocket[2] = {-1, -1};

static void onTerminationSignal(int)
{
    const int savedErrno = errno;
    const char byte = 1;
    [[maybe_unused]] const ssize_t written = ::write(s_terminationSocket[1], &byte, 1);
    errno = savedErrno;
}

namespace Lingmo {

SessionApplication::SessionApplication(int argc, char **argv, base::EventLoop &loop)
    : m_loop(loop)
    , m_processManager(std::make_unique<SessionProcessManager>())
{
    endSessionOnTerminationSignals();
    exportSession();

    base::CommandLineParser parser;
    parser.setApplicationDescription("Lingmo Session");
    parser.addHelpOption();
    parser.process(argc, argv);

    createConfigDirectory();
    initLanguage();
    initEnvironments();

    if (!syncDBusEnvironment())
        LM_WARNING(sessionLog()) << "Could not sync environment to dbus.";

    // Import systemd's environment only after ours reached D-Bus; the other
    // order applies LANG and LC_* in the wrong order.
    unsetenv("LC_ALL");
    importSystemdEnvironment();

    // The language the session runs in is known only now.
    base::Locale::setProcessLocale();
    Lingmo::I18n::setDomain("lingmo-core");

    m_startTimer.setSingleShot(true);
    m_startTimer.onTimeout = [this] {
        m_processManager->start();
    };
    m_startTimer.start(100ms);
}

SessionApplication::~SessionApplication()
{
    // Deleted here, while the loop still exists, the process manager stops
    // what it started and removes its Xwayland readiness directory.
    m_processManager.reset();
    if (m_terminationWatch)
        m_loop.unwatch(m_terminationWatch);
}

void SessionApplication::reboot()
{
    m_processManager->endAfterApplications(SessionProcessManager::Ending::Restart, [this] {
        const bool rebooting = Power::reboot();
        if (rebooting)
            m_loop.quit(0);
        return rebooting;
    });
}

void SessionApplication::powerOff()
{
    m_processManager->endAfterApplications(SessionProcessManager::Ending::Shutdown, [this] {
        const bool poweringOff = Power::powerOff();
        if (poweringOff)
            m_loop.quit(0);
        return poweringOff;
    });
}

bool SessionApplication::suspend()
{
    return Power::suspend();
}

void SessionApplication::exportSession()
{
    dbus::Connection *bus = dbus::Connection::session(&m_loop);
    if (!bus)
        return;

    m_sessionObject = std::make_unique<dbus::Object>(
        bus, "/org/lingmo/Session", "org.lingmo.Session");
    m_sessionObject->addMethod("logout",             [this] { logout(); });
    m_sessionObject->addMethod("reboot",             [this] { reboot(); });
    m_sessionObject->addMethod("powerOff",           [this] { powerOff(); });
    m_sessionObject->addMethod("suspend",            [this] { return suspend(); }, {{}, {"result"}});
    m_sessionObject->addMethod("startDesktopProcess",[this] { startDesktopProcess(); });
    m_sessionObject->publish();
    bus->requestName("org.lingmo.Session");
    m_language = std::make_unique<base::SessionLanguage>(bus);
}

void SessionApplication::endSessionOnTerminationSignals()
{
    // lingmo-dm and systemd end a session with SIGTERM; dying of it would leave
    // the compositor and services holding the Wayland socket against the next login.
    // A handler, unlike signalfd's blocked mask, is not inherited by child processes.
    if (::socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0,
                     s_terminationSocket) != 0) {
        LM_WARNING(sessionLog()) << "Cannot handle termination signals";
        return;
    }
    m_terminationWatch = m_loop.watch(s_terminationSocket[0], EPOLLIN, [this](uint32_t) {
        m_loop.unwatch(m_terminationWatch);
        m_terminationWatch = 0;
        m_processManager->endSession();
    });
    struct sigaction action = {};
    action.sa_handler = onTerminationSignal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    for (const int number : {SIGTERM, SIGINT, SIGHUP})
        ::sigaction(number, &action, nullptr);
}

void SessionApplication::initEnvironments()
{
    const QString home = QString::fromStdString(base::paths::home());

    if (isEmptyVariable("XDG_DATA_HOME"))
        setenv("XDG_DATA_HOME", (home + "/.local/share").toLocal8Bit().constData(), 1);
    if (isEmptyVariable("XDG_DESKTOP_DIR"))
        setenv("XDG_DESKTOP_DIR", (home + "/Desktop").toLocal8Bit().constData(), 1);
    if (isEmptyVariable("XDG_CONFIG_HOME"))
        setenv("XDG_CONFIG_HOME", (home + "/.config").toLocal8Bit().constData(), 1);
    if (isEmptyVariable("XDG_CACHE_HOME"))
        setenv("XDG_CACHE_HOME", (home + "/.cache").toLocal8Bit().constData(), 1);
    if (isEmptyVariable("XDG_DATA_DIRS"))
        setenv("XDG_DATA_DIRS", "/usr/local/share/:/usr/share/", 1);
    if (isEmptyVariable("XDG_CONFIG_DIRS"))
        setenv("XDG_CONFIG_DIRS", "/etc/xdg", 1);

    setenv("DESKTOP_SESSION",    "Lingmo", 1);
    setenv("XDG_CURRENT_DESKTOP","Lingmo", 1);
    setenv("XDG_SESSION_DESKTOP","Lingmo", 1);

    // GLib plain-text backend: lingmo-services writes GTK appearance keys into
    // ~/.config/glib-2.0/settings/keyfile, so no dconf daemon is needed.
    setenv("GSETTINGS_BACKEND", "keyfile", 1);

    // A list, not one name: Qt tries Wayland first, and an application whose
    // Qt has no Wayland plugin, or that cannot reach the compositor (a root
    // program under pkexec), starts on XWayland instead of failing to find a
    // platform. Qt reads the names in order.
    setenv("QT_QPA_PLATFORM",    "wayland;xcb", 1);
    // GTK picks Wayland by itself in a Wayland session, and X11 elsewhere; a
    // forced backend leaves GTK applications without a choice.
    if (isEmptyVariable("WAYLAND_DISPLAY"))
        setenv("WAYLAND_DISPLAY", "lingmo-0", 1);
    setenv("QT_QPA_PLATFORMTHEME", "lingmo", 1);

    // Reduce QML repaint latency on Wayland.
    setenv("QT_QPA_UPDATE_IDLE_TIME", "10", 1);

    // im-config's /etc/profile.d script exports these for Wayland sessions;
    // they would bypass the compositor's text-input protocols.
    // XMODIFIERS stays for Xwayland clients.
    unsetenv("GTK_IM_MODULE");
    unsetenv("QT_IM_MODULE");
}

void SessionApplication::initLanguage()
{
    Lingmo::Core::Environment environment = Lingmo::Core::processEnvironment();
    Lingmo::Core::language::apply(environment);
    for (const auto &[name, value] : environment) {
        if (name == "LANG" || name == "LANGUAGE" || name.starts_with("LC_"))
            setenv(name.c_str(), value.c_str(), 1);
    }
    setenv("LC_ALL", "", 1);
}

bool SessionApplication::syncDBusEnvironment()
{
    int exitCode = 0;

    if (!base::paths::findExecutable("dbus-update-activation-environment").empty())
        exitCode = runSync("dbus-update-activation-environment",
                           {"--systemd", "--all"});

    const QString systemctl = QString::fromStdString(
        base::paths::findExecutable("systemctl"));
    if (!systemctl.isEmpty())
        runSync(systemctl, {"--user", "unset-environment",
                            "GTK_IM_MODULE", "QT_IM_MODULE", "LC_ALL"});

    return exitCode == 0;
}

// Import systemd user environment (~/.config/environment.d).
// Has no effect when Lingmo is not started by systemd.
void SessionApplication::importSystemdEnvironment()
{
    const auto environment = getSystemdEnvironment();
    if (!environment)
        return;

    for (const QString &env : *environment) {
        const int idx = env.indexOf(QLatin1Char('='));
        if (idx <= 0)
            continue;
        const QString name  = env.left(idx);
        const QString value = env.mid(idx + 1);
        if (isShellVariable(name) || isSessionVariable(name)
            || isInputMethodModuleVariable(name) || name == QLatin1String("LC_ALL"))
            continue;
        setEnvironmentVariable(name, value);
    }
}

void SessionApplication::createConfigDirectory()
{
    const QString configDir = QString::fromStdString(base::paths::configHome());
    std::error_code error;
    std::filesystem::create_directories(configDir.toStdString(), error);
    if (!std::filesystem::is_directory(configDir.toStdString(), error))
        LM_WARNING(sessionLog()) << "Could not create XDG_CONFIG_HOME:" << configDir;
}

int SessionApplication::runSync(const QString &program, const QStringList &args)
{
    base::Process p(&m_loop);
    p.setOutputForwarded(true);
    int exitCode = 0;
    p.onFinished = [&exitCode](int code, bool) { exitCode = code; };

    if (!p.start(program.toStdString(), toStdStrings(args))) {
        LM_WARNING(sessionLog()) << program << "could not be started";
        return 255;
    }
    p.waitForFinished(std::chrono::hours(24));

    if (exitCode)
        LM_WARNING(sessionLog()) << program << args.join(QLatin1Char(' '))
                                 << "exited with code" << exitCode;
    return exitCode;
}

} // namespace Lingmo
