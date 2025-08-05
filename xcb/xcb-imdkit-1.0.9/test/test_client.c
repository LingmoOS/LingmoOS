/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
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

xcb_connection_t *connection;
xcb_window_t w;
xcb_screen_t *screen;
bool end = false;

void forward_event(xcb_xim_t *im, xcb_xic_t ic, xcb_key_press_event_t *event,
                   void *user_data) {
    fprintf(stderr, "Key %s Keycode %u, State %u\n",
            event->response_type == XCB_KEY_PRESS ? "press" : "release",
            event->detail, event->state);
}

void commit_string(xcb_xim_t *im, xcb_xic_t ic, uint32_t flag, char *str,
                   uint32_t length, uint32_t *keysym, size_t nKeySym,
                   void *user_data) {
    fprintf(stderr, "key commit: %.*s\n", length, str);
}

xcb_xim_im_callback callback = {.forward_event = forward_event,
                                .commit_string = commit_string};

void destroy_ic_callback(xcb_xim_t *im, xcb_xic_t ic, void *user_data) {
    fprintf(stderr, "ic %d destroyed\n", ic);
    // end = true;
}

void get_ic_values_callback(xcb_xim_t *im, xcb_xic_t ic,
                            xcb_im_get_ic_values_reply_fr_t *reply,
                            void *user_data) {
    fprintf(stderr, "get ic %d done\n", ic);
    xcb_xim_destroy_ic(im, ic, destroy_ic_callback, NULL);
    xcb_key_press_event_t event;
    memset(&event, 0, sizeof(event));
    event.root = screen->root;
    event.detail = 65;
    event.state = 0x4;
    event.event = w;
    event.response_type = XCB_KEY_PRESS;
    xcb_xim_forward_event(im, ic, &event);
    event.detail = 38;
    event.state = 0;
    xcb_xim_forward_event(im, ic, &event);
    event.detail = 56;
    event.state = 0;
    xcb_xim_forward_event(im, ic, &event);
    event.detail = 38;
    event.state = 0;
    xcb_xim_forward_event(im, ic, &event);
    event.detail = 56;
    event.state = 0;
    xcb_xim_forward_event(im, ic, &event);
    event.detail = 38;
    event.state = 0;
    xcb_xim_forward_event(im, ic, &event);
    event.detail = 56;
    event.state = 0;
    xcb_xim_forward_event(im, ic, &event);
    event.detail = 38;
    event.state = 0;
    xcb_xim_forward_event(im, ic, &event);
    event.detail = 56;
    event.state = 0;
    xcb_xim_forward_event(im, ic, &event);
}

void set_ic_values_callback(xcb_xim_t *im, xcb_xic_t ic, void *user_data) {
    fprintf(stderr, "set ic %d done\n", ic);
    xcb_xim_get_ic_values(im, ic, get_ic_values_callback, NULL,
                          XCB_XIM_XNPreeditAttributes, NULL);
}

void create_ic_callback(xcb_xim_t *im, xcb_xic_t ic, void *user_data) {
    if (ic) {
        fprintf(stderr, "icid:%u\n", ic);
        xcb_point_t spot;
        spot.x = 0;
        spot.y = 0;
        xcb_xim_nested_list nested =
            xcb_xim_create_nested_list(im, XCB_XIM_XNSpotLocation, &spot, NULL);
        xcb_xim_set_ic_values(im, ic, set_ic_values_callback, NULL,
                              XCB_XIM_XNPreeditAttributes, &nested, NULL);
        free(nested.data);
    } else {
        fprintf(stderr, "failed.\n");
    }
}

void get_im_values_callback(xcb_xim_t *im,
                            xcb_im_get_im_values_reply_fr_t *reply,
                            void *user_data) {
    w = xcb_generate_id(connection);
    xcb_create_window(connection, XCB_COPY_FROM_PARENT, w, screen->root, 0, 0,
                      1, 1, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, 0, NULL);
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

void open_callback(xcb_xim_t *im, void *user_data) {
    xcb_xim_get_im_values(im, get_im_values_callback, NULL,
                          XCB_XIM_XNQueryInputStyle, NULL);
}

void logger(const char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vprintf(fmt, argp);
    va_end(argp);
}

int main(int argc, char *argv[]) {
    /* Open the connection to the X server */
    int screen_default_nbr;
    connection = xcb_connect(NULL, &screen_default_nbr);
    screen = xcb_aux_get_screen(connection, screen_default_nbr);

    if (!screen) {
        return false;
    }
    xcb_xim_t *im =
        xcb_xim_create(connection, screen_default_nbr, "@im=test_server");

    xcb_xim_set_im_callback(im, &callback, NULL);
    xcb_xim_set_log_handler(im, logger);
    assert(xcb_xim_open(im, open_callback, true, NULL));

    xcb_generic_event_t *event;
    while ((event = xcb_wait_for_event(connection))) {
        xcb_xim_filter_event(im, event);
        free(event);
        if (end) {
            break;
        }
    }

    xcb_xim_close(im);
    xcb_xim_destroy(im);

    xcb_disconnect(connection);

    return 0;
}
