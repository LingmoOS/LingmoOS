// SPDX-License-Identifier: GPL-3.0-or-later
#include "network/connectivity.h"
#include "base/event_loop.h"
#include <iostream>
#include <stdexcept>

namespace dbus = Lingmo::base::dbus;
void check(bool ok, const char *message) { if (!ok) throw std::runtime_error(message); }
int main()
{
    try {
        Lingmo::base::EventLoop loop;
        auto server = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        auto client = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        uint32_t checks = 0;
        uint32_t prompts = 0;
        uint32_t state = 2;
        bool enabled = true;
        std::string uri = "http://network-test.example/check";
        std::string type = "802-11-wireless";
        dbus::ObjectPath primary{"/active/one"};
        dbus::Object nm(server.get(), "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager");
        nm.addMethod("CheckConnectivity", [&] { ++checks; return state; });
        nm.addProperty("Connectivity", [&] { return state; });
        nm.addProperty("ConnectivityCheckEnabled", [&] { return enabled; });
        nm.addProperty("ConnectivityCheckAvailable", [] { return true; });
        nm.addProperty("ConnectivityCheckUri", [&] { return uri; });
        nm.addProperty("PrimaryConnection", [&] { return primary; });
        dbus::Object active(server.get(), "/active/one", "org.freedesktop.NetworkManager.Connection.Active");
        active.addProperty("Type", [&] { return type; });
        active.addProperty("State", [] { return uint32_t(2); });
        check(nm.publish() && active.publish(), "publish NetworkManager mock");
        check(server->requestName("org.freedesktop.NetworkManager"), "own NetworkManager");
        dbus::Object notifications(server.get(), "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
        notifications.addMethod("Notify", "susssasa{sv}i", "u", [&](dbus::MethodCall &call) {
            ++prompts;
            call.reply(uint32_t(1));
        });
        notifications.addMethod("CloseNotification", [](uint32_t) {});
        check(notifications.publish(), "publish notification mock");
        check(server->requestName("org.freedesktop.Notifications"), "own notifications");
        unsigned opened = 0;
        std::string openedUri;
        Connectivity service(client.get(), client.get(), [&](const std::string &url) {
            ++opened;
            openedUri = url;
            return true;
        });
        check(service.publish(), "publish connectivity service");
        check(client->requestName("org.lingmo.Services"), "own service");
        auto settle = [&] {
            Lingmo::base::Timer timer;
            timer.setSingleShot(true);
            timer.onTimeout = [&] { loop.quit(); };
            timer.start(std::chrono::milliseconds(50));
            loop.run();
        };
        settle();
        check(service.state() == Connectivity::Portal && service.wireless(), "portal detection");
        check(opened == 0, "no unsolicited browser");
        check(checks == 1, "probe new route");
        check(prompts == 1, "one login prompt");
        service.refresh();
        settle();
        check(prompts == 1 && checks == 1, "do not repeat prompt or probe on same snapshot");
        check(service.openLogin() && opened == 1 && openedUri == uri, "login URL");
        dbus::Proxy publicApi(server.get(), "org.lingmo.Services", "/org/lingmo/Services/Connectivity",
                              "org.lingmo.Services.Connectivity");
        bool response = false;
        auto request = publicApi.callAsync("openLogin", [&](const dbus::Reply &reply) {
            response = reply.value<bool>().value_or(false);
        });
        settle();
        check(response && opened == 2, "D-Bus login action");
        state = 4;
        nm.emitPropertiesChanged({"Connectivity"});
        settle();
        check(service.state() == Connectivity::Full && !service.openLogin(), "login completed");
        state = 3;
        nm.emitPropertiesChanged({"Connectivity"});
        settle();
        check(service.state() == Connectivity::Limited && !service.openLogin(), "limited is not portal");
        enabled = false;
        state = 2;
        nm.emitPropertiesChanged({"ConnectivityCheckEnabled", "Connectivity"});
        settle();
        check(service.state() == Connectivity::Unknown && !service.openLogin(), "disabled checks are unknown");
        enabled = true;
        uri = "file:///tmp/test";
        service.refresh();
        settle();
        check(!service.openLogin(), "reject non-HTTP scheme");
        type = "802-3-ethernet";
        uri = "http://network-test.example/check";
        service.refresh();
        settle();
        check(!service.wireless() && !service.openLogin(), "do not attribute ethernet portal to wifi");
        primary.path = "/";
        service.refresh();
        settle();
        check(!service.wireless() && !service.openLogin(), "disconnected");
        // Owner loss must cancel outstanding replies, not resurrect a portal.
        primary.path = "/active/one";
        type = "802-11-wireless";
        service.refresh();
        server->releaseName("org.freedesktop.NetworkManager");
        settle();
        check(service.state() == Connectivity::Unknown && !service.wireless(), "owner loss");
        std::cout << "Connectivity tests passed\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
