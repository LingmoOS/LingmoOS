// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "common/environment.h"

// The session's application runtime, org.lingmo.AppRuntime, as the other
// core processes use it. Calls block for at most five seconds.
namespace Lingmo::Core::runtime {

// What an application is started in when the runtime does not start it:
// this process's environment without its Wayland connection or activation
// token, with the user's languages and proxy.
Environment applicationEnvironment(const std::string& activationToken = {});

// appId is only a hint for the runtime: it is the key an instance is tracked
// and can later be quit under. Without the runtime the command is started
// here.
bool launchCommand(const std::vector<std::string>& command, const std::string& workingDirectory = {},
                   const std::string& appId = {});
// A desktop entry id or path; false without the runtime.
bool launchApplication(const std::string& appId, const std::vector<std::string>& arguments = {});

bool quitApplication(const std::string& appId);
bool quitAll();
std::vector<std::string> runningApplications();

} // namespace Lingmo::Core::runtime
