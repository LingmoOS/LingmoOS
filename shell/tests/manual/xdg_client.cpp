// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// A small xdg-shell client, owned by the repository so that xdg-shell can be
// exercised without depending on a toolkit being installed.
//
// It maps an xdg_toplevel, honours configure, draws a pattern that shows the
// window size, opens a grabbing popup menu on a pointer press, and drives an
// interactive move from the drag that follows. With --script it runs the whole
// state machine on a timer and exits, which is what the VT harness uses.

#include <linux/input-event-codes.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>

namespace {

struct Buffer {
    wl_buffer* buffer = nullptr;
    uint32_t* pixels = nullptr;
    size_t bytes = 0;
    int width = 0;
    int height = 0;
    bool busy = false;
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

    wl_surface* surface = nullptr;
    xdg_surface* xdgSurface = nullptr;
    xdg_toplevel* toplevel = nullptr;
    Buffer buffer;

    // The popup menu, when one is open.
    wl_surface* popupSurface = nullptr;
    xdg_surface* popupXdgSurface = nullptr;
    xdg_popup* popup = nullptr;
    Buffer popupBuffer;

    int width = 480;
    int height = 320;
    int pendingWidth = 0;
    int pendingHeight = 0;
    bool maximized = false;
    bool fullscreen = false;
    bool activated = false;
    bool configured = false;
    bool running = true;
    bool scripted = false;

    uint32_t lastPointerSerial = 0;
    double pointerX = 0;
    double pointerY = 0;

