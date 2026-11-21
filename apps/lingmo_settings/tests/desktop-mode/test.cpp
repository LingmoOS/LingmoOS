// SPDX-License-Identifier: GPL-3.0-or-later
#include "pages/dock/docksettings.h"
#include "base/event_loop.h"
#include <chrono>
#include <poll.h>
#include <cstdio>
#include <stdexcept>
using namespace Lingmo::base;
void check(bool ok, const char *message) { if (!ok) throw std::runtime_error(message); }
int main() {
    try {
        EventLoop loop;
        auto server = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        bool efficient = false, reject = false;
        dbus::Object dock(server.get(), "/Dock", "org.lingmo.Dock");
        dock.addProperty("efficientMode", [&] { return efficient; });
        dock.addMethod("setEfficientMode", "b", "", [&](dbus::MethodCall &call) {
            if (reject) { call.replyError("org.lingmo.Error", "Rejected"); return; }
            call.read(efficient);
            dock.emitPropertiesChanged({"efficientMode"}); call.reply();
        });
        check(dock.publish(), "publish fake Dock");
        settings::DockSettings client;
        check(!client.modeAvailable(), "unavailable without owner");
        auto wait = [&](const std::function<bool()> &done) {
            const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(4);
            while (!done() && std::chrono::steady_clock::now() < end) {
                pollfd fd{loop.fd(), POLLIN, 0}; poll(&fd, 1, 10); loop.dispatch();
            }
            check(done(), "timeout");
        };
        check(server->requestName("org.lingmo.Dock"), "own Dock");
        wait([&] { return client.modeAvailable(); });
        client.setEfficientMode(true);
        check(client.modePending(), "pending state");
        wait([&] { return !client.modePending() && client.efficientMode(); });
        reject = true;
        client.setEfficientMode(false);
        wait([&] { return !client.modePending() && !client.modeError().empty(); });
        check(client.efficientMode(), "rejected change must preserve actual state");
        server->releaseName("org.lingmo.Dock");
        wait([&] { return !client.modeAvailable(); });
        reject = false; efficient = false;
        server->requestName("org.lingmo.Dock");
        wait([&] { return client.modeAvailable() && !client.efficientMode(); });
        std::puts("Desktop mode Settings service state tests passed");
    } catch (const std::exception &e) { std::fprintf(stderr,"%s\n",e.what()); return 1; }
}
