// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "presentation_wire_fixture.h"

namespace {

TEST_F(PresentationProtocolTest, ExactWireCompletionMultipleBindingsAndManagerDestruction)
{
    EXPECT_EQ(version, 1u);
    EXPECT_EQ(clockId, uint32_t(CLOCK_MONOTONIC));
    auto* first = bindOutput();
    auto& a = request();
    auto& b = request();
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(serverFeedbackCount(), 2u);
    submit(1);
    complete(1);
    EXPECT_TRUE(a.events.empty());
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    submit(2);
    wp_presentation_destroy(manager);
    manager = nullptr;
    auto* late = bindOutput();
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(a.events.empty());
    complete(2);
    for (const auto* f : {&a, &b}) {
        EXPECT_EQ(f->outputs, (std::vector<wl_output*> {first, late}));
        EXPECT_EQ(f->events, (std::vector<char> {'o', 'o', 'p'}));
        EXPECT_EQ(f->secondsHi, 0u);
        EXPECT_EQ(f->secondsLo, 123u);
        EXPECT_EQ(f->ns, 456789000u);
        EXPECT_EQ(f->refresh, 13333333u);
        EXPECT_EQ(f->seqHi, 0u);
        EXPECT_EQ(f->seqLo, 42u);
        EXPECT_EQ(f->flags, 7u);
    }
    EXPECT_EQ(serverFeedbackCount(), 0u);
    EXPECT_FALSE(tracker.complete(2, 555, 1, 999, true, 60, output.presentation));
}

TEST_F(PresentationProtocolTest, NoBindingAndReleasedBinding)
{
    auto* released = bindOutput();
    wl_output_release(released);
    outputs.clear();
    auto& a = request();
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    submit(1);
    complete(1);
    EXPECT_EQ(a.events, (std::vector<char> {'p'}));
    EXPECT_TRUE(a.outputs.empty());
}

TEST_F(PresentationProtocolTest, SeparateCommitsAndSupersededContent)
{
    auto& a = request();
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    submit(1);
    auto& b = request();
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    auto& c = request();
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(a.events.empty());
    EXPECT_EQ(b.events, (std::vector<char> {'d'}));
    complete(1);
    EXPECT_EQ(a.seqLo, 42u);
    EXPECT_TRUE(c.events.empty());
    submit(2);
    complete(2, 43);
    EXPECT_EQ(c.seqLo, 43u);
    EXPECT_EQ(serverFeedbackCount(), 0u);
}

TEST_F(PresentationProtocolTest, SurfaceDestructionDiscardsAndLateCompletionIsSafe)
{
    bindOutput();
    auto& a = request();
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    submit(1);
    wl_surface_destroy(surface);
    surface = nullptr;
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(a.events, (std::vector<char> {'d'}));
    EXPECT_TRUE(a.outputs.empty());
    complete(1);
    EXPECT_EQ(a.events.size(), 1u);
    EXPECT_EQ(serverFeedbackCount(), 0u);
}

TEST_F(PresentationProtocolTest, LocalProxyDestroyDoesNotCancelServerResource)
{
    auto& a = request();
    ASSERT_GE(roundtrip(), 0);
    wp_presentation_feedback_destroy(a.proxy);
    a.proxy = nullptr;
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(serverFeedbackCount(), 1u);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    submit(1);
    complete(1);
    EXPECT_EQ(serverFeedbackCount(), 0u);
}

TEST_F(PresentationProtocolTest, HighSecondsAndSequencePacking)
{
    auto& a = request();
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    auto content = compositor.surfaces().front()->node()->presentationContent();
    render::PresentationResult r;
    r.presented = true;
    r.seconds = (uint64_t(7) << 32) | 123;
    r.nanoseconds = 456789000;
    r.sequence = (uint64_t(9) << 32) | 42;
    r.output = output.presentation;
    content->complete(r);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(a.secondsHi, 7u);
    EXPECT_EQ(a.secondsLo, 123u);
    EXPECT_EQ(a.seqHi, 9u);
    EXPECT_EQ(a.seqLo, 42u);
}

TEST_F(PresentationProtocolTest, AnotherClientsOutputIsNeverSent)
{
    int sockets[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets), 0);
    auto* otherServer = wl_client_create(server, sockets[0]);
    ASSERT_TRUE(otherServer);
    auto* otherClient = wl_display_connect_to_fd(sockets[1]);
    ASSERT_TRUE(otherClient);
    auto* otherRegistry = wl_display_get_registry(otherClient);
    auto* otherOutput = static_cast<wl_output*>(
        wl_registry_bind(otherRegistry, outputName, &wl_output_interface, 3));
    std::swap(client, otherClient);
    ASSERT_GE(roundtrip(), 0);
    std::swap(client, otherClient);
    auto& a = request();
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    submit(1);
    complete(1);
    EXPECT_EQ(a.events, (std::vector<char> {'p'}));
    EXPECT_TRUE(a.outputs.empty());
    wl_output_release(otherOutput);
    wl_registry_destroy(otherRegistry);
    wl_display_disconnect(otherClient);
    wl_client_destroy(otherServer);
}

TEST_F(PresentationProtocolTest, ClientDisconnectCancelsResourcesBeforeLateCompletion)
{
    request();
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    submit(1);
    wl_client_destroy(serverClient);
    serverClient = nullptr;
    EXPECT_TRUE(compositor.surfaces().empty());
    EXPECT_TRUE(tracker.complete(1, 123, 456789, 42, true, 75, output.presentation));
}
} // namespace
