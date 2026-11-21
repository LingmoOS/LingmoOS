// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "editorsettings.h"

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/i18n.h"
#include "ui/application.h"

#include <csignal>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <unistd.h>

#ifndef TEXT_EDITOR_VERSION
#define TEXT_EDITOR_VERSION "1.0"
#endif

namespace dbus = Lingmo::base::dbus;

static void printHelp(const char *program)
{
    std::printf("Usage: %s [options] [+LINE] [FILE...] [-]\nLingmo Text Editor\n\nOptions:\n"
                "  -h, --help     Displays help on commandline options.\n"
                "  --help-all     Displays help.\n"
                "  -v, --version  Displays version information.\n"
                "  --new-window   Open a new window.\n\n"
                "Arguments:\n"
                "  files          Files to open; +LINE goes to that line of the next file, - reads\n"
                "                 standard input.\n",
                program);
}

// The live test runs the same with a scratch data folder and settings file,
// without the D-Bus service, so it never meets the user's session.
#ifdef TEXT_EDITOR_LIVE_TEST
#include "session.h"
#endif

int main(int argc, char **argv)
{
#ifdef TEXT_EDITOR_LIVE_TEST
    // argv[1] is the scratch folder.
    const std::string scratch = argv[1];
    ++argv;
    --argc;
    Session::setDataDirectory(scratch + "/data");
#endif
    std::vector<std::string> files;
    bool newWindow = false;
    bool readInput = false;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "-h" || argument == "--help" || argument == "-?" || argument == "--help-all") {
            printHelp(argv[0]);
            return 0;
        }
        if (argument == "-v" || argument == "--version") {
            std::printf("lingmo-text-editor %s\n", TEXT_EDITOR_VERSION);
            return 0;
        }
        if (argument == "--new-window") {
            newWindow = true;
            continue;
        }
        if (argument == "-") {
            readInput = true;
            continue;
        }
        if (argument.size() > 1 && argument[0] == '-' && argument[1] != '-' && !std::isdigit(static_cast<unsigned char>(argument[1]))) {
            std::fprintf(stderr, "Unknown option '%s'.\n", argument.substr(1).c_str());
            return 1;
        }
        if (argument.starts_with("--")) {
            std::fprintf(stderr, "Unknown option '%s'.\n", argument.substr(2).c_str());
            return 1;
        }
        files.push_back(argument);
    }

    std::optional<std::string> standardInput;
    if (readInput)
        standardInput = std::string(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());

    // Logging out ends the process with SIGTERM: the session is kept, as on
    // quit. Blocked before any thread starts, so the signals reach the fd.
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGTERM);
    sigaddset(&signals, SIGHUP);
    sigprocmask(SIG_BLOCK, &signals, nullptr);

    Lingmo::ui::Application app(argc, argv);
#ifdef TEXT_EDITOR_LIVE_TEST
    app.setApplicationId("lingmo-text-editor-live");
#else
    app.setApplicationId("lingmo-text-editor");
#endif
    Lingmo::i18n::setDomain("lingmo-text-editor");

    std::error_code error;
    const std::string workingDirectory = std::filesystem::current_path(error).string();

#ifdef TEXT_EDITOR_LIVE_TEST
    EditorSettings settings(scratch + "/text-editor.conf");
    EditorApplication application(settings);
    if (false) {
#else
    EditorSettings settings(EditorSettings::defaultPath());
    EditorApplication application(settings);
    if (!application.registerService()) {
#endif
        // A running editor takes the files and this process ends.
        dbus::Proxy existing(dbus::Connection::session(), "org.lingmo.TextEditor", "/TextEditor",
                             "org.lingmo.TextEditor");
        existing.setTimeout(std::chrono::seconds(5));
        if (newWindow)
            existing.call("NewWindow");
        if (!files.empty() || !standardInput)
            existing.call("Open", files, workingDirectory);
        if (standardInput)
            existing.call("OpenText", *standardInput);
        return 0;
    }

    const int signalFd = signalfd(-1, &signals, SFD_NONBLOCK | SFD_CLOEXEC);
    if (signalFd >= 0) {
        app.loop().watch(signalFd, EPOLLIN, [&application, signalFd](uint32_t) {
            signalfd_siginfo info;
            while (::read(signalFd, &info, sizeof(info)) == ssize_t(sizeof(info))) {
            }
            application.terminate();
        });
    }

    application.start(newWindow && files.empty() ? std::vector<std::string>() : files, workingDirectory, standardInput);
    const int result = app.run();
    if (signalFd >= 0)
        ::close(signalFd);
    return result;
}
