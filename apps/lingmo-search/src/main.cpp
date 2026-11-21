// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "searchwindow.h"
#include "base/i18n.h"
#include "ui/application.h"
#include <cstdio>
#include <string>
int main(int argc, char** argv) {
    bool service = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--service") service = true;
        else if (arg == "--help" || arg == "-h") { std::puts("Usage: lingmo-search [--service]\nToggle Lingmo global search. --service waits for D-Bus activation."); return 0; }
        else if (arg == "--version") { std::puts("Lingmo Search 1.0.0"); return 0; }
        else { std::fprintf(stderr, "Unknown argument: %s\n", argv[i]); return 1; }
    }
    Lingmo::ui::Application app(argc, argv); app.setApplicationId("lingmo-search");
    Lingmo::I18n::setDomain("lingmo-search");
    auto* bus = app.sessionBus();
    if (!bus) { std::fprintf(stderr, "A session D-Bus is required\n"); return 1; }
    constexpr auto name = "org.lingmo.Search";
    if (!bus->requestName(name, Lingmo::base::dbus::Connection::DoNotQueue)) {
        if (service) return 0;
        Lingmo::base::dbus::Proxy existing(bus, name, "/org/lingmo/Search", name);
        existing.setTimeout(std::chrono::seconds(5));
        return existing.call("toggle", app.takeActivationToken()).isError() ? 1 : 0;
    }
    Search::SearchWindow window;
    Lingmo::base::dbus::Object object(bus, "/org/lingmo/Search", name);
    object.addMethod("toggle", [&window](const std::string& token) { window.toggle(token); });
    object.addMethod("show", [&window](const std::string& token) { window.show(token); });
    object.addMethod("hide", [&window] { window.hide(); });
    if (!object.publish()) return 1;
    if (!service) window.show(app.takeActivationToken());
    return app.run();
}
