#include "daemon.h"

#include <algorithm>
#include <fcntl.h>
#include <filesystem>
#include <pwd.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/logging.h"
#include "base/paths.h"
#include "base/process.h"
#include "base/settings.h"

#include "sessions.h"
#include "terminal.h"

using namespace Lingmo;
using Lingmo::i18n::tr;

namespace dm {

static bool oobeComplete(const std::string &user)
{
    const passwd *entry = getpwnam(user.c_str());
    if (!entry || !entry->pw_dir)
        return false;
    const std::string marker = std::string(entry->pw_dir) + "/.config/lingmo/oobe.conf";
    return access(marker.c_str(), F_OK) == 0;
}

static bool oobeAutologin()
{
    const base::Settings state("/etc/lingmo-initial-setup-account.conf");
    return state.boolValue("Account/Autologin", true);
}

static std::string oobeAccountUser()
{
    const base::Settings state("/etc/lingmo-initial-setup-account.conf");
    return state.stringValue("Account/User");
}

// How long the login screen or a session may take to open its display.
static constexpr int kControlTimeoutMs = 10000;
static constexpr int kControlPollMs = 20;

// How long the login screen outlives the switch to a session: the kernel takes
// its cursor down with it, and the session's replaces it once its displays settle.
static constexpr int kGreeterLingerMs = 3000;

static std::optional<uid_t> uidOf(const std::string &user)
{
    const passwd *entry = getpwnam(user.c_str());
    if (!entry)
        return std::nullopt;

    return entry->pw_uid;
}

// Runs a program to its end with this process's output; its exit code, or -2
// when it cannot be started.
static int execute(const std::string &program, const std::vector<std::string> &arguments)
{
    base::Process process;
    process.setOutputForwarded(true);
    int exitCode = -1;
    process.onFinished = [&exitCode](int code, bool crashed) { exitCode = crashed ? -1 : code; };
    if (!process.start(program, arguments))
        return -2;
    while (!process.waitForFinished(std::chrono::hours(1))) {
    }
    return exitCode;
}

// As GDM does: plymouth gives up the display with its splash left on screen.
static bool bootSplashRunning()
{
    const std::string plymouth = base::paths::findExecutable("plymouth");
    return !plymouth.empty() && execute(plymouth, {"--ping"}) == 0;
}

Daemon::Daemon(Config config)
    : m_config(std::move(config))
    , m_state(m_config.statePath)
    , m_backgrounds(m_config.backgroundDirectory)
{
    m_greeterRestart.setSingleShot(true);
    m_greeterRestart.onTimeout = [this] { startGreeter(); };

    m_server.onCreateSession = [this](const std::string &username) { createSession(username); };
    m_server.onPostAuthMessageResponse = [this](const Json &response) { postAuthMessageResponse(response); };
    m_server.onStartSession = [this](const std::vector<std::string> &command,
                                     const std::vector<std::string> &environment, const std::string &session) {
        startSession(command, environment, session);
    };
    m_server.onCancelSession = [this] { cancelSession(); };
    m_server.onDisconnected = [this] {
        release(m_login);
    };
}

Daemon::~Daemon()
{
    m_stopping = true;
    replySwitchRequests(false);

    m_login.reset();
    m_session.reset();
    m_greeter.reset();
    m_released.clear();

    restoreTextMode(m_config.sessionVt);
    restoreTextMode(m_config.greeterVt);
}

bool Daemon::start()
{
    const std::string setupUser = oobeAccountUser();
    if (!setupUser.empty())
        cleanupCompletedOobeSession(setupUser);

    const auto greeter = uidOf(m_config.greeterUser);
    if (!greeter) {
        LM_CRITICAL(base::LogCategory::defaultCategory()) << "No login screen user" << m_config.greeterUser;
        return false;
    }

    std::error_code error;
    std::filesystem::create_directories(m_config.runtimeDirectory, error);
    const std::string socket = m_config.runtimeDirectory + "/greeter.sock";
    if (!m_server.listen(socket, *greeter)) {
        LM_CRITICAL(base::LogCategory::defaultCategory()) << "Cannot listen on" << socket;
        return false;
    }

    m_bus = base::dbus::Connection::system();
    if (m_bus) {
        m_object = std::make_unique<base::dbus::Object>(m_bus, "/org/lingmo/DisplayManager",
                                                        "org.lingmo.DisplayManager");
        m_object->addMethod("SwitchToGreeter", "", "",
                            [this](base::dbus::MethodCall &call) { switchToGreeter(call); });
        m_object->addMethod("SetBackgroundPicture", "h", "",
                            [this](base::dbus::MethodCall &call) { setBackgroundPicture(call); });
        m_object->addMethod("SetBackgroundColor", "s", "",
                            [this](base::dbus::MethodCall &call) { setBackgroundColor(call); });
    }
    if (!m_bus || !m_object->publish() || !m_bus->requestName("org.lingmo.DisplayManager"))
        LM_WARNING(base::LogCategory::defaultCategory())
                << "Cannot register org.lingmo.DisplayManager; sessions will end on screen";

    if (bootSplashRunning()) {
        m_bootSplash = true;
        execute("plymouth", {"deactivate"});
    }

    m_backgrounds.removeStale();

    // A run that ended mid-login may have left it in KD_GRAPHICS with nobody on it.
    restoreTextMode(m_config.sessionVt);
    if (!autologin())
        startGreeter();
    return true;
}

bool Daemon::autologin()
{
    if (m_config.autologinUser.empty())
        return false;

    // The runtime directory is emptied at boot, so a restarted daemon shows the
    // login screen instead of logging in again.
    const std::string marker = m_config.runtimeDirectory + "/autologin";
    const int fd = open(marker.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0)
        return false;
    close(fd);

    std::string id = m_config.autologinSession.empty() ? m_config.defaultSession : m_config.autologinSession;
    if (id == "lingmo-initial-setup") {
        if (oobeComplete(m_config.autologinUser)) {
            if (!oobeAutologin())
                return false;
            id = m_config.defaultSession;
        }
    }
    const auto entry = findSession(m_config.sessionDirectories, id);
    if (!entry) {
        LM_WARNING(base::LogCategory::defaultCategory()) << "No session" << id << "to log in to automatically";
        return false;
    }

    PendingStart start;
    start.command = {DM_SESSION_WRAPPER};
    start.command.insert(start.command.end(), entry->exec.begin(), entry->exec.end());
    start.environment = sessionEnvironment(*entry, m_config.sessionVt);
    start.session = id;

    m_login = std::make_unique<HelperProcess>();
    auto *login = m_login.get();
    login->onAuthenticated = [this, id, start = std::move(start)] {
        m_state.remember(m_login->user(), id);
        launchSession(start);
    };
    login->onFailed = [this, login](bool, const std::string &message) {
        LM_WARNING(base::LogCategory::defaultCategory()) << "Automatic login:" << message;
        if (m_login.get() == login) {
            release(m_login);
            startGreeter();
        }
    };
    login->onFinished = [this, login] {
        if (m_login.get() == login) {
            release(m_login);
            startGreeter();
        }
    };

    login->login(m_config.autologinService, m_config.autologinUser, false, m_config.sessionVt);
    return true;
}

void Daemon::startGreeter()
{
    if (m_greeter || m_stopping)
        return;

    // Until the compositor draws, VT1 keeps the previous frame, not the console.
    setGraphicsMode(m_config.greeterVt);
    m_greeterExpected = false;
    m_greeterSpent = false;

    m_greeter = std::make_unique<HelperProcess>();
    m_greeter->onAuthenticated = [this] {
        m_greeter->start(m_config.greeterCommand, greeterEnvironment(m_config.greeterVt, m_server.path()));
    };
    m_greeter->onStarted = [this](const std::string &) {
        if (std::exchange(m_bootSplash, false))
            base::Process::startDetached("plymouth", {"quit", "--retain-splash"});

        showGreeter();
    };
    m_greeter->onFailed = [](bool, const std::string &message) {
        LM_WARNING(base::LogCategory::defaultCategory()) << "Login screen:" << message;
    };
    m_greeter->onFinished = [this] { onGreeterFinished(); };

    m_greeter->login(m_config.greeterService, m_config.greeterUser, false, m_config.greeterVt);
}

void Daemon::showGreeter()
{
    if (!m_greeter || !m_greeter->hasStarted())
        return;

    // Created on the VT in front, as at boot, the login screen is active already.
    if (activeVt() == m_config.greeterVt) {
        replySwitchRequests(true);
        return;
    }

    HelperProcess *greeter = m_greeter.get();
    whenControlled(greeter, m_config.greeterVt, [this, greeter](bool controlled) {
        if (m_greeter.get() != greeter)
            return;

        if (controlled && activateSession(greeter->sessionId())) {
            replySwitchRequests(true);
            return;
        }

        // Never switch to a VT nobody took; restart the login screen instead.
        LM_WARNING(base::LogCategory::defaultCategory())
                << "Cannot switch to the login screen on VT" << m_config.greeterVt;
        greeter->terminate();
    });
}

void Daemon::stopGreeter()
{
    m_greeterExpected = true;
    if (m_greeter)
        m_greeter->terminate();
}

void Daemon::returnToGreeter()
{
    // A login screen that already handed a login over is replaced, not shown.
    if (m_greeter && m_greeterSpent) {
        m_restartGreeter = true;
        stopGreeter();
    } else if (m_greeter) {
        showGreeter();
    } else {
        startGreeter();
    }
}

void Daemon::onGreeterFinished()
{
    release(m_greeter);
    restoreTextMode(m_config.greeterVt);
    if (m_stopping)
        return;

    if (std::exchange(m_restartGreeter, false)) {
        startGreeter();
        return;
    }

    if (m_greeterExpected) {
        m_greeterFailures = 0;
        return;
    }

    // Restart a login screen that died, with backoff, unless a session is up.
    replySwitchRequests(false);
    if (m_session)
        return;

    ++m_greeterFailures;
    m_greeterRestart.start(std::chrono::milliseconds(std::min(m_greeterFailures, 5) * 1000));
}

void Daemon::replySwitchRequests(bool switched)
{
    for (base::dbus::MethodCall &request : std::exchange(m_switchRequests, {})) {
        if (switched)
            request.reply();
        else
            request.replyError("org.freedesktop.DBus.Error.Failed", "The login screen is not up");
    }
}

void Daemon::switchToGreeter(base::dbus::MethodCall &call)
{
    // A caller that is gone before it is looked up is refused, not taken for root.
    const base::dbus::Proxy driver(m_bus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
    const std::optional<uint32_t> caller =
            driver.call("GetConnectionUnixUser", call.message().sender()).value<uint32_t>();
    const auto owner = m_session ? uidOf(m_session->user()) : std::nullopt;
    if (!caller || (*caller != 0 && (!owner || *caller != *owner))) {
        call.replyError("org.freedesktop.DBus.Error.AccessDenied", "Only the session's own user can end it");
        return;
    }

    m_switchRequests.push_back(call);
    returnToGreeter();
}

std::string Daemon::callerName(base::dbus::MethodCall &call)
{
    const base::dbus::Proxy driver(m_bus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
    const std::optional<uint32_t> caller =
            driver.call("GetConnectionUnixUser", call.message().sender()).value<uint32_t>();
    const passwd *entry = caller ? getpwuid(*caller) : nullptr;
    if (!entry || !entry->pw_name || m_config.greeterUser == entry->pw_name) {
        call.replyError("org.freedesktop.DBus.Error.AccessDenied", "The caller is not a user who logs in");
        return {};
    }

    return entry->pw_name;
}

void Daemon::setBackgroundPicture(base::dbus::MethodCall &call)
{
    base::dbus::UnixFd file;
    if (!call.read(file) || !file.isValid()) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Expected the picture's file descriptor");
        return;
    }

    const std::string user = callerName(call);
    if (user.empty())
        return;

    m_backgrounds.setPicture(user, file, [call](bool copied) mutable {
        if (copied)
            call.reply();
        else
            call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "The picture cannot be copied");
    });
}

void Daemon::setBackgroundColor(base::dbus::MethodCall &call)
{
    std::string color;
    if (!call.read(color)) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Expected a colour");
        return;
    }

