// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Libwayland-only subsurface hardware demo. Static buffers are never modified after
// attach; a desynchronized child provides presentation-driven animation while
// its sibling's blue state is deliberately held in the synchronized cache.

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <list>
#include <poll.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>

namespace {

struct FrameRequest {
    wl_callback* resource;
    int* count;
};

struct State {
    std::list<FrameRequest> callbacks;
    wl_display* display = nullptr;
    wl_compositor* compositor = nullptr;
    wl_subcompositor* subcompositor = nullptr;
    wl_shm* shm = nullptr;
    wl_output* output = nullptr;
    int width = 0, height = 0, refresh = 0, scale = 0, done = 0;
    int animatedFrames = 0, cachedFrames = 0, parentFrames = 0;
    bool forbidden = false;
};

const wl_output_listener outputListener = {
    [](void*, wl_output*, int x, int y, int w, int h, int, const char* make, const char* model,
       int) {
        std::printf("output: %s %s at %d,%d physical %dx%d mm\n", make, model, x, y, w, h);
    },
    [](void* data, wl_output*, uint32_t flags, int w, int h, int refresh) {
        auto& s = *static_cast<State*>(data);
        s.width = w;
        s.height = h;
        s.refresh = refresh;
        std::printf("mode: %dx%d %.3f Hz flags=%u\n", w, h, refresh / 1000.0, flags);
    },
    [](void* data, wl_output*) {
        ++static_cast<State*>(data)->done;
    },
    [](void* data, wl_output*, int scale) {
        static_cast<State*>(data)->scale = scale;
    },
    nullptr,
    nullptr,
};

const wl_registry_listener registryListener = {
    [](void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
        auto& s = *static_cast<State*>(data);
        std::printf("global: %s v%u\n", interface, version);
        if (!strcmp(interface, "wl_compositor"))
            s.compositor = static_cast<wl_compositor*>(
                wl_registry_bind(registry, name, &wl_compositor_interface, 4));
        if (!strcmp(interface, "wl_subcompositor"))
            s.subcompositor = static_cast<wl_subcompositor*>(
                wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
        if (!strcmp(interface, "wl_shm"))
            s.shm = static_cast<wl_shm*>(wl_registry_bind(registry, name, &wl_shm_interface, 1));
        if (!strcmp(interface, "wl_output")) {
            s.output = static_cast<wl_output*>(
                wl_registry_bind(registry, name, &wl_output_interface, std::min(version, 3u)));
            wl_output_add_listener(s.output, &outputListener, &s);
        }
        if (!strcmp(interface, "wl_seat") || !strcmp(interface, "xdg_wm_base"))
            s.forbidden = true;
    },
    [](void*, wl_registry*, uint32_t) {}};

struct Buffer {
    wl_buffer* resource;
    void* pixels;
    size_t bytes;
    int releases = 0;
};

const wl_buffer_listener bufferListener = {[](void* data, wl_buffer*) {
    ++static_cast<Buffer*>(data)->releases;
}};

bool makeBuffer(State& s, Buffer& b, int w, int h, uint32_t color, int stripe = 0)
{
    b.bytes = static_cast<size_t>(w) * h * 4;
    int fd = memfd_create("lingmo-subsurface-demo", MFD_CLOEXEC);
    if (fd < 0)
        return false;
    if (ftruncate(fd, b.bytes) < 0) {
        close(fd);
        return false;
    }
    b.pixels = mmap(nullptr, b.bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (b.pixels == MAP_FAILED) {
        close(fd);
        return false;
    }
    auto* pixels = static_cast<uint32_t*>(b.pixels);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            pixels[y * w + x] = (x < 4 || y < 4 || x >= w - 4 || y >= h - 4)
                ? 0xffffff
                : (stripe && ((x + stripe) / 20) % 2 ? color / 2 : color);
    auto* pool = wl_shm_create_pool(s.shm, fd, b.bytes);
    b.resource = wl_shm_pool_create_buffer(pool, 0, w, h, w * 4, WL_SHM_FORMAT_XRGB8888);
    wl_buffer_add_listener(b.resource, &bufferListener, &b);
    wl_shm_pool_destroy(pool);
    close(fd);
    return true;
}

void attach(wl_surface* surface, Buffer& buffer)
{
    wl_surface_attach(surface, buffer.resource, 0, 0);
    wl_surface_damage(surface, 0, 0, 1000, 1000);
}

const wl_callback_listener callbackListener = {[](void* data, wl_callback* cb, uint32_t) {
    auto& request = *static_cast<FrameRequest*>(data);
    ++*request.count;
    request.resource = nullptr;
    wl_callback_destroy(cb);
}};

void frame(State& s, wl_surface* surface, int& count)
{
    s.callbacks.remove_if([](const auto& request) {
        return !request.resource;
    });
    s.callbacks.push_back({wl_surface_frame(surface), &count});
    auto& request = s.callbacks.back();
    wl_callback_add_listener(request.resource, &callbackListener, &request);
}

bool pump(State& s, int timeoutMs)
{
    if (wl_display_dispatch_pending(s.display) < 0)
        return false;
    if (wl_display_flush(s.display) < 0)
        return false;
    pollfd fd {wl_display_get_fd(s.display), POLLIN, 0};
    int result = poll(&fd, 1, timeoutMs);
    return result >= 0 && (!(fd.revents & POLLIN) || wl_display_dispatch(s.display) >= 0);
}
} // namespace

int main()
{
    State s;
    s.display = wl_display_connect(nullptr);
    if (!s.display)
        return 1;
    auto* registry = wl_display_get_registry(s.display);
    wl_registry_add_listener(registry, &registryListener, &s);
    if (wl_display_roundtrip(s.display) < 0 || wl_display_roundtrip(s.display) < 0)
        return 1;
    if (!s.compositor || !s.subcompositor || !s.shm || !s.output || s.forbidden || s.width <= 0 ||
        s.height <= 0 || s.refresh <= 0 || s.scale != 1 || s.done != 1)
        return 1;
    Buffer buffers[5] {};
    if (!makeBuffer(s, buffers[0], 360, 260, 0x444455) ||
        !makeBuffer(s, buffers[1], 170, 180, 0xee4433) ||
        !makeBuffer(s, buffers[2], 170, 180, 0x3388ff) ||
        !makeBuffer(s, buffers[3], 200, 150, 0x33dd66, 10) ||
        !makeBuffer(s, buffers[4], 200, 150, 0xeebb22, 30))
        return 1;
    auto* parent = wl_compositor_create_surface(s.compositor);
    auto* child = wl_compositor_create_surface(s.compositor);
    auto* animated = wl_compositor_create_surface(s.compositor);
    auto* sub = wl_subcompositor_get_subsurface(s.subcompositor, child, parent);
    auto* animSub = wl_subcompositor_get_subsurface(s.subcompositor, animated, parent);
    // Red extends beyond the parent's right edge, but its overlap is BELOW it.
    // Green overlaps both surfaces ABOVE them, exposing all three stack levels.
    wl_subsurface_set_position(sub, 290, 40);
    wl_subsurface_place_below(sub, parent);
    wl_subsurface_set_position(animSub, 230, 170);
    wl_subsurface_place_above(animSub, parent);
    attach(child, buffers[1]);
    wl_surface_commit(child);
    attach(animated, buffers[3]);
    wl_surface_commit(animated);
    attach(parent, buffers[0]);
    frame(s, parent, s.parentFrames);
    wl_surface_commit(parent);
    const auto start = std::chrono::steady_clock::now();
    auto seconds = [&] {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    };
    while (!s.parentFrames && seconds() < 3)
        if (!pump(s, 20))
            return 1;
    if (!s.parentFrames)
        return 1;
    std::puts("A presented: red child below gray parent, green child above both.");
    wl_subsurface_set_desync(animSub);
    // First commit A without parent, then replace its cache with B. On screen
    // the red A must remain for the entire hold interval.
    attach(child, buffers[1]);
    wl_surface_commit(child);
    attach(child, buffers[2]);
    frame(s, child, s.cachedFrames);
    wl_surface_commit(child);
    wl_subsurface_set_position(sub, 270, 60);
    const double heldAt = seconds();
    bool submitted = false;
    int last = -1;
    bool failed = false;
    while (seconds() < heldAt + 1.5) {
        if (last != s.animatedFrames) {
            last = s.animatedFrames;
            attach(animated, buffers[3 + (last % 2)]);
            frame(s, animated, s.animatedFrames);
            wl_surface_commit(animated);
        }
        if (!pump(s, 20)) {
            failed = true;
            break;
        }
        if (s.cachedFrames || buffers[2].releases) {
            failed = true;
            break;
        }
    }
    std::printf("cache hold: %d independent presentations, B callbacks=%d releases=%d\n",
                s.animatedFrames, s.cachedFrames, buffers[2].releases);
    if (s.animatedFrames < 10)
        failed = true;
    if (!failed) {
        wl_subsurface_place_above(sub, parent); // blue now covers the overlap
        wl_surface_commit(parent);
        submitted = true;
        while (!s.cachedFrames && seconds() < heldAt + 3)
            if (!pump(s, 20)) {
                failed = true;
                break;
            }
        if (!s.cachedFrames)
            failed = true;
        std::puts("Parent committed: B is blue, moved, and now above the gray parent.");
    }
    // Desync target child, then animate its contents without any parent commits.
    int independent = 0;
    if (!failed) {
        wl_subsurface_set_desync(sub);
        double until = seconds() + 1.5;
        last = -1;
        while (seconds() < until) {
            if (last != independent) {
                last = independent;
                attach(child, buffers[1 + (last % 2)]);
                frame(s, child, independent);
                wl_surface_commit(child);
            }
            if (!pump(s, 20)) {
                failed = true;
                break;
            }
        }
        if (independent < 10)
            failed = true;
    }
    std::printf("desynchronized child: %d independent presentations\n", independent);
    std::printf("RESULT: %s\n", !failed && submitted ? "ok" : "FAILED");
    for (auto& callback : s.callbacks)
        if (callback.resource)
            wl_callback_destroy(callback.resource);
    s.callbacks.clear();
    wl_subsurface_destroy(sub);
    wl_subsurface_destroy(animSub);
    wl_surface_destroy(child);
    wl_surface_destroy(animated);
    wl_surface_destroy(parent);
    for (auto& b : buffers) {
        wl_buffer_destroy(b.resource);
        munmap(b.pixels, b.bytes);
    }
    wl_output_release(s.output);
    wl_shm_destroy(s.shm);
    wl_subcompositor_destroy(s.subcompositor);
    wl_compositor_destroy(s.compositor);
    wl_registry_destroy(registry);
    wl_display_roundtrip(s.display);
    wl_display_disconnect(s.display);
    return failed || !submitted ? 1 : 0;
}
