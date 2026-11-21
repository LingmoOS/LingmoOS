#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/timer.h"

#include "backgrounds.h"
#include "config.h"
#include "greeterserver.h"
#include "helperprocess.h"
#include "state.h"

namespace dm {

// Runs the login screen and the session on VTs of their own; see README.md.
class Daemon
{
public:
    explicit Daemon(Config config);
    ~Daemon();

    Daemon(const Daemon &) = delete;
    Daemon &operator=(const Daemon &) = delete;

    bool start();

    // org.lingmo.DisplayManager.SwitchToGreeter: called by the session
    // before it ends; replies once the login screen is up.
    void switchToGreeter(Lingmo::base::dbus::MethodCall &call);
    // org.lingmo.DisplayManager.SetBackgroundPicture and SetBackgroundColor:
    // the caller's own desktop background, for the login screen to show.
    void setBackgroundPicture(Lingmo::base::dbus::MethodCall &call);
    void setBackgroundColor(Lingmo::base::dbus::MethodCall &call);

private:
    struct PendingStart {
        std::vector<std::string> command;
        std::vector<std::string> environment;
    };

    // Starts the configured user's session without the login screen, the first
    // time this boot; false when there is none to start.
    bool autologin();
    void startGreeter();
    void showGreeter();
    void stopGreeter();
    void returnToGreeter();
    void onGreeterFinished();
    void replySwitchRequests(bool switched);
    // The calling user's name; empty, with the call refused, when it cannot be told.
    std::string callerName(Lingmo::base::dbus::MethodCall &call);

    void createSession(const std::string &username);
    void postAuthMessageResponse(const Json &response);
    void startSession(const std::vector<std::string> &command, const std::vector<std::string> &environment,
                      const std::string &session);
    void cancelSession();
    void launchSession(PendingStart start);
    void onSessionFinished();

    // then(true) once logind has taken `vt` for the helper's session, then(false)
    // after a timeout; nothing runs once the helper is released.
    void whenControlled(HelperProcess *helper, int vt, std::function<void(bool)> then);
    // HelperProcess objects end in their own callbacks; they are destroyed later.
    void release(std::unique_ptr<HelperProcess> &helper);

    Config m_config;
    State m_state;
    Backgrounds m_backgrounds;
    GreeterServer m_server;
    Lingmo::base::dbus::Connection *m_bus = nullptr;
    std::unique_ptr<Lingmo::base::dbus::Object> m_object;
    std::unique_ptr<HelperProcess> m_greeter;
    std::unique_ptr<HelperProcess> m_login;
    std::unique_ptr<HelperProcess> m_session;
    std::vector<std::unique_ptr<HelperProcess>> m_released;
    std::optional<PendingStart> m_pendingStart;
    std::vector<Lingmo::base::dbus::MethodCall> m_switchRequests;
    Lingmo::base::Timer m_greeterRestart;
    std::vector<std::unique_ptr<Lingmo::base::Timer>> m_lingers;
    int m_greeterFailures = 0;
    bool m_greeterExpected = false;
    // The login screen has handed a login over and cannot take another.
    bool m_greeterSpent = false;
    bool m_restartGreeter = false;
    bool m_bootSplash = false;
    bool m_stopping = false;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace dm