    const std::string user = callerName(call);
    if (user.empty())
        return;

    if (m_backgrounds.setColor(user, color))
        call.reply();
    else
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Not a colour that can be stored");
}

void Daemon::createSession(const std::string &username)
{
    release(m_login);
    // A start still waiting for the previous session went with the login it was for.
    m_pendingStart.reset();

    m_login = std::make_unique<HelperProcess>();
    auto *login = m_login.get();
    login->onQuestion = [this](const std::string &style, const std::string &text) {
        m_server.respond(GreeterServer::authMessage(style, text));
    };
    login->onAuthenticated = [this] {
        m_server.respond(GreeterServer::success());
    };
    login->onFailed = [this, login](bool authentication, const std::string &message) {
        m_server.respond(GreeterServer::error(authentication, message));
        if (m_login.get() == login)
            release(m_login);
    };
    login->onFinished = [this, login] {
        if (m_login.get() == login)
            release(m_login);
    };

    login->login(m_config.sessionService, username, true, m_config.sessionVt);
}

void Daemon::postAuthMessageResponse(const Json &response)
{
    if (!m_login)
        m_server.respond(GreeterServer::error(false, tr("No session is being created")));
    else if (m_login->isAuthenticated())
        m_server.respond(GreeterServer::error(false, tr("No authentication message is pending")));
    else
        m_login->answer(response);
}

