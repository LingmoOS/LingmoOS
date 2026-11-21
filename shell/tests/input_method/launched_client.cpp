// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <wayland-client.h>

// Connects through WAYLAND_SOCKET, appends "ime" or "none" to argv[2] depending
// on whether the input-method global is visible, then exits or stays connected.
// The "display" mode appends its DISPLAY instead, or "-", and stays.
int main(int argc, char **argv)
{
    if (argc < 3)
        return 2;
    auto *display = wl_display_connect(nullptr);
    if (!display)
        return 3;
    bool visible = false;
    auto *registry = wl_display_get_registry(display);
    static const wl_registry_listener listener = {
            [](void *data, wl_registry *, uint32_t, const char *interface, uint32_t) {
                if (!std::strcmp(interface, "zwp_input_method_manager_v2"))
                    *static_cast<bool *>(data) = true;
            },
            [](void *, wl_registry *, uint32_t) {}};
    wl_registry_add_listener(registry, &listener, &visible);
    wl_display_roundtrip(display);
    const char *logged = nullptr;
    if (!std::strcmp(argv[1], "display"))
        logged = "DISPLAY";
    if (auto *file = std::fopen(argv[2], "a")) {
        if (logged)
            std::fprintf(file, "%s\n", std::getenv(logged) ? std::getenv(logged) : "-");
        else
            std::fputs(visible ? "ime\n" : "none\n", file);
        std::fclose(file);
    }
    if (!std::strcmp(argv[1], "stay") || logged) {
        while (wl_display_dispatch(display) != -1) {
        }
    }
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
    return 0;
}
