// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdlib>
#include <cstring>
#include <xcb/xcb.h>

// Maps one X11 window and keeps it until killed. The first argument, when
// given, is written to WM_CLIENT_MACHINE.
int main(int argc, char **argv)
{
    xcb_connection_t *c = xcb_connect(nullptr, nullptr);
    if (xcb_connection_has_error(c))
        return 1;

    const xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
    const xcb_window_t id = xcb_generate_id(c);
    const uint32_t values[] = {screen->white_pixel};
    xcb_create_window(c, XCB_COPY_FROM_PARENT, id, screen->root, 0, 0, 200, 100, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, XCB_CW_BACK_PIXEL,
                      values);

    const char title[] = "Lingmo X11 client";
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, id, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                        sizeof(title) - 1, title);
    const char clazz[] = "x11client\0X11Client\0";
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, id, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8,
                        sizeof(clazz) - 1, clazz);
    if (argc > 1)
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, id, XCB_ATOM_WM_CLIENT_MACHINE,
                            XCB_ATOM_STRING, 8, strlen(argv[1]), argv[1]);

    xcb_map_window(c, id);
    xcb_flush(c);
    while (xcb_generic_event_t *event = xcb_wait_for_event(c))
        free(event);

    xcb_disconnect(c);
    return 0;
}
