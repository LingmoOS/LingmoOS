// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "shell/common/pending_calls.h"

namespace Lingmo::base {
struct DesktopEntry;
} // namespace Lingmo::Base

namespace Lingmo::Shell {

// Starts applications the way every start in the session goes: through the
// session's AppRuntime (org.lingmo.AppRuntime), which puts each in a scope
// of its own, or here when the runtime does not answer. Nothing waits: the
// runtime's answer, or its absence, arrives later.
class ApplicationLauncher
{
public:
    using Done = std::function<void(bool started)>;

    // `createActivationToken` mints the token that lets the window a launch
    // opens come to the front: without one the compositor has no proof the
    // user asked for it, and Wayland puts it behind. Unset, launches go out
    // without a token, as they did before the shell had one to give.
    explicit ApplicationLauncher(base::dbus::Connection *bus = base::dbus::Connection::session(),
                                 std::function<std::string()> createActivationToken = {});

    // The entry by its id, as DesktopEntry::launch() did: the runtime starts
    // it and ties it to the entry; its command is the fallback.
    void launch(const base::DesktopEntry &entry, const std::vector<std::string> &arguments = {},
                Done done = {});
    // A command for the application `appId`.
    void launchCommand(const std::vector<std::string> &command, const std::string &workingDirectory,
                       const std::string &appId, Done done = {});
    // Asks the runtime to end what still runs of the application.
    void quitApplication(const std::string &appId);
    // Runs a program of the session's own, such as Settings, on its own;
    // `environment` holds `NAME=value` entries set over the compositor's.
    static bool startDetached(const std::string &program, const std::vector<std::string> &arguments = {},
                              const std::vector<std::string> &environment = {});

private:
    // The one place a launch leaves this class from, with the token both
    // public entries minted for it.
    void startCommand(const std::vector<std::string> &command, const std::string &workingDirectory,
                      const std::string &appId, const std::string &activationToken, Done done);

    std::function<std::string()> m_createActivationToken;
    base::dbus::Proxy m_runtime;
    PendingCalls m_calls;
};

} // namespace Lingmo::Shell

