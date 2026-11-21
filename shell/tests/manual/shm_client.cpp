// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// A minimal libwayland-client program for exercising wl_shm by hand: it binds
// wl_compositor and wl_shm, paints a moving pattern into an shm buffer and
// drives repeated commits from frame callbacks. Deliberately no xdg-shell and
// no toolkit, so that a failure points at the compositor rather than at one.

#include <wayland-client.h>

#include <poll.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cerrno>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <csignal>

namespace {

constexpr int kWidth = 400;
constexpr int kHeight = 300;
constexpr int kStride = kWidth * 4;
constexpr size_t kBufferBytes = static_cast<size_t>(kStride) * kHeight;
// Two buffers, so the next frame can be painted while the compositor still
// holds the one on screen.
constexpr int kBufferCount = 2;

volatile sig_atomic_t g_running = 1;

struct Buffer {
    wl_buffer* buffer = nullptr;
    uint32_t* pixels = nullptr;
    bool busy = false;
};

struct State {
    wl_display* display = nullptr;
    wl_compositor* compositor = nullptr;
    wl_shm* shm = nullptr;
    wl_surface* surface = nullptr;

    int fd = -1;
    void* pool = nullptr;
    Buffer buffers[kBufferCount];

    int frames = 0;
};

void handleGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface,
                  uint32_t version)
{
    auto* state = static_cast<State*>(data);
    if (std::strcmp(interface, "wl_compositor") == 0) {
        const uint32_t bindVersion = version < 4 ? version : 4;
        state->compositor = static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, bindVersion));
        std::printf("bound wl_compositor v%u\n", bindVersion);
    } else if (std::strcmp(interface, "wl_shm") == 0) {
        state->shm = static_cast<wl_shm*>(
            wl_registry_bind(registry, name, &wl_shm_interface, 1));
        std::printf("bound wl_shm v1\n");
    }
}

void handleShmFormat(void*, wl_shm*, uint32_t format)
{
    std::printf("wl_shm format 0x%08x\n", format);
}

void handleBufferRelease(void* data, wl_buffer*)
{
    static_cast<Buffer*>(data)->busy = false;
}

const wl_buffer_listener kBufferListener = { handleBufferRelease };

// An animated checkerboard with a diagonal gradient; every commit visibly
// changes, so a stalled frame-callback loop is obvious on screen.
void paint(Buffer* buffer, int frame)
{
    const double phase = frame * 0.05;
    const int shift = static_cast<int>(frame * 2) % 64;

    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            const bool light = (((x + shift) / 32) + (y / 32)) % 2 == 0;
            const uint8_t r = static_cast<uint8_t>(127.5 * (1.0 + std::sin(phase + x * 0.01)));
            const uint8_t g = static_cast<uint8_t>(127.5 * (1.0 + std::cos(phase + y * 0.013)));
            const uint8_t b = light ? 220 : 60;
            // WL_SHM_FORMAT_XRGB8888 is a little-endian 0xXXRRGGBB word.
            buffer->pixels[y * kWidth + x] =
                (static_cast<uint32_t>(light ? r : r / 3) << 16)
                | (static_cast<uint32_t>(light ? g : g / 3) << 8) | b;
        }
    }

    // A visible border, so the surface bounds are unambiguous.
    for (int x = 0; x < kWidth; ++x) {
        buffer->pixels[x] = 0x00ffffff;
        buffer->pixels[(kHeight - 1) * kWidth + x] = 0x00ffffff;
    }
    for (int y = 0; y < kHeight; ++y) {
        buffer->pixels[y * kWidth] = 0x00ffffff;
        buffer->pixels[y * kWidth + kWidth - 1] = 0x00ffffff;
    }
}

Buffer* freeBuffer(State* state)
{
    for (Buffer& buffer : state->buffers) {
        if (!buffer.busy)
            return &buffer;
    }
    return nullptr;
}

void drawFrame(State* state);

void handleFrameCallback(void* data, wl_callback* callback, uint32_t time)
{
    auto* state = static_cast<State*>(data);
    wl_callback_destroy(callback);

    if (state->frames % 60 == 0)
        std::printf("frame %d, callback time %u ms\n", state->frames, time);

    drawFrame(state);
}

const wl_callback_listener kFrameListener = { handleFrameCallback };

