// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/stat.h>
#include <unistd.h>

#include <chrono>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

#include "base/process.h"

#include "clipboard_test_util.h"
#include "ext-data-control-v1-client-protocol.h"
#include "temporary_directory.h"
#include "wayland/privileged_client.h"
#include "wayland/protocols/data_control.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/wl_subcompositor.h"
#include "wayland/protocols/xdg_decoration.h"
#include "wayland/protocols/xdg_shell.h"
#include "wlr-data-control-unstable-v1-client-protocol.h"
#include "wm/window_manager.h"

using namespace clipboard_test;

namespace {

class Control
{
public:
    Harness& h;
    Client& c;
    bool ext;
    wl_registry* registry;
    wl_proxy* manager = nullptr;
    wl_proxy* device = nullptr;
    wl_proxy* selection = nullptr;
    wl_proxy* primary = nullptr;
    std::vector<wl_proxy*> offers;
    std::vector<wl_proxy*> sources;
    std::vector<std::string> events;

    uint32_t wlrVersion;

    Control(Harness& h, Client& c, bool ext, uint32_t wlrVersion = 2)
        : h(h)
        , c(c)
        , ext(ext)
        , registry(wl_display_get_registry(c.display))
        , wlrVersion(wlrVersion)
    {
        static const wl_registry_listener listener = {
            [](void* data, wl_registry* registry, uint32_t name, const char* interface,
               uint32_t version) {
                auto* self = static_cast<Control*>(data);
                const wl_interface* type = self->ext ? &ext_data_control_manager_v1_interface
                                                     : &zwlr_data_control_manager_v1_interface;
                if (std::string(interface) == type->name)
                    self->manager = static_cast<wl_proxy*>(wl_registry_bind(
                        registry, name, type, std::min(version, self->ext ? 1u : self->wlrVersion)));
            },
            [](void*, wl_registry*, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
        h.sync(c);
        if (!manager)
            return;
        device = wl_proxy_marshal_flags(manager, 1,
                                        ext ? &ext_data_control_device_v1_interface
                                            : &zwlr_data_control_device_v1_interface,
                                        wl_proxy_get_version(manager), 0, nullptr, c.seat);
        static const ext_data_control_offer_v1_listener offerListener = {
            [](void* data, ext_data_control_offer_v1*, const char*) {
                static_cast<Control*>(data)->events.push_back("mime");
            }};
        static const ext_data_control_device_v1_listener listener2 = {
            [](void* data, ext_data_control_device_v1*, ext_data_control_offer_v1* offer) {
                auto* self = static_cast<Control*>(data);
                auto* proxy = reinterpret_cast<wl_proxy*>(offer);
                self->offers.push_back(proxy);
                self->events.push_back("offer");
                wl_proxy_add_listener(
                    proxy,
                    reinterpret_cast<void (**)(void)>(
                        const_cast<ext_data_control_offer_v1_listener*>(&offerListener)),
                    data);
            },
            [](void* data, ext_data_control_device_v1*, ext_data_control_offer_v1* offer) {
                auto* self = static_cast<Control*>(data);
                self->selection = reinterpret_cast<wl_proxy*>(offer);
                self->events.push_back(offer ? "selection" : "null");
            },
            [](void*, ext_data_control_device_v1*) {},
            [](void* data, ext_data_control_device_v1*, ext_data_control_offer_v1* offer) {
                auto* self = static_cast<Control*>(data);
                self->primary = reinterpret_cast<wl_proxy*>(offer);
                self->events.push_back(offer ? "primary" : "primary-null");
            }};
        wl_proxy_add_listener(device,
                              reinterpret_cast<void (**)(void)>(
                                  const_cast<ext_data_control_device_v1_listener*>(&listener2)),
                              this);
        h.sync(c);
    }

    ~Control()
    {
        for (auto* offer : offers)
            wl_proxy_marshal_flags(offer, 1, nullptr, 1, WL_MARSHAL_FLAG_DESTROY);
        for (auto* source : sources)
            wl_proxy_marshal_flags(source, 1, nullptr, 1, WL_MARSHAL_FLAG_DESTROY);
        if (device)
            wl_proxy_marshal_flags(device, 1, nullptr, wl_proxy_get_version(device),
                                   WL_MARSHAL_FLAG_DESTROY);
        if (manager)
            wl_proxy_marshal_flags(manager, 2, nullptr, wl_proxy_get_version(manager),
                                   WL_MARSHAL_FLAG_DESTROY);
        wl_registry_destroy(registry);
    }

    std::string receive(wl_proxy* offer, const char* mime)
    {
        int fds[2];
        EXPECT_EQ(0, pipe2(fds, O_CLOEXEC | O_NONBLOCK));
        wl_proxy_marshal_flags(offer, 0, nullptr, 1, 0, mime, fds[1]);
        close(fds[1]);
        std::string bytes;
        EXPECT_TRUE(h.until([&] {
            char buffer[4096];
            const auto n = read(fds[0], buffer, sizeof(buffer));
            if (n > 0)
                bytes.append(buffer, n);
            return n == 0;
        }));
        close(fds[0]);
        return bytes;
    }

    wl_proxy* publish(const std::map<std::string, std::string>& bytes, bool primary = false)
    {
        c.payloads = bytes;
        auto* source = wl_proxy_marshal_flags(manager, 0,
                                              ext ? &ext_data_control_source_v1_interface
                                                  : &zwlr_data_control_source_v1_interface,
                                              1, 0, nullptr);
        sources.push_back(source);
        static const ext_data_control_source_v1_listener listener = {
            [](void* data, ext_data_control_source_v1*, const char* mime, int fd) {
                auto* c = static_cast<Client*>(data);
                fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
                c->writes.push_back({fd, c->payloads[mime]});
                ++c->sends;
            },
            [](void* data, ext_data_control_source_v1*) {
                ++static_cast<Client*>(data)->cancelled;
            }};
        wl_proxy_add_listener(source,
                              reinterpret_cast<void (**)(void)>(
                                  const_cast<ext_data_control_source_v1_listener*>(&listener)),
                              &c);
        for (auto& [mime, data] : bytes)
            wl_proxy_marshal_flags(source, 0, nullptr, 1, 0, mime.c_str());
        wl_proxy_marshal_flags(device, primary ? 2 : 0, nullptr, wl_proxy_get_version(device), 0,
                               source);
        h.sync(c);
        return source;
    }
};

} // namespace

TEST(DataControl, BothProtocolsShareSelectionWithoutFocusAndInvalidateOldOffers)
{
    for (bool ext : {true, false}) {
        Harness h;
        wayland::DataControlManager manager(h.server.display(), &h.waylandSeat,
                                            [](const wl_client*) {
                                                return true;
                                            });
        auto& a = h.addClient();
        auto& b = h.addClient();
        Control control(h, b, ext);
        ASSERT_NE(nullptr, control.manager);
        EXPECT_EQ((std::vector<std::string> {"null", "primary-null"}), control.events);
        h.publish(a, {{"text/plain", "original"}});
        h.sync(b);
        ASSERT_NE(nullptr, control.selection);
        auto* old = control.selection;
        wl_data_source_offer(a.sources.back(), "application/x-late");
        h.sync(a);
        EXPECT_EQ("", control.receive(old, "application/x-late"));
        h.seat.setFocusedSurface(nullptr);
        EXPECT_EQ("original", control.receive(old, "text/plain"));
        control.publish({{"text/plain", "persisted"}, {"image/png", std::string("PNG\0bytes", 9)}});
        EXPECT_EQ("", control.receive(old, "text/plain"));
        h.focus(a);
        EXPECT_EQ("persisted", h.receive(a, a.selection, "text/plain"));
        EXPECT_EQ(std::string("PNG\0bytes", 9), h.receive(a, a.selection, "image/png"));
        wl_proxy_marshal_flags(control.device, 0, nullptr, wl_proxy_get_version(control.device), 0,
                               nullptr);
        h.sync(b);
        h.sync(a);
        EXPECT_FALSE(h.seat.selection()->source());
        EXPECT_EQ(nullptr, a.selection);
        EXPECT_EQ(1u, b.cancelled);
        wl_display_destroy_clients(h.server.display());
    }
}

TEST(DataControl, PrimarySelectionIsSetAndOfferedSeparatelyFromTheClipboard)
{
    for (bool ext : {true, false}) {
        Harness h;
        wayland::DataControlManager manager(h.server.display(), &h.waylandSeat,
                                            [](const wl_client*) {
                                                return true;
                                            });
        auto& a = h.addClient();
        auto& b = h.addClient();
        Control setter(h, a, ext);
        Control reader(h, b, ext);
        ASSERT_NE(nullptr, reader.manager);
        reader.events.clear();
        setter.publish({{"text/plain", "highlighted"}}, true);
        h.sync(b);
        EXPECT_TRUE(h.seat.primarySelection()->source());
        EXPECT_FALSE(h.seat.selection()->source());
        EXPECT_EQ((std::vector<std::string> {"offer", "mime", "primary"}), reader.events);
        EXPECT_EQ(nullptr, reader.selection);
        ASSERT_NE(nullptr, reader.primary);
        EXPECT_EQ("highlighted", reader.receive(reader.primary, "text/plain"));
        wl_proxy_marshal_flags(setter.device, 2, nullptr, wl_proxy_get_version(setter.device), 0,
                               nullptr);
        h.sync(a);
        h.sync(b);
        EXPECT_FALSE(h.seat.primarySelection()->source());
        EXPECT_EQ(nullptr, reader.primary);
        EXPECT_EQ(1u, a.cancelled);
        wl_display_destroy_clients(h.server.display());
    }
}

TEST(DataControl, WlrVersionOneDeviceGetsNoPrimarySelection)
{
    Harness h;
    wayland::DataControlManager manager(h.server.display(), &h.waylandSeat,
                                        [](const wl_client*) {
                                            return true;
                                        });
    auto& a = h.addClient();
    auto& b = h.addClient();
    Control setter(h, a, false);
    Control reader(h, b, false, 1);
    ASSERT_NE(nullptr, reader.manager);
    setter.publish({{"text/plain", "highlighted"}}, true);
    h.sync(b);
    EXPECT_TRUE(h.seat.primarySelection()->source());
    EXPECT_EQ((std::vector<std::string> {"null"}), reader.events);
    wl_display_destroy_clients(h.server.display());
}

TEST(DataControl, RegistryFilterSeparatesClipboardPrivilegeFromShellPrivilege)
{
    Harness h;
    wayland::PrivilegedClients privileged(h.server.display());
    wl_client* authorized = nullptr;
    const auto allow = [&](const wl_client* client) {
        return client == authorized;
    };
    wayland::DataControlManager manager(h.server.display(), &h.waylandSeat, allow);
    privileged.addRestrictedGlobal(manager.extGlobal(), allow);
    privileged.addRestrictedGlobal(manager.wlrGlobal(), allow);
    auto& a = h.addClient();
    Control denied(h, a, true);
    EXPECT_EQ(nullptr, denied.manager);
    auto& b = h.addClient();
    authorized = b.serverClient;
    Control allowed(h, b, true);
    EXPECT_NE(nullptr, allowed.manager);
    EXPECT_FALSE(privileged.isTrusted(b.serverClient));
    wl_display_destroy_clients(h.server.display());
}

TEST(DataControl, UsedSourceAndLateMimeAreProtocolErrors)
{
    for (bool ext : {true, false}) {
        for (bool lateMime : {true, false}) {
            Harness h;
            wayland::DataControlManager manager(h.server.display(), &h.waylandSeat,
                                                [](const wl_client*) {
                                                    return true;
                                                });
            auto& a = h.addClient();
            Control control(h, a, ext);
            auto* source = control.publish({{"text/plain", "one"}});
            a.expectError = true;
            if (lateMime)
                wl_proxy_marshal_flags(source, 0, nullptr, 1, 0, "text/html");
            else
                wl_proxy_marshal_flags(control.device, 0, nullptr,
                                       wl_proxy_get_version(control.device), 0, source);
            ASSERT_TRUE(h.until([&] {
                return wl_display_get_error(a.display) != 0;
            }));
            EXPECT_EQ(EPROTO, wl_display_get_error(a.display));
            const wl_interface* interface = nullptr;
            uint32_t id;
            EXPECT_EQ(1u, wl_display_get_protocol_error(a.display, &interface, &id));
            wl_display_destroy_clients(h.server.display());
        }
    }
}

TEST(DataControl, ClipboardServicePreservesBytesAfterSourceExitWithExtAndWlr)
{
    const char* helper = std::getenv("LINGMO_CLIPBOARD_TEST_HELPER");
    if (!helper || !*helper)
        GTEST_SKIP() << "Set LINGMO_CLIPBOARD_TEST_HELPER to the standalone clipboard helper";
    for (bool ext : {true, false}) {
        Lingmo::test::TemporaryDirectory runtime;
        ASSERT_TRUE(runtime.isValid());
        chmod(runtime.path().c_str(), 0700);
        const char* runtimeVariable = std::getenv("XDG_RUNTIME_DIR");
        const std::string previousRuntime = runtimeVariable ? runtimeVariable : "";
        setenv("XDG_RUNTIME_DIR", runtime.path().c_str(), 1);
        Harness h;
        wayland::PrivilegedClients privileges(h.server.display());
        wayland::DataControlManager manager(h.server.display(), &h.waylandSeat,
                                            [](const wl_client*) {
                                                return true;
                                            });
        if (!ext)
            privileges.addRestrictedGlobal(manager.extGlobal(), [](const wl_client*) {
                return false;
            });
        wm::WindowManager wm(&h.compositor, &h.seat);
        wayland::WaylandXdgShell xdg(&h.compositor, &wm, &h.seat, h.server.serialTracker());
        wayland::createXdgShellGlobal(h.server.display(), &xdg);
        wayland::createXdgDecorationGlobal(h.server.display());
        wayland::createShmGlobal(h.server.display(), &h.compositor);
        wayland::createSubcompositorGlobal(h.server.display());
        ASSERT_TRUE(h.server.addSocket("clipboard-test"));
        auto& a = h.addClient();
        auto& b = h.addClient();
        const std::map<std::string, std::string> bytes {
            {"text/plain;charset=utf-8", "hello \xf0\x9f\x90\x9f"},
            {"image/png", std::string("\x89PNG\0\xff", 6)},
            {"application/x-test", std::string(262144, 'x')},
            {"text/uri-list", "file:///not-opened\r\n"},
            {"application/x-empty", ""}};
        h.publish(a, bytes);
        Lingmo::base::Process child;
        std::vector<std::string> env;
        for (char** entry = environ; *entry; ++entry) {
            const std::string_view variable(*entry);
            if (!variable.starts_with("QT_QPA_PLATFORM=") && !variable.starts_with("WAYLAND_DISPLAY=")
                && !variable.starts_with("WAYLAND_SOCKET="))
                env.emplace_back(variable);
        }
        env.emplace_back("QT_QPA_PLATFORM=wayland");
        env.emplace_back("WAYLAND_DISPLAY=clipboard-test");
        child.setEnvironment(env);
        ASSERT_TRUE(child.start(helper));
        EXPECT_TRUE(h.until(
            [&] {
                auto source = h.seat.selection()->source();
                return source && source->supports("application/x-lingmo-clipboard");
            },
            10000))
            << child.standardError();
        h.clients.erase(h.clients.begin());
        h.focus(b);
        ASSERT_NE(nullptr, b.selection);
        for (auto& [mime, data] : bytes)
            EXPECT_EQ(data, h.receive(b, b.selection, mime));
        const auto generation = h.seat.selection()->generation();
        for (int i = 0; i < 20; ++i)
            h.pump();
        EXPECT_EQ(generation, h.seat.selection()->generation());
        child.terminate();
        if (!child.waitForFinished(std::chrono::seconds(2))) {
            child.kill();
            child.waitForFinished(std::chrono::seconds(30));
        }
        wl_display_destroy_clients(h.server.display());
        setenv("XDG_RUNTIME_DIR", previousRuntime.c_str(), 1);
    }
}
