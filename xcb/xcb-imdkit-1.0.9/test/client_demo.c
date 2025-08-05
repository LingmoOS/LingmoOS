/*
 * SPDX-FileCopyrightText: 2020 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
#include "encoding.h"
#include "imclient.h"
#include "ximproto.h"
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xproto.h>

xcb_window_t w;
xcb_xic_t ic;

void forward_event(xcb_xim_t *im, xcb_xic_t ic, xcb_key_press_event_t *event,
                   void *user_data) {
    fprintf(stderr, "Key %s Keycode %u, State %u\n",
            event->response_type == XCB_KEY_PRESS ? "press" : "release",
            event->detail, event->state);
}

void commit_string(xcb_xim_t *im, xcb_xic_t ic, uint32_t flag, char *str,
                   uint32_t length, uint32_t *keysym, size_t nKeySym,
                   void *user_data) {
    if (xcb_xim_get_encoding(im) == XCB_XIM_UTF8_STRING) {
        fprintf(stderr, "key commit utf8: %.*s\n", length, str);
    } else if (xcb_xim_get_encoding(im) == XCB_XIM_COMPOUND_TEXT) {
        size_t newLength = 0;
        char *utf8 = xcb_compound_text_to_utf8(str, length, &newLength);
        if (utf8) {
            int l = newLength;
            fprintf(stderr, "key commit: %.*s\n", l, utf8);
        }
    }
}

void disconnected(xcb_xim_t *im, void *user_data) {
    fprintf(stderr, "Disconnected from input method server.\n");
    ic = 0;
}

xcb_xim_im_callback callback = {
    .forward_event = forward_event,
    .commit_string = commit_string,
    .disconnected = disconnected,
};

void logger(const char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vprintf(fmt, argp);
    va_end(argp);
}

void create_ic_callback(xcb_xim_t *im, xcb_xic_t new_ic, void *user_data) {
    ic = new_ic;
    if (ic) {
        fprintf(stderr, "icid:%u\n", ic);
        xcb_xim_set_ic_focus(im, ic);
    }
}

void open_callback(xcb_xim_t *im, void *user_data) {
    uint32_t input_style = XCB_IM_PreeditPosition | XCB_IM_StatusArea;
    xcb_point_t spot;
    spot.x = 0;
    spot.y = 0;
    xcb_xim_nested_list nested =
        xcb_xim_create_nested_list(im, XCB_XIM_XNSpotLocation, &spot, NULL);
    xcb_xim_create_ic(im, create_ic_callback, NULL, XCB_XIM_XNInputStyle,
                      &input_style, XCB_XIM_XNClientWindow, &w,
                      XCB_XIM_XNFocusWindow, &w, XCB_XIM_XNPreeditAttributes,
                      &nested, NULL);
    free(nested.data);
}

int main(int argc, char *argv[]) {
    // Init global state for compound text encoding.
    xcb_compound_text_init();

    // Open connection to X server
    int screen_default_nbr;
    xcb_connection_t *connection = xcb_connect(NULL, &screen_default_nbr);
    xcb_screen_t *screen = xcb_aux_get_screen(connection, screen_default_nbr);

    if (!screen) {
        return 1;
    }

    xcb_xim_t *im = xcb_xim_create(connection, screen_default_nbr, NULL);

    xcb_xim_set_im_callback(im, &callback, NULL);
    xcb_xim_set_log_handler(im, logger);
    xcb_xim_set_use_compound_text(im, true);
    xcb_xim_set_use_utf8_string(im, true);

    // Open connection to XIM server.
    xcb_xim_open(im, open_callback, true, NULL);

    // Create a dummy window for testing.
    w = xcb_generate_id(connection);
    xcb_create_window(connection, XCB_COPY_FROM_PARENT, w, screen->root, 0, 0,
                      100, 100, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, 0, NULL);
    uint32_t mask = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                    XCB_EVENT_MASK_FOCUS_CHANGE;
    xcb_change_window_attributes(connection, w, XCB_CW_EVENT_MASK, &mask);
    xcb_map_window(connection, w);
    xcb_flush(connection);

    xcb_generic_event_t *event;
    while ((event = xcb_wait_for_event(connection))) {
        logger("event_type=%d\n", (event->response_type & ~0x80));
        if (!xcb_xim_filter_event(im, event)) {
            // Forward event to input method if IC is created.
            if (ic && (((event->response_type & ~0x80) == XCB_KEY_PRESS) ||
                       ((event->response_type & ~0x80) == XCB_KEY_RELEASE))) {
                xcb_xim_forward_event(im, ic, (xcb_key_press_event_t *)event);
            }
        }
        free(event);
    }

    xcb_xim_close(im);
    xcb_xim_destroy(im);

    xcb_disconnect(connection);

    return 0;
}