void drawFrame(State* state)
{
    Buffer* buffer = freeBuffer(state);
    if (!buffer) {
        // Every buffer is still with the compositor; ask to be woken again.
        wl_callback* callback = wl_surface_frame(state->surface);
        wl_callback_add_listener(callback, &kFrameListener, state);
        wl_surface_commit(state->surface);
        return;
    }

    paint(buffer, state->frames++);
    buffer->busy = true;

    wl_callback* callback = wl_surface_frame(state->surface);
    wl_callback_add_listener(callback, &kFrameListener, state);

    wl_surface_attach(state->surface, buffer->buffer, 0, 0);
    wl_surface_damage_buffer(state->surface, 0, 0, kWidth, kHeight);
    wl_surface_commit(state->surface);
}

bool createBuffers(State* state)
{
    const size_t poolSize = kBufferBytes * kBufferCount;

    state->fd = memfd_create("lingmo-shm-demo", MFD_CLOEXEC);
    if (state->fd < 0 || ftruncate(state->fd, static_cast<off_t>(poolSize)) != 0) {
        std::fprintf(stderr, "failed to create the shm backing store\n");
        return false;
    }

    state->pool = mmap(nullptr, poolSize, PROT_READ | PROT_WRITE, MAP_SHARED, state->fd, 0);
    if (state->pool == MAP_FAILED) {
        std::fprintf(stderr, "failed to map the shm backing store\n");
        return false;
    }

    wl_shm_pool* pool = wl_shm_create_pool(state->shm, state->fd,
                                           static_cast<int32_t>(poolSize));
    for (int i = 0; i < kBufferCount; ++i) {
        const size_t offset = kBufferBytes * i;
        state->buffers[i].pixels =
            reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(state->pool) + offset);
        state->buffers[i].buffer =
            wl_shm_pool_create_buffer(pool, static_cast<int32_t>(offset), kWidth, kHeight,
                                      kStride, WL_SHM_FORMAT_XRGB8888);
        if (!state->buffers[i].buffer) {
            std::fprintf(stderr, "failed to create wl_buffer %d\n", i);
            return false;
        }
        wl_buffer_add_listener(state->buffers[i].buffer, &kBufferListener, &state->buffers[i]);
    }
    // The buffers keep the memory alive on the compositor side.
    wl_shm_pool_destroy(pool);

    std::printf("created %d %dx%d XRGB8888 buffers\n", kBufferCount, kWidth, kHeight);
    return true;
}

} // namespace

int main()
{
    // Installed without SA_RESTART, so a signal arriving during poll() actually
    // returns instead of silently restarting the call.
    struct sigaction action = {};
    action.sa_handler = [](int) { g_running = 0; };
    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGTERM, &action, nullptr);

    State state;
    state.display = wl_display_connect(nullptr);
    if (!state.display) {
        std::fprintf(stderr, "failed to connect to WAYLAND_DISPLAY\n");
        return 1;
    }

    static const wl_registry_listener registryListener = {
        handleGlobal,
        [](void*, wl_registry*, uint32_t) {},
    };
    wl_registry* registry = wl_display_get_registry(state.display);
    wl_registry_add_listener(registry, &registryListener, &state);
    wl_display_roundtrip(state.display);

    if (!state.compositor || !state.shm) {
        std::fprintf(stderr, "the compositor does not offer wl_compositor and wl_shm\n");
        return 1;
    }

    static const wl_shm_listener shmListener = { handleShmFormat };
    wl_shm_add_listener(state.shm, &shmListener, &state);
    wl_display_roundtrip(state.display);

    state.surface = wl_compositor_create_surface(state.compositor);
    if (!state.surface) {
        std::fprintf(stderr, "failed to create wl_surface\n");
        return 1;
    }

    if (!createBuffers(&state))
        return 1;

    drawFrame(&state);

    // Polled by hand rather than with wl_display_dispatch(), so Ctrl-C is
    // noticed promptly instead of at the next compositor event.
    while (g_running) {
        while (wl_display_prepare_read(state.display) != 0) {
            if (wl_display_dispatch_pending(state.display) < 0)
                break;
        }
        wl_display_flush(state.display);

        pollfd pfd = { wl_display_get_fd(state.display), POLLIN, 0 };
        const int ready = poll(&pfd, 1, 100);
        if (ready > 0) {
            wl_display_read_events(state.display);
        } else {
            wl_display_cancel_read(state.display);
            if (ready < 0 && errno != EINTR)
                break;
        }

        if (wl_display_dispatch_pending(state.display) < 0)
            break;
    }

    if (const int error = wl_display_get_error(state.display))
        std::fprintf(stderr, "wayland protocol error: %d\n", error);

    std::printf("drew %d frames\n", state.frames);
    return 0;
}