    int scriptStep = 0;
    int frames = 0;
};

constexpr int kPopupWidth = 160;
constexpr int kPopupHeight = 120;

Buffer createBuffer(Client* client, int width, int height)
{
    Buffer buffer;
    buffer.width = width;
    buffer.height = height;
    const int stride = width * 4;
    buffer.bytes = static_cast<size_t>(stride) * height;

    const int fd = memfd_create("lingmo-xdg-demo", MFD_CLOEXEC);
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

// A frame with a visible border, a corner marker per edge and diagonal
// hatching, so a resize or a move is obvious on a real screen.
void paint(Buffer* buffer, uint32_t base, uint32_t accent)
{
    if (!buffer->pixels)
        return;
    for (int y = 0; y < buffer->height; ++y) {
        for (int x = 0; x < buffer->width; ++x) {
            const bool border = x < 3 || y < 3 || x >= buffer->width - 3
                || y >= buffer->height - 3;
            const bool corner = (x < 24 || x >= buffer->width - 24)
                && (y < 24 || y >= buffer->height - 24);
            const bool hatch = ((x + y) / 12) % 2 == 0;
            uint32_t colour = hatch ? base : base + 0x00101010;
            if (corner)
                colour = accent;
            if (border)
                colour = 0x00ffffff;
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
    paint(&client->buffer, client->activated ? 0x00204060 : 0x00303030,
          client->fullscreen ? 0x00c04020 : (client->maximized ? 0x0020a060 : 0x00d0a020));

    wl_surface_attach(client->surface, client->buffer.buffer, 0, 0);
    wl_surface_damage_buffer(client->surface, 0, 0, client->width, client->height);
    wl_surface_commit(client->surface);
    ++client->frames;
}

void closePopup(Client* client)
{
    if (client->popup) {
        xdg_popup_destroy(client->popup);
        client->popup = nullptr;
    }
    if (client->popupXdgSurface) {
        xdg_surface_destroy(client->popupXdgSurface);
        client->popupXdgSurface = nullptr;
    }
    if (client->popupSurface) {
        wl_surface_destroy(client->popupSurface);
        client->popupSurface = nullptr;
    }
    destroyBuffer(&client->popupBuffer);
}

const xdg_popup_listener popupListener = {
    [](void* data, xdg_popup*, int32_t x, int32_t y, int32_t width, int32_t height) {
        auto* client = static_cast<Client*>(data);
        printf("popup configure %dx%d at %d,%d\n", width, height, x, y);
        fflush(stdout);
        client->popupBuffer.width = width;
        client->popupBuffer.height = height;
    },
    [](void* data, xdg_popup*) {
        printf("popup done\n");
        fflush(stdout);
        closePopup(static_cast<Client*>(data));
    },
    [](void*, xdg_popup*, uint32_t token) {
        printf("popup repositioned %u\n", token);
        fflush(stdout);
    },
};

const xdg_surface_listener popupSurfaceListener = {
    [](void* data, xdg_surface* xdgSurface, uint32_t serial) {
        auto* client = static_cast<Client*>(data);
        xdg_surface_ack_configure(xdgSurface, serial);
        if (!client->popupSurface)
            return;
        if (!client->popupBuffer.buffer) {
            client->popupBuffer =
                createBuffer(client, client->popupBuffer.width ? client->popupBuffer.width
                                                               : kPopupWidth,
                             client->popupBuffer.height ? client->popupBuffer.height
                                                        : kPopupHeight);
        }
        paint(&client->popupBuffer, 0x00402040, 0x00e0c040);
        wl_surface_attach(client->popupSurface, client->popupBuffer.buffer, 0, 0);
        wl_surface_damage_buffer(client->popupSurface, 0, 0, client->popupBuffer.width,
                                 client->popupBuffer.height);
        wl_surface_commit(client->popupSurface);
    },
};

void openPopup(Client* client, int anchorX, int anchorY, bool grab)
{
    if (client->popup)
        closePopup(client);

    client->popupBuffer.width = kPopupWidth;
    client->popupBuffer.height = kPopupHeight;
    client->popupSurface = wl_compositor_create_surface(client->compositor);
    client->popupXdgSurface = xdg_wm_base_get_xdg_surface(client->wmBase, client->popupSurface);
    xdg_surface_add_listener(client->popupXdgSurface, &popupSurfaceListener, client);

    xdg_positioner* positioner = xdg_wm_base_create_positioner(client->wmBase);
    xdg_positioner_set_size(positioner, kPopupWidth, kPopupHeight);
    xdg_positioner_set_anchor_rect(positioner, anchorX, anchorY, 1, 1);
    xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_BOTTOM_LEFT);
    xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
    xdg_positioner_set_constraint_adjustment(
        positioner, XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X
                        | XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y
                        | XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X
                        | XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y);

    client->popup = xdg_surface_get_popup(client->popupXdgSurface, client->xdgSurface, positioner);
    xdg_popup_add_listener(client->popup, &popupListener, client);
    if (grab && client->seat && client->lastPointerSerial)
        xdg_popup_grab(client->popup, client->seat, client->lastPointerSerial);
    xdg_positioner_destroy(positioner);
    wl_surface_commit(client->popupSurface);
    printf("popup opened at %d,%d\n", anchorX, anchorY);
    fflush(stdout);
}

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
        printf("toplevel configure %dx%d states:%s\n", width, height,
               names.empty() ? " none" : names.c_str());
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

const wl_pointer_listener pointerListener = {
    [](void* data, wl_pointer*, uint32_t serial, wl_surface*, wl_fixed_t x, wl_fixed_t y) {
        auto* client = static_cast<Client*>(data);
        client->lastPointerSerial = serial;
        client->pointerX = wl_fixed_to_double(x);
        client->pointerY = wl_fixed_to_double(y);
    },
    [](void*, wl_pointer*, uint32_t, wl_surface*) {},
    [](void* data, wl_pointer*, uint32_t, wl_fixed_t x, wl_fixed_t y) {
        auto* client = static_cast<Client*>(data);
        client->pointerX = wl_fixed_to_double(x);
        client->pointerY = wl_fixed_to_double(y);
    },
    [](void* data, wl_pointer*, uint32_t serial, uint32_t, uint32_t button, uint32_t state) {
        auto* client = static_cast<Client*>(data);
        if (state != WL_POINTER_BUTTON_STATE_PRESSED)
            return;
        client->lastPointerSerial = serial;
        const int x = static_cast<int>(client->pointerX);
        const int y = static_cast<int>(client->pointerY);
        if (button == BTN_RIGHT) {
            openPopup(client, x, y, /*grab=*/true);
        } else if (button == BTN_LEFT) {
            // Near an edge, resize; otherwise move.
            const int margin = 24;
            uint32_t edge = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
            if (x >= client->width - margin && y >= client->height - margin)
                edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
            else if (x < margin && y < margin)
                edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
            else if (x < margin)
                edge = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
            else if (y < margin)
                edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP;

            if (edge == XDG_TOPLEVEL_RESIZE_EDGE_NONE) {
                printf("starting move\n");
                xdg_toplevel_move(client->toplevel, client->seat, serial);
            } else {
                printf("starting resize edge %u\n", edge);
                xdg_toplevel_resize(client->toplevel, client->seat, serial, edge);
            }
            fflush(stdout);
        }
    },
    [](void*, wl_pointer*, uint32_t, uint32_t, wl_fixed_t) {},
    [](void*, wl_pointer*) {},
    [](void*, wl_pointer*, uint32_t) {},
    [](void*, wl_pointer*, uint32_t, uint32_t) {},
    [](void*, wl_pointer*, uint32_t, int32_t) {},
    nullptr,
    nullptr,
};

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

// One step of the scripted run. Each step logs what it asked for, so the VT
// harness can check the sequence from the client log.
void runScriptStep(Client* client)
{
    switch (client->scriptStep++) {
    case 0:
        printf("script: set_title\n");
        xdg_toplevel_set_title(client->toplevel, "Lingmo xdg demo");
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
        printf("script: open popup\n");
        openPopup(client, client->width / 3, client->height / 3, /*grab=*/false);
        break;
    case 6:
        printf("script: close popup\n");
        closePopup(client);
        break;
    case 7:
        printf("script: set_minimized\n");
        xdg_toplevel_set_minimized(client->toplevel);
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
    printf("bound xdg_wm_base\n");

    client.surface = wl_compositor_create_surface(client.compositor);
    client.xdgSurface = xdg_wm_base_get_xdg_surface(client.wmBase, client.surface);
    xdg_surface_add_listener(client.xdgSurface, &surfaceListener, &client);
    client.toplevel = xdg_surface_get_toplevel(client.xdgSurface);
    xdg_toplevel_add_listener(client.toplevel, &toplevelListener, &client);
    xdg_toplevel_set_title(client.toplevel, "Lingmo xdg demo");
    xdg_toplevel_set_app_id(client.toplevel, "org.lingmo.XdgDemo");
    xdg_toplevel_set_min_size(client.toplevel, 200, 150);
    // The first commit carries no buffer: it asks for the initial configure.
    wl_surface_commit(client.surface);

    if (client.scripted) {
        // One step per round, each preceded by a roundtrip so the compositor's
        // answer to the previous one is in. A blocking dispatch would not do:
        // most of these requests produce no event of their own.
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

    closePopup(&client);
    destroyBuffer(&client.buffer);
    xdg_toplevel_destroy(client.toplevel);
    xdg_surface_destroy(client.xdgSurface);
    wl_surface_destroy(client.surface);
    if (client.keyboard)
        wl_keyboard_destroy(client.keyboard);
    if (client.pointer)
        wl_pointer_destroy(client.pointer);
    if (client.seat)
        wl_seat_destroy(client.seat);
    xdg_wm_base_destroy(client.wmBase);
    wl_shm_destroy(client.shm);
    wl_compositor_destroy(client.compositor);
    wl_registry_destroy(client.registry);
    wl_display_disconnect(client.display);
    return 0;
}
