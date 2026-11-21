// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <string>
#include <vector>

// Opening files with applications: the session's AppRuntime starts them,
// mimeapps.list chooses them.
namespace fm::launcher {

struct Application
{
    std::string desktopFile;
    std::string name;
    std::string icon;
};

// The application that opens the type, or empty.
std::string defaultApplication(const std::string& mimeType);
// Every application that opens it, the installed first.
std::vector<std::string> recommendedApplications(const std::string& mimeType);
// For the Open With menu and dialog: the default first, then the others.
std::vector<Application> applicationsFor(const std::string& path, const std::string& mimeType);
bool setDefaultApplication(const std::string& mimeType, const std::string& desktopFile);

// Where launches get an xdg-activation token: the active window's, asked
// for asynchronously; unset launches without one.
void setActivationTokenSource(std::function<void(std::function<void(const std::string& token)>)> source);
// Starts the application (a desktop file path or id) with the file.
bool launchApplication(const std::string& desktopFile, const std::string& file = {});
// Runs a program or a .desktop launcher.
bool launchFile(const std::string& path);
// A launcher's Name and Icon can pose as anything, so only installed ones run.
bool isTrusted(const std::string& path);
// Makes the file executable for its owner and runs it, as the launch prompt's Run does.
bool trustAndLaunch(const std::string& path);
// The preferred terminal in the folder.
void launchTerminal(const std::string& folder);
// Any other program, through the runtime as well.
bool startCommand(const std::vector<std::string>& command, const std::string& workingDirectory = {},
                  const std::string& appId = {});
// A descriptor this process was handed stays out of every program it starts.
bool keepFromChildren(int fd);

} // namespace fm::launcher