void Daemon::startSession(const std::vector<std::string> &command, const std::vector<std::string> &environment,
                          const std::string &session)
{
    if (!m_login || !m_login->isAuthenticated()) {
        m_server.respond(GreeterServer::error(false, tr("Not authenticated")));
        return;
    }

    PendingStart start;
    // The client's variables first: the session's own, set after, win.
    start.environment = environment;

    if (!session.empty()) {
        const auto entry = findSession(m_config.sessionDirectories, session);
        if (!entry) {
            m_server.respond(GreeterServer::error(false, tr("No such session")));
            return;
        }

        start.command = {DM_SESSION_WRAPPER};
        start.command.insert(start.command.end(), entry->exec.begin(), entry->exec.end());
        const auto variables = sessionEnvironment(*entry, m_config.sessionVt);
        start.environment.insert(start.environment.end(), variables.begin(), variables.end());
        start.session = session;
        m_state.remember(m_login->user(), session);
    } else if (!command.empty()) {
        start.command = {DM_SESSION_WRAPPER};
        start.command.insert(start.command.end(), command.begin(), command.end());
        const auto variables = sessionEnvironment(SessionEntry {}, m_config.sessionVt);
        start.environment.insert(start.environment.end(), variables.begin(), variables.end());
    } else {
        m_server.respond(GreeterServer::error(false, tr("Nothing to start")));
        return;
    }

    m_server.respond(GreeterServer::success());

    // The previous session is still ending; the next one follows on its VT.
    if (m_session) {
        m_pendingStart = std::move(start);
        m_session->terminate();
        return;
    }

    launchSession(std::move(start));
}

