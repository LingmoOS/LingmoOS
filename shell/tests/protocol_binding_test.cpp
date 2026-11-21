// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/clients/interface_binding_test.cc
//           chromium/src/components/exo/wayland/clients/test/client_version_test.cc
//           chromium/src/components/exo/wayland/clients/security_delegate_binding_test.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: binds the globals ProtocolGlobals registers, over
// socketpair clients, and checks the registry filter for the restricted ones.

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include <wayland-client.h>

#include "protocol_interfaces.h"
#include "temporary_directory.h"

#include "compositor/compositor.h"
#include "compositor/output.h"
#include "core/preferences.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "wayland/privileged_client.h"
#include "wayland/protocol_globals.h"
#include "wayland/protocols/wl_output.h"
#include "wayland/security_delegate.h"
#include "wayland/server.h"
#include "wm/window_manager.h"

using namespace Lingmo;
using Lingmo::testing::knownInterfaces;

namespace {

// No executable is allowlisted: every restricted global needs the trusted shell,
// except what the one computer-use client, if any, may see.
class DenyingSecurityDelegate final : public wayland::SecurityDelegate
{
public:
    bool canManageClipboard(const wl_client *) const override { return false; }

    bool canCaptureOutputs(const wl_client *) const override { return false; }

    bool canControlComputer(const wl_client *client) const override
    {
        return client && client == computerUse;
    }

    bool showsSystemDialogs(int64_t) const override { return false; }

    const wl_client *computerUse = nullptr;
};

struct Global {
    uint32_t name = 0;
    uint32_t version = 0;
};

struct Client {
    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    std::map<std::string, Global> globals;
    std::vector<wl_proxy *> bound;

    ~Client()
    {
        // Local destruction only, so a connection that already failed is fine
        // and the server sees each object go with the client.
        for (wl_proxy *proxy : bound)
            wl_proxy_destroy(proxy);
        if (registry)
            wl_registry_destroy(registry);
        if (display)
            wl_display_disconnect(display);
    }
};

class ProtocolBindingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_home.isValid());
        for (const char *variable : {"XDG_DATA_HOME", "XDG_RUNTIME_DIR"})
            setenv(variable, m_home.path().c_str(), 1);
        Lingmo::setConfigHomeForTesting(m_home.path());

        m_scene.setSize(Lingmo::base::Size(1280, 800));
        m_manager.setBounds(Lingmo::base::Size(1280, 800));
        m_windowManager.setOutputBounds(Lingmo::base::Rect(0, 0, 1280, 800));
        m_output.pixelSize = Lingmo::base::Size(1280, 800);
        m_output.refreshMilliHz = 60000;

        wayland::ProtocolGlobals::Dependencies dependencies;
        dependencies.server = &m_server;
        dependencies.compositor = &m_compositor;
        dependencies.seat = &m_seat;
        dependencies.inputManager = &m_manager;
        dependencies.windowManager = &m_windowManager;
        dependencies.inputMethodPopupLayer = m_popups;
        dependencies.security = &m_security;
        m_globals = std::make_unique<wayland::ProtocolGlobals>(dependencies);
        ASSERT_TRUE(m_globals->initialize());
        m_outputGlobal = wayland::createOutputGlobal(m_server.display(), &m_output);
        ASSERT_TRUE(m_outputGlobal);
    }

    void TearDown() override
    {
        wl_display_destroy_clients(m_server.display());
        EXPECT_TRUE(m_compositor.surfaces().empty());
        EXPECT_TRUE(m_windowManager.windows().empty());
        if (m_outputGlobal)
            wayland::removeOutputGlobal(m_outputGlobal);
        if (m_globals)
            m_globals->shutdown();
    }

    std::unique_ptr<Client> connect(bool trusted = false, bool computerUse = false)
    {
        int fds[2];
        EXPECT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds));
        wl_client *serverClient = trusted ? m_globals->privileged()->adoptShellConnection(fds[0])
                                          : wl_client_create(m_server.display(), fds[0]);
        EXPECT_NE(nullptr, serverClient);
        if (computerUse)
            m_security.computerUse = serverClient;
        return connectTo(fds[1]);
    }

    // A client on `fd`, the other end of a connection the compositor made.
    std::unique_ptr<Client> connectTo(int fd)
    {
        auto client = std::make_unique<Client>();
        client->display = wl_display_connect_to_fd(fd);
        EXPECT_NE(nullptr, client->display);
        client->registry = wl_display_get_registry(client->display);
        static const wl_registry_listener listener = {
                [](void *data, wl_registry *, uint32_t name, const char *interface,
                   uint32_t version) {
                    static_cast<Client *>(data)->globals[interface] = {name, version};
                },
                [](void *, wl_registry *, uint32_t) {}};
        wl_registry_add_listener(client->registry, &listener, client.get());
        EXPECT_GE(roundtrip(*client), 0);
        return client;
    }

    // The client roundtrips on a worker while this thread serves it.
    int roundtrip(Client &client)
    {
        std::atomic<bool> done = false;
        int result = -1;
        std::thread worker([&] {
            result = wl_display_roundtrip(client.display);
            done = true;
        });
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!done && std::chrono::steady_clock::now() < deadline) {
            m_server.dispatch(std::chrono::milliseconds(1));
            m_server.flush();
        }
        if (!done) {
            ADD_FAILURE() << "roundtrip timed out";
            shutdown(wl_display_get_fd(client.display), SHUT_RDWR);
        }
        worker.join();
        return result;
    }

    // Drops disconnected clients, so each check starts from a quiet server.
    void settle()
    {
        for (int i = 0; i < 10; ++i)
            m_server.dispatch(std::chrono::milliseconds(0));
    }

    // Binds `interface` at `version` on a fresh connection; true when the
    // compositor accepted it and answered a roundtrip.
    bool bindsCleanly(const std::string &interface, uint32_t version, bool trusted)
    {
        auto client = connect(trusted);
        const auto global = client->globals.find(interface);
        EXPECT_NE(client->globals.end(), global) << interface;
        if (global == client->globals.end())
            return false;
        client->bound.push_back(static_cast<wl_proxy *>(wl_registry_bind(
                client->registry, global->second.name, knownInterfaces().at(interface), version)));
        const bool ok = roundtrip(*client) >= 0 && wl_display_get_error(client->display) == 0;
        client.reset();
        settle();
        return ok;
    }

    void checkVersionSkew(const std::map<std::string, Global> &globals, bool trusted)
    {
        for (const auto &[interface, global] : globals) {
            SCOPED_TRACE(interface);
            EXPECT_FALSE(bindsCleanly(interface, 0, trusted));
            for (uint32_t version = 1; version <= global.version; ++version)
                EXPECT_TRUE(bindsCleanly(interface, version, trusted)) << "version " << version;
            EXPECT_FALSE(bindsCleanly(interface, global.version + 1, trusted));
        }
        // Still serving after all of that.
        auto client = connect(trusted);
        for (const auto &[interface, global] : globals)
            EXPECT_TRUE(client->globals.contains(interface)) << interface;
    }

    test::TemporaryDirectory m_home;
    scene::Scene m_scene;
    scene::Node *m_surfaces =
            m_scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::Node *m_popups =
            m_scene.root()->addChild(std::make_unique<scene::Node>("popups"));
    compositor::Compositor m_compositor {m_surfaces};
    input::Seat m_seat {&m_compositor};
    input::InputManager m_manager {&m_seat};
    wm::WindowManager m_windowManager {&m_compositor, &m_seat};
    DenyingSecurityDelegate m_security;
    compositor::Output m_output;
    wayland::Server m_server;
    std::unique_ptr<wayland::ProtocolGlobals> m_globals;
    wl_global *m_outputGlobal = nullptr;
};

