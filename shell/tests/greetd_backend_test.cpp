// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "auth/auth_controller.h"
#include "auth/greetd_backend.h"
#include "base/json.h"
#include "loop_test_util.h"
#include "temporary_directory.h"

#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <gtest/gtest.h>

using namespace Lingmo;
using namespace Lingmo::Auth;
using base::json::Value;

TEST(GreetdBackendTest, CancellationAcknowledgementCannotAuthenticate)
{
    test::TemporaryDirectory directory;
    const std::string path = directory.filePath("greetd");
    const int server = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    sockaddr_un address {};
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path.c_str());
    ASSERT_EQ(bind(server, reinterpret_cast<const sockaddr *>(&address), sizeof(address)), 0);
    ASSERT_EQ(listen(server, 1), 0);
    setenv("GREETD_SOCK", path.c_str(), 1);

    GreetdBackend backend;
    AuthController controller(AuthController::Login, &backend);
    int successes = 0;
    auto succeeded = controller.authenticationSucceeded.connect([&] { ++successes; });
    controller.setUserName("test-user");
    controller.setSessions({{"gnome", "GNOME", {"gnome-session"}, {"XDG_SESSION_TYPE=wayland"}}});
    backend.start();
    int peer = -1;
    ASSERT_TRUE(test::runUntil([&] {
        peer = accept4(server, nullptr, nullptr, SOCK_CLOEXEC | SOCK_NONBLOCK);
        return peer >= 0;
    }));

    const auto send = [&](const Value &message) {
        const std::string data = message.serialize();
        const uint32_t length = uint32_t(data.size());
        ASSERT_EQ(write(peer, &length, 4), 4);
        ASSERT_EQ(write(peer, data.data(), data.size()), ssize_t(data.size()));
    };
    const auto request = [&]() -> Value {
        std::string data;
        Value message;
        test::runUntil([&] {
            char chunk[4096];
            const ssize_t received = read(peer, chunk, sizeof(chunk));
            if (received > 0)
                data.append(chunk, size_t(received));
            if (data.size() < 4)
                return false;
            uint32_t length;
            memcpy(&length, data.data(), 4);
            if (data.size() != 4 + length)
                return false;
            message = Value::parse(std::string_view(data).substr(4)).value_or(Value());
            return true;
        });
        return message;
    };

    controller.authenticate("wrong");
    EXPECT_EQ(request()["type"].toString(), "create_session");
    send(Value::object({{"type", "error"}, {"error_type", "auth_error"}}));
    EXPECT_TRUE(test::runUntil([&] { return controller.failed(); }));
    controller.reset();
    EXPECT_EQ(request()["type"].toString(), "cancel_session");
    controller.authenticate("right");
    send(Value::object({{"type", "success"}}));
    EXPECT_EQ(request()["type"].toString(), "create_session");
    EXPECT_EQ(successes, 0);
    send(Value::object({{"type", "auth_message"}, {"auth_message_type", "secret"}}));
    const Value response = request();
    EXPECT_EQ(response["type"].toString(), "post_auth_message_response");
    EXPECT_EQ(response["response"].toString(), "right");
    send(Value::object({{"type", "success"}}));
    const Value start = request();
    EXPECT_EQ(start["type"].toString(), "start_session");
    // lingmo-dm starts the session by id; greetd runs the command.
    EXPECT_EQ(start["session"].toString(), "gnome");
    EXPECT_EQ(start["cmd"][size_t(0)].toString(), "gnome-session");
    EXPECT_EQ(start["env"][size_t(0)].toString(), "XDG_SESSION_TYPE=wayland");
    // The members in sorted order.
    EXPECT_EQ(start.serialize(),
              "{\"cmd\":[\"gnome-session\"],\"env\":[\"XDG_SESSION_TYPE=wayland\"],"
              "\"session\":\"gnome\",\"type\":\"start_session\"}");
    EXPECT_EQ(successes, 0);
    send(Value::object({{"type", "error"}}));
    EXPECT_TRUE(test::runUntil([&] { return controller.failed(); }));
    EXPECT_EQ(successes, 0);
    unsetenv("GREETD_SOCK");
    close(peer);
    close(server);
}
