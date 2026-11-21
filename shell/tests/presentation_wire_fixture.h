// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wp_presentation.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// What is taken: the protocol lifecycle. The wire assertions are Lingmo's.

#pragma once

#include <atomic>
#include <chrono>
#include <cstring>
#include <gtest/gtest.h>
#include <presentation-time-client-protocol.h>
#include <sys/socket.h>
#include <thread>
#include <wayland-client.h>
#include <wayland-server-core.h>

#include "backend/drm/presentation_tracker.h"
#include "compositor/compositor.h"
#include "compositor/output.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "shm_test_util.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_output.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/wp_presentation.h"

using namespace Lingmo;

namespace {

struct Feedback {
    struct wp_presentation_feedback* proxy = nullptr;
    std::vector<wl_output*> outputs;
    std::vector<char> events;
    uint32_t secondsHi = 0, secondsLo = 0, ns = 0, refresh = 0;
    uint32_t seqHi = 0, seqLo = 0, flags = 0;
};

class PresentationProtocolTest : public ::testing::Test
{
protected:
    scene::Scene scene;
    compositor::Compositor compositor {scene.root()};
    testsupport::FakeRenderer renderer;
    compositor::Output output;
    backend::PresentationTracker tracker;
    wl_display* server = nullptr;
    wl_display* client = nullptr;
    wl_client* serverClient = nullptr;
    wl_registry* registry = nullptr;
    wl_compositor* factory = nullptr;
    wp_presentation* manager = nullptr;
    wl_shm* shm = nullptr;
    wl_surface* surface = nullptr;
    wl_buffer* buffer = nullptr;
    uint32_t outputName = 0, version = 0, clockId = UINT32_MAX;
    std::vector<wl_output*> outputs;
    std::vector<std::unique_ptr<Feedback>> feedbacks;

    int roundtrip()
    {
        std::atomic<bool> done = false;
        int result = -1;
        std::thread worker([&] {
            result = wl_display_roundtrip(client);
            done = true;
        });
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (!done && std::chrono::steady_clock::now() < deadline) {
            wl_event_loop_dispatch(wl_display_get_event_loop(server), 1);
            wl_display_flush_clients(server);
        }
        if (!done)
            shutdown(wl_display_get_fd(client), SHUT_RDWR);
        worker.join();
        return result;
    }

