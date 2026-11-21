// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "searchwindow.h"
#include "ui/application.h"
#include <cstdlib>
#include <iostream>
#include <poll.h>
using namespace Search;
using namespace Lingmo;
void check(bool ok, const char* message) { if (!ok) { std::cerr << message << '\n'; std::exit(1); } }
void wait(const std::function<bool()>& ready) {
    auto& loop = ui::Application::instance()->loop(); auto end = std::chrono::steady_clock::now() + std::chrono::seconds(4);
    while (!ready() && std::chrono::steady_clock::now() < end) { pollfd fd{loop.fd(), POLLIN, 0}; ::poll(&fd, 1, 5); loop.dispatch(); }
    check(ready(), "runtime timeout");
}
int main(int argc, char** argv) {
    ui::Application app(argc, argv, ui::Application::Platform::Headless);
    auto* bus = base::dbus::Connection::session(); check(bus, "session bus");
    std::string launched; std::vector<std::string> command; bool failLaunch = false;
    base::dbus::Object runtime(bus, "/AppRuntime", "org.lingmo.AppRuntime");
    runtime.addMethod("launchApplication", [&](const std::string& id, const std::vector<std::string>&, const std::string&) { launched = id; return uint32_t(failLaunch ? 0 : 42); });
    runtime.addMethod("launchCommand", [&](const std::string&, const std::vector<std::string>& args, const std::string&, const std::string&) { command = args; return uint32_t(43); });
    check(runtime.publish() && bus->requestName("org.lingmo.AppRuntime"), "fake runtime");
    SearchWindow window(false, bus); window.setTestSources({{"editor.desktop", "Editor", "", "", "", {}}}, {});
    window.show(); window.setQuery("Editor"); wait([&] { return !window.searching(); }); window.activateSelected();
    wait([&] { return !window.visible(); }); check(launched == "editor.desktop", "application ID handed to runtime");
    window.show(); window.setCategory(Category::Settings); window.setQuery("display"); wait([&] { return !window.searching(); });
    check(!window.response().results.empty(), "settings found"); window.activateSelected(); wait([&] { return !window.visible(); });
    check(command == std::vector<std::string>({"lingmo-settings", "-m", "display"}), "settings launch arguments");
    failLaunch = true;
    window.show(); window.setCategory(Category::Applications); window.setQuery("Editor"); wait([&] { return !window.searching(); });
    window.activateSelected(); wait([&] { return window.response().results.empty(); });
    check(window.visible(), "runtime failure keeps search visible for correction");
    window.hide(); app.loop().dispatch();
    std::cout << "Search runtime integration passed\n";
}
