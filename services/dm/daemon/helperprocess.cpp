#include "helperprocess.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/logging.h"

using namespace Lingmo;
using Lingmo::i18n::tr;

namespace dm {

static std::string &helperPath()
{
    static std::string path = DM_HELPER_PATH;
    return path;
}

// The helper's own deadline for its session (10 s), up to 5 s for a killed
// session's group to go, then PAM's close.
static constexpr std::chrono::milliseconds kStopTimeout(20000);

HelperProcess::HelperProcess()
    : m_stopTimeout(kStopTimeout)
{
    m_stopDeadline.setSingleShot(true);
    m_stopDeadline.onTimeout = [this] {
        LM_WARNING(base::LogCategory::defaultCategory()) << "The login helper did not stop; killing it";
        m_process.kill();
    };

    // The session's output shares the helper's stderr: the daemon's journal.
    m_process.setStandardErrorForwarded(true);
    m_process.setStandardInputOpen(true);

    m_process.onStandardOutput = [this](std::string_view bytes) { readMessages(bytes); };
    m_process.onFinished = [this](int, bool) {
        m_stopDeadline.stop();
        if (!m_authenticated && !m_failed)
            failed(false, tr("The login helper exited"));
        finished();
    };
}

HelperProcess::~HelperProcess()
{
    disconnect();
    m_process.onStandardOutput = nullptr;
    m_process.onFinished = nullptr;
    if (m_process.isRunning()) {
        // EOF ends a helper waiting for a message even when SIGTERM lands just before its read().
        m_process.closeStandardInput();
        terminate();

        // A helper that may own a session gets its whole bounded stop, which ends by closing PAM.
        if (!m_process.waitForFinished(isAuthenticated() ? m_stopTimeout : std::chrono::milliseconds(3000)))
            m_process.kill();
        m_process.waitForFinished(std::chrono::milliseconds(1000));
    }
}

void HelperProcess::setProgram(std::string program)
{
    helperPath() = std::move(program);
}

void HelperProcess::disconnect()
{
    onQuestion = nullptr;
    onAuthenticated = nullptr;
    onStarted = nullptr;
    onFailed = nullptr;
    onFinished = nullptr;
}

void HelperProcess::login(const std::string &service, const std::string &user, bool authenticate, int vt)
{
    m_user = user;
    if (!m_process.start(helperPath())) {
        // Reported from the loop, as the callers expect it after login() returns.
        base::EventLoop::current()->post([alive = std::weak_ptr<bool>(m_alive), this] {
            if (alive.expired())
                return;
            m_failed = true;
            failed(false, tr("The login helper did not start"));
            finished();
        });
        return;
    }
    send(Json::object({{"type", "login"},
                       {"service", service},
                       {"user", user},
                       {"authenticate", authenticate},
                       {"vt", vt}}));
}

void HelperProcess::answer(const Json &response)
{
    send(Json::object({{"type", "answer"}, {"response", response}}));
}

void HelperProcess::start(const std::vector<std::string> &command, const std::vector<std::string> &environment)
{
    Json::Array cmd;
    for (const std::string &argument : command)
        cmd.push_back(argument);
    Json::Array env;
    for (const std::string &variable : environment)
        env.push_back(variable);
    send(Json::object({{"type", "start"}, {"cmd", std::move(cmd)}, {"env", std::move(env)}}));
}

void HelperProcess::terminate()
{
    // Never the session's pid itself: once reaped, another process may have it.
    if (!m_process.isRunning())
        return;

    m_process.terminate();
    if (!m_stopDeadline.isActive())
        m_stopDeadline.start(m_stopTimeout);
}

base::Timer *HelperProcess::addTimer()
{
    m_timers.push_back(std::make_unique<base::Timer>());
    return m_timers.back().get();
}

void HelperProcess::send(const Json &message)
{
    std::string line = encodeLine(message);
    m_process.writeStandardInput(line);
    wipe(line);
}

void HelperProcess::failed(bool authentication, const std::string &message)
{
    if (auto callback = onFailed)
        callback(authentication, message);
}

void HelperProcess::finished()
{
    if (auto callback = onFinished)
        callback();
}

void HelperProcess::readMessages(std::string_view bytes)
{
    m_buffer.append(bytes);

    size_t end = 0;
    while ((end = m_buffer.find('\n')) != std::string::npos) {
        const auto message = decodeLine(std::string_view(m_buffer).substr(0, end));
        m_buffer.erase(0, end + 1);
        if (!message)
            continue;

        const std::string type = (*message)["type"].toString();
        if (type == "question") {
            if (auto callback = onQuestion)
                callback((*message)["style"].toString(), (*message)["text"].toString());
        } else if (type == "authenticated") {
            m_authenticated = true;
            if (auto callback = onAuthenticated)
                callback();
        } else if (type == "started") {
            m_sessionPid = (*message)["pid"].toInt();
            m_sessionId = (*message)["session"].toString();
            if (auto callback = onStarted)
                callback(m_sessionId);
        } else if (type == "error") {
            m_failed = true;
            failed((*message)["auth"].toBool(), (*message)["message"].toString());
        } else if (type == "finished") {
            m_sessionPid = 0;
        }
    }
}

} // namespace dm
