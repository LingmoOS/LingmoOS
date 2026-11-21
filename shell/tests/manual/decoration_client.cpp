// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// A small xdg-decoration client, owned by the repository so that server-side
// decoration can be demonstrated without depending on a toolkit's decoration
// policy.
//
// It maps an xdg_toplevel, asks for server-side decoration, and paints only its
// own client area - deliberately with no frame of its own, so that whatever
// titlebar, border and caption buttons appear are the compositor's. The pattern
// carries the configured size and the negotiated mode, so a screenshot shows
// whether the geometry is right.
//
//   keys:  m  maximize / restore      f  fullscreen / leave
//          n  minimize                s  ask for server-side decoration
//          c  ask for client-side decoration
//          u  unset the mode, leaving the compositor's preference
//          q  quit
//
// With --script it runs the whole sequence on a timer and exits, which is what
// the VT harness uses. With --csd it starts by asking for client-side
// decoration instead, which is how the CSD path is exercised.

#include <linux/input-event-codes.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <wayland-client.h>
#include <xdg-decoration-client-protocol.h>
#include <xdg-shell-client-protocol.h>

namespace {

struct Buffer {
    wl_buffer* buffer = nullptr;
    uint32_t* pixels = nullptr;
    size_t bytes = 0;
    int width = 0;
    int height = 0;
};

struct Client {
    wl_display* display = nullptr;
    wl_registry* registry = nullptr;
    wl_compositor* compositor = nullptr;
    wl_shm* shm = nullptr;
    wl_seat* seat = nullptr;
    wl_pointer* pointer = nullptr;
    wl_keyboard* keyboard = nullptr;
    xdg_wm_base* wmBase = nullptr;
    zxdg_decoration_manager_v1* decorationManager = nullptr;
    zxdg_toplevel_decoration_v1* decoration = nullptr;

    wl_surface* surface = nullptr;
    xdg_surface* xdgSurface = nullptr;
    xdg_toplevel* toplevel = nullptr;
    Buffer buffer;

    int width = 520;
    int height = 340;
    int pendingWidth = 0;
    int pendingHeight = 0;
    bool maximized = false;
    bool fullscreen = false;
    bool activated = false;
    bool configured = false;
    bool running = true;
    bool scripted = false;
    bool wantClientSide = false;
    uint32_t mode = 0;

