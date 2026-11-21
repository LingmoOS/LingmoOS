// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/common/app_launcher.h"

#include <chrono>
#include <utility>

#include "base/desktop_entry.h"
#include "base/process.h"

namespace Lingmo::Shell {

// A runtime that stops answering must not leave the start hanging for the
// bus's half minute.
static constexpr std::chrono::seconds kRuntimeTimeout(5);

ApplicationLauncher::ApplicationLauncher(base::dbus::Connection *bus,
                                         std::function<std::string()> createActivationToken)
    : m_createActivationToken(std::move(createActivationToken))
    , m_runtime(bus, "org.lingmo.AppRuntime", "/AppRuntime", "org.lingmo.AppRuntime")
{
    m_runtime.setTimeout(kRuntimeTimeout);
}

// The token travels in the environment for a program started here, and in the
// runtime's arguments for one it starts; either way the application reads it
// back and asks to be activated with it.
static std::vector<std::string> activationEnvironment(const std::string &token)
{
    return token.empty() ? std::vector<std::string>()
                         : std::vector<std::string> {"XDG_ACTIVATION_TOKEN=" + token};
}

static bool startHere(const std::vector<std::string> &command, const std::string &workingDirectory,
                      const std::vector<std::string> &environment)
{
    if (command.empty() || command.front().empty())
        return false;
    const std::vector<std::string> arguments(command.begin() + 1, command.end());
    return base::Process::startDetached(command.front(), arguments, workingDirectory, environment) > 0;
}

void ApplicationLauncher::launch(const base::DesktopEntry &entry, const std::vector<std::string> &arguments,
                                 Done done)
{
    const std::vector<std::string> command = entry.commandForArguments(arguments);
    const std::string workingDirectory = entry.workingDirectory;
    const std::string id = entry.id;
    // One token for the whole launch: the runtime's answer and its fallback
    // are the same start.
    const std::string token = m_createActivationToken ? m_createActivationToken() : std::string();
    if (!m_runtime.isValid() || id.empty()) {
        startCommand(command, workingDirectory, id, token, std::move(done));
        return;
    }

    m_calls.track([&](std::function<void()> finished) {
        return m_runtime.callAsync(
                "launchApplication",
                [this, command, workingDirectory, id, token, done, finished](const base::dbus::Reply &reply) {
                    if (reply.value<uint32_t>().value_or(0) != 0) {
                        if (done)
                            done(true);
                    } else {
                        startCommand(command, workingDirectory, id, token, done);
                    }
                    finished();
                },
                id, arguments, token);
    });
}

void ApplicationLauncher::launchCommand(const std::vector<std::string> &command,
                                        const std::string &workingDirectory, const std::string &appId, Done done)
{
    startCommand(command, workingDirectory, appId,
                 m_createActivationToken ? m_createActivationToken() : std::string(), std::move(done));
}

void ApplicationLauncher::startCommand(const std::vector<std::string> &command,
                                       const std::string &workingDirectory, const std::string &appId,
                                       const std::string &activationToken, Done done)
{
    if (command.empty() || command.front().empty()) {
        if (done)
            done(false);
        return;
    }
    const std::vector<std::string> environment = activationEnvironment(activationToken);
    if (!m_runtime.isValid()) {
        const bool started = startHere(command, workingDirectory, environment);
        if (done)
            done(started);
        return;
    }

    m_calls.track([&](std::function<void()> finished) {
        return m_runtime.callAsync(
                "launchCommand",
                [command, workingDirectory, environment, done, finished](const base::dbus::Reply &reply) {
                    const bool started = reply.value<uint32_t>().value_or(0) != 0
                            || startHere(command, workingDirectory, environment);
                    if (done)
                        done(started);
                    finished();
                },
                appId, command, workingDirectory, activationToken);
    });
}

void ApplicationLauncher::quitApplication(const std::string &appId)
{
    if (!appId.empty())
        m_runtime.send("quitApplication", appId);
}

bool ApplicationLauncher::startDetached(const std::string &program, const std::vector<std::string> &arguments,
                                        const std::vector<std::string> &environment)
{
    return base::Process::startDetached(program, arguments, {}, environment) > 0;
}

} // namespace Lingmo::Shell
