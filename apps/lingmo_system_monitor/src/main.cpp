// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icons.h"
#include "monitor.h"
#include "preferences.h"
#include "views/mainwindow.h"

#include "base/application_registry.h"
#include "base/dbus.h"
#include "base/i18n.h"
#include "ui/application.h"
#include "ui/icon.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>

using namespace Lingmo;

static const char* const kService = "org.lingmo.SystemMonitor";
static const char* const kPath = "/SystemMonitor";

// As the Qt version's command line: --help and --version, nothing else.
static int parseArguments(int argc, char** argv, bool* quit)
{
    *quit = false;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "-h" || argument == "--help" || argument == "-?") {
            std::printf("Usage: %s [options]\nLingmo System Monitor\n\nOptions:\n"
                        "  -h, --help     Displays help on commandline options.\n"
                        "  -v, --version  Displays version information.\n",
                        argv[0]);
            *quit = true;
            return 0;
        }
        if (argument == "-v" || argument == "--version") {
            std::printf("lingmo-system-monitor %s\n", SYSTEM_MONITOR_VERSION);
            *quit = true;
            return 0;
        }
        std::fprintf(stderr, "Unknown option '%s'.\n", argument.substr(argument.find_first_not_of('-')).c_str());
        *quit = true;
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    bool quit = false;
    const int status = parseArguments(argc, argv, &quit);
    if (quit)
        return status;

    ui::Application app(argc, argv);
    app.setApplicationId("lingmo-system-monitor");
    i18n::setDomain("lingmo-system-monitor");
    for (const auto& [name, svg] : embeddedIcons())
        ui::Icon::registerGlyph(std::string(name), std::string(svg));

    base::dbus::Connection* bus = app.sessionBus();
    if (bus && !bus->requestName(kService)) {
        base::dbus::Proxy running(bus, kService, kPath, kService);
        running.setTimeout(std::chrono::milliseconds(1500));
        running.call("activateWindow");
        return 0;
    }

    base::ApplicationRegistry& registry = base::ApplicationRegistry::instance();
    ApplicationLookup lookup;
    lookup.resolve = [&registry](const std::string& name, const std::string& program) -> std::optional<DesktopApplication> {
        const auto entry = registry.resolveWindow(name, {}, program, name);
        if (!entry)
            return std::nullopt;
        return DesktopApplication{entry->id, entry->name, entry->icon, entry->noDisplay, entry->hidden};
    };
    lookup.byId = [&registry](const std::string& id) -> std::optional<DesktopApplication> {
        const auto entry = registry.byId(id);
        if (!entry)
            return std::nullopt;
        return DesktopApplication{entry->id, entry->name, entry->icon, entry->noDisplay, entry->hidden};
    };

    Monitor monitor(lookup, Preferences::defaultPath());
    const base::Subscription applications = registry.changed.connect([&monitor] { monitor.applicationsChanged(); });
    MainWindow window(monitor);

    std::unique_ptr<base::dbus::Object> service;
    if (bus) {
        service = std::make_unique<base::dbus::Object>(bus, kPath, kService);
        service->addMethod("activateWindow", [&window] { window.activate(); });
        service->publish();
    }

    window.window().show();
    return app.run();
}
