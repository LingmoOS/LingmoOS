// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/application.h"
#include "core/launcher.h"

#include "base/command_line.h"
#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/timer.h"
#include "ui/application.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

using namespace Lingmo;

enum class Request {
    Open,
    EmptyTrash,
    MoveToTrash,
};

static std::vector<std::string> formatUriList(const std::vector<std::string>& list)
{
    std::vector<std::string> paths;
    char current[4096];
    const std::string directory = ::getcwd(current, sizeof(current)) ? current : std::string();
    for (const std::string& path : list)
        paths.push_back(path == "." ? directory : path);
    if (paths.empty())
        paths.push_back(directory);
    return paths;
}

// Hands the request to the instance that holds the name. False when there is
// none, or it was quitting just now: then this process serves it.
static bool forward(base::dbus::Connection& bus, Request request, const std::vector<std::string>& arguments)
{
    // Not activatable: a D-Bus started file manager would also open its cwd.
    base::dbus::Proxy daemon(&bus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
    const base::dbus::Reply owned = daemon.call("NameHasOwner", std::string("org.lingmo.FileManager"));
    bool hasOwner = false;
    if (owned.isError() || !owned.read(hasOwner) || !hasOwner)
        return false;

    base::dbus::Message call =
            bus.newMethodCall("org.lingmo.FileManager", "/FileManager", "org.lingmo.FileManager",
                              request == Request::EmptyTrash    ? "emptyTrash"
                              : request == Request::MoveToTrash ? "moveToTrash"
                                                                : "openFiles");
    if (request == Request::MoveToTrash)
        call.append(arguments);
    else if (request == Request::Open)
        call.append(formatUriList(arguments));
    return !bus.call(call).isError();
}

int main(int argc, char** argv)
{
    base::CommandLineParser parser;
    parser.setApplicationDescription("Files");
    parser.addHelpOption();
    parser.addPositionalArgument("files", "Files", "[FILE1, FILE2,...]");
    parser.addOption({"dbus-activated", "Wait for a D-Bus file manager request"});
    parser.addOption({"daemon", "Keep running for the session"});
    parser.addOption({std::vector<std::string> {"e", "empty-trash"}, "Empty Trash"});
    parser.addOption({std::vector<std::string> {"mtr", "move-to-trash"}, "Move To Trash"});
    // The session hands the daemon its trusted Wayland connection for the desktop.
    parser.addOption({"desktop-socket", "Show the desktop on this Wayland socket descriptor", "fd"});
    parser.addOption({"desktop-window", "Show the desktop in an ordinary window"});
    parser.process(argc, argv);

    // The session clears close-on-exec for the exec into this process only;
    // set again before anything can start a program that would inherit it.
    int desktopSocket = -1;
    if (parser.isSet("desktop-socket")) {
        const std::string value = parser.value("desktop-socket");
        char* end = nullptr;
        const long fd = std::strtol(value.c_str(), &end, 10);
        if (!value.empty() && *end == '\0' && fd >= 0 && fd <= INT_MAX && fm::launcher::keepFromChildren(int(fd)))
            desktopSocket = int(fd);
        else
            std::fprintf(stderr, "No desktop: %s is no open descriptor.\n", value.c_str());
    }

    const bool daemon = parser.isSet("daemon");
    const bool dbusActivated = parser.isSet("dbus-activated");
    const Request request = parser.isSet("empty-trash")    ? Request::EmptyTrash
                            : parser.isSet("move-to-trash") ? Request::MoveToTrash
                                                            : Request::Open;
    const std::vector<std::string> arguments = parser.positionalArguments();

    // Before any window or GPU context: most launches only pass their request on.
    if (!daemon && !dbusActivated) {
        base::EventLoop loop;
        if (auto bus = base::dbus::Connection::open(base::dbus::Connection::Bus::Session, &loop)) {
            if (forward(*bus, request, arguments))
                return EXIT_SUCCESS;
        }
    }

    ui::Application app(argc, argv);
    app.setApplicationId("lingmo-filemanager");
    i18n::setDomain("lingmo-filemanager");
    fm::Application application(app, daemon, dbusActivated);

    if (daemon) {
        application.startDaemon();
        if (desktopSocket >= 0) {
            if (!application.startDesktop(desktopSocket))
                std::fprintf(stderr, "No desktop: socket %d does not offer the shell interface.\n", desktopSocket);
        } else if (parser.isSet("desktop-window") && !parser.isSet("desktop-socket")) {
            application.startDesktop(-1);
        }
        return app.run();
    }

    if (!application.registerServices()) {
        if (dbusActivated)
            return EXIT_FAILURE;
        // Another instance took the name in between.
        if (app.sessionBus() && forward(*app.sessionBus(), request, arguments))
            return EXIT_SUCCESS;
    }

    // Activation leaves the bus time to deliver the request after the name is
    // ours, without opening an unrelated default window.
    base::Timer activationGrace;
    if (dbusActivated) {
        activationGrace.setSingleShot(true);
        activationGrace.onTimeout = [&application] { application.scheduleIdleCheck(); };
        activationGrace.start(std::chrono::seconds(10));
    } else {
        // Each action leaves a window or a task behind, or nothing: then the
        // idle check ends the process.
        if (request == Request::EmptyTrash)
            application.emptyTrash();
        else if (request == Request::MoveToTrash)
            application.moveToTrash(arguments);
        else
            application.openFiles(formatUriList(arguments));
        application.scheduleIdleCheck();
    }

    const int code = app.run();
    return code ? code : application.exitCode();
}
