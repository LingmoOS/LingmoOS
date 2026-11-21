// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <wayland-server-core.h>

#include <memory>
#include <string>
#include <vector>

#include "base/elapsed_timer.h"
#include "base/signal.h"
#include "base/timer.h"
#include "core/process_environment.h"

namespace Lingmo::base {
class EventLoop;
class Process;
} // namespace Lingmo::Base

namespace Lingmo::Wayland {

// Runs the selected input method on a Wayland connection the compositor
// creates, like the Xwayland child; that connection is the only one allowed to
// act as the input method. A lost connection is restarted with backoff.
class InputMethodLauncher
{
public:
    // Runs its processes and timers on `loop`, the main loop when null.
    explicit InputMethodLauncher(wl_display *display, base::EventLoop *loop = nullptr);
    ~InputMethodLauncher();
    InputMethodLauncher(const InputMethodLauncher &) = delete;
    InputMethodLauncher &operator=(const InputMethodLauncher &) = delete;

    // An empty command stops the input method. Setting the current command
    // again restarts a method that gave up.
    void setCommand(const std::vector<std::string> &command);

    // NAME=value entries.
    void setEnvironment(std::vector<std::string> environment)
    {
        m_environment = std::move(environment);
    }
    // Starts the current command again, with the current environment.
    void restart();

    void setRestartDelay(int milliseconds) { m_restartDelay = milliseconds; }

    bool isInputMethod(const wl_client *client) const { return client && client == m_client; }

    bool isRunning() const { return m_client; }

    base::Signal<> runningChanged;

private:
    struct DestroyListener {
        wl_listener listener;
        InputMethodLauncher *owner;
    };
    struct Child;

    void start();
    void stop();
    void connectionLost();
    void terminate(Child &child);
    void forget(base::Process *process);

    base::EventLoop &m_loop;
    wl_display *m_display;
    wl_client *m_client = nullptr;
    DestroyListener m_destroy {};
    std::vector<std::string> m_command;
    std::vector<std::string> m_environment = childEnvironment();
    // Every process started and not yet ended, the current one among them.
    std::vector<std::unique_ptr<Child>> m_children;
    base::Process *m_process = nullptr;
    base::Timer m_restartTimer;
    base::ElapsedTimer m_uptime;
    int m_restartDelay = 1000;
    int m_failures = 0;
};

} // namespace Lingmo::Wayland
