// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/input_method_launcher.h"

#include <algorithm>
#include <sys/socket.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/process.h"
#include "core/logging.h"
#include "core/main_loop.h"

namespace Lingmo::Wayland {
namespace {
constexpr int kMaxFailures = 5;
constexpr int64_t kStableUptimeMs = 60000;
constexpr std::chrono::milliseconds kTerminateTimeout(3000);
constexpr std::chrono::milliseconds kFinishWait(500);

// Replaces or adds NAME=value in NAME=value entries.
void setVariable(std::vector<std::string> &environment, const std::string &name,
                 const std::string &value)
{
    const std::string prefix = name + '=';
    std::erase_if(environment, [&](const std::string &entry) {
        return entry.starts_with(prefix);
    });
    environment.push_back(prefix + value);
}

std::string joined(const std::vector<std::string> &command)
{
    std::string text;
    for (const std::string &part : command)
        text += (text.empty() ? "" : " ") + part;
    return text;
}
} // namespace

struct InputMethodLauncher::Child {
    explicit Child(base::EventLoop &loop)
        : process(&loop)
        , kill(&loop)
    {
    }

    base::Process process;
    std::string program;
    // Started once the process was asked to end; gives up waiting.
    base::Timer kill;
};

InputMethodLauncher::InputMethodLauncher(wl_display *display, base::EventLoop *loop)
    : m_loop(loop ? *loop : *core::mainLoop())
    , m_display(display)
    , m_restartTimer(&m_loop)
{
    m_restartTimer.setSingleShot(true);
    m_restartTimer.onTimeout = [this] {
        start();
    };
}

InputMethodLauncher::~InputMethodLauncher()
{
    stop();
    for (const auto &child : m_children) {
        child->process.onFinished = nullptr;
        if (!child->process.waitForFinished(kFinishWait))
            child->process.kill();
        child->process.waitForFinished(kFinishWait);
    }
}

void InputMethodLauncher::setCommand(const std::vector<std::string> &command)
{
    if (command == m_command && (m_client || m_restartTimer.isActive()))
        return;
    stop();
    m_command = command;
    m_failures = 0;
    start();
}

void InputMethodLauncher::restart()
{
    if (m_command.empty())
        return;
    stop();
    m_failures = 0;
    start();
}

void InputMethodLauncher::start()
{
    if (m_command.empty() || m_client)
        return;

    int fds[2] {-1, -1};
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds)) {
        LM_WARNING(lingmoWayland) << "Cannot create the input method connection";
        return;
    }
    m_client = wl_client_create(m_display, fds[0]);
    if (!m_client) {
        close(fds[0]);
        close(fds[1]);
        LM_WARNING(lingmoWayland) << "Cannot adopt the input method connection";
        return;
    }
    m_destroy.owner = this;
    m_destroy.listener.notify = [](wl_listener *listener, void *) {
        reinterpret_cast<DestroyListener *>(listener)->owner->connectionLost();
    };
    wl_client_add_destroy_listener(m_client, &m_destroy.listener);

    auto child = std::make_unique<Child>(m_loop);
    base::Process *process = &child->process;
    child->program = m_command.front();
    auto environment = m_environment;
    setVariable(environment, "WAYLAND_SOCKET", std::to_string(base::Process::kInheritedFd));
    process->setEnvironment(std::move(environment));
    process->setOutputForwarded(true);
    process->setInheritedFd(fds[1]);
    // Restarted when lost, so not one of the processes the session protects.
    process->setOomProtectionDropped(true);
    process->onFinished = [this, process](int code, bool) {
        const auto it = std::find_if(m_children.begin(), m_children.end(), [&](const auto &c) {
            return &c->process == process;
        });
        LM_INFO(lingmoWayland) << "Input method process" << (*it)->program << "exited with" << code;
        forget(process);
    };
    m_process = process;
    m_uptime.start();
    const bool started = process->start(
            m_command.front(), std::vector<std::string>(m_command.begin() + 1, m_command.end()));
    // The child holds the only other end, so its exit closes the connection.
    close(fds[1]);
    if (started) {
        m_children.push_back(std::move(child));
    } else {
        LM_WARNING(lingmoWayland) << "Cannot start input method" << m_command.front();
        m_process = nullptr;
    }
    LM_INFO(lingmoWayland) << "Started input method" << joined(m_command);
    runningChanged();
}

void InputMethodLauncher::stop()
{
    m_restartTimer.stop();
    if (auto *client = m_client) {
        wl_list_remove(&m_destroy.listener.link);
        m_client = nullptr;
        wl_client_destroy(client);
        runningChanged();
    }
    if (auto *process = std::exchange(m_process, nullptr)) {
        for (const auto &child : m_children) {
            if (&child->process == process)
                terminate(*child);
        }
    }
}

void InputMethodLauncher::terminate(Child &child)
{
    if (!child.process.isRunning())
        return;
    child.process.terminate();
    child.kill.setSingleShot(true);
    child.kill.onTimeout = [process = &child.process] {
        process->kill();
    };
    child.kill.start(kTerminateTimeout);
}

// Deleted from the loop: a process cannot go inside its own callback.
void InputMethodLauncher::forget(base::Process *process)
{
    if (m_process == process)
        m_process = nullptr;
    const auto it = std::find_if(m_children.begin(), m_children.end(), [&](const auto &child) {
        return &child->process == process;
    });
    if (it == m_children.end())
        return;
    std::shared_ptr<Child> doomed(std::move(*it));
    m_children.erase(it);
    m_loop.post([doomed] {});
}

void InputMethodLauncher::connectionLost()
{
    wl_list_remove(&m_destroy.listener.link);
    m_client = nullptr;
    runningChanged();

    // A process without its connection cannot serve input; do not leave a duplicate running.
    if (auto *process = std::exchange(m_process, nullptr)) {
        for (const auto &child : m_children) {
            if (&child->process == process)
                terminate(*child);
        }
    }
    if (m_uptime.isValid() && m_uptime.elapsedMs() >= kStableUptimeMs)
        m_failures = 0;
    if (++m_failures > kMaxFailures) {
        LM_WARNING(lingmoWayland) << "Input method" << joined(m_command)
                              << "keeps losing its connection; not restarting it";
        return;
    }
    const int delay = std::min(m_restartDelay << (m_failures - 1), m_restartDelay * 16);
    LM_WARNING(lingmoWayland) << "Input method connection lost; restarting in" << delay << "ms";
    m_restartTimer.start(std::chrono::milliseconds(delay));
}

} // namespace Lingmo::Wayland