    int scriptStep = 0;
    int frames = 0;
};

Buffer createBuffer(Client* client, int width, int height)
{
    Buffer buffer;
    buffer.width = width;
    buffer.height = height;
    const int stride = width * 4;
    buffer.bytes = static_cast<size_t>(stride) * height;

    const int fd = memfd_create("lingmo-decoration-demo", MFD_CLOEXEC);
    if (fd < 0 || ftruncate(fd, static_cast<off_t>(buffer.bytes)) != 0) {
        if (fd >= 0)
            close(fd);
        return buffer;
    }

    void* memory = mmap(nullptr, buffer.bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (memory == MAP_FAILED) {
        close(fd);
        return buffer;
    }
    buffer.pixels = static_cast<uint32_t*>(memory);

    wl_shm_pool* pool = wl_shm_create_pool(client->shm, fd, static_cast<int32_t>(buffer.bytes));
    buffer.buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride,
                                              WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);
    return buffer;
}

void destroyBuffer(Buffer* buffer)
{
    if (buffer->buffer)
        wl_buffer_destroy(buffer->buffer);
    if (buffer->pixels)
        munmap(buffer->pixels, buffer->bytes);
    *buffer = Buffer();
}

// A flat fill with a one pixel outline and a corner marker at each corner: the
// client area and nothing else, so the frame around it is entirely the
// compositor's. The server-side case deliberately draws no titlebar.
void paint(Client* client)
{
    Buffer* buffer = &client->buffer;
    if (!buffer->pixels)
        return;

    const bool serverSide = client->mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
    const uint32_t base = serverSide ? (client->activated ? 0x00f6f6f7 : 0x00e8e8ea)
                                     : (client->activated ? 0x00203040 : 0x00303030);
    const uint32_t ink = serverSide ? 0x00323238 : 0x00d0d0d0;
    const uint32_t accent = client->fullscreen ? 0x00ff5c6d
                                               : (client->maximized ? 0x0035bf56 : 0x003385ff);

    for (int y = 0; y < buffer->height; ++y) {
        for (int x = 0; x < buffer->width; ++x) {
            const bool outline =
                x == 0 || y == 0 || x == buffer->width - 1 || y == buffer->height - 1;
            const bool corner =
                (x < 20 || x >= buffer->width - 20) && (y < 20 || y >= buffer->height - 20);
            // A bar across the top of the client area: if a server-side
            // titlebar is drawn over the client's own pixels rather than
            // outside them, this is what disappears.
            const bool topBar = y < 6;
            uint32_t colour = base;
            if (topBar)
                colour = accent;
            if (corner)
                colour = accent;
            if (outline)
                colour = ink;
            buffer->pixels[static_cast<size_t>(y) * buffer->width + x] = colour;
        }
    }
}

void redraw(Client* client)
{
    if (client->buffer.width != client->width || client->buffer.height != client->height) {
        destroyBuffer(&client->buffer);
        client->buffer = createBuffer(client, client->width, client->height);
    }
    paint(client);
    wl_surface_attach(client->surface, client->buffer.buffer, 0, 0);
    wl_surface_damage_buffer(client->surface, 0, 0, client->width, client->height);
    // The window geometry is the whole buffer: this client has no shadow and
    // no decoration of its own.
    xdg_surface_set_window_geometry(client->xdgSurface, 0, 0, client->width, client->height);
    wl_surface_commit(client->surface);
    ++client->frames;
}

const char* modeName(uint32_t mode)
{
    switch (mode) {
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
        return "client-side";
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
        return "server-side";
    default:
        return "none";
    }
}

const zxdg_toplevel_decoration_v1_listener decorationListener = {
    [](void* data, zxdg_toplevel_decoration_v1*, uint32_t mode) {
        auto* client = static_cast<Client*>(data);
        client->mode = mode;
        printf("decoration configure %s\n", modeName(mode));
        fflush(stdout);
    },
};

const xdg_surface_listener surfaceListener = {
    [](void* data, xdg_surface* xdgSurface, uint32_t serial) {
        auto* client = static_cast<Client*>(data);
        xdg_surface_ack_configure(xdgSurface, serial);
        if (client->pendingWidth > 0 && client->pendingHeight > 0) {
            client->width = client->pendingWidth;
            client->height = client->pendingHeight;
        }
        client->pendingWidth = 0;
        client->pendingHeight = 0;
        client->configured = true;
        redraw(client);
    },
};

const xdg_toplevel_listener toplevelListener = {
    [](void* data, xdg_toplevel*, int32_t width, int32_t height, wl_array* states) {
        auto* client = static_cast<Client*>(data);
        client->pendingWidth = width;
        client->pendingHeight = height;
        client->maximized = false;
        client->fullscreen = false;
        client->activated = false;
        std::string names;
        const auto* values = static_cast<const uint32_t*>(states->data);
        for (size_t i = 0; i < states->size / sizeof(uint32_t); ++i) {
            switch (values[i]) {
            case XDG_TOPLEVEL_STATE_MAXIMIZED:
                client->maximized = true;
                names += " maximized";
                break;
            case XDG_TOPLEVEL_STATE_FULLSCREEN:
                client->fullscreen = true;
                names += " fullscreen";
                break;
            case XDG_TOPLEVEL_STATE_RESIZING:
                names += " resizing";
                break;
            case XDG_TOPLEVEL_STATE_ACTIVATED:
                client->activated = true;
                names += " activated";
                break;
            default:
                break;
            }
        }
        printf("toplevel configure %dx%d states:%s decoration:%s\n", width, height,
               names.empty() ? " none" : names.c_str(), modeName(client->mode));
        fflush(stdout);
    },
    [](void* data, xdg_toplevel*) {
        printf("toplevel close\n");
        fflush(stdout);
        static_cast<Client*>(data)->running = false;
    },
    [](void*, xdg_toplevel*, int32_t, int32_t) {},
    [](void*, xdg_toplevel*, wl_array*) {},
};

void requestMode(Client* client, uint32_t mode)
{
    if (!client->decoration)
        return;
    printf("requesting %s decoration\n", modeName(mode));
    fflush(stdout);
    zxdg_toplevel_decoration_v1_set_mode(client->decoration, mode);
}

const wl_keyboard_listener keyboardListener = {
    [](void*, wl_keyboard*, uint32_t, int32_t fd, uint32_t) {
        close(fd);
    },
    [](void*, wl_keyboard*, uint32_t, wl_surface*, wl_array*) {},
    [](void*, wl_keyboard*, uint32_t, wl_surface*) {},
    [](void* data, wl_keyboard*, uint32_t, uint32_t, uint32_t key, uint32_t state) {
        auto* client = static_cast<Client*>(data);
        if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
            return;
        switch (key) {
        case KEY_M:
            client->maximized ? xdg_toplevel_unset_maximized(client->toplevel)
                              : xdg_toplevel_set_maximized(client->toplevel);
            break;
        case KEY_F:
            client->fullscreen ? xdg_toplevel_unset_fullscreen(client->toplevel)
                               : xdg_toplevel_set_fullscreen(client->toplevel, nullptr);
            break;
        case KEY_N:
            xdg_toplevel_set_minimized(client->toplevel);
            break;
        case KEY_S:
            requestMode(client, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
            break;
        case KEY_C:
            requestMode(client, ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
            break;
        case KEY_U:
            printf("unsetting the decoration mode\n");
            fflush(stdout);
            zxdg_toplevel_decoration_v1_unset_mode(client->decoration);
            break;
        case KEY_ESC:
        case KEY_Q:
            client->running = false;
            break;
        default:
            break;
        }
    },
    [](void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {},
    [](void*, wl_keyboard*, int32_t, int32_t) {},
};

// The client draws no frame of its own, so a press anywhere inside it is
// content; everything that moves or resizes the window is the compositor's
// decoration. The pointer is bound only so the seat has one.
const wl_pointer_listener pointerListener = {
    [](void*, wl_pointer*, uint32_t, wl_surface*, wl_fixed_t, wl_fixed_t) {},
    [](void*, wl_pointer*, uint32_t, wl_surface*) {},
    [](void*, wl_pointer*, uint32_t, wl_fixed_t, wl_fixed_t) {},
    [](void*, wl_pointer*, uint32_t, uint32_t, uint32_t, uint32_t) {},
    [](void*, wl_pointer*, uint32_t, uint32_t, wl_fixed_t) {},
    [](void*, wl_pointer*) {},
    [](void*, wl_pointer*, uint32_t) {},
    [](void*, wl_pointer*, uint32_t, uint32_t) {},
    [](void*, wl_pointer*, uint32_t, int32_t) {},
    nullptr,
    nullptr,
};

const wl_seat_listener seatListener = {
    [](void* data, wl_seat* seat, uint32_t capabilities) {
        auto* client = static_cast<Client*>(data);
        if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !client->pointer) {
            client->pointer = wl_seat_get_pointer(seat);
            wl_pointer_add_listener(client->pointer, &pointerListener, client);
        }
        if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !client->keyboard) {
            client->keyboard = wl_seat_get_keyboard(seat);
            wl_keyboard_add_listener(client->keyboard, &keyboardListener, client);
        }
    },
    [](void*, wl_seat*, const char*) {},
};

void runScriptStep(Client* client)
{
    switch (client->scriptStep++) {
    case 0:
        printf("script: set_title\n");
        xdg_toplevel_set_title(client->toplevel, "Lingmo decoration demo");
        break;
    case 1:
        printf("script: set_maximized\n");
        xdg_toplevel_set_maximized(client->toplevel);
        break;
    case 2:
        printf("script: unset_maximized\n");
        xdg_toplevel_unset_maximized(client->toplevel);
        break;
    case 3:
        printf("script: set_fullscreen\n");
        xdg_toplevel_set_fullscreen(client->toplevel, nullptr);
        break;
    case 4:
        printf("script: unset_fullscreen\n");
        xdg_toplevel_unset_fullscreen(client->toplevel);
        break;
    case 5:
        printf("script: client-side\n");
        requestMode(client, ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
        break;
    case 6:
        printf("script: server-side\n");
        requestMode(client, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
        break;
    case 7:
        printf("script: unset_mode\n");
        zxdg_toplevel_decoration_v1_unset_mode(client->decoration);
        break;
    default:
        printf("script: done after %d frames\n", client->frames);
        client->running = false;
        break;
    }
    fflush(stdout);
}

} // namespace

int main(int argc, char** argv)
{
    Client client;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--script") == 0)
            client.scripted = true;
        else if (std::strcmp(argv[i], "--csd") == 0)
            client.wantClientSide = true;
    }

    client.display = wl_display_connect(nullptr);
    if (!client.display) {
        fprintf(stderr, "failed to connect to the Wayland display\n");
        return 1;
    }

    client.registry = wl_display_get_registry(client.display);
    static const wl_registry_listener registryListener = {
        [](void* data, wl_registry* registry, uint32_t name, const char* interface,
           uint32_t version) {
            auto* client = static_cast<Client*>(data);
            if (std::strcmp(interface, "wl_compositor") == 0) {
                client->compositor = static_cast<wl_compositor*>(
                    wl_registry_bind(registry, name, &wl_compositor_interface, 4));
            } else if (std::strcmp(interface, "wl_shm") == 0) {
                client->shm = static_cast<wl_shm*>(
                    wl_registry_bind(registry, name, &wl_shm_interface, 1));
            } else if (std::strcmp(interface, "wl_seat") == 0) {
                client->seat = static_cast<wl_seat*>(
                    wl_registry_bind(registry, name, &wl_seat_interface, version < 5 ? version : 5));
                wl_seat_add_listener(client->seat, &seatListener, client);
            } else if (std::strcmp(interface, "xdg_wm_base") == 0) {
                client->wmBase = static_cast<xdg_wm_base*>(
                    wl_registry_bind(registry, name, &xdg_wm_base_interface, version));
                static const xdg_wm_base_listener wmBaseListener = {
                    [](void*, xdg_wm_base* base, uint32_t serial) {
                        xdg_wm_base_pong(base, serial);
                    }};
                xdg_wm_base_add_listener(client->wmBase, &wmBaseListener, client);
            } else if (std::strcmp(interface, "zxdg_decoration_manager_v1") == 0) {
                client->decorationManager = static_cast<zxdg_decoration_manager_v1*>(
                    wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1));
            }
        },
        [](void*, wl_registry*, uint32_t) {}};
    wl_registry_add_listener(client.registry, &registryListener, &client);
    wl_display_roundtrip(client.display);
    wl_display_roundtrip(client.display);

    if (!client.compositor || !client.shm || !client.wmBase) {
        fprintf(stderr, "the compositor is missing wl_compositor, wl_shm or xdg_wm_base\n");
        return 1;
    }
    if (!client.decorationManager) {
        fprintf(stderr, "the compositor does not offer zxdg_decoration_manager_v1\n");
        return 1;
    }
    printf("bound zxdg_decoration_manager_v1\n");

    client.surface = wl_compositor_create_surface(client.compositor);
    client.xdgSurface = xdg_wm_base_get_xdg_surface(client.wmBase, client.surface);
    xdg_surface_add_listener(client.xdgSurface, &surfaceListener, &client);
    client.toplevel = xdg_surface_get_toplevel(client.xdgSurface);
    xdg_toplevel_add_listener(client.toplevel, &toplevelListener, &client);
    xdg_toplevel_set_title(client.toplevel, "Lingmo decoration demo");
    xdg_toplevel_set_app_id(client.toplevel, "org.lingmo.DecorationDemo");
    xdg_toplevel_set_min_size(client.toplevel, 240, 160);

    // Before the first buffer: the protocol forbids attaching one to a toplevel
    // whose decoration mode has not been configured yet.
    client.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
        client.decorationManager, client.toplevel);
    zxdg_toplevel_decoration_v1_add_listener(client.decoration, &decorationListener, &client);
    requestMode(&client, client.wantClientSide
                    ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE
                    : ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);