    void SetUp() override
    {
        server = wl_display_create();
        ASSERT_TRUE(server);
        ASSERT_TRUE(wayland::createCompositorGlobal(server, &compositor));
        ASSERT_TRUE(wayland::createShmGlobal(server, &compositor));
        ASSERT_TRUE(wayland::createPresentationGlobal(server));
        output.pixelSize = {1920, 1080};
        output.refreshMilliHz = 75000;
        ASSERT_TRUE(wayland::createOutputGlobal(server, &output));
        scene.setSize(output.logicalSize());
        int sockets[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets), 0);
        serverClient = wl_client_create(server, sockets[0]);
        ASSERT_TRUE(serverClient);
        client = wl_display_connect_to_fd(sockets[1]);
        ASSERT_TRUE(client);
        registry = wl_display_get_registry(client);
        static const wl_registry_listener listener = {
            [](void* data, wl_registry* r, uint32_t name, const char* interface, uint32_t version) {
                auto& self = *static_cast<PresentationProtocolTest*>(data);
                if (!strcmp(interface, "wl_compositor"))
                    self.factory = static_cast<wl_compositor*>(
                        wl_registry_bind(r, name, &wl_compositor_interface, 4));
                else if (!strcmp(interface, "wp_presentation")) {
                    self.version = version;
                    self.manager = static_cast<wp_presentation*>(
                        wl_registry_bind(r, name, &wp_presentation_interface, 1));
                    static const wp_presentation_listener clock = {
                        [](void* data, wp_presentation*, uint32_t id) {
                            static_cast<PresentationProtocolTest*>(data)->clockId = id;
                        }};
                    wp_presentation_add_listener(self.manager, &clock, &self);
                } else if (!strcmp(interface, "wl_shm"))
                    self.shm =
                        static_cast<wl_shm*>(wl_registry_bind(r, name, &wl_shm_interface, 1));
                else if (!strcmp(interface, "wl_output"))
                    self.outputName = name;
            },
            [](void*, wl_registry*, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
        ASSERT_GE(roundtrip(), 0);
        ASSERT_TRUE(factory);
        ASSERT_TRUE(manager);
        ASSERT_TRUE(shm);
        surface = wl_compositor_create_surface(factory);
        int fd = testsupport::makeMemfd(4096);
        ASSERT_GE(fd, 0);
        auto* pool = wl_shm_create_pool(shm, fd, 4096);
        close(fd);
        buffer = wl_shm_pool_create_buffer(pool, 0, 16, 16, 64, WL_SHM_FORMAT_XRGB8888);
        wl_shm_pool_destroy(pool);
        wl_surface_attach(surface, buffer, 0, 0);
        ASSERT_GE(roundtrip(), 0);
        compositor.surfaces().front()->setPosition({0, 0});
    }

    Feedback& request()
    {
        auto f = std::make_unique<Feedback>();
        f->proxy = wp_presentation_feedback(manager, surface);
        static const wp_presentation_feedback_listener listener = {
            [](void* data, struct wp_presentation_feedback*, wl_output* output) {
                auto& f = *static_cast<Feedback*>(data);
                f.outputs.push_back(output);
                f.events.push_back('o');
            },
            [](void* data, struct wp_presentation_feedback* proxy, uint32_t hi, uint32_t lo,
               uint32_t ns, uint32_t refresh, uint32_t seqHi, uint32_t seqLo, uint32_t flags) {
                auto& f = *static_cast<Feedback*>(data);
                f.secondsHi = hi;
                f.secondsLo = lo;
                f.ns = ns;
                f.refresh = refresh;
                f.seqHi = seqHi;
                f.seqLo = seqLo;
                f.flags = flags;
                f.events.push_back('p');
                wp_presentation_feedback_destroy(proxy);
                f.proxy = nullptr;
            },
            [](void* data, struct wp_presentation_feedback* proxy) {
                auto& f = *static_cast<Feedback*>(data);
                f.events.push_back('d');
                wp_presentation_feedback_destroy(proxy);
                f.proxy = nullptr;
            }};
        wp_presentation_feedback_add_listener(f->proxy, &listener, f.get());
        feedbacks.push_back(std::move(f));
        return *feedbacks.back();
    }

    wl_output* bindOutput()
    {
        auto* result = static_cast<wl_output*>(
            wl_registry_bind(registry, outputName, &wl_output_interface, 3));
        outputs.push_back(result);
        return result;
    }

    void submit(uint64_t token)
    {
        compositor.updateTextures(renderer);
        render::PresentationSubmission submission;
        renderer.presentationSubmission = &submission;
        scene.render(renderer);
        renderer.presentationSubmission = nullptr;
        tracker.submitted(token, std::move(submission));
    }

    void complete(uint64_t token, uint32_t seq = 42)
    {
        ASSERT_TRUE(tracker.complete(token, 123, 456789, seq, true, 75, output.presentation));
        ASSERT_GE(roundtrip(), 0);
    }

    size_t serverFeedbackCount()
    {
        size_t count = 0;
        wl_client_for_each_resource(
            serverClient,
            [](wl_resource* r, void* data) {
                if (!strcmp(wl_resource_get_class(r), "wp_presentation_feedback"))
                    ++*static_cast<size_t*>(data);
                return WL_ITERATOR_CONTINUE;
            },
            &count);
        return count;
    }

    void TearDown() override
    {
        for (auto& f : feedbacks)
            if (f->proxy)
                wp_presentation_feedback_destroy(f->proxy);
        if (surface)
            wl_surface_destroy(surface);
        if (buffer)
            wl_buffer_destroy(buffer);
        for (auto* o : outputs)
            wl_output_release(o);
        if (manager)
            wp_presentation_destroy(manager);
        if (shm)
            wl_shm_destroy(shm);
        if (factory)
            wl_compositor_destroy(factory);
        if (registry)
            wl_registry_destroy(registry);
        if (client)
            wl_display_disconnect(client);
        if (server) {
            wl_display_destroy_clients(server);
            wl_display_destroy(server);
        }
    }
};

} // namespace
