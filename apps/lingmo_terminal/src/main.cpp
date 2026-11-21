// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <pwd.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "base/i18n.h"
#include "ui/application.h"

#include "terminalsettings.h"
#include "terminalwindow.h"
#include "windows.h"

using namespace Lingmo;

static void printHelp(const char* program)
{
    std::printf("Usage: %s [options]\nLingmo Terminal\n\nOptions:\n"
                "  -h, --help                 Displays help on commandline options.\n"
                "  --help-all                 Displays help.\n"
                "  -w, --workdir <directory>  Start in <directory>.\n"
                "  -e, --execute              Run <command> and its arguments instead of the\n"
                "                             shell.\n",
                program);
}

// The user's login shell, what a session's SHELL is set to.
static std::string userShell()
{
    if (const passwd* entry = getpwuid(getuid()); entry && entry->pw_shell && *entry->pw_shell)
        return entry->pw_shell;
    return "/bin/sh";
}

int main(int argc, char** argv)
{
    std::vector<std::string> arguments(argv, argv + argc);
    std::vector<std::string> command;

    // Everything after -e is the command and its own options, so it is split
    // off before the options are read; the first of the two counts, a later
    // one belongs to the command.
    const auto shortIndex = std::find(arguments.begin(), arguments.end(), "-e");
    const auto longIndex = std::find(arguments.begin(), arguments.end(), "--execute");
    const auto executeIndex = std::min(shortIndex, longIndex);
    if (executeIndex != arguments.end() && executeIndex != arguments.begin()) {
        command.assign(executeIndex + 1, arguments.end());
        arguments.erase(executeIndex, arguments.end());
    }

    std::string workdir;
    for (size_t i = 1; i < arguments.size(); ++i) {
        const std::string& argument = arguments[i];
        if (argument == "-h" || argument == "--help" || argument == "-?" || argument == "--help-all") {
            printHelp(argv[0]);
            return 0;
        }
        if (argument == "-w" || argument == "--workdir") {
            if (i + 1 >= arguments.size()) {
                std::fprintf(stderr, "Missing value after '%s'.\n", argument.c_str());
                return 1;
            }
            workdir = arguments[++i];
            continue;
        }
        if (argument.starts_with("--workdir=")) {
            workdir = argument.substr(10);
            continue;
        }
        if (argument.starts_with("-w") && argument.size() > 2) {
            workdir = argument.substr(2);
            continue;
        }
        if (argument.starts_with('-')) {
            std::fprintf(stderr, "Unknown option '%s'.\n", argument.substr(argument.find_first_not_of('-')).c_str());
            return 1;
        }
        // Positional arguments are accepted and left unused.
    }

    // A single argument with spaces is a command line, as xterm reads it.
    if (command.size() == 1 && command.front().find(' ') != std::string::npos)
        command = {userShell(), "-c", command.front()};

    if (!workdir.empty()) {
        std::error_code error;
        workdir = std::filesystem::absolute(workdir, error).lexically_normal().string();
        while (workdir.size() > 1 && workdir.back() == '/')
            workdir.pop_back();
    }

    ui::Application app(argc, argv);
    app.setApplicationId("lingmo-terminal");
    i18n::setDomain("lingmo-terminal");

    TerminalSettings settings(TerminalSettings::defaultPath());
    Windows windows(settings);

    TerminalWindow::Options options;
    // Given on the command line: lingmo-terminal --workdir <dir> -e <command>.
    options.startDirectory = workdir.empty() ? "$PWD" : workdir;
    options.command = command;
    windows.create(std::move(options));

    return app.run();
}