TEST_F(ProtocolBindingTest, AdvertisedGlobalsAreKnown)
{
    for (bool trusted : {false, true}) {
        auto client = connect(trusted);
        EXPECT_FALSE(client->globals.empty());
        for (const auto &[interface, global] : client->globals) {
            EXPECT_TRUE(knownInterfaces().contains(interface)) << interface;
            EXPECT_GE(global.version, 1u) << interface;
        }
    }
}

TEST_F(ProtocolBindingTest, OrdinaryClientsDoNotSeeRestrictedGlobals)
{
    auto ordinary = connect();
    auto trusted = connect(true);
    for (const char *restricted : {
                 "lingmo_shell_v1",
                 "ext_data_control_manager_v1",
                 "zwlr_data_control_manager_v1",
                 "ext_output_image_capture_source_manager_v1",
                 "ext_foreign_toplevel_image_capture_source_manager_v1",
                 "ext_image_copy_capture_manager_v1",
                 "ext_foreign_toplevel_list_v1",
         }) {
        EXPECT_FALSE(ordinary->globals.contains(restricted)) << restricted;
        EXPECT_TRUE(trusted->globals.contains(restricted)) << restricted;
    }
    // Only the input method the compositor spawned may drive text input.
    for (const char *inputMethod : {"zwp_input_method_manager_v2", "zwp_virtual_keyboard_manager_v1"}) {
        EXPECT_FALSE(ordinary->globals.contains(inputMethod)) << inputMethod;
        EXPECT_FALSE(trusted->globals.contains(inputMethod)) << inputMethod;
    }
    // What the ordinary client sees, the shell sees too.
    for (const auto &[interface, global] : ordinary->globals)
        EXPECT_TRUE(trusted->globals.contains(interface)) << interface;
}

