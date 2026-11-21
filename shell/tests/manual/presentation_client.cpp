// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/clients/simple.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// What is taken: the listener and request ordering. A standalone diagnostic
// client.
#include <cstdio>
#include <cstring>
#include <ctime>
#include <poll.h>
#include <presentation-time-client-protocol.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>
#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>

struct Client {
    wl_compositor* compositor = nullptr;
    wl_shm* shm = nullptr;
    xdg_wm_base* shell = nullptr;
    wp_presentation* presentation = nullptr;
    uint32_t clock = UINT32_MAX;
    bool configured = false, finished = false;
    std::vector<wl_output*> outputs;
};

int main()
{
    Client c;
    auto* display = wl_display_connect(nullptr);
    if (!display)
        return 1;
    auto* registry = wl_display_get_registry(display);
    static const wl_registry_listener globals = {
        [](void* data, wl_registry* r, uint32_t name, const char* interface, uint32_t) {
            auto& c = *static_cast<Client*>(data);
            if (!strcmp(interface, "wl_compositor"))
                c.compositor = static_cast<wl_compositor*>(
                    wl_registry_bind(r, name, &wl_compositor_interface, 4));
            else if (!strcmp(interface, "wl_shm"))
                c.shm = static_cast<wl_shm*>(wl_registry_bind(r, name, &wl_shm_interface, 1));
            else if (!strcmp(interface, "wp_presentation")) {
                c.presentation = static_cast<wp_presentation*>(
                    wl_registry_bind(r, name, &wp_presentation_interface, 1));
                static const wp_presentation_listener listener = {
                    [](void* data, wp_presentation*, uint32_t clock) {
                        static_cast<Client*>(data)->clock = clock;
                        std::printf("clock_id=%u\n", clock);
                    }};
                wp_presentation_add_listener(c.presentation, &listener, &c);
            } else if (!strcmp(interface, "xdg_wm_base")) {
                c.shell =
                    static_cast<xdg_wm_base*>(wl_registry_bind(r, name, &xdg_wm_base_interface, 1));
                static const xdg_wm_base_listener listener = {
                    [](void*, xdg_wm_base* shell, uint32_t serial) {
                        xdg_wm_base_pong(shell, serial);
                    }};
                xdg_wm_base_add_listener(c.shell, &listener, nullptr);
            } else if (!strcmp(interface, "wl_output")) {
                auto* output =
                    static_cast<wl_output*>(wl_registry_bind(r, name, &wl_output_interface, 2));
                c.outputs.push_back(output);
                std::printf("output global=%u proxy=%u\n", name,
                            wl_proxy_get_id(reinterpret_cast<wl_proxy*>(output)));
                static const wl_output_listener listener = {
                    [](void*, wl_output* o, int32_t x, int32_t y, int32_t, int32_t, int32_t,
                       const char* make, const char* model, int32_t) {
                        std::printf("output=%u position=%d,%d make=%s model=%s\n",
                                    wl_proxy_get_id(reinterpret_cast<wl_proxy*>(o)), x, y, make,
                                    model);
                    },
                    [](void*, wl_output*, uint32_t flags, int32_t w, int32_t h, int32_t refresh) {
                        if (flags & WL_OUTPUT_MODE_CURRENT)
                            std::printf("mode=%dx%d refresh_mHz=%d\n", w, h, refresh);
                    },
                    [](void*, wl_output*) {},
                    [](void*, wl_output*, int32_t) {},
                    nullptr,
                    nullptr};
                wl_output_add_listener(output, &listener, nullptr);
            }
        },
        [](void*, wl_registry*, uint32_t) {}};
    wl_registry_add_listener(registry, &globals, &c);
    if (wl_display_roundtrip(display) < 0 || !c.compositor || !c.shell || !c.shm ||
        !c.presentation) {
        std::fprintf(
            stderr,
            "Required globals unavailable (headless Lingmo does not advertise presentation).\n");
        wl_display_disconnect(display);
        return 2;
    }
    auto* surface = wl_compositor_create_surface(c.compositor);
    auto* xdgSurface = xdg_wm_base_get_xdg_surface(c.shell, surface);
    auto* top = xdg_surface_get_toplevel(xdgSurface);
    xdg_toplevel_set_title(top, "Presentation feedback diagnostic");
    static const xdg_toplevel_listener topListener = {
        [](void*, xdg_toplevel*, int32_t, int32_t, wl_array*) {},
        [](void* data, xdg_toplevel*) {
            static_cast<Client*>(data)->finished = true;
        },
        nullptr, nullptr};
    xdg_toplevel_add_listener(top, &topListener, &c);
    static const xdg_surface_listener configure = {[](void* data, xdg_surface* s, uint32_t serial) {
        xdg_surface_ack_configure(s, serial);
        static_cast<Client*>(data)->configured = true;
    }};
    xdg_surface_add_listener(xdgSurface, &configure, &c);
    wl_surface_commit(surface);
    while (!c.configured && wl_display_dispatch(display) >= 0) {
    }
    constexpr int width = 320, height = 200, bytes = width * height * 4;
    int fd = memfd_create("presentation-client", MFD_CLOEXEC);
    if (fd < 0 || ftruncate(fd, bytes) < 0)
        return 3;
    auto* pixels =
        static_cast<uint32_t*>(mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (pixels == MAP_FAILED)
        return 3;
    for (int i = 0; i < width * height; ++i)
        pixels[i] = 0xff3687b5;
    auto* pool = wl_shm_create_pool(c.shm, fd, bytes);
    auto* buffer =
        wl_shm_pool_create_buffer(pool, 0, width, height, width * 4, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);
    munmap(pixels, bytes);
    auto* feedback = wp_presentation_feedback(c.presentation, surface);
    static const wp_presentation_feedback_listener listener = {
        [](void*, struct wp_presentation_feedback*, wl_output* o) {
            std::printf("sync_output=%u\n", wl_proxy_get_id(reinterpret_cast<wl_proxy*>(o)));
        },
        [](void* data, struct wp_presentation_feedback*, uint32_t hi, uint32_t lo, uint32_t ns,
           uint32_t refresh, uint32_t seqHi, uint32_t seqLo, uint32_t flags) {
            auto& c = *static_cast<Client*>(data);
            std::printf("presented clock_id=%u timestamp=%llu.%09u sequence=%llu refresh_ns=%u "
                        "flags=0x%x\n",
                        c.clock, static_cast<unsigned long long>((uint64_t(hi) << 32) | lo), ns,
                        static_cast<unsigned long long>((uint64_t(seqHi) << 32) | seqLo), refresh,
                        flags);
            c.finished = true;
        },
        [](void* data, struct wp_presentation_feedback*) {
            std::printf("discarded clock_id=%u timestamp=unavailable sequence=unavailable "
                        "refresh=unavailable flags=unavailable\n",
                        static_cast<Client*>(data)->clock);
            static_cast<Client*>(data)->finished = true;
        }};
    wp_presentation_feedback_add_listener(feedback, &listener, &c);
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage_buffer(surface, 0, 0, width, height);
    wl_surface_commit(surface);
    while (!c.finished && wl_display_dispatch(display) >= 0) {
    }
    wp_presentation_feedback_destroy(feedback);
    xdg_toplevel_destroy(top);
    xdg_surface_destroy(xdgSurface);
    wl_surface_destroy(surface);
    wl_buffer_destroy(buffer);
    wp_presentation_destroy(c.presentation);
    xdg_wm_base_destroy(c.shell);
    wl_shm_destroy(c.shm);
    wl_compositor_destroy(c.compositor);
    for (auto* o : c.outputs)
        wl_output_destroy(o);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
    return c.finished ? 0 : 1;
}
