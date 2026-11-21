#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/process.h"
#include "base/timer.h"

#include "../common/wire.h"

namespace dm {

// The daemon's side of one lingmo-dm-helper: a login, then its session.
class HelperProcess
{
public:
    HelperProcess();
    ~HelperProcess();

    HelperProcess(const HelperProcess &) = delete;
    HelperProcess &operator=(const HelperProcess &) = delete;

    // The helper executable, for tests; DM_HELPER_PATH otherwise.
    static void setProgram(std::string program);

    void login(const std::string &service, const std::string &user, bool authenticate, int vt);
    // A string for a prompt, null for an informational message.
    void answer(const Json &response);
    void start(const std::vector<std::string> &command, const std::vector<std::string> &environment);
    // SIGTERM to the helper, which passes it on to its session, or stops the
    // login while none runs yet. A helper still running after the stop timeout
    // is killed, so onFinished always follows.
    void terminate();
    void setStopTimeout(std::chrono::milliseconds timeout) { m_stopTimeout = timeout; }

    // A timer that ends with this helper, for waits on its session.
    Lingmo::base::Timer *addTimer();

    std::string user() const { return m_user; }
    bool isAuthenticated() const { return m_authenticated; }
    bool hasStarted() const { return m_sessionPid > 0; }
    std::string sessionId() const { return m_sessionId; }

    // Each may release the helper (Daemon::release), not destroy it.
    std::function<void(const std::string &style, const std::string &text)> onQuestion;
    std::function<void()> onAuthenticated;
    std::function<void(const std::string &sessionId)> onStarted;
    // `authentication`: the credentials were wrong, rather than something broke.
    std::function<void(bool authentication, const std::string &message)> onFailed;
    // The helper exited; after onStarted, the session ended and PAM closed it.
    std::function<void()> onFinished;

    // Drops every callback, as before the helper is released.
    void disconnect();

private:
    void send(const Json &message);
    void readMessages(std::string_view bytes);
    void failed(bool authentication, const std::string &message);
    void finished();

    Lingmo::base::Process m_process;
    Lingmo::base::Timer m_stopDeadline;
    std::chrono::milliseconds m_stopTimeout;
    std::vector<std::unique_ptr<Lingmo::base::Timer>> m_timers;
    std::string m_buffer;
    std::string m_user;
    std::string m_sessionId;
    int64_t m_sessionPid = 0;
    bool m_authenticated = false;
    bool m_failed = false;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace dm