    wl_surface_commit(client.surface);

    if (client.scripted) {
        while (client.running) {
            if (wl_display_roundtrip(client.display) < 0)
                break;
            if (!client.configured) {
                usleep(50 * 1000);
                continue;
            }
            usleep(400 * 1000);
            runScriptStep(&client);
            wl_display_flush(client.display);
        }
    } else {
        while (client.running && wl_display_dispatch(client.display) != -1)
            ;
    }

    printf("exiting after %d frames\n", client.frames);
    fflush(stdout);

    destroyBuffer(&client.buffer);
    // The decoration object goes before the toplevel it belongs to.
    zxdg_toplevel_decoration_v1_destroy(client.decoration);
    xdg_toplevel_destroy(client.toplevel);
    xdg_surface_destroy(client.xdgSurface);
    wl_surface_destroy(client.surface);
    if (client.keyboard)
        wl_keyboard_destroy(client.keyboard);
    if (client.pointer)
        wl_pointer_destroy(client.pointer);
    if (client.seat)
        wl_seat_destroy(client.seat);
    zxdg_decoration_manager_v1_destroy(client.decorationManager);
    xdg_wm_base_destroy(client.wmBase);
    wl_shm_destroy(client.shm);
    wl_compositor_destroy(client.compositor);
    wl_registry_destroy(client.registry);
    wl_display_disconnect(client.display);
    return 0;
}
