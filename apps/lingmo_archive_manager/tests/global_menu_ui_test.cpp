// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "archivewindow.h"
#include "base/dbus.h"
#include "ui/application.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <poll.h>
using namespace Lingmo;
using namespace std::chrono_literals;
void require(bool value, const char* message) { if (!value) { std::cerr << message << '\n'; std::exit(1); } }
ui::View* find(ui::View* view, const std::string& name) {
    if (view->name() == name) return view;
    for (const auto& child : view->children()) if (auto* result = find(child.get(), name)) return result;
    return nullptr;
}
void wait(base::EventLoop& loop, const std::function<bool()>& ready) {
    const auto end = std::chrono::steady_clock::now() + 3s;
    while (!ready() && std::chrono::steady_clock::now() < end) { pollfd fd{loop.fd(), POLLIN, 0}; ::poll(&fd, 1, 10); loop.dispatch(); }
    require(ready(), "menu UI update timed out");
}
using Layout = std::tuple<int32_t, base::dbus::VariantMap, std::vector<base::dbus::Variant>>;
int32_t item(const Layout& layout, const std::string& label) {
    const auto& properties = std::get<1>(layout);
    if (auto it = properties.find("label"); it != properties.end() && it->second.value<std::string>() == label) return std::get<0>(layout);
    for (const auto& child : std::get<2>(layout)) if (auto id = item(child.value<Layout>(), label); id > 0) return id;
    return -1;
}
int main(int argc, char** argv) {
    ui::Application app(argc, argv, ui::Application::Platform::Headless);
    auto* bus = base::dbus::Connection::session(&app.loop()); require(bus, "application bus");
    auto shell = base::dbus::Connection::open(base::dbus::Connection::Bus::Session, &app.loop()); require(bool(shell), "shell bus");
    constexpr auto name = "com.canonical.AppMenu.Registrar";
    base::dbus::Object registrar(shell.get(), "/com/canonical/AppMenu/Registrar", name);
    registrar.addMethod("RegisterWindow", [](uint32_t, base::dbus::ObjectPath) {});
    registrar.addMethod("UnregisterWindow", [](uint32_t) {});
    require(registrar.publish(), "registrar object");
    ArchiveManager::ArchiveWindow window(bus); window.show(); window.window().grab();
    auto* fallback = find(window.window().contentView(), "local-menu"); require(fallback && fallback->isVisible(), "fallback before registrar");
    require(shell->requestName(name), "start global menus");
    wait(app.loop(), [&] { return !fallback->isVisible(); });
    base::dbus::Proxy menu(shell.get(), bus->uniqueName(), ArchiveManager::GlobalMenu::kPath, "com.canonical.dbusmenu");
    bool ready = false; Layout layout; uint32_t revision = 0;
    auto pending = menu.callAsync("GetLayout", [&](const base::dbus::Reply& reply) {
        require(reply.read(revision, layout), "menu layout"); ready = true;
    }, int32_t(0), int32_t(-1), std::vector<std::string>{});
    wait(app.loop(), [&] { return ready; });
    require(std::get<2>(layout).size() == 5, "five global menus");
    const int32_t newArchive = item(layout, "New Archive"); require(newArchive > 0, "new archive action exported");
    menu.send("Event", newArchive, std::string("clicked"), base::dbus::Variant(std::string()), uint32_t(0));
    wait(app.loop(), [&] { return window.page() == ArchiveManager::ArchiveWindow::Page::Create; });
    shell->releaseName(name); wait(app.loop(), [&] { return fallback->isVisible(); });
    require(shell->requestName(name), "restart global menus"); wait(app.loop(), [&] { return !fallback->isVisible(); });
    require(window.page() == ArchiveManager::ArchiveWindow::Page::Create, "menu restart preserves page");
    std::cout << "Archive global menu UI integration passed\n";
}