void Daemon::cancelSession()
{
    release(m_login);
    m_pendingStart.reset();
    m_server.respond(GreeterServer::success());
}

void Daemon::launchSession(PendingStart start)
{
    if (!m_login)
        return;

    m_activeSession = start.session;
    setGraphicsMode(m_config.sessionVt);
    m_greeterSpent = true;

    m_session = std::move(m_login);
    HelperProcess *session = m_session.get();
    // The greeter's start request is answered; what the session reports now answers none.
    session->onQuestion = nullptr;
    session->onAuthenticated = nullptr;
    session->onFailed = [](bool, const std::string &message) {
        LM_WARNING(base::LogCategory::defaultCategory()) << "Session:" << message;
    };
    session->onStarted = [this, session](const std::string &id) {
        whenControlled(session, m_config.sessionVt, [this, session, id](bool controlled) {
            if (m_session.get() != session)
                return;

            // Never switch to a VT nobody took; end the session instead.
            if (!controlled || !activateSession(id)) {
                LM_WARNING(base::LogCategory::defaultCategory())
                        << "Cannot switch to the session on VT" << m_config.sessionVt << "- ending it";
                session->terminate();
                return;
            }

            // An automatic login takes the display from the boot splash itself.
            if (std::exchange(m_bootSplash, false))
                base::Process::startDetached("plymouth", {"quit", "--retain-splash"});

            // The timer goes with the login screen it is for.
            if (HelperProcess *greeter = m_greeter.get()) {
                base::Timer *linger = greeter->addTimer();
                linger->setSingleShot(true);
                linger->onTimeout = [this, greeter] {
                    if (greeter == m_greeter.get())
                        stopGreeter();
                };
                linger->start(std::chrono::milliseconds(kGreeterLingerMs));
            }
        });
    };
    m_session->onFinished = [this] { onSessionFinished(); };

    m_session->start(start.command, start.environment);
}

