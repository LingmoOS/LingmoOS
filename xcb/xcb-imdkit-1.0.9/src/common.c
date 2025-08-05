/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
#include "common.h"
#include "parser.h"
#include "ximproto.h"
#include <stdlib.h>
#include <string.h>

bool _xcb_im_init_atoms(xcb_connection_t *conn, size_t n,
                        const char **atom_names, xcb_atom_t *atoms) {
    // here we alloc some array on stack, but since we only use this function
    // internally and all atom_names size are small (less than 10), so it
    // doesn't matter.
    xcb_intern_atom_cookie_t atom_cookies[n];
    for (size_t i = 0; i < n; i++) {
        atom_cookies[i] =
            xcb_intern_atom(conn, false, strlen(atom_names[i]), atom_names[i]);
    }
    size_t i;
    for (i = 0; i < n; i++) {
        xcb_intern_atom_reply_t *atom_reply =
            xcb_intern_atom_reply(conn, atom_cookies[i], NULL);
        if (atom_reply) {
            atoms[i] = atom_reply->atom;
            free(atom_reply);
        } else {
            break;
        }
    }
    return (i == n);
}

uint32_t _xcb_get_event_mask(xcb_connection_t *conn, xcb_window_t window) {
    if (window == XCB_NONE) {
        return 0;
    }
    xcb_get_window_attributes_cookie_t cookie =
        xcb_get_window_attributes(conn, window);
    xcb_get_window_attributes_reply_t *reply =
        xcb_get_window_attributes_reply(conn, cookie, NULL);
    if (!reply) {
        return 0;
    }
    uint32_t your_event_mask = reply->your_event_mask;
    free(reply);
    return your_event_mask;
}

bool _xcb_change_event_mask(xcb_connection_t *conn, xcb_window_t window,
                            uint32_t mask, bool remove) {
    uint32_t your_event_mask = _xcb_get_event_mask(conn, window);

    uint32_t masks[1];
    if (remove) {
        masks[0] = your_event_mask & (~mask);
    } else {
        masks[0] = your_event_mask | mask;
    }

    if (masks[0] == your_event_mask) {
        return true;
    }

    xcb_void_cookie_t change_attr_cookie = xcb_change_window_attributes_checked(
        conn, window, XCB_CW_EVENT_MASK, masks);

    xcb_generic_error_t *error = NULL;
    if ((error = xcb_request_check(conn, change_attr_cookie)) != NULL) {
        free(error);
        return false;
    }
    return true;
}

size_t _xcb_im_ic_attr_size(uint32_t type) {
    switch (type) {
    case XimType_CARD32:
    case XimType_Window: {
        return sizeof(uint32_t);
    }
    case XimType_XRectangle: {
        xcb_im_xrectangle_fr_t fr;
        return xcb_im_xrectangle_fr_size(&fr);
    }
    case XimType_XPoint: {
        xcb_im_xpoint_fr_t fr;
        return xcb_im_xpoint_fr_size(&fr);
    }
    }
    return 0;
}

uint8_t *_xcb_im_get_ic_value(void *p, uint32_t type, uint8_t *data,
                              bool swap) {
    switch (type) {
    case XimType_CARD32:
    case XimType_Window: {
        uint32_t *result = p;
        data = uint32_t_write(result, data, swap);
        break;
    }
    case XimType_XRectangle: {
        xcb_rectangle_t *result = p;
        xcb_im_xrectangle_fr_t fr;
        fr.x = result->x;
        fr.y = result->y;
        fr.width = result->width;
        fr.height = result->height;
        data = xcb_im_xrectangle_fr_write(&fr, data, swap);
        break;
    }
    case XimType_XPoint: {
        xcb_point_t *result = p;
        xcb_im_xpoint_fr_t fr;
        fr.x = result->x;
        fr.y = result->y;
        data = xcb_im_xpoint_fr_write(&fr, data, swap);
        break;
    }
    case XimType_XFontSet: {
        break;
    }
    }
    return data;
}