// The computer-use service binds output capture at startup, before the user
// lets it capture anything, and never learns the window list.
TEST_F(ProtocolBindingTest, TheComputerUseServiceBindsOutputCaptureButSeesNoWindows)
{
    auto client = connect(false, true);
    for (const char *hidden : {
                 "ext_foreign_toplevel_image_capture_source_manager_v1",
                 "ext_foreign_toplevel_list_v1",
         })
        EXPECT_FALSE(client->globals.contains(hidden)) << hidden;
    for (const auto *interface : {
                 &ext_output_image_capture_source_manager_v1_interface,
                 &ext_image_copy_capture_manager_v1_interface,
         }) {
        const auto global = client->globals.find(interface->name);
        ASSERT_NE(client->globals.end(), global) << interface->name;
        client->bound.push_back(static_cast<wl_proxy *>(
                wl_registry_bind(client->registry, global->second.name, interface, 1)));
    }
    EXPECT_GE(roundtrip(*client), 0);
    EXPECT_EQ(0, wl_display_get_error(client->display));
}

// The recorder the shell starts captures on the connection the compositor made
// for it, and on nothing else: neither its executable nor the connection makes
// it the shell.
TEST_F(ProtocolBindingTest, ACaptureConnectionSeesTheCaptureGlobalsAlone)
{
    const int fd = m_globals->privileged()->createCaptureConnection();
    ASSERT_GE(fd, 0);
    auto recorder = connectTo(fd);
    auto ordinary = connect();
    for (const char *capture : {
                 "ext_output_image_capture_source_manager_v1",
                 "ext_foreign_toplevel_image_capture_source_manager_v1",
                 "ext_image_copy_capture_manager_v1",
                 "ext_foreign_toplevel_list_v1",
         }) {
        EXPECT_TRUE(recorder->globals.contains(capture)) << capture;
        EXPECT_FALSE(ordinary->globals.contains(capture)) << capture;
    }
    for (const char *hidden : {"lingmo_shell_v1", "ext_data_control_manager_v1", "zwlr_data_control_manager_v1"})
        EXPECT_FALSE(recorder->globals.contains(hidden)) << hidden;
    const auto global = recorder->globals.find(ext_image_copy_capture_manager_v1_interface.name);
    ASSERT_NE(recorder->globals.end(), global);
    recorder->bound.push_back(static_cast<wl_proxy *>(wl_registry_bind(
            recorder->registry, global->second.name, &ext_image_copy_capture_manager_v1_interface, 1)));
    EXPECT_GE(roundtrip(*recorder), 0);
    EXPECT_EQ(0, wl_display_get_error(recorder->display));
}

// Exo's security binding checks: a hidden global stays out of reach even when
// its name is guessed.
TEST_F(ProtocolBindingTest, AGuessedRestrictedGlobalCannotBeBound)
{
    const auto trusted = connect(true)->globals;
    const auto ordinary = connect()->globals;
    settle();
    for (const auto &[interface, global] : trusted) {
        if (ordinary.contains(interface))
            continue;
        SCOPED_TRACE(interface);
        auto client = connect();
        client->bound.push_back(static_cast<wl_proxy *>(wl_registry_bind(
                client->registry, global.name, knownInterfaces().at(interface), 1)));
        EXPECT_LT(roundtrip(*client), 0);
        EXPECT_EQ(EPROTO, wl_display_get_error(client->display));
        client.reset();
        settle();
    }
}

// Exo's InterfaceBinding: version 0 and one past the advertised version are
// refused, every version from 1 to the advertised one binds and answers.
TEST_F(ProtocolBindingTest, InterfaceBindingVersionSkew)
{
    const auto globals = connect()->globals;
    settle();
    checkVersionSkew(globals, false);
}

TEST_F(ProtocolBindingTest, TrustedInterfaceBindingVersionSkew)
{
    const auto ordinary = connect()->globals;
    const auto trusted = connect(true)->globals;
    std::map<std::string, Global> restricted;
    for (const auto &[interface, global] : trusted) {
        if (!ordinary.contains(interface))
            restricted.emplace(interface, global);
    }
    settle();
    EXPECT_FALSE(restricted.empty());
    checkVersionSkew(restricted, true);
}

// Everything bound at once, then the connection dropped without a single
// destructor request: the server unwinds every resource on its own.
TEST_F(ProtocolBindingTest, BindingEverythingAndDisconnecting)
{
    for (bool trusted : {false, true}) {
        for (int round = 0; round < 3; ++round) {
            auto client = connect(trusted);
            for (const auto &[interface, global] : client->globals) {
                client->bound.push_back(static_cast<wl_proxy *>(
                        wl_registry_bind(client->registry, global.name,
                                         knownInterfaces().at(interface), global.version)));
            }
            EXPECT_GE(roundtrip(*client), 0);
            EXPECT_EQ(0, wl_display_get_error(client->display));
            client.reset();
            settle();
        }
    }
    EXPECT_EQ(0, m_server.trackedClientCount());
}

} // namespace
