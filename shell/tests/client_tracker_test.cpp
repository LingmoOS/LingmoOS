// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Derived from chromium/src/components/exo/wayland/client_tracker_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "wayland/client_tracker.h"

#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wayland-server-protocol.h>

#include <algorithm>
#include <vector>

namespace Lingmo::Wayland {
namespace {

class ClientTrackerTest : public testing::Test
{
protected:
    struct ClientData {
        wl_client* client = nullptr;
        int fds[2] = {-1, -1};
    };

    void SetUp() override
    {
        m_display = wl_display_create();
        ASSERT_TRUE(m_display);
        m_tracker = std::make_unique<ClientTracker>(m_display);
    }

    void TearDown() override
    {
        while (!m_clients.empty())
            destroyClient(m_clients.back().client);
        m_tracker.reset();
        wl_display_destroy(m_display);
    }

    wl_client* createClient()
    {
        ClientData data;
        EXPECT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, data.fds));
        data.client = wl_client_create(m_display, data.fds[0]);
        m_clients.push_back(data);
        return m_clients.back().client;
    }

    void destroyClient(wl_client* client)
    {
        auto it = std::find_if(m_clients.begin(), m_clients.end(),
                               [&](const ClientData& data) { return data.client == client; });
        if (it == m_clients.end())
            return;

        const int peerFd = it->fds[1];
        m_clients.erase(it);
        wl_client_destroy(client);
        close(peerFd);
    }

    wl_display* m_display = nullptr;
    std::unique_ptr<ClientTracker> m_tracker;
    std::vector<ClientData> m_clients;
};

TEST_F(ClientTrackerTest, ClientRegistersAndDeregistersSuccessfully)
{
    EXPECT_EQ(0, m_tracker->trackedClientCount());

    wl_client* client1 = createClient();
    ASSERT_TRUE(client1);
    EXPECT_EQ(1, m_tracker->trackedClientCount());
    EXPECT_FALSE(m_tracker->isClientDestroyed(client1));

    wl_client* client2 = createClient();
    ASSERT_TRUE(client2);
    EXPECT_EQ(2, m_tracker->trackedClientCount());
    EXPECT_FALSE(m_tracker->isClientDestroyed(client2));

    destroyClient(client2);
    EXPECT_EQ(1, m_tracker->trackedClientCount());
    EXPECT_FALSE(m_tracker->isClientDestroyed(client1));

    destroyClient(client1);
    EXPECT_EQ(0, m_tracker->trackedClientCount());
}

// A client's resources are torn down after destruction has begun but before the
// wl_client itself is freed. The tracker must already report the client as
// destroyed at that point.
TEST_F(ClientTrackerTest, ClientTaggedDestroyedAfterDestructionStarted)
{
    wl_client* client = createClient();
    EXPECT_FALSE(m_tracker->isClientDestroyed(client));

    struct UserData {
        ClientTracker* tracker = nullptr;
        wl_client* client = nullptr;
        bool destructorRan = false;
    };
    UserData data { m_tracker.get(), client, false };

    wl_resource* outputResource = wl_resource_create(client, &wl_output_interface, 2, 0);
    ASSERT_TRUE(outputResource);
    wl_resource_set_user_data(outputResource, &data);
    wl_resource_set_destructor(outputResource, [](wl_resource* resource) {
        auto* data = static_cast<UserData*>(wl_resource_get_user_data(resource));
        data->destructorRan = true;
        EXPECT_TRUE(data->tracker->isClientDestroyed(data->client));
    });

    destroyClient(client);
    EXPECT_TRUE(data.destructorRan);
}

// Destroying the tracker while clients are still alive must not leave dangling
// listeners on their destroy signals.
TEST_F(ClientTrackerTest, TrackerOutlivedByItsClients)
{
    wl_client* client = createClient();
    EXPECT_EQ(1, m_tracker->trackedClientCount());

    m_tracker.reset();
    destroyClient(client);
}

} // namespace
} // namespace Lingmo::Wayland