void Daemon::onSessionFinished()
{
    const std::string user = m_session ? m_session->user() : std::string {};
    const std::string session = std::exchange(m_activeSession, std::string {});
    release(m_session);
    restoreTextMode(m_config.sessionVt);
    if (session == "lingmo-initial-setup")
        cleanupCompletedOobeSession(user);
    if (m_stopping)
        return;

    // A login screen that went away meanwhile took its login along.
    if (auto pending = std::exchange(m_pendingStart, std::nullopt); pending && m_login) {
        launchSession(std::move(*pending));
        return;
    }

    // A crash, another desktop's own logout, or a session that failed before
    // it was shown: the login screen comes back.
    returnToGreeter();
}

void Daemon::cleanupCompletedOobeSession(const std::string &user)
{
    if (user.empty() || !oobeComplete(user))
        return;
    if (!removeSessionDesktopFiles(m_config.sessionDirectories, "lingmo-initial-setup"))
        LM_WARNING(base::LogCategory::defaultCategory())
                << "Cannot remove the completed initial setup session entry";
}

void Daemon::whenControlled(HelperProcess *helper, int vt, std::function<void(bool)> then)
{
    // The helper's, so the wait ends with it.
    base::Timer *timer = helper->addTimer();
    auto waited = std::make_shared<int>(0);
    timer->onTimeout = [timer, waited, vt, then = std::move(then)] {
        *waited += kControlPollMs;
        const bool controlled = isControlled(vt);
        if (!controlled && *waited < kControlTimeoutMs)
            return;

        timer->stop();
        then(controlled);
    };
    timer->start(std::chrono::milliseconds(kControlPollMs));
}

void Daemon::release(std::unique_ptr<HelperProcess> &helper)
{
    if (!helper)
        return;

    helper->disconnect();
    HelperProcess *released = helper.get();
    m_released.push_back(std::move(helper));
    base::EventLoop::current()->post([alive = std::weak_ptr<bool>(m_alive), this, released] {
        if (alive.expired())
            return;
        std::erase_if(m_released, [released](const auto &entry) { return entry.get() == released; });
    });
}

} // namespace dm
